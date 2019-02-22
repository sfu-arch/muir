#define DEBUG_TYPE "graph"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include <experimental/numeric>
#include <iostream>

#include "AliasEdgeWriter.h"
#include "Dandelion/Node.h"
#include "GraphGeneratorPass.h"

using namespace std;
using namespace llvm;
using namespace graphgen;
using namespace dandelion;

using InstructionList = std::list<InstructionNode>;
using ArgumentList = std::list<ArgumentNode>;
using BasicBlockList = std::list<SuperNode>;
using NodeList = std::list<Node>;

extern cl::opt<string> target_fn;
extern cl::opt<string> config_path;

namespace graphgen {

char GraphGeneratorPass::ID = 0;

RegisterPass<GraphGeneratorPass> X("graphgen", "Generating graph pass");
}  // namespace graphgen

// Right now I couldn't move to C++17 because of tapir, so I used a simulated
// implementation of transform_reduce from C++17
// later on by moving to c++, this function is not needed anymore
template <class InputIt, class T, class BinaryOp, class UnaryOp>
T transform_reduce(InputIt first, InputIt last, T init, BinaryOp binop,
                   UnaryOp unary_op) {
    T generalizedSum = init;
    for (auto iter = first; iter != last; iter++) {
        generalizedSum = binop(unary_op(*iter), generalizedSum);
    }
    return generalizedSum;
}

template <typename Iter, typename Q>
void push_range(Q &q, Iter begin, Iter end) {
    for (; begin != end; ++begin) q.push(*begin);
}

void inline findAllLoops(Loop *L, SetVector<Loop *> &Loops) {
    // Recursively find all subloops.
    for (Loop *SL : L->getSubLoops()) {
        findAllLoops(SL, Loops);
    }
    // Store current loop
    Loops.insert(L);
}

uint32_t returnNumPred(BasicBlock *BB) {
    uint32_t c = 0;
    for (auto _bb_it : llvm::predecessors(BB)) {
        if (BB != _bb_it) c++;
    }
    return c;
}

/**
 * definedInCaller - Return true if the specified value is defined in the
 * function being code extracted, but not in the region being extracted. These
 * values must be passed in as live-ins to the function.
 */
bool definedInCaller(const SetVector<BasicBlock *> &Blocks, Value *V) {
    if (isa<Argument>(V)) return true;
    if (Instruction *I = dyn_cast<Instruction>(V)) {
        if (!Blocks.count(I->getParent())) return true;
    }
    return false;
}

/** definedInRegion - Return true if the specified value is defined in the
 * extracted region.
 */
bool definedInRegion(const SetVector<BasicBlock *> &Blocks, Value *V) {
    if (Instruction *I = dyn_cast<Instruction>(V))
        if (Blocks.count(I->getParent())) return true;
    return false;
}

template <class T>
Instruction *findParallelInstruction(Function &F) {
    for (auto &BB : F) {
        for (auto &I : BB) {
            if (isa<T>(I)) return &I;
        }
    }
    return nullptr;
}

template <class T>
std::vector<T *> getNodeList(Graph *_graph) {
    std::vector<T *> return_list;
    for (auto &_node : _graph->instructions()) {
        if (auto cast_node = dyn_cast<T>(&*_node))
            return_list.push_back(cast_node);
    }
    return return_list;
}

static SetVector<Loop *> getLoops(LoopInfo &LI) {
    SetVector<Loop *> Loops;

    // iterate through top level loops. Store all subloops
    // and top level loop in Loops SetVector.
    for (auto &L : LI) {
        findAllLoops(L, Loops);
    }
    return Loops;
}

std::set<Loop *> getOuterLoops(LoopInfo &LI) {
    std::set<Loop *> outer_loops;
    for (auto &L : getLoops(LI)) {
        if (L->getLoopDepth() == 1) outer_loops.insert(L);
    }
    return outer_loops;
}

void UpdateLiveInConnections(Loop *_loop, LoopNode *_loop_node,
                             std::map<llvm::Value *, Node *> map_value_node) {
    for (auto B : _loop->blocks()) {
        for (auto &I : *B) {
            // Detecting Live-ins
            for (auto OI = I.op_begin(); OI != I.op_end(); OI++) {
                Value *_value = *OI;

                if (definedInCaller(
                        SetVector<BasicBlock *>(_loop->blocks().begin(),
                                                _loop->blocks().end()),
                        _value)) {
                    if (map_value_node.find(_value) == map_value_node.end())
                        assert(!"Couldn't find the live-in source");
                    if (map_value_node.find(&I) == map_value_node.end())
                        assert(!"Couldn't find the live-in target");

                    auto new_live_in = _loop_node->insertLiveInArgument(
                        _value, ArgumentNode::LoopLiveIn);
                    auto _src = map_value_node[_value];
                    auto _tar = map_value_node[&I];

                    // TODO later we need to get ride of these lines
                    if (auto call_out = dyn_cast<CallNode>(_tar))
                        _tar = call_out->getCallOut();
                    if (auto call_in = dyn_cast<CallNode>(_src))
                        _src = call_in->getCallIn();

                    if (!_src->existDataOutput(new_live_in)) {
                        _src->replaceDataOutputNode(_tar, new_live_in);
                        _tar->replaceDataInputNode(_src, new_live_in);
                        new_live_in->addDataInputPort(_src);
                    } else {
                        _src->removeNodeDataOutputNode(_tar);
                        _tar->replaceDataInputNode(_src, new_live_in);
                    }

                    new_live_in->addDataOutputPort(_tar);
                }
            }
        }
    }
}

void UpdateLiveOutConnections(Loop *_loop, LoopNode *_loop_node,
                              std::map<llvm::Value *, Node *> map_value_node) {
    for (auto B : _loop->blocks()) {
        for (auto &I : *B) {
            /**
             * The function needs these steps:
             * 1) Detect Live-outs
             * 2) Insert a new Live-outs into loop header
             * 3) Update the dependencies
             */
            for (auto *U : I.users()) {
                if (!definedInRegion(
                        SetVector<BasicBlock *>(_loop->blocks().begin(),
                                                _loop->blocks().end()),
                        U)) {
                    auto new_live_out = _loop_node->insertLiveOutArgument(
                        U, ArgumentNode::LoopLiveOut);

                    auto _src = map_value_node[&I];
                    auto _tar = map_value_node[U];

                    // TODO later we need to get ride of these lines
                    if (auto call_out = dyn_cast<CallNode>(_tar))
                        _tar = call_out->getCallOut();
                    if (auto call_in = dyn_cast<CallNode>(_src))
                        _src = call_in->getCallIn();

                    _src->replaceDataOutputNode(_tar, new_live_out);
                    _tar->replaceDataInputNode(_src, new_live_out);
                    new_live_out->addDataInputPort(_src);

                    new_live_out->addDataOutputPort(_tar);
                }
            }
        }
    }
}

