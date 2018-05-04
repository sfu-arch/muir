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
#include "luacpptemplater/LuaTemplater.h"

#include "Dandelion/Edge.h"
#include "Dandelion/Node.h"

namespace dandelion {

using InstructionList = std::list<std::unique_ptr<InstructionNode>>;
using ArgumentList = std::list<std::unique_ptr<ArgumentNode>>;
using BasicBlockList = std::list<std::unique_ptr<SuperNode>>;
using GlobalValueList = std::list<GlobalValueNode>;
using ConstIntList = std::list<ConstIntNode>;
using EdgeList = std::list<Edge>;

class Graph {
   public:
    using ins_citerator = std::list<std::unique_ptr<InstructionNode>>::const_iterator;

   private:
    // Node information
    NodeInfo graph_info;

    // List of elements inside each graph
    InstructionList inst_list;
    ArgumentList arg_list;
    BasicBlockList super_node_list;
    GlobalValueList glob_list;
    ConstIntList const_list;

    // Splitcall for the function
    std::unique_ptr<SplitCallNode> split_call;

    // List of the edges between nodes inside the graph
    EdgeList edge_list;

    bool graph_empty;
    llvm::Function *function_ptr;
    llvm::raw_ostream &outCode;

   public:
    // TODO make these two modules private
    // Memory units inside each graph
    MemoryUnitNode memory_unit;

    explicit Graph(NodeInfo _n_info)
        : graph_info(_n_info),
          split_call(std::make_unique<SplitCallNode>(NodeInfo(0, "InputSplitter"))),
          memory_unit(NodeInfo(0, "MemCtrl")),
          graph_empty(false),
          outCode(llvm::outs()),
          function_ptr(nullptr) {}
    explicit Graph(NodeInfo _n_info, llvm::raw_ostream &_output)
        : graph_info(_n_info),
          split_call(std::make_unique<SplitCallNode>(NodeInfo(0, "InputSplitter"))),
          memory_unit(NodeInfo(0, "MemCtrl")),
          graph_empty(false),
          outCode(_output),
          function_ptr(nullptr) {}
    explicit Graph(NodeInfo _n_info, llvm::raw_ostream &_output,
                   llvm::Function *_fn)
        : graph_info(_n_info),
          split_call(std::make_unique<SplitCallNode>(NodeInfo(0, "InputSplitter"))),
          memory_unit(NodeInfo(0, "MemCtrl")),
          graph_empty(false),
          outCode(_output),
          function_ptr(_fn) {}

    void init(BasicBlockList &, InstructionList &, ArgumentList &,
              GlobalValueList &, ConstIntList &, EdgeList &);

    void printGraph(PrintType);

    bool isEmpty() { return graph_empty; }
    MemoryUnitNode *getMemoryUnit() { return &memory_unit; }

    // InstructionList *getInstructionList();
    auto instList_begin() {return this->inst_list.cbegin();}
    auto instList_end() {return this->inst_list.cend();}

    auto funarg_begin() { return this->arg_list.cbegin(); }
    auto funarg_end() { return this->arg_list.cend(); }

    void insertInstruction(llvm::Instruction &);
    void setFunction(llvm::Function *);
    SuperNode *insertSuperNode(llvm::BasicBlock &);
    InstructionNode *insertBinaryOperatorNode(llvm::BinaryOperator &);
    InstructionNode *insertIcmpOperatorNode(llvm::ICmpInst &);
    InstructionNode *insertBranchNode(llvm::BranchInst &);
    InstructionNode *insertPhiNode(llvm::PHINode &);
    InstructionNode *insertAllocaNode(llvm::AllocaInst &);
    InstructionNode *insertGepNode(llvm::GetElementPtrInst &);
    InstructionNode *insertLoadNode(llvm::LoadInst &);
    InstructionNode *insertStoreNode(llvm::StoreInst &);
    InstructionNode *insertReturnNode(llvm::ReturnInst &);
    InstructionNode *insertCallNode(llvm::CallInst &);
    ArgumentNode *insertFunctionArgument(llvm::Argument &);
    GlobalValueNode *insertFunctionGlobalValue(llvm::GlobalValue &);
    ConstIntNode *insertConstIntNode(llvm::ConstantInt &);

    Edge *insertEdge(Edge::EdgeType, Node *, Node *);
    Edge *insertMemoryEdge(Edge::EdgeType, Node *, Node *);

    //void setNumSplitCallInput(uint32_t _n) { this->split_call.setNumInput(_n); }
    SplitCallNode *getSplitCall() const { return split_call.get(); }

   protected:
    // General print functions with accepting print type
    void printBasicBlocks(PrintType);
    void printInstructions(PrintType);
    void printMemoryModules(PrintType);
    void printBasickBlockPredicateEdges(PrintType);
    void printBasickBLockInstructionEdges(PrintType);
    void printPhiNodesConnections(PrintType);
    void printDatadependencies(PrintType);

    // Scala specific functions
    void printScalaHeader(std::string, std::string);
    void printScalaFunctionHeader();
    void printScalaInputSpliter();
};
}

#endif  // end of DANDDELION_GRAPH_H
