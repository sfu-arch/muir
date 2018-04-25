#ifndef DANDELION_GRAPH_H
#define DANDELION_GRAPH_H
#include <stdint.h>
#include <list>

#include "llvm/IR/Argument.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"

#include <fstream>
#include <ostream>

#include "json/json.h"

#include "Dandelion/Edge.h"
#include "Dandelion/Node.h"


namespace dandelion {

using InstructionList = std::list<InstructionNode>;
using ArgumentList = std::list<ArgumentNode>;
using BasicBlockList = std::list<SuperNode>;
using GlobalValueList = std::list<GlobalValueNode>;
using ConstIntList = std::list<ConstIntNode>;
using EdgeList = std::list<Edge>;

class Graph {

   private:
    InstructionList inst_list;
    ArgumentList arg_list;
    BasicBlockList super_node_list;
    GlobalValueList glob_list;
    ConstIntList const_list;
    EdgeList edge_list;

    bool graph_empty;

   public:
    explicit Graph():graph_empty(false) {}
    void init(BasicBlockList &, InstructionList &, ArgumentList &,
              GlobalValueList &, ConstIntList &, EdgeList &);

    void printGraph(PrintType);

    bool isEmpty(){return graph_empty;}

    const InstructionList getInstructionList();
    void insertInstruction(llvm::Instruction &);
    SuperNode * const insertSuperNode(llvm::BasicBlock &);
    InstructionNode * const insertBinaryOperatorNode(llvm::BinaryOperator &);
    InstructionNode * const insertIcmpOperatorNode(llvm::ICmpInst &);
    InstructionNode * const insertBranchNode(llvm::BranchInst &);
    InstructionNode * const insertPhiNode(llvm::PHINode &);
    InstructionNode * const insertAllocaNode(llvm::AllocaInst &);
    InstructionNode * const insertGepNode(llvm::GetElementPtrInst &);
    InstructionNode * const insertLoadNode(llvm::LoadInst &);
    InstructionNode * const insertStoreNode(llvm::StoreInst &);
    InstructionNode * const insertReturnNode(llvm::ReturnInst &);
    InstructionNode * const insertCallNode(llvm::CallInst &);
    ArgumentNode * const insertFunctionArgument(llvm::Argument &);
    GlobalValueNode * const insertFunctionGlobalValue(llvm::GlobalValue &);
    ConstIntNode* const insertConstIntNode(llvm::ConstantInt &);

    Edge * const insertEdge(Edge::EdgeType , Node * const, Node * const);

   protected:
    void printBasicBlocks(PrintType);

    // Scala specific functions
    void printScalaHeader();
};
}

#endif  // end of DANDDELION_GRAPH_H