void UpdateInnerLiveInConnections(
    Loop *_loop, std::map<llvm::Loop *, LoopNode *> loop_value_node,
    std::map<llvm::Value *, Node *> map_value_node) {
    for (auto B : _loop->blocks()) {
        for (auto &I : *B) {
            for (auto OI = I.op_begin(); OI != I.op_end(); OI++) {
                Value *_value = *OI;

                auto *N = I.getMetadata("UID");
                auto *S = dyn_cast<MDString>(N->getOperand(0));

                if (isa<llvm::PHINode>(I) && S->getString() == "47") {
                    auto *N = dyn_cast<Instruction>(OI)->getMetadata("UID");
                    auto *S = dyn_cast<MDString>(N->getOperand(0));

                    errs() << PURPLE("[DEBUG] ");
                    errs() << "Phi instruciton: ";
                    I.dump();
                    errs() << PURPLE("[DEBUG] ");
                    errs() << "Operand: ";
                    _value->dump();
                    errs() << "Meta:  " << S->getString() << "\n";
                }

                if (definedInCaller(
                        SetVector<BasicBlock *>(_loop->blocks().begin(),
                                                _loop->blocks().end()),
                        _value)) {
                    auto _loop_node = loop_value_node[_loop];
                    auto _parent_loop_node =
                        loop_value_node[_loop->getParentLoop()];

                    // TODO: This part needs to be re-thinked!
                    auto new_live_in = _loop_node->insertLiveInArgument(
                        _value, ArgumentNode::LoopLiveIn);

                    Node *_src = nullptr;
                    if (_parent_loop_node->findLiveInNode(_value) == nullptr)
                        _src = map_value_node[_value];
                    else
                        _src = _parent_loop_node->findLiveInNode(_value);
                    auto _tar = map_value_node[&I];

                    // TODO later we need to get ride of these lines
                    if (auto call_out = dyn_cast<CallNode>(_tar))
                        _tar = call_out->getCallOut();
                    if (auto call_in = dyn_cast<CallNode>(_src))
                        _src = call_in->getCallIn();

                    if (!_src->existDataOutput(new_live_in)) {
                        _src->replaceDataOutputNode(_tar, new_live_in);
                        _tar->replaceDataInputNode(_src, new_live_in);
                        new_live_in->addDataInputPort(_src);
                    } else {
                        _src->removeNodeDataOutputNode(_tar);
                        _tar->replaceDataInputNode(_src, new_live_in);
                    }

                    new_live_in->addDataOutputPort(_tar);
                }
            }
        }
    }
}

void UpdateInnerLiveOutConnections(
    Loop *_loop, std::map<llvm::Loop *, LoopNode *> loop_value_node,
    std::map<llvm::Value *, Node *> map_value_node) {
    for (auto B : _loop->blocks()) {
        for (auto &I : *B) {
            auto _loop_node = loop_value_node[_loop];
            //auto _parent_loop_node = loop_value_node[_loop->getParentLoop()];

            // The function needs these steps:
            // 1) Detect Live-outs
            // 2) Insert a new Live-outs into loop header
            // 3) Update the dependencies
            for (auto *U : I.users()) {
                if (!definedInRegion(
                        SetVector<BasicBlock *>(_loop->blocks().begin(),
                                                _loop->blocks().end()),
                        U)) {
                    auto new_live_out = _loop_node->insertLiveOutArgument(
                        U, ArgumentNode::LoopLiveOut);

                    auto _src = map_value_node[&I];
                    auto _tar = map_value_node[U];

                    // auto _tar = _parent_loop_node->findLiveOut(U);
                    // Node * _tar = nullptr;
                    // if(_parent_loop_node->findLiveOut(U) == nullptr)
                    //_tar = map_value_node[U];
                    // else
                    //_tar = _parent_loop_node->findLiveOut(U);

                    // TODO later we need to get ride of these lines
                    if (auto call_out = dyn_cast<CallNode>(_tar))
                        _tar = call_out->getCallOut();
                    if (auto call_in = dyn_cast<CallNode>(_src))
                        _src = call_in->getCallIn();

                    _src->replaceDataOutputNode(_tar, new_live_out);
                    _tar->replaceDataInputNode(_src, new_live_out);
                    new_live_out->addDataInputPort(_src);
                    new_live_out->addDataOutputPort(_tar);
                }
            }
        }
    }
}

/**
 * TODO: Right now we don't run loop simplify pass, because of cross checking
 * with our old output, but next step is to run simplify pass
 * because of the gauranties that the pass gives us
 */
LoopSummary GraphGeneratorPass::summarizeLoop(Loop *L, LoopInfo &LI) {
    LoopSummary summary;

    // Check if loop is in a simplify form
    if (!L->isLoopSimplifyForm()) {
        errs() << "[llvm-clouser] Loop not in Simplify Form\n";
    }

    // Start filling loop summary
    // 1)Loop enable signal and backward edge these two are always uniqu,
    summary.header = L->getHeader();
    for (auto _bb_it : llvm::predecessors(L->getHeader())) {
        // Forward edge
        if (!L->contains(_bb_it)) summary.enable = _bb_it->getTerminator();
        // Backward edge
        else
            summary.loop_back = _bb_it->getTerminator();
        blacklist_control_edge[_bb_it->getTerminator()].push_back(
            L->getHeader());
    }

    // 2)Looping exit branches
    L->getExitBlocks(summary.exit_blocks);
    for (auto _bb_it : summary.exit_blocks) {
        for (auto _bb_ex_it : llvm::predecessors(_bb_it)) {
            if (L->contains(_bb_ex_it)) {
                summary.loop_finish.insert(_bb_ex_it->getTerminator());
                blacklist_control_edge[_bb_ex_it->getTerminator()].push_back(
                    _bb_it);
            }
        }
    }

    // 3)Get all the sub-loops
    for (auto SL : L->getSubLoops()) {
        findAllLoops(SL, summary.sub_loops);
    }

    // 4)Extracting all the live-ins
    for (auto &bb : L->blocks()) {
        for (auto &ins : *bb) {
            for (auto &op : ins.operands()) {
                bool is_live_in = isa<Argument>(&*op);
                if (auto _inst = dyn_cast<Instruction>(op))
                    is_live_in = !L->contains(_inst) ? true : false;

                // If the value is live-in, we need to check if
                // subloops contain the instruction or not (not the operand).
                if (is_live_in) {
                    bool contain = transform_reduce(
                        L->getSubLoopsVector().begin(),
                        L->getSubLoopsVector().end(), false,
                        [](auto _l, auto _r) { return _l | _r; },
                        [&ins, &op, &summary](auto _sub_l) {
                            if (_sub_l->contains(&ins)) {
                                summary.live_in_loop[op].push_back(_sub_l);
                                return true;
                            } else
                                return false;
                        });
                    if (!contain) summary.live_in_ins[op].push_back(&ins);

                    // Adding to data edge blacklist
                    blacklist_loop_live_in_data_edge[op].push_back(&ins);
                }
            }
        }
    }

    // Extracting loop live-outs
    for (auto B : L->blocks()) {
        for (auto &ins : *B) {
            // Detecting Live-ins
            for (auto user : ins.users()) {
                bool is_live_out = false;
                auto _inst_user = dyn_cast<Instruction>(user);
                is_live_out = !L->contains(_inst_user) ? true : false;

                if (is_live_out) {
                    auto parent_loop = L->getParentLoop();
                    summary.live_out_ins[&ins].push_back(_inst_user);
                    while (parent_loop) {
                        summary.live_out_loop[user].push_back(parent_loop);
                        if (parent_loop->contains(_inst_user)) break;
                        parent_loop = parent_loop->getParentLoop();
                    }

                    blacklist_loop_live_out_data_edge[&ins].push_back(
                        dyn_cast<Instruction>(user));
                }
            }
        }
    }

    for (auto B : L->blocks()) {
        for (auto &ins : *B) {
            // Detecting Live-ins
            for (auto user : ins.users()) {
                // Checking for carry loop dependency
                bool is_carry = false;
                bool fail = false;
                auto _inst_user = dyn_cast<Instruction>(user);

                for (auto _p_B : L->blocks()) {
                    for (auto &_p_ins : *_p_B) {
                        if (&_p_ins == &ins) {
                            fail = true;
                            break;
                        }
                        if(&_p_ins == user){
                            is_carry = true;
                            break;
                        }
                    }
                    if (fail || is_carry) break;
                }

                if (is_carry) {
                    //ins.dump();
                    //_inst_user->dump();
                    summary.carry_dependencies[&ins].push_back(_inst_user);
                    blacklist_carry_dependency_data_edge[&ins].push_back(
                        dyn_cast<Instruction>(user));
                }
            }
        }
    }

    return summary;
}

