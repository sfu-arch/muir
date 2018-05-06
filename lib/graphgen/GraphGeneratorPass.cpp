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
    // TODO: Add code here if it's needed before pas

    // TODO: Uncomment to grab all the loop information
    // for (auto &F : M) {
    // if (F.isDeclaration()) continue;
    // if (F.getName() == XKETCHName) {
    // this->LI = &getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    //}
    //}

    return false;
}

bool GraphGeneratorPass::doFinalization(Module &M) {
    // TODO: Add code here to do post pass
    return false;
}

/**
 * Iterating over target function's basicblocks and
 * then make supernode for each of them and add them to the node list
 */
void GraphGeneratorPass::visitBasicBlock(BasicBlock &BB) {
    map_value_node[&BB] = this->dependency_graph.insertSuperNode(BB);
}

void GraphGeneratorPass::visitInstruction(Instruction &Ins) {
    // Here we have to check see whether we have missed any instruction or not
    Ins.dump();
    assert(!"Instruction is not supported");
}

void GraphGeneratorPass::visitBinaryOperator(llvm::BinaryOperator &I) {
    map_value_node[&I] = this->dependency_graph.insertBinaryOperatorNode(I);
}

void GraphGeneratorPass::visitICmpInst(llvm::ICmpInst &I) {
    map_value_node[&I] = this->dependency_graph.insertIcmpOperatorNode(I);
}

void GraphGeneratorPass::visitBranchInst(llvm::BranchInst &I) {
    map_value_node[&I] = this->dependency_graph.insertBranchNode(I);
}

void GraphGeneratorPass::visitPHINode(llvm::PHINode &I) {
    map_value_node[&I] = this->dependency_graph.insertPhiNode(I);
}

void GraphGeneratorPass::visitAllocaInst(llvm::AllocaInst &I) {
    map_value_node[&I] = this->dependency_graph.insertAllocaNode(I);
}

void GraphGeneratorPass::visitGetElementPtrInst(llvm::GetElementPtrInst &I) {
    map_value_node[&I] = this->dependency_graph.insertGepNode(I);
}

void GraphGeneratorPass::visitLoadInst(llvm::LoadInst &I) {
    map_value_node[&I] = this->dependency_graph.insertLoadNode(I);
}

void GraphGeneratorPass::visitStoreInst(llvm::StoreInst &I) {
    map_value_node[&I] = this->dependency_graph.insertStoreNode(I);
}

void GraphGeneratorPass::visitReturnInst(llvm::ReturnInst &I) {
    map_value_node[&I] = this->dependency_graph.insertReturnNode(I);
}

void GraphGeneratorPass::visitCallInst(llvm::CallInst &I) {
    map_value_node[&I] = this->dependency_graph.insertCallNode(I);
}

