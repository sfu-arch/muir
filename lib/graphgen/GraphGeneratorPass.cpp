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
}

/**
 * This function is a helper function which only gets a new instruction
 * and insert a new entry to our map to instruction value map
 */
void inline HelperInsertInstructionMap(
    InstructionList &ins_list, std::map<llvm::Value *, Node *> &map_node,
    llvm::Value &V) {
    auto ff = std::find_if(ins_list.begin(), ins_list.end(),
                           [&V](InstructionNode &arg) -> bool {
                               return arg.getInstruction() == &V;
                           });
    map_node[&V] = &*ff;
}

bool GraphGeneratorPass::doInitialization(Module &M) {
    // TODO: Add code here if it's needed before pas
    return false;
}

bool GraphGeneratorPass::doFinalization(Module &M) {
    cout << "Number of instruction nodes: " << this->GraphDependency.getInstructionList().size() << endl;
    // TODO: Add code here to do post pass
    return false;
}

/**
 * Iterating over target function's basicblocks and
 * then make supernode for each of them and add them to the node list
 */
void GraphGeneratorPass::visitBasicBlock(BasicBlock &BB) {
    map_value_node[&BB] = this->GraphDependency.insertSuperNode(BB);
}

void GraphGeneratorPass::visitInstruction(Instruction &Ins) {
    // Here we have to check see whether we have missed any instruction or not
    Ins.dump();
    assert(!"Instruction is not supported");
}

void GraphGeneratorPass::visitBinaryOperator(llvm::BinaryOperator &I) {
    this->instruction_list.push_back(BinaryOperatorNode(&I));
    HelperInsertInstructionMap(instruction_list, map_value_node, I);
}

void GraphGeneratorPass::visitICmpInst(llvm::ICmpInst &I) {
    this->instruction_list.push_back(IcmpNode(&I));
    auto ff = std::find_if(instruction_list.begin(), instruction_list.end(),
                           [&I](InstructionNode &arg) -> bool {
                               return arg.getInstruction() == &I;
                           });

    map_value_node[&I] = &*ff;
}

void GraphGeneratorPass::visitBranchInst(llvm::BranchInst &I) {
    this->instruction_list.push_back(BranchNode(&I));
    HelperInsertInstructionMap(instruction_list, map_value_node, I);
}

void GraphGeneratorPass::visitPHINode(llvm::PHINode &I) {
    this->instruction_list.push_back(PhiNode(&I));
    HelperInsertInstructionMap(instruction_list, map_value_node, I);
}

void GraphGeneratorPass::visitAllocaInst(llvm::AllocaInst &I) {
    this->instruction_list.push_back(AllocaNode(&I));
    HelperInsertInstructionMap(instruction_list, map_value_node, I);
}

void GraphGeneratorPass::visitGetElementPtrInst(llvm::GetElementPtrInst &I) {
    this->instruction_list.push_back(GEPNode(&I));
    HelperInsertInstructionMap(instruction_list, map_value_node, I);
}

void GraphGeneratorPass::visitLoadInst(llvm::LoadInst &I) {
    this->instruction_list.push_back(LoadNode(&I));
    HelperInsertInstructionMap(instruction_list, map_value_node, I);
}

void GraphGeneratorPass::visitStoreInst(llvm::StoreInst &I) {
    this->instruction_list.push_back(StoreNode(&I));
    HelperInsertInstructionMap(instruction_list, map_value_node, I);
}

void GraphGeneratorPass::visitReturnInst(llvm::ReturnInst &I) {
    this->instruction_list.push_back(ReturnNode(&I));
    HelperInsertInstructionMap(instruction_list, map_value_node, I);
}

void GraphGeneratorPass::visitCallInst(llvm::CallInst &I) {
    this->instruction_list.push_back(CallNode(&I));
    HelperInsertInstructionMap(instruction_list, map_value_node, I);
}