bool GraphGeneratorPass::doInitialization(Module &M) {
    for (auto &F : M) {
        if (F.isDeclaration()) continue;

        if (F.getName() == target_fn) {
            this->LI = &getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
        }
    }
    // TODO: Add code here if it's needed before pas

    return false;
}

bool GraphGeneratorPass::doFinalization(Module &M) {
    // TODO: Add code here to do post pass
    return false;
}

/**
 * Set pass dependencies
 */

/**
 * Iterating over target function's basicblocks and
 * then make supernode for each of them and add them to the node list
 */
void GraphGeneratorPass::visitBasicBlock(BasicBlock &BB) {
    map_value_node[&BB] = this->dependency_graph->insertSuperNode(BB);
}

void GraphGeneratorPass::visitInstruction(Instruction &Ins) {
    // Here we have to check see whether we have missed any instruction or not
    // TODO Couldn't figure out about Tapir visitor functions
    if (auto _detach_ins = dyn_cast<llvm::DetachInst>(&Ins))
        map_value_node[&Ins] =
            this->dependency_graph->insertDetachNode(*_detach_ins);
    else if (auto _reattach_ins = dyn_cast<llvm::ReattachInst>(&Ins))
        map_value_node[&Ins] =
            this->dependency_graph->insertReattachNode(*_reattach_ins);
    else if (auto _sync_ins = dyn_cast<llvm::SyncInst>(&Ins))
        map_value_node[&Ins] =
            this->dependency_graph->insertSyncNode(*_sync_ins);
    else {
        Ins.print(errs(), true);
        errs() << "\n";
        assert(!"Instruction is not supported");
    }
}

void GraphGeneratorPass::visitFAdd(llvm::BinaryOperator &I) {
    map_value_node[&I] = this->dependency_graph->insertFaddNode(I);
}

void GraphGeneratorPass::visitFSub(llvm::BinaryOperator &I) {
    map_value_node[&I] = this->dependency_graph->insertFsubNode(I);
}

void GraphGeneratorPass::visitFMul(llvm::BinaryOperator &I) {
    map_value_node[&I] = this->dependency_graph->insertFmulNode(I);
}

void GraphGeneratorPass::visitFDiv(llvm::BinaryOperator &I) {
    map_value_node[&I] = this->dependency_graph->insertFdiveNode(I);
}

void GraphGeneratorPass::visitBinaryOperator(llvm::BinaryOperator &I) {
    map_value_node[&I] = this->dependency_graph->insertBinaryOperatorNode(I);
}

void GraphGeneratorPass::visitICmpInst(llvm::ICmpInst &I) {
    map_value_node[&I] = this->dependency_graph->insertIcmpOperatorNode(I);
}

void GraphGeneratorPass::visitBranchInst(llvm::BranchInst &I) {
    map_value_node[&I] = this->dependency_graph->insertBranchNode(I);
}

void GraphGeneratorPass::visitPHINode(llvm::PHINode &I) {
    map_value_node[&I] = this->dependency_graph->insertPhiNode(I);
}

void GraphGeneratorPass::visitSelectInst(llvm::SelectInst &I) {
    map_value_node[&I] = this->dependency_graph->insertSelectNode(I);
}

void GraphGeneratorPass::visitFCmp(llvm::FCmpInst &I) {
    map_value_node[&I] = this->dependency_graph->insertFcmpNode(I);
}

void GraphGeneratorPass::visitSExtInst(llvm::SExtInst &I) {
    map_value_node[&I] = this->dependency_graph->insertSextNode(I);
}

void GraphGeneratorPass::visitZExtInst(llvm::ZExtInst &I) {
    map_value_node[&I] = this->dependency_graph->insertZextNode(I);
}

void GraphGeneratorPass::visitAllocaInst(llvm::AllocaInst &I) {
    auto alloca_type = I.getAllocatedType();
    auto DL = I.getModule()->getDataLayout();
    auto num_byte = DL.getTypeAllocSize(alloca_type);
    uint32_t size = 1;

    if (alloca_type->isIntegerTy() || alloca_type->isArrayTy()) {
        map_value_node[&I] =
            this->dependency_graph->insertAllocaNode(I, size, num_byte);
    } else if (alloca_type->isPointerTy()) {
        map_value_node[&I] =
            this->dependency_graph->insertAllocaNode(I, size, num_byte);
        I.print(errs(), true);
        errs() << "Alloca is pointer\n";
        // assert(!"Don't support for this alloca");
    } else {
        I.print(errs(), true);
        assert(!"Don't support for this alloca");
    }
}

