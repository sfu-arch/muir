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

#include "iterator_range.h"
#include "Dandelion/Edge.h"
#include "Dandelion/Node.h"

namespace dandelion {

using InstructionList = std::list<std::unique_ptr<InstructionNode>>;
using ArgumentList = std::list<std::unique_ptr<ArgumentNode>>;
using BasicBlockList = std::list<std::unique_ptr<SuperNode>>;
using GlobalValueList = std::list<GlobalValueNode>;
using ConstIntList = std::list<ConstIntNode>;
using LoopNodeList = std::list<std::unique_ptr<LoopNode>>;
using EdgeList = std::list<std::unique_ptr<Edge>>;

class Graph {
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

    // Memory units inside each graph
    std::unique_ptr<MemoryNode> memory_unit;

    // Loop nodes
    LoopNodeList loop_nodes;


    // Keep track of nodes and values
    std::map<llvm::Value *, Node *> map_value_node;
   public:

    explicit Graph(NodeInfo _n_info)
        : graph_info(_n_info),
          split_call(std::make_unique<SplitCallNode>(NodeInfo(0, "InputSplitter"))),
          memory_unit(std::make_unique<MemoryNode>(NodeInfo(0, "MemCtrl"))),
          graph_empty(false),
          outCode(llvm::outs()),
          function_ptr(nullptr) {}
    explicit Graph(NodeInfo _n_info, llvm::raw_ostream &_output)
        : graph_info(_n_info),
          split_call(std::make_unique<SplitCallNode>(NodeInfo(0, "InputSplitter"))),
          memory_unit(std::make_unique<MemoryNode>(NodeInfo(0, "MemCtrl"))),
          graph_empty(false),
          outCode(_output),
          function_ptr(nullptr) {}
    explicit Graph(NodeInfo _n_info, llvm::raw_ostream &_output,
                   llvm::Function *_fn)
        : graph_info(_n_info),
          split_call(std::make_unique<SplitCallNode>(NodeInfo(0, "InputSplitter"))),
          memory_unit(std::make_unique<MemoryNode>(NodeInfo(0, "MemCtrl"))),
          graph_empty(false),
          outCode(_output),
          function_ptr(_fn) {}

    void init(BasicBlockList &, InstructionList &, ArgumentList &,
              GlobalValueList &, ConstIntList &, EdgeList &);

    void printGraph(PrintType);

    bool isEmpty() { return graph_empty; }
    MemoryNode *getMemoryUnit() const{ return memory_unit.get(); }

    // InstructionList *getInstructionList();
    auto instList_begin() {return this->inst_list.cbegin();}
    auto instList_end() {return this->inst_list.cend();}
    auto instructions() {return helpers::make_range(instList_begin(), instList_end());}

    auto funarg_begin() { return this->arg_list.cbegin(); }
    auto funarg_end() { return this->arg_list.cend(); }
    auto args() {return helpers::make_range(funarg_begin(), funarg_end());}

    auto edge_begin() { return this->edge_list.cbegin(); }
    auto edge_end() { return this->edge_list.cend(); }
    auto edges() { return helpers::make_range(edge_begin(), edge_end()); }

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

    LoopNode *insertLoopNode(std::unique_ptr<LoopNode>);

    void breakEdge(Node*, Node*, Node *);


    Edge *insertEdge(Edge::EdgeType, Node *, Node *);
    void removeDataEdge(Node*, Node *);
    void removeControlEdge(Node*, Node *);

    Edge *insertMemoryEdge(Edge::EdgeType, Node *, Node *);

    SplitCallNode *getSplitCall() const { return split_call.get(); }

   protected:
    // General print functions with accepting print type
    void printFunctionArgument(PrintType);
    void printBasicBlocks(PrintType);
    void printInstructions(PrintType);
    void printMemoryModules(PrintType);
    void printBasickBlockPredicateEdges(PrintType);
    void printBasickBLockInstructionEdges(PrintType);
    void printPhiNodesConnections(PrintType);
    void printDatadependencies(PrintType);
    void printClosingclass(PrintType);
    void printLoopBranchEdges(PrintType);

    void PrintLoopHeader(PrintType);

    // Scala specific functions
    void printScalaHeader(std::string, std::string);
    void printScalaFunctionHeader();
    void printScalaInputSpliter();
    void printScalaMainClass();
};
}

#endif  // end of DANDDELION_GRAPH_H
