#define DEBUG_TYPE "lx"
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

#include <iostream>

#include "Dandelion/Node.h"
#include "GraphGeneratorPass.h"

using namespace llvm;
using namespace std;
using namespace graphgen;
using namespace dandelion;

using InstructionList = std::list<InstructionNode>;
using ArgumentList = std::list<ArgumentNode>;
using BasicBlockList = std::list<SuperNode>;
using NodeList = std::list<Node>;

extern cl::opt<string> XKETCHName;

namespace graphgen {

char GraphGeneratorPass::ID = 0;

RegisterPass<GraphGeneratorPass> X("graphgen", "Generating xketch graph");
}  // namespace graphgen

void inline findAllLoops(Loop *L, SetVector<Loop *> &Loops) {
    // Recursively find all subloops.
    for (Loop *SL : L->getSubLoops()) {
        findAllLoops(SL, Loops);
    }
    // Store current loop
    Loops.insert(L);
}

/**
 * definedInCaller - Return true if the specified value is defined in the
 * function being code extracted, but not in the region being extracted. These
 * values must be passed in as live-ins to the function.
 */
bool definedInCaller(const SetVector<BasicBlock *> &Blocks, Value *V) {
    if (isa<Argument>(V)) return true;
    if (Instruction *I = dyn_cast<Instruction>(V))
        if (!Blocks.count(I->getParent())) return true;
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

static SetVector<Loop *> getLoops(LoopInfo &LI) {
    SetVector<Loop *> Loops;

    // iterate through top level loops. Store all subloops
    // and top level loop in Loops SetVector.
    for (auto &L : LI) {
        findAllLoops(L, Loops);
    }
    return Loops;
}

bool GraphGeneratorPass::doInitialization(Module &M) {
    for (auto &F : M) {
        if (F.isDeclaration()) continue;

        if (F.getName() == XKETCHName) {
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
void GraphGeneratorPass::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<helpers::GEPAddrCalculation>();
    AU.addRequired<llvm::LoopInfoWrapperPass>();
    AU.setPreservesAll();
}

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
        Ins.dump();
        assert(!"Instruction is not supported");
    }
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

void GraphGeneratorPass::visitAllocaInst(llvm::AllocaInst &I) {
    map_value_node[&I] = this->dependency_graph->insertAllocaNode(I);
}

void GraphGeneratorPass::visitGetElementPtrInst(llvm::GetElementPtrInst &I) {
    map_value_node[&I] = this->dependency_graph->insertGepNode(I);
    auto node = map_value_node[&I];
    auto &gep_pass_ctx = getAnalysis<helpers::GEPAddrCalculation>();

    // Check wether it's gepOne or gepTwo
    if (I.getNumOperands() == 2)
        dyn_cast<GEPNode>(node)->addNumByte(
            gep_pass_ctx.SingleGepIns[&I].numByte);
    else if (I.getNumOperands() == 3) {
        dyn_cast<GEPNode>(node)->addNumByte(
            gep_pass_ctx.TwoGepIns[&I].numByte1);
        dyn_cast<GEPNode>(node)->addNumByte(
            gep_pass_ctx.TwoGepIns[&I].numByte2);
    } else
        assert(!"Not supported gep node");

    // errs() << gep_pass_ctx.SingleGepIns[&I].numByte << "\n";
}

void GraphGeneratorPass::visitLoadInst(llvm::LoadInst &I) {
    map_value_node[&I] = this->dependency_graph->insertLoadNode(I);
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

    // TODO
    // Here we make a graph
    // Filling function argument nodes
    for (auto &f_arg : F.args()) {
        map_value_node[&f_arg] =
            this->dependency_graph->getSplitCall()->insertLiveInArgument(
                &f_arg);
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
void GraphGeneratorPass::findDataPort(Function &F) {
    // Check wether we already have iterated over the instructions
    assert(map_value_node.size() > 0 && "Instruction map can not be empty!");

    // Connecting function arguments to the spliter
    for (auto _fun_arg_it = this->dependency_graph->funarg_begin();
         _fun_arg_it != this->dependency_graph->funarg_end(); _fun_arg_it++) {
        auto _fun_arg_node = dyn_cast<ArgumentNode>(_fun_arg_it->get());
        auto _spliter = this->dependency_graph->getSplitCall();

        auto src_idx = _spliter->addDataOutputPort(_fun_arg_node);
        auto dst_idx = _fun_arg_node->addDataInputPort(_spliter);

        // this->dependency_graph->insertEdge(
        // Edge::DataTypeEdge, std::make_pair(_spliter, src_idx),
        // std::make_pair(_fun_arg_node, dst_idx));
    }

    for (auto ins_it = inst_begin(F); ins_it != inst_end(F); ++ins_it) {
        // Connecting DFG and CFG edges
        //
        if (auto _call_node =
                dyn_cast<CallNode>(this->map_value_node.find(&*ins_it)->second))
            continue;

        for (uint32_t c = 0; c < ins_it->getNumOperands(); ++c) {
            auto operand = ins_it->getOperand(c);
            if (auto target_bb = dyn_cast<llvm::BasicBlock>(operand)) {
                // Here we have to add a new edge from instruction to BB
                // 1) First find the basicblock node
                // 2) Add the bb as a control output
                // 3) Add the ins ass a control input
                auto _node_dest = this->map_value_node.find(
                    operand);  // it should be supernode
                assert(isa<SuperNode>(_node_dest->second) &&
                       "Destination node should be super node!");

                auto _node_src = this->map_value_node.find(
                    &*ins_it);  // it should be Insnode
                assert(isa<InstructionNode>(_node_src->second) &&
                       "Source node should be instruction node!");

                auto src_idx =
                    _node_src->second->addControlOutputPort(_node_dest->second);
                auto dst_idx =
                    _node_dest->second->addControlInputPort(_node_src->second);

                // this->dependency_graph->insertEdge(
                // Edge::ControlTypeEdge,
                // std::make_pair(_node_src->second, src_idx),
                // std::make_pair(_node_dest->second, dst_idx));

            } else {
                // If the operand is constant we have to create a new node
                if (auto const_value = dyn_cast<llvm::ConstantInt>(operand))
                    map_value_node[operand] =
                        this->dependency_graph->insertConstIntNode(
                            *const_value);
                // if (auto called =
                // dyn_cast<Function>(CallSite(operand)
                //.getCalledValue()
                //->stripPointerCasts())) {
                // continue;
                //}

                auto _node_src = this->map_value_node.find(operand);
                auto _node_dest = this->map_value_node.find(&*ins_it);

                if (_node_src == this->map_value_node.end()) {
                    ins_it->dump();
                    assert(!"The destination instruction couldn't find!");
                }

                if (_node_dest == this->map_value_node.end()) {
                    DEBUG(ins_it->dump());
                    assert(!"The destination instruction couldn't find!");
                }

                auto src_idx =
                    _node_src->second->addDataOutputPort(_node_dest->second);
                auto dst_idx =
                    _node_dest->second->addDataInputPort(_node_src->second);

                // this->dependency_graph->insertEdge(
                // Edge::DataTypeEdge,
                // std::make_pair(_node_src->second, src_idx),
                // std::make_pair(_node_dest->second, dst_idx));
            }
        }

        // Connecting LD and ST nodes to Memory system
        if (auto _ld_node = dyn_cast<LoadNode>(
                this->map_value_node.find(&*ins_it)->second)) {
            // TODO right now we consider all the connections to the cache or
            // regfile
            // We need a pass to trace the pointers
            auto _dst_req_idx =
                this->dependency_graph->getMemoryUnit()->addReadMemoryReqPort(
                    _ld_node);
            auto _src_resp_idx =
                this->dependency_graph->getMemoryUnit()->addReadMemoryRespPort(
                    _ld_node);
            auto _src_req_idx = _ld_node->addReadMemoryReqPort(
                this->dependency_graph->getMemoryUnit());
            auto _dst_resp_idx = _ld_node->addReadMemoryRespPort(
                this->dependency_graph->getMemoryUnit());

        } else if (auto _st_node = dyn_cast<StoreNode>(
                       this->map_value_node.find(&*ins_it)->second)) {
            auto _dst_req_idx =
                this->dependency_graph->getMemoryUnit()->addWriteMemoryReqPort(
                    _st_node);
            auto _src_resp_idx =
                this->dependency_graph->getMemoryUnit()->addWriteMemoryRespPort(
                    _st_node);
            auto _src_req_idx = _st_node->addWriteMemoryReqPort(
                this->dependency_graph->getMemoryUnit());
            auto _dst_resp_idx = _st_node->addWriteMemoryRespPort(
                this->dependency_graph->getMemoryUnit());
        }
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
            auto _src_idx =
                this->dependency_graph->getSplitCall()->addControlOutputPort(
                    _en_bb);
            auto _dst_idx = _en_bb->addControlInputPort(
                this->dependency_graph->getSplitCall());

            // this->dependency_graph->insertEdge(
            // Edge::ControlTypeEdge,
            // std::make_pair(this->dependency_graph->getSplitCall(),
            //_src_idx),
            // std::make_pair(_en_bb, _dst_idx));
        }
        if (auto _bb = dyn_cast<SuperNode>(this->map_value_node[&BB])) {
            for (auto &I : BB) {
                // Iterate over the basicblock's instructions
                if (auto _ins =
                        dyn_cast<InstructionNode>(this->map_value_node[&I])) {
                    _bb->addInstruction(_ins);

                    // Detect Phi instrucctions
                    if (auto _phi_ins = dyn_cast<PhiSelectNode>(_ins)) {
                        _bb->addPhiInstruction(_phi_ins);
                        _phi_ins->setParentNode(_bb);
                        // this->dependency_graph->insertEdge(
                        // Edge::MaskTypeEdge, std::make_pair(_bb, 0),
                        // std::make_pair(_phi_ins, 0));
                    }

                    // Make a control edge
                    auto _src_idx = _bb->addControlOutputPort(_ins);
                    auto _dst_idx = _ins->addControlInputPort(_bb);
                    // this->dependency_graph->insertEdge(
                    // Edge::ControlTypeEdge, std::make_pair(_bb, _src_idx),
                    // std::make_pair(_ins, _dst_idx));
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
void GraphGeneratorPass::fillLoopDependencies(llvm::LoopInfo &loop_info) {
    uint32_t c = 0;
    for (auto &L : getLoops(loop_info)) {
        auto _new_loop = std::make_unique<LoopNode>(
            NodeInfo(c, "Loop_" + std::to_string(c)),
            dyn_cast<SuperNode>(map_value_node[L->getHeader()]),
            dyn_cast<SuperNode>(map_value_node[L->getLoopLatch()]),
            dyn_cast<SuperNode>(map_value_node[L->getExitBlock()]));

        // Insert the loop node
        auto _loop_node =
            this->dependency_graph->insertLoopNode(std::move(_new_loop));
        auto _l_head = dyn_cast<SuperNode>(map_value_node[L->getHeader()]);
        auto _l_exit = dyn_cast<SuperNode>(map_value_node[L->getExitBlock()]);

        // Change the type of loop head basic block
        _l_head->setNodeType(SuperNode::SuperNodeType::LoopHead);

        auto _src_br_inst_it = *std::find_if(
            _l_head->inputControl_begin(), _l_head->inputControl_end(),
            [&L](auto const _node_it) {
                return !L->contains(
                    dyn_cast<BranchNode>(_node_it)->getInstruction());
            });

        // Add basic block node between the branch and head basic block
        auto _old_edge =
            this->dependency_graph->findEdge(_src_br_inst_it, _l_head);

        // dependency_graph->insertEdge(
        // Edge::ControlTypeEdge,
        // std::make_pair(
        //_src_br_inst_it,
        //_src_br_inst_it->returnControlOutputPortIndex(_l_head)),
        // std::make_pair(_loop_node, 0));

        // dependency_graph->insertEdge(
        // Edge::ControlTypeEdge, std::make_pair(_loop_node, 0),
        // std::make_pair(
        //_l_head,
        //_l_head->returnControlInputPortIndex(_src_br_inst_it)));

        // dependency_graph->removeEdge(_src_br_inst_it, _l_head);

        _loop_node->setEnableLoopSignal(_src_br_inst_it);
        _loop_node->setActiveOutputLoopSignal(_l_head);

        _src_br_inst_it->replaceControlOutputNode(_l_head, _loop_node);
        _l_head->replaceControlInputNode(_src_br_inst_it, _loop_node);

        // Connect the latch ending branch to loopNode
        if (auto _latch_br = dyn_cast<BranchNode>(
                map_value_node[&L->getLoopLatch()->back()])) {
            auto _src_idx = _latch_br->addControlOutputPort(_loop_node);
            _loop_node->setLoopLatchEnable(_latch_br);

            // dependency_graph->insertEdge(Edge::ControlTypeEdge,
            // std::make_pair(_latch_br, _src_idx),
            // std::make_pair(_loop_node, 1));

        } else
            assert(!"Unexpected terminator!");

        // Connecting end branch to the loop end input
        auto _tar_exit_br_inst_it = *std::find_if(
            _l_exit->inputControl_begin(), _l_exit->inputControl_end(),
            [&L](auto const _node_it) {
                return L->contains(
                    dyn_cast<BranchNode>(_node_it)->getInstruction());
            });

        auto _src_idx = _loop_node->pushLoopExitLatch(_tar_exit_br_inst_it);
        // dependency_graph->insertEdge(
        // Edge::ControlTypeEdge,
        // std::make_pair(
        //_tar_exit_br_inst_it,
        //_tar_exit_br_inst_it->returnControlOutputPortIndex(_l_exit)),
        // std::make_pair(_loop_node, _src_idx));

        // dependency_graph->insertEdge(
        // Edge::ControlTypeEdge, std::make_pair(_loop_node, 1),
        // std::make_pair(
        //_l_exit,
        //_l_exit->returnControlInputPortIndex(_tar_exit_br_inst_it)));
        // dependency_graph->removeEdge(_tar_exit_br_inst_it, _l_exit);

        _loop_node->setLoopEndEnable(_l_exit);

        _tar_exit_br_inst_it->replaceControlOutputNode(_l_exit, _loop_node);
        _l_exit->replaceControlInputNode(_tar_exit_br_inst_it, _loop_node);

        // Update data dependencies
        /**
         * The function needs these steps:
         * 1) Detect Live-ins
         * 2) Insert a new Live-in into loop header
         * 3) Update the dependencies
         */
        for (auto B : L->blocks()) {
            for (auto &I : *B) {
                // Detecting Live-ins
                for (auto OI = I.op_begin(); OI != I.op_end(); OI++) {
                    Value *V = *OI;
                    if (definedInCaller(
                            SetVector<BasicBlock *>(L->blocks().begin(),
                                                    L->blocks().end()),
                            V)) {
                        auto new_live_in = _loop_node->insertLiveInArgument(V);

                        auto _src = map_value_node[V];
                        auto _tar = map_value_node[&I];

                        // dependency_graph->removeEdge(_src, _tar);
                        //_tar->removeNodeDataInputNode(_src);

                        // if (!dependency_graph->edgeExist(
                        // std::make_pair(_src,
                        //_src->returnDataOutputPortIndex(
                        // new_live_in)),
                        // std::make_pair(
                        // new_live_in,
                        // new_live_in->returnDataInputPortIndex(
                        //_src)))) {
                        if (_src->existDataOutput(new_live_in)) {
                            _src->replaceDataOutputNode(_tar, new_live_in);

                            // dependency_graph->insertEdge(
                            // Edge::DataTypeEdge,
                            // std::make_pair(_src,
                            //_src->returnDataOutputPortIndex(
                            // new_live_in)),
                            // std::make_pair(
                            // new_live_in,
                            // new_live_in->returnDataInputPortIndex(
                            //_src)));
                        } else
                            _src->removeNodeDataOutputNode(_tar);

                        auto _src_idx = new_live_in->addDataOutputPort(_tar);
                        auto _dst_idx = _tar->addDataInputPort(new_live_in);
                        // dependency_graph->insertEdge(
                        // Edge::DataTypeEdge,
                        // std::make_pair(new_live_in, _src_idx),
                        // std::make_pair(_tar, _dst_idx));

                        // We add only one connection between src and new
                        // live_in
                        // if (!dependency_graph->edgeExist(
                        // std::make_pair(_src,
                        //_src->returnDataOutputPortIndex(
                        // new_live_in)),
                        // std::make_pair(
                        // new_live_in,
                        // new_live_in->returnDataInputPortIndex(
                        //_src)))) {
                        // auto _src_idx =
                        //_src->addDataOutputPort(new_live_in);
                        // auto _dst_idx = new_live_in->addDataInputPort(_src);
                        // dependency_graph->insertEdge(
                        // Edge::DataTypeEdge,
                        // std::make_pair(_src, _src_idx),
                        // std::make_pair(new_live_in, _dst_idx));
                        //}
                    }
                }

                for (auto *U : I.users()) {
                    if (!definedInRegion(
                            SetVector<BasicBlock *>(L->blocks().begin(),
                                                    L->blocks().end()),
                            U)) {
                        auto new_live_out =
                            _loop_node->insertLiveOutArgument(U);

                        auto _src = map_value_node[U];
                        auto _tar = map_value_node[&I];

                        _src->removeNodeDataOutputNode(_tar);
                        _tar->removeNodeDataInputNode(_src);
                        // dependency_graph->removeEdge(_src, _tar);

                        auto _src_idx = new_live_out->addDataOutputPort(_tar);
                        auto _dst_idx = _tar->addDataInputPort(new_live_out);

                        // dependency_graph->insertEdge(
                        // Edge::DataTypeEdge,
                        // std::make_pair(new_live_out, _src_idx),
                        // std::make_pair(_tar, _dst_idx));

                        // We add only one connection between src and new
                        // live_in
                        // if (!dependency_graph->edgeExist(
                        // std::make_pair(_src,
                        //_src->returnDataOutputPortIndex(
                        // new_live_out)),
                        // std::make_pair(
                        // new_live_out,
                        // new_live_out->returnDataInputPortIndex(
                        //_src)))) {
                        if (_src->existDataOutput(new_live_out)) {
                            auto _src_idx =
                                _src->addDataOutputPort(new_live_out);
                            auto _dst_idx =
                                new_live_out->addDataInputPort(_src);

                            // dependency_graph->insertEdge(
                            // Edge::DataTypeEdge,
                            // std::make_pair(_src, _src_idx),
                            // std::make_pair(new_live_out, _dst_idx));
                        }
                    }
                }
            }
        }

        // Increament the counter
        c++;

        // Filling the containers
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
        _loop_node->setEndingInstructions();

        for (auto _en_instruction : _loop_node->endings()) {
            auto _en = _en_instruction->getInstruction();
            auto &_br_ins = map_value_node[&_en_instruction->getInstruction()
                                                ->getParent()
                                                ->getInstList()
                                                .back()];

            _en_instruction->addControlOutputPort(_br_ins);
            _br_ins->addControlInputPort(_en_instruction);
            // this->dependency_graph->insertEdge(
            // Edge::ControlTypeEdge,
            // make_pair(
            //_en_instruction,
            //_en_instruction->returnControlOutputPortIndex(_br_ins)),
            // make_pair(
            //_br_ins,
            //_br_ins->returnControlInputPortIndex(_en_instruction)));
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

/**
 * All the initializations for function members
 */
void GraphGeneratorPass::init(Function &F) {
    // Running analysis on the elements
    findDataPort(F);
    fillBasicBlockDependencies(F);
    fillLoopDependencies(*LI);
    connectOutToReturn(F);

    // Printing the graph
    dependency_graph->printGraph(PrintType::Scala);
}

// bool GraphGeneratorPass::runOnFunction(Function &F) {
bool GraphGeneratorPass::runOnModule(Module &M) {
    for (auto &F : M) {
        if (F.isDeclaration()) continue;
        if (F.getName() == XKETCHName) {
            stripDebugInfo(F);
            visit(F);
            init(F);
        }
    }

    return false;
}