void GraphGeneratorPass::visitGetElementPtrInst(llvm::GetElementPtrInst &I) {
    auto &gep_pass_ctx = getAnalysis<helpers::GepInformation>();

    if (gep_pass_ctx.GepAddress.count(&I) == 0) {
        I.dump();
        assert(!"No gep information");
    }

    // auto src_type = I.getSourceElementType();
    map_value_node[&I] =
        this->dependency_graph->insertGepNode(I, gep_pass_ctx.GepAddress[&I]);
    if (map_value_node.count(&I) == 0) {
        I.dump();
        assert(!"Gep node is not picked up!!");
    }
}

void GraphGeneratorPass::visitLoadInst(llvm::LoadInst &I) {
    map_value_node[&I] = this->dependency_graph->insertLoadNode(I);
}

void GraphGeneratorPass::visitBitCastInst(llvm::BitCastInst &I) {
    map_value_node[&I] = this->dependency_graph->insertBitcastNode(I);
}

void GraphGeneratorPass::visitStoreInst(llvm::StoreInst &I) {
    map_value_node[&I] = this->dependency_graph->insertStoreNode(I);
}

void GraphGeneratorPass::visitReturnInst(llvm::ReturnInst &I) {
    map_value_node[&I] = this->dependency_graph->insertReturnNode(I);
}

void GraphGeneratorPass::visitCallInst(llvm::CallInst &I) {
    map_value_node[&I] = this->dependency_graph->insertCallNode(I);
}

void GraphGeneratorPass::visitFunction(Function &F) {
    dependency_graph->setFunction(&F);

    // Here we make a graph
    // Filling function argument nodes
    for (auto &f_arg : F.args()) {
        map_value_node[&f_arg] =
            this->dependency_graph->getSplitCall()->insertLiveInArgument(
                &f_arg, ArgumentNode::LiveIn);
    }

    // this->dependency_graph.setNumSplitCallInput(F.arg_size());

    // Filling function global nodes
    for (auto &g_var : F.getParent()->getGlobalList()) {
        map_value_node[&g_var] =
            this->dependency_graph->insertFunctionGlobalValue(g_var);
    }
}

/**
 * In this function we iterate over each function argument and connect all of
 * its
 * successors as a data input port.
 * If the input is constant value we find the value and make a ConstNode for
 * that value
 */