void GraphGeneratorPass::visitFunction(Function &F) {
    dependency_graph.setFunction(&F);

    // TODO
    // Here we make a graph
    // Graph gg()
    // Filling function argument nodes
    for (auto &f_arg : F.args()) {
        map_value_node[&f_arg] =
            this->dependency_graph.insertFunctionArgument(f_arg);
        // this->dependency_graph.getSplitCall()->insertNewDataPort(f_arg.getNumUses());
    }

    // this->dependency_graph.setNumSplitCallInput(F.arg_size());

    // Filling function global nodes
    for (auto &g_var : F.getParent()->getGlobalList()) {
        map_value_node[&g_var] =
            this->dependency_graph.insertFunctionGlobalValue(g_var);
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
    for (auto _fun_arg_it = this->dependency_graph.funarg_begin();
         _fun_arg_it != this->dependency_graph.funarg_end(); _fun_arg_it++) {
        auto _fun_arg_node = dyn_cast<ArgumentNode>(_fun_arg_it->get());
        auto _spliter = this->dependency_graph.getSplitCall();

        _fun_arg_node->addDataInputPort(_spliter);
        _spliter->addDataOutputPort(_fun_arg_node);

        this->dependency_graph.insertEdge(Edge::DataTypeEdge, _spliter,
                                          _fun_arg_node);
    }

    for (auto ins_it = inst_begin(F); ins_it != inst_end(F); ++ins_it) {
        // Connecting DFG and CFG edges
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

                _node_dest->second->addControlInputPort(_node_src->second);
                _node_src->second->addControlOutputPort(_node_dest->second);

                this->dependency_graph.insertEdge(Edge::ControlTypeEdge,
                                                  _node_src->second,
                                                  _node_dest->second);

            } else {
                // If the operand is constant we have to create a new node
                if (auto const_value = dyn_cast<llvm::ConstantInt>(operand))
                    map_value_node[operand] =
                        this->dependency_graph.insertConstIntNode(*const_value);

                auto _node_src = this->map_value_node.find(operand);
                auto _node_dest = this->map_value_node.find(&*ins_it);

                if (_node_src == this->map_value_node.end()) {
                    DEBUG(ins_it->dump());
                    assert(!"The destination instruction couldn't find!");
                }

                if (_node_dest == this->map_value_node.end()) {
                    DEBUG(ins_it->dump());
                    assert(!"The destination instruction couldn't find!");
                }

                _node_src->second->addDataOutputPort(_node_dest->second);
                _node_dest->second->addDataInputPort(_node_src->second);

                this->dependency_graph.insertEdge(
                    Edge::DataTypeEdge, _node_src->second, _node_dest->second);
            }
        }

        // Connecting LD and ST nodes to Memory system
        if (auto _ld_node = dyn_cast<LoadNode>(
                this->map_value_node.find(&*ins_it)->second)) {
            // TODO right now we consider all the connections to the cache or
            // regfile
            // We need a pass to trace the pointers
            this->dependency_graph.getMemoryUnit()->addReadMemoryReqPort(
                _ld_node);
            this->dependency_graph.getMemoryUnit()->addReadMemoryRespPort(
                _ld_node);
            _ld_node->addReadMemoryReqPort(
                this->dependency_graph.getMemoryUnit());
            _ld_node->addReadMemoryRespPort(
                this->dependency_graph.getMemoryUnit());
        } else if (auto _ld_node = isa<StoreNode>(
                       this->map_value_node.find(&*ins_it)->second)) {
            // TODO right now we consider all the connections to the cache or
            // regfile
            // We need a pass to trace the pointers
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
            this->dependency_graph.insertEdge(
                Edge::ControlTypeEdge, this->dependency_graph.getSplitCall(),
                _en_bb);
            _en_bb->addControlInputPort(this->dependency_graph.getSplitCall());
            this->dependency_graph.getSplitCall()->addControlOutputPort(_en_bb);
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
                        this->dependency_graph.insertEdge(Edge::MaskTypeEdge,
                                                          _bb, _phi_ins);
                    }

                    // Make a control edge
                    _bb->addControlOutputPort(_ins);
                    _ins->addControlInputPort(_bb);
                    this->dependency_graph.insertEdge(Edge::ControlTypeEdge,
                                                      _bb, _ins);
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
void GraphGeneratorPass::fillLoopDependencies(llvm::LoopInfo &loop_info){

    uint32_t c = 0;
    for(auto &L : getLoops(loop_info)){
        auto _new_loop = std::make_unique<LoopNode>(NodeInfo(c, "Loop_" + std::to_string(c)));
        _new_loop->setHeadNode(dyn_cast<SuperNode>(map_value_node[L->getHeader()]));
        _new_loop->setLatchNode(dyn_cast<SuperNode>(map_value_node[L->getLoopLatch()]));
        c++;
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

    // Printing the graph
    dependency_graph.printGraph(PrintType::Scala);
}

bool GraphGeneratorPass::runOnFunction(Function &F) {
    stripDebugInfo(F);
    visit(F);
    init(F);

    return false;
}