void GraphGeneratorPass::visitFunction(Function &F) {
    // TODO
    // Here we make a graph
    // Graph gg()
    // Filling function argument nodes
    for (auto &f_arg : F.getArgumentList()) {
        this->argument_list.push_back(ArgumentNode(&f_arg));
        auto ff = std::find_if(argument_list.begin(), argument_list.end(),
                               [&f_arg](ArgumentNode &arg_nd) -> bool {
                                   return arg_nd.getArgumentValue() == &f_arg;
                               });

        map_value_node[&f_arg] = &*ff;
    }

    // Filling global variables
    for (auto &g_var : F.getParent()->getGlobalList()) {
        this->glob_list.push_back(GlobalValueNode(&g_var));
        auto ff = std::find_if(glob_list.begin(), glob_list.end(),
                               [&g_var](GlobalValueNode &arg_nd) -> bool {
                                   return arg_nd.getGlobalValue() == &g_var;
                               });

        map_value_node[&g_var] = &*ff;
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

    for (auto ins_it = inst_begin(F); ins_it != inst_end(F); ++ins_it) {
        for (uint32_t c = 0; c < ins_it->getNumOperands(); ++c) {
            auto operand = ins_it->getOperand(c);

            if (auto target_bb = dyn_cast<llvm::BasicBlock>(operand)) {
                // Here we have to add a new edge from instruction to BB
                // 1) First find the basicblock node
                // 2) Add the bb as a control output
                // 3) Add the ins ass a control input
                //
                auto _node_dest = this->map_value_node.find(
                    operand);  // it should be supernode
                assert(isa<SuperNode>(_node_dest->second) &&
                       "Destination node should be super node!");

                auto _node_src =
                    this->map_value_node.find(&*ins_it);  // it should be node
                assert(isa<InstructionNode>(_node_src->second) &&
                       "Source node should be instruction node!");

                _node_dest->second->addControlInputPort(_node_src->second);
                _node_src->second->addControlOutputPort(_node_dest->second);

                edge_list.push_back(Edge(Edge::DataToControlTypeEdge,
                                         _node_src->second,
                                         _node_dest->second));

            } else {
                // If the operand is constant we have to create a new node
                if (auto const_value = dyn_cast<llvm::ConstantInt>(operand)) {
                    this->const_int_list.push_back(ConstIntNode(const_value));
                    auto ff = std::find_if(
                        const_int_list.begin(), const_int_list.end(),
                        [const_value](ConstIntNode &nd) -> bool {
                            return nd.getConstantParent() == const_value;
                        });
                    map_value_node[operand] = &*ff;
                }

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

                edge_list.push_back(Edge(Edge::DataTypeEdge, _node_src->second,
                                         _node_dest->second));
            }
        }
    }
}

/**
 * This function has two tasks:
 * 1) Iterate over the basicblock's insturcitons and make a list of those
 * instructions
 * 2) Make control dependnce edges
 */
void GraphGeneratorPass::fillBasicBlockDependencies(Function &F) {
    for (auto &BB : F) {
        if (auto _bb = dyn_cast<SuperNode>(this->map_value_node[&BB])) {
            for (auto &I : BB) {
                // Iterate over the basicblock's instructions
                if (auto _ins =
                        dyn_cast<InstructionNode>(this->map_value_node[&I])) {
                    _bb->addInstruction(_ins);

                    // Detect Phi instrucctions
                    if (auto _phi_ins = dyn_cast<PhiNode>(_ins))
                        _bb->addPhiInstruction(_phi_ins);

                    // Make a control edge
                    edge_list.push_back(Edge(Edge::ControlTypeEdge, _bb, _ins));
                } else
                    assert(!"The instruction is not visited!");

                // Iterate over the basicblock's predecessors
                // for (auto _bb_pp : llvm::predecessors(&BB)) {
                //}
            }

        } else
            assert(!"The basicblock is not visited!");
    }
}

/**
 * Does all the initializations for function members
 */
void GraphGeneratorPass::init(Function &F) {

    //Running analysis on the elements
    findDataPort(F);
    fillBasicBlockDependencies(F);

    // Initilizing the graph
    graph.init(super_node_list, instruction_list, argument_list, glob_list,
               const_int_list, edge_list);
    graph.printGraph(PrintType::Scala);
}

bool GraphGeneratorPass::runOnFunction(Function &F) {
    stripDebugInfo(F);
    visit(F);
    init(F);

    return false;
}