void GraphGeneratorPass::findPorts(Function &F) {
    // Check wether we already have iterated over the instructions
    assert(map_value_node.size() > 0 && "Instruction map can not be empty!");

    // auto loop_context = &getAnalysis<loopclouser::LoopClouser>();

    // Connecting function arguments to the spliter
    for (auto _fun_arg_it = this->dependency_graph->funarg_begin();
         _fun_arg_it != this->dependency_graph->funarg_end(); _fun_arg_it++) {
        auto _fun_arg_node = dyn_cast<ArgumentNode>(_fun_arg_it->get());
        auto _spliter = this->dependency_graph->getSplitCall();

        _spliter->addDataOutputPort(_fun_arg_node);
        _fun_arg_node->addDataInputPort(_spliter);
    }

    for (auto ins_it = inst_begin(F); ins_it != inst_end(F); ++ins_it) {
        auto _node_src = this->map_value_node.find(
            &*ins_it);  // it should be Instruction node

        for (uint32_t c = 0; c < ins_it->getNumOperands(); ++c) {
            auto operand = ins_it->getOperand(c);

            // First operand of a call function is a pointer to itself
            if (auto fn = dyn_cast<llvm::Function>(operand)) continue;

            if (auto target_bb = dyn_cast<llvm::BasicBlock>(operand)) {
                // If target operand is basicblock it means the instruction is
                // control instruction
                // 1) First find the basicblock node
                // 2) Add bb as a control output
                // 3) Add ins as a control input

                // First we check if the edes is listed as blacklist
                bool find =
                    (find_if(blacklist_control_edge[&*ins_it].begin(),
                             blacklist_control_edge[&*ins_it].end(),
                             [operand](auto _b_edge) {
                                 return _b_edge == operand;
                             }) != blacklist_control_edge[&*ins_it].end())
                        ? true
                        : false;
                if (find) continue;

                auto _node_dest = this->map_value_node.find(
                    operand);  // it should be supernode
                assert(isa<SuperNode>(_node_dest->second) &&
                       "Destination node should be super node!");

                assert(isa<InstructionNode>(_node_src->second) &&
                       "Source node should be instruction node!");

                // We don't connect reattach node data dependency
                if (isa<ReattachNode>(_node_src->second)) continue;

                auto _dst = _node_dest->second;
                Node *_src = nullptr;
                if (isa<BranchNode>(_node_src->second)) {
                    _src = dyn_cast<BranchNode>(_node_src->second);
                    if (ins_it->getNumOperands() == 3) {
                        // LLVM IR -> CBranch(cmpInput(0), trueDst(1),
                        // falseDst(2))
                        // XXX There is a bug in llvm IR
                        // in CBranch first element in the IR actually is C = 2
                        // and second elemnt is C = 1
                        auto _inst_branch = dyn_cast<BranchInst>(&*ins_it);
                        // outs() << "DEBUG BEGIN\n";
                        // ins_it->dump();
                        //_inst_branch->getSuccessor(c - 1)->dump();
                        // outs() << "DEBUG END\n";
                        auto _inst_operand = this->map_value_node.find(
                            _inst_branch->getSuccessor(c - 1));
                        if (c == 1) {
                            // Handeling LLVM Bug
                            dyn_cast<BranchNode>(_src)->addTrueBranch(
                                _inst_operand->second);
                            //_inst_operand->second);
                        } else if (c == 2) {
                            dyn_cast<BranchNode>(_src)->addFalseBranch(
                                _inst_operand->second);
                            //_inst_operand->second);
                        }
                    } else {
                        // The node is Ubranch
                        _src->addControlOutputPort(_dst, c);
                    }

                    _dst->addControlInputPort(_src);

                } else if (isa<DetachNode>(_node_src->second)) {
                    // TODO fix the Detachnode connections
                    _src = dyn_cast<DetachNode>(_node_src->second);
                    _src->addControlOutputPort(_dst, c);
                    _dst->addControlInputPort(_src);

                } else if (isa<ReattachNode>(_node_src->second)) {
                    // TODO fix the Reattachnode connections
                    _src = dyn_cast<ReattachNode>(_node_src->second);
                    _src->addControlOutputPort(_dst, c);
                    _dst->addControlInputPort(_src);

                } else if (isa<SyncNode>(_node_src->second)) {
                    // TODO fix the Sync node connections
                    _src = dyn_cast<SyncNode>(_node_src->second);
                    _src->addControlOutputPort(_dst, c);
                    _dst->addControlInputPort(_src);

                } else {
                    assert(!"Wrong cast of control node!");
                }

            } else {
                // If the operand is constant we have to create a new node
                if (isa<llvm::AllocaInst>(&*ins_it)) continue;

                Node *_const_node = nullptr;
                if (auto const_value = dyn_cast<llvm::ConstantInt>(operand)) {
                    _const_node = this->dependency_graph->insertConstIntNode(
                        *const_value);
                    map_value_node[operand] = _const_node;

                    /**
                     * Constant values should get their enable signal from
                     * their incoming BB
                     */
                    if (auto _phi_ins = dyn_cast<PHINode>(&*ins_it)) {
                        DEBUG(operand->dump());
                        DEBUG(outs() << "Index : " << c << "\n");
                        DEBUG(_phi_ins->getIncomingBlock(c)->dump());

                        _const_node->addControlInputPort(
                            this->map_value_node[_phi_ins->getParent()]);

                        this->map_value_node[_phi_ins->getParent()]
                            ->addControlOutputPort(_const_node);

                        dyn_cast<SuperNode>(
                            this->map_value_node[_phi_ins->getParent()])
                            ->addconstIntNode(
                                dyn_cast<ConstIntNode>(_const_node));

                    } else {
                        _const_node->addControlInputPort(
                            this->map_value_node[ins_it->getParent()]);
                        this->map_value_node[ins_it->getParent()]
                            ->addControlOutputPort(_const_node);
                        dyn_cast<SuperNode>(
                            this->map_value_node[ins_it->getParent()])
                            ->addconstIntNode(
                                dyn_cast<ConstIntNode>(_const_node));
                    }

                } else if (auto const_value =
                               dyn_cast<llvm::ConstantFP>(operand)) {
                    DEBUG(errs() << PURPLE("[DEBUG] "));
                    DEBUG(operand->dump());
                    _const_node =
                        this->dependency_graph->insertConstFPNode(*const_value);
                    map_value_node[operand] = _const_node;

                    /**
                      * Constant values should get their
                      * enable signal from
                      * their incoming BB
                    */
                    if (auto _phi_ins = dyn_cast<PHINode>(&*ins_it)) {
                        DEBUG(operand->dump());
                        DEBUG(outs() << "Index : " << c << "\n");
                        DEBUG(_phi_ins->getIncomingBlock(c)->dump());

                        _const_node->addControlInputPort(
                            this->map_value_node[_phi_ins->getParent()]);

                        this->map_value_node[_phi_ins->getParent()]
                            ->addControlOutputPort(_const_node);

                        dyn_cast<SuperNode>(
                            this->map_value_node[_phi_ins->getParent()])
                            ->addconstFPNode(
                                dyn_cast<ConstFPNode>(_const_node));

                    } else {
                        _const_node->addControlInputPort(
                            this->map_value_node[ins_it->getParent()]);
                        this->map_value_node[ins_it->getParent()]
                            ->addControlOutputPort(_const_node);
                        dyn_cast<SuperNode>(
                            this->map_value_node[ins_it->getParent()])
                            ->addconstFPNode(
                                dyn_cast<ConstFPNode>(_const_node));
                    }
                } else if (auto undef_value =
                               dyn_cast<llvm::UndefValue>(operand)) {
                    // TODO define an undef node instead of uisng empty const
                    // node!
                    _const_node = this->dependency_graph->insertConstIntNode();
                    map_value_node[operand] = _const_node;

                    _const_node->addControlInputPort(
                        this->map_value_node[ins_it->getParent()]);
                    this->map_value_node[ins_it->getParent()]
                        ->addControlOutputPort(_const_node);
                    dyn_cast<SuperNode>(
                        this->map_value_node[ins_it->getParent()])
                        ->addconstIntNode(dyn_cast<ConstIntNode>(_const_node));
                }

                auto _node_src = this->map_value_node.find(operand);
                auto _node_dest = this->map_value_node.find(&*ins_it);

                // Live in
                bool find_live_in = false;
                for (auto _data_src : blacklist_loop_live_in_data_edge) {
                    for (auto _data_edge : _data_src.getSecond()) {
                        if ((_data_src.getFirst() == operand) &&
                            (_data_edge == &*ins_it))
                            find_live_in = true;
                    }
                }

                if (find_live_in) {
                    auto _live_in =
                        loop_edge_map[std::make_pair(operand, &*ins_it)];

                    _node_src->second->addDataOutputPort(_live_in);
                    _live_in->addDataInputPort(_node_src->second);

                    _live_in->addDataOutputPort(_node_dest->second);
                    _node_dest->second->addDataInputPort(_live_in);

                    continue;
                }

                // Live out
                bool find_live_out = false;
                for (auto _data_src : blacklist_loop_live_out_data_edge) {
                    for (auto _data_edge : _data_src.getSecond()) {
                        if ((_data_src.getFirst() == operand) &&
                            (_data_edge == &*ins_it))
                            find_live_out = true;
                    }
                }
                if (find_live_out) {
                    auto _live_out =
                        loop_edge_map[std::make_pair(operand, &*ins_it)];

                    _node_src->second->addDataOutputPort(_live_out);
                    _live_out->addDataInputPort(_node_src->second);

                    _live_out->addDataOutputPort(_node_dest->second);
                    _node_dest->second->addDataInputPort(_live_out);

                    continue;
                }

                // carry dependencies
                bool find_carry = false;
                for (auto _data_src : blacklist_carry_dependency_data_edge) {
                    for (auto _data_edge : _data_src.getSecond()) {
                        if ((_data_src.getFirst() == operand) &&
                            (_data_edge == &*ins_it))
                            find_carry = true;
                    }
                }
                if (find_carry) {
                    auto _carry =
                        loop_edge_map[std::make_pair(operand, &*ins_it)];

                    _node_src->second->addDataOutputPort(_carry);
                    _carry->addDataInputPort(_node_src->second);

                    _carry->addDataOutputPort(_node_dest->second);
                    _node_dest->second->addDataInputPort(_carry);

                    continue;
                }

                if (_node_src == this->map_value_node.end()) {
                    DEBUG(operand->print(errs(), true));
                    DEBUG(errs() << "\n");
                    DEBUG(ins_it->print(errs(), true));
                    DEBUG(operand->dump());
                    assert(!"The source instruction couldn't find!\n"
                            "[HINT] Look at the LLVM type it you don't have one to one mapping"
                            "between LLVM and your graph library");
                }

                if (_node_dest == this->map_value_node.end()) {
                    DEBUG(operand->print(errs(), true));
                    DEBUG(errs() << "\n");
                    DEBUG(ins_it->print(errs(), true));
                    assert(!"The destination instruction couldn't find!\n"
                            "[HINT] Look at the LLVM type it you don't have one to one mapping"
                            "between LLVM and your graph library");
                }

                auto _src = _node_src->second;
                auto _dst = _node_dest->second;

                // Make sure that _src and _dst pointing
                // to correct node
                if (_const_node) _src = _const_node;
                if (auto call_out = dyn_cast<CallNode>(_dst))
                    _dst = call_out->getCallOut();
                if (auto call_in = dyn_cast<CallNode>(_src))
                    _src = call_in->getCallIn();

                _src->addDataOutputPort(_dst);
                _dst->addDataInputPort(_src);
            }
        }

        // Connecting Load and Store nodes to Memory system
        // TODO: We need to decide how to connect the memory nodes
        // to the memory system
        if (auto _ld_node = dyn_cast<LoadNode>(
                this->map_value_node.find(&*ins_it)->second)) {
            // TODO right now we consider all the connections to the cache or
            // regfile
            // We need a pass to trace the pointers
            this->dependency_graph->getMemoryUnit()->addReadMemoryReqPort(
                _ld_node);
            this->dependency_graph->getMemoryUnit()->addReadMemoryRespPort(
                _ld_node);
            _ld_node->addReadMemoryReqPort(
                this->dependency_graph->getMemoryUnit());
            _ld_node->addReadMemoryRespPort(
                this->dependency_graph->getMemoryUnit());

        }

        // Store node connection
        //
        else if (auto _st_node = dyn_cast<StoreNode>(
                     this->map_value_node.find(&*ins_it)->second)) {
            this->dependency_graph->getMemoryUnit()->addWriteMemoryReqPort(
                _st_node);
            this->dependency_graph->getMemoryUnit()->addWriteMemoryRespPort(
                _st_node);
            _st_node->addWriteMemoryReqPort(
                this->dependency_graph->getMemoryUnit());
            _st_node->addWriteMemoryRespPort(
                this->dependency_graph->getMemoryUnit());
        }

        // Alloca node connection to the stack allocator
        //
        else if (auto _alloca_node = dyn_cast<AllocaNode>(
                     this->map_value_node.find(&*ins_it)->second)) {
            this->dependency_graph->getStackAllocator()->addReadMemoryReqPort(
                _alloca_node);
            this->dependency_graph->getStackAllocator()->addReadMemoryRespPort(
                _alloca_node);
            _alloca_node->addReadMemoryReqPort(
                this->dependency_graph->getStackAllocator());
            _alloca_node->addReadMemoryRespPort(
                this->dependency_graph->getStackAllocator());
        }

        // Connection FPNode operations
        else if (auto _fpdiv_node = dyn_cast<FdiveOperatorNode>(
                     this->map_value_node.find(&*ins_it)->second)) {
            _fpdiv_node->setRouteID(
                this->dependency_graph->getFPUNode()->numMemReqPort());
            this->dependency_graph->getFPUNode()->addReadMemoryReqPort(
                _fpdiv_node);
            this->dependency_graph->getFPUNode()->addReadMemoryRespPort(
                _fpdiv_node);
            _fpdiv_node->addReadMemoryReqPort(
                this->dependency_graph->getFPUNode());
            _fpdiv_node->addReadMemoryRespPort(
                this->dependency_graph->getFPUNode());
        }

        // TODO: We need to have diveider as well and connect the diveder node
        // like above!
    }
}

/**
 * This function has two tasks:
 * 1) Iterate over the basicblock's insturcitons and make a list of instructions
 * 2) Make control dependnce edges
 */
void GraphGeneratorPass::fillBasicBlockDependencies(Function &F) {
    // Find the entry basic block and connect it to the splitnode
    for (auto &BB : F) {
        // Find the entry basic block and connect it to the split node
        if (&BB == &F.getEntryBlock()) {
            auto _en_bb = dyn_cast<SuperNode>(this->map_value_node[&BB]);
            this->dependency_graph->getSplitCall()->addControlOutputPort(
                _en_bb);
            _en_bb->addControlInputPort(this->dependency_graph->getSplitCall());
        }
        // Here we check wether the basicblock is reached from multiple places.
        // If it has been reached from multiple places it means it CAN have a
        // value which is feeded from multiple places. But still we are not sure
        // and we have to make sure that if there is PHI node whithin the
        // basicblock.
        if (auto _bb = dyn_cast<SuperNode>(this->map_value_node[&BB])) {
            for (auto &I : BB) {
                // Iterate over the basicblock's instructions
                if (auto _ins =
                        dyn_cast<InstructionNode>(this->map_value_node[&I])) {
                    _bb->addInstruction(_ins);

                    // TODO: Why we are skipping reattach node?
                    // Because you don't need to make reattach node's
                    // connecitons
                    // they already exist.
                    if (auto reatach = dyn_cast<ReattachNode>(_ins)) continue;

                    // Detect Phi instrucctions
                    if (auto _phi_ins = dyn_cast<PhiSelectNode>(_ins)) {
                        //_bb->setNodeType(SuperNode::Mask);
                        _bb->addPhiInstruction(_phi_ins);
                        _phi_ins->setParentNode(_bb);

                        // Since we know that the BB has a PHI node,
                        // we change the type to Mask
                        _bb->setNodeType(SuperNode::Mask);
                    }

                    if (auto _call_node = dyn_cast<CallNode>(_ins)) {
                        _bb->addControlOutputPort(_call_node->getCallOut());
                        _call_node->setCallOutEnable(_bb);
                    } else {
                        //
                        _bb->addControlOutputPort(_ins);
                        _ins->addControlInputPort(_bb);
                    }

                } else
                    assert(!"The instruction is not visited!");
            }

        } else
            assert(!"The basicblock is not visited!");
    }
}

/**
 * This funciton iterates over function loops and generate loop nodes
 */
void GraphGeneratorPass::updateLoopDependencies(llvm::LoopInfo &loop_info) {
    uint32_t c = 0;

    for (auto &L : getLoops(loop_info)) {
        // Getting list of loop's exit basicblock
        // Remember for loops can have multiple exit point
        auto _l_head = dyn_cast<SuperNode>(map_value_node[L->getHeader()]);

        SmallVector<BasicBlock *, 8> _exit_blocks;
        L->getExitBlocks(_exit_blocks);

        std::vector<std::pair<BasicBlock *, SuperNode *>> _l_exit_blocks;

        for (auto _l : _exit_blocks) {
            _l_exit_blocks.push_back(
                std::make_pair(_l, dyn_cast<SuperNode>(map_value_node[_l])));
        }

        // Change the type of loop head basic block
        // Since we know that there is a backedge
        //_l_head->setNodeType(SuperNode::SuperNodeType::LoopHead);

        // Try to find forward edge to the loop head basic block
        auto _src_forward_br_inst_it = *std::find_if(
            _l_head->inputControl_begin(), _l_head->inputControl_end(),
            [&L](auto const _node_it) {
                return !L->contains(
                    dyn_cast<BranchNode>(_node_it.first)->getInstruction());
            });

        // Try to find the backedge to the loop head basic block
        auto _src_back_br_inst_it = *std::find_if(
            _l_head->inputControl_begin(), _l_head->inputControl_end(),
            [&L](auto const _node_it) {
                return L->contains(
                    dyn_cast<BranchNode>(_node_it.first)->getInstruction());
            });

        std::list<SuperNode *> _list_exit;
        std::transform(_l_exit_blocks.begin(), _l_exit_blocks.end(),
                       std::back_inserter(_list_exit),
                       [](auto _l_e) -> SuperNode * { return _l_e.second; });

        auto _new_loop = std::make_unique<LoopNode>(
            NodeInfo(c, "Loop_" + std::to_string(c)),
            dyn_cast<SuperNode>(map_value_node[L->getHeader()]),
            dyn_cast<SuperNode>(map_value_node[L->getLoopLatch()]), _list_exit);

        // Insert loop node
        auto _loop_node =
            this->dependency_graph->insertLoopNode(std::move(_new_loop));

        loop_value_node[&*L] = _loop_node;

        _loop_node->setInputControlLoopSignal(_src_forward_br_inst_it.first);
        _loop_node->setActiveOutputLoopSignal(_l_head);
        _l_head->replaceControlInputNode(_src_forward_br_inst_it.first,
                                         _loop_node);
        _src_forward_br_inst_it.first->replaceControlOutputNode(_l_head,
                                                                _loop_node);

        for (auto _l : _list_exit) {
            _loop_node->setLoopEndEnable(_l);
        }

        // Connect the latch ending branch to loopNode
        if (auto _latch_br = dyn_cast<BranchNode>(_src_back_br_inst_it.first)) {
            _loop_node->addLoopBackEdge(_latch_br);
            _latch_br->addControlOutputPort(_loop_node);
            // Check wether branch instruction is conditional
            if (_latch_br->getInstruction()->getNumOperands() > 0) {
                if (dyn_cast<BranchInst>(_latch_br->getInstruction())
                        ->getSuccessor(0) == _l_head->getBasicBlock()) {
                    _latch_br->output_predicate.push_back(std::make_pair(
                        _loop_node, BranchNode::PredicateResult::True));
                } else {
                    _latch_br->output_predicate.push_back(std::make_pair(
                        _loop_node, BranchNode::PredicateResult::False));
                }
            }
        } else
            assert(!"Unexpected terminator!");

        // Connecting end branch to the loop end input
        for (auto _le : _list_exit) {
            auto _tar_exit_br_inst_it = *std::find_if(
                _le->inputControl_begin(), _le->inputControl_end(),
                [&L](auto const _node_it) {
                    if (_node_it.first == nullptr) return false;

                    if (dyn_cast<BranchNode>(_node_it.first))
                        return L->contains(dyn_cast<BranchNode>(_node_it.first)
                                               ->getInstruction());
                    else
                        return false;
                });

            // TODO I don't know why sometimes there is no ending instruciton
            // add here, need to investigate latter
            if (_tar_exit_br_inst_it == *_le->inputControl_end()) {
                assert(!"Don't run loop-simplify pass optimization on LL file,"
                        "we don't support this type of loop structure right now!\n");
            }

            DEBUG(dyn_cast<BranchNode>(_tar_exit_br_inst_it.first)
                      ->getInstruction()
                      ->dump());

            // Make the branch instruction as ending branch of the loop
            dyn_cast<BranchNode>(_tar_exit_br_inst_it.first)
                ->setEndingLoopBranch();

            _loop_node->pushLoopExitLatch(_tar_exit_br_inst_it.first);
            _le->replaceControlInputNode(_tar_exit_br_inst_it.first,
                                         _loop_node);
            _tar_exit_br_inst_it.first->replaceControlOutputNode(_le,
                                                                 _loop_node);
        }

        // Increament loop counter
        c++;

    }  // Get loops

    for (auto &L : getLoops(loop_info)) {
        auto _loop_node = loop_value_node[L];

        // Filling loop containers
        for (auto B : L->blocks()) {
            if (!B->empty()) {
                _loop_node->pushSuperNode(
                    dyn_cast<SuperNode>(map_value_node[B]));
                for (auto &I : *B) {
                    _loop_node->pushInstructionNode(
                        dyn_cast<InstructionNode>(map_value_node[&I]));
                }
            }
        }
    }

    for (auto &L : getOuterLoops(loop_info)) {
        auto _loop_node = loop_value_node[L];
        _loop_node->setOuterLoop();

        // This function should be called after filling the containers
        // always
        // Here we look for Store nodes and then connect them to their endinge
        // branch instruction
        _loop_node->setEndingInstructions();

        for (auto _en_instruction : _loop_node->endings()) {
            // We look for the ending branch instruction of each store node
            _en_instruction->getInstruction();
            auto &_br_ins = map_value_node[&_en_instruction->getInstruction()
                                                ->getParent()
                                                ->getInstList()
                                                .back()];

            // For now we connect all the store nodes within a for loop
            // to their ending branch instruction
            // this condition can be ease down later on
            _en_instruction->addControlOutputPort(_br_ins);
            _br_ins->addControlInputPort(_en_instruction);
        }
    }

    for (auto &L : getOuterLoops(loop_info)) {
        // At this stage we know that outer loop dominante all other loops
        // therefore each live-in for subLoops is a live-in for the outer
        // loop as well.
        // We first connect all the live-ins to the outer loop
        // and then iteratively go trought the subloops and update the
        // connections.
        auto _loop_node = loop_value_node[&*L];
        UpdateLiveInConnections(L, _loop_node, map_value_node);
        UpdateLiveOutConnections(L, _loop_node, map_value_node);

        std::list<Loop *> _loop_list;

        for (auto _l : L->getSubLoops()) {
            _loop_list.push_back(_l);
        }

        while (!_loop_list.empty()) {
            auto _sub_loop = _loop_list.front();
            _loop_list.pop_front();

            UpdateInnerLiveInConnections(_sub_loop, loop_value_node,
                                         map_value_node);
            UpdateInnerLiveOutConnections(_sub_loop, loop_value_node,
                                          map_value_node);

            for (auto _tmp_sub : _sub_loop->getSubLoopsVector()) {
                _loop_list.push_back(_tmp_sub);
            }
        }
    }
}

void GraphGeneratorPass::connectOutToReturn(Function &F) {
    for (auto &BB : F) {
        for (auto &I : BB) {
            if (isa<llvm::ReturnInst>(I)) {
                dependency_graph->setOutputNode(map_value_node[&I]);
            }
        }
    }
}

void GraphGeneratorPass::connectParalleNodes(Function &F) {
    if (!findParallelInstruction<llvm::SyncInst>(F)) return;
    auto _sync_node =
        this->map_value_node[findParallelInstruction<llvm::SyncInst>(F)];
    auto _detach_node =
        this->map_value_node[findParallelInstruction<llvm::DetachInst>(F)];
    auto _reattach_node =
        this->map_value_node[findParallelInstruction<llvm::ReattachInst>(F)];

    _sync_node->addControlInputPort(_detach_node);
    _sync_node->addControlInputPort(_reattach_node);

    _detach_node->addControlOutputPort(_sync_node);
    _reattach_node->addControlOutputPort(_sync_node);
}

void GraphGeneratorPass::connectingCalldependencies(Function &F) {
    auto call_instructions =
        getNodeList<CallNode>(this->dependency_graph.get());
    for (auto _call_node : call_instructions) {
        auto _ins = _call_node->getInstruction();
        auto &_end_ins = _ins->getParent()->back();
        auto _end_node = map_value_node[&_end_ins];
        if (isa<ReattachNode>(_end_node) || isa<BranchNode>(_end_node)) {
            if (isa<BranchNode>(_end_node)) {
                _end_node->addDataInputPort(_call_node->getCallIn());
                _call_node->getCallIn()->addDataOutputPort(_end_node);
            }

            // TODO needs to be checked!
            _end_node->addControlInputPort(_call_node->getCallIn());
            _call_node->getCallIn()->addControlOutputPort(_end_node);
        }
    }
}

// void GraphGeneratorPass::connectingAliasEdges(Function &F) {
// auto alias_context = &getAnalysis<aew::AliasEdgeWriter>();

void GraphGeneratorPass::buildLoopNodes(Function &F,
                                        llvm::LoopInfo &loop_info) {
    uint32_t c_id = 0;
    for (auto &L : getLoops(loop_info)) {
        auto _new_loop = std::make_unique<LoopNode>(
            NodeInfo(c_id, "Loop_" + std::to_string(c_id)));

        // Insert loop node
        auto _loop_node =
            this->dependency_graph->insertLoopNode(std::move(_new_loop));

        loop_value_node[&*L] = _loop_node;

        auto summary = loop_sum[&*L];

        // Connect enable
        this->map_value_node[summary.enable]->addControlOutputPort(_loop_node);
        _loop_node->setInputControlLoopSignal(
            this->map_value_node[summary.enable]);

        _loop_node->setActiveOutputLoopSignal(
            this->map_value_node[summary.header]);
        map_value_node[summary.header]->addControlInputPort(_loop_node);

        // Connecting backedge
        if (summary.loop_back->getNumOperands() > 0) {
            if (dyn_cast<BranchInst>(summary.loop_back)->getSuccessor(0) ==
                summary.header) {
                dyn_cast<BranchNode>(this->map_value_node[summary.loop_back])
                    ->addTrueBranch(_loop_node);
            } else {
                dyn_cast<BranchNode>(this->map_value_node[summary.loop_back])
                    ->addFalseBranch(_loop_node);
            }
            _loop_node->setInputControlLoopSignal(
                this->map_value_node[summary.loop_back]);
        } else {
            this->map_value_node[summary.loop_back]->addControlOutputPort(
                _loop_node);
            _loop_node->setInputControlLoopSignal(
                this->map_value_node[summary.loop_back]);
        }

        _loop_node->setActiveBackSignal(this->map_value_node[summary.header]);
        this->map_value_node[summary.header]->addControlInputPort(_loop_node);

        // Connecting exit signals
        for (auto _l_f : summary.loop_finish) {
            if (_l_f->getNumOperands() > 0) {
                auto _f = std::find(
                    summary.exit_blocks.begin(), summary.exit_blocks.end(),
                    dyn_cast<BranchInst>(summary.loop_back)->getSuccessor(0));
                if (_f != summary.exit_blocks.end()) {
                    dyn_cast<BranchNode>(
                        this->map_value_node[summary.loop_back])
                        ->addTrueBranch(_loop_node);
                } else {
                    dyn_cast<BranchNode>(
                        this->map_value_node[summary.loop_back])
                        ->addFalseBranch(_loop_node);
                }
                _loop_node->setInputControlLoopSignal(
                    this->map_value_node[summary.loop_back]);
            } else {
                this->map_value_node[_l_f]->addControlOutputPort(_loop_node);
                _loop_node->setInputControlLoopSignal(
                    this->map_value_node[_l_f]);
            }
        }

        // Connecting loop exit signals
        for (auto _exit_b : summary.exit_blocks) {
            this->map_value_node[_exit_b]->addControlInputPort(_loop_node);
            _loop_node->setActiveExitSignal(this->map_value_node[_exit_b]);
        }

        // Connecting live-in values
        for (auto _live_in : summary.live_in_ins) {
            auto new_live_in = _loop_node->insertLiveInArgument(
                _live_in.getFirst(), ArgumentNode::LoopLiveIn);
            for (auto _use : _live_in.getSecond()) {
                loop_edge_map[std::make_pair(_live_in.getFirst(), _use)] =
                    new_live_in;
            }
        }

        // Connecting live-outs values
        for (auto _live_out : summary.live_out_ins) {
            auto new_live_out = _loop_node->insertLiveOutArgument(
                _live_out.getFirst(), ArgumentNode::LoopLiveOut);
            for (auto _use : _live_out.getSecond()) {
                loop_edge_map[std::make_pair(_live_out.getFirst(), _use)] =
                    new_live_out;
            }
        }

        // Connecting carry values
        for (auto _carry_depen : summary.carry_dependencies) {
            auto new_carry_depen = _loop_node->insertCarryDepenArgument(
                _carry_depen.getFirst(), ArgumentNode::CarryDependency);
            for (auto _use : _carry_depen.getSecond()) {
                loop_edge_map[std::make_pair(_carry_depen.getFirst(), _use)] =
                    new_carry_depen;
            }
        }
    }
}

/**
 * All the initializations for function members
 */
void GraphGeneratorPass::init(Function &F) {
    // Running analysis on the elements
    buildLoopNodes(F, *LI);
    findPorts(F);
    fillBasicBlockDependencies(F);
    // updateLoopDependencies(*LI);
    connectOutToReturn(F);
    connectParalleNodes(F);
    connectingCalldependencies(F);
    // connectingAliasEdges(F);

    // Printing the graph
    dependency_graph->optimizationPasses();
    dependency_graph->printGraph(PrintType::Scala, config_path);
}

bool GraphGeneratorPass::runOnModule(Module &M) {
    for (auto &F : M) {
        if (F.isDeclaration()) continue;
        if (F.getName() == target_fn) {
            // Iterating over loops, and extracting loops information
            // before running anyother analysis
            auto &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
            auto loops = getLoops(LI);

            bool hasLoop = (loops.size() > 0);
            do {
                if (loops.size() == loop_sum.size()) break;

                for (auto &L : loops) {
                    if (L->getSubLoops().size() == 0 ||
                        loop_sum.count(L) == 0) {
                        this->loop_sum.insert(
                            std::make_pair(L, summarizeLoop(L, LI)));
                    }
                }

            } while (hasLoop);

            stripDebugInfo(F);
            visit(F);
            init(F);
        }
    }

    return false;
}
