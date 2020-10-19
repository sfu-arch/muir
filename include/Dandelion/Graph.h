#ifndef DANDELION_GRAPH_H
#define DANDELION_GRAPH_H
#include <stdint.h>
#include <list>

#include "llvm/IR/Argument.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"

#include <fstream>
#include <ostream>
#include <set>

#ifdef __APPLE__
#include "json/json.h"
#else
#include "jsoncpp/json/json.h"
#endif

#include "Dandelion/Edge.h"
#include "Dandelion/Node.h"
#include "iterator_range.h"

using common::GepInfo;

namespace dandelion {

using InstructionList = std::list<std::unique_ptr<InstructionNode>>;
using ArgumentList    = std::list<std::unique_ptr<ArgumentNode>>;
using BasicBlockList  = std::list<std::unique_ptr<SuperNode>>;
using GlobalValueList = std::list<GlobalValueNode>;
using ConstIntList    = std::list<std::unique_ptr<ConstIntNode>>;
using ConstFPList     = std::list<std::unique_ptr<ConstFPNode>>;
using LoopNodeList    = std::list<std::unique_ptr<LoopNode>>;
using ScratchpadList  = std::list<std::unique_ptr<ScratchpadNode>>;
using EdgeList        = std::list<std::unique_ptr<Edge>>;
using Port            = std::pair<Node*, PortID>;

class Graph {
public:
  NodeInfo graph_info;

private:
  // Node information

  // List of elements inside each graph
  InstructionList inst_list;
  ArgumentList arg_list;
  BasicBlockList super_node_list;
  GlobalValueList glob_list;
  ConstIntList const_int_list;
  ConstFPList const_fp_list;

  std::list<CallInNode*> call_in_list;
  std::list<CallOutNode*> call_out_list;

  // Splitcall for the function
  std::unique_ptr<SplitCallNode> split_call;

  // List of the edges between nodes inside the graph
  EdgeList edge_list;

  // Memory units inside each graph
  std::unique_ptr<MemoryNode> memory_unit;

  // Local memories
  ScratchpadList scratchpad_memories;

  // Floating point unit
  std::unique_ptr<FloatingPointNode> floating_point_unit;

  // Loop nodes
  LoopNodeList loop_nodes;

  llvm::Function* function_ptr;

  // Out interface
  Node* out_node;

  // Keep track of nodes and values
  // std::map<llvm::Value *, Node *> map_value_node;
  //
  llvm::raw_ostream& outCode;
  bool graph_empty;

public:
  explicit Graph(NodeInfo _n_info)
    : graph_info(_n_info),
      split_call(std::make_unique<SplitCallNode>(NodeInfo(0, "ArgSplitter"))),
      memory_unit(std::make_unique<MemoryNode>(NodeInfo(0, "mem_ctrl_cache"))),
      floating_point_unit(std::make_unique<FloatingPointNode>(NodeInfo(0, "SharedFPU"))),
      function_ptr(nullptr),
      out_node(nullptr),
      outCode(llvm::outs()),
      graph_empty(false) {}
  explicit Graph(NodeInfo _n_info, llvm::raw_ostream& _output)
    : graph_info(_n_info),
      split_call(std::make_unique<SplitCallNode>(NodeInfo(0, "ArgSplitter"))),
      memory_unit(std::make_unique<MemoryNode>(NodeInfo(0, "mem_ctrl_cache"))),
      floating_point_unit(std::make_unique<FloatingPointNode>(NodeInfo(0, "SharedFPU"))),
      function_ptr(nullptr),
      out_node(nullptr),
      outCode(_output),
      graph_empty(false) {}
  explicit Graph(NodeInfo _n_info, llvm::raw_ostream& _output, llvm::Function* _fn)
    : graph_info(_n_info),
      split_call(std::make_unique<SplitCallNode>(NodeInfo(0, "ArgSplitter"))),
      memory_unit(std::make_unique<MemoryNode>(NodeInfo(0, "mem_ctrl_cache"))),
      floating_point_unit(std::make_unique<FloatingPointNode>(NodeInfo(0, "SharedFPU"))),
      function_ptr(_fn),
      out_node(nullptr),
      outCode(_output),
      graph_empty(false) {}

  void init(BasicBlockList&,
            InstructionList&,
            ArgumentList&,
            GlobalValueList&,
            ConstIntList&,
            EdgeList&);

  void doInitialization();
  void doFinalization();

  void printGraph(PrintType);
  void printGraph(PrintType, std::string json_path);

  ScratchpadNode* returnScratchpadMem(AllocaInst* alloca);

  bool
  isEmpty() {
    return graph_empty;
  }
  auto
  getMemoryUnit() const {
    return memory_unit.get();
  }
  auto
  getFPUNode() const {
    return floating_point_unit.get();
  }


  // InstructionList *getInstructionList();
  auto
  scratchpad_begin() {
    return this->scratchpad_memories.cbegin();
  }
  auto
  scratchpad_end() {
    return this->scratchpad_memories.cend();
  }
  auto
  scratchpads() {
    return helpers::make_range(scratchpad_begin(), scratchpad_end());
  }


  auto
  instList_begin() {
    return this->inst_list.cbegin();
  }
  auto
  instList_end() {
    return this->inst_list.cend();
  }
  auto
  instructions() {
    return helpers::make_range(instList_begin(), instList_end());
  }

  auto
  funarg_begin() {
    return this->arg_list.cbegin();
  }
  auto
  funarg_end() {
    return this->arg_list.cend();
  }
  auto
  args() {
    return helpers::make_range(funarg_begin(), funarg_end());
  }

  auto
  edge_begin() {
    return this->edge_list.cbegin();
  }
  auto
  edge_end() {
    return this->edge_list.cend();
  }
  auto
  edges() {
    return helpers::make_range(edge_begin(), edge_end());
  }

  auto
  loop_begin() {
    return this->loop_nodes.cbegin();
  }
  auto
  loop_end() {
    return this->loop_nodes.cend();
  }
  auto
  loops() {
    return helpers::make_range(loop_begin(), loop_end());
  }

  void
  pushCallIn(CallInNode* _call_node) {
    call_in_list.push_back(_call_node);
  }
  void
  pushCallOut(CallOutNode* _call_node) {
    call_out_list.push_back(_call_node);
  }

  void insertInstruction(llvm::Instruction&);
  void setFunction(llvm::Function*);
  ScratchpadNode* createBufferMemory(AllocaNode* alloca,
                                     uint32_t size,
                                     uint32_t num_byte);
  SuperNode* insertSuperNode(llvm::BasicBlock&);
  InstructionNode* insertBinaryOperatorNode(llvm::BinaryOperator&);
  InstructionNode* insertBitcastNode(llvm::BitCastInst&);
  InstructionNode* insertIcmpOperatorNode(llvm::ICmpInst&);
  InstructionNode* insertBranchNode(llvm::BranchInst&);
  InstructionNode* insertPhiNode(llvm::PHINode&);
  InstructionNode* insertSelectNode(llvm::SelectInst&);
  AllocaNode* insertAllocaNode(llvm::AllocaInst&, uint32_t size, uint32_t num_byte);
  // InstructionNode *insertGepNode(llvm::GetElementPtrInst &, GepArrayInfo);
  // InstructionNode *insertGepNode(llvm::GetElementPtrInst &, GepStructInfo);
  InstructionNode* insertGepNode(llvm::GetElementPtrInst&, GepInfo);

  InstructionNode* insertLoadNode(llvm::LoadInst&);
  InstructionNode* insertStoreNode(llvm::StoreInst&);
  InstructionNode* insertReturnNode(llvm::ReturnInst&);
  InstructionNode* insertCallNode(llvm::CallInst&);
  InstructionNode* insertDetachNode(llvm::DetachInst&);
  InstructionNode* insertReattachNode(llvm::ReattachInst&);
  InstructionNode* insertSyncNode(llvm::SyncInst&);
  ArgumentNode* insertFunctionArgument(llvm::Argument&);
  GlobalValueNode* insertFunctionGlobalValue(llvm::GlobalValue&);
  ConstIntNode* insertConstIntNode(llvm::ConstantInt&);
  ConstIntNode* insertConstIntNode();

  InstructionNode* insertSextNode(llvm::SExtInst&);
  InstructionNode* insertZextNode(llvm::ZExtInst&);
  InstructionNode* insertSTIoFPNode(llvm::SIToFPInst&);
  InstructionNode* insertFPToUINode(llvm::FPToUIInst&);
  InstructionNode* insertTruncNode(llvm::TruncInst&);

  InstructionNode* insertFaddNode(llvm::BinaryOperator&);
  InstructionNode* insertFsubNode(llvm::BinaryOperator&);
  InstructionNode* insertFmulNode(llvm::BinaryOperator&);
  InstructionNode* insertFdiveNode(llvm::BinaryOperator&);
  InstructionNode* insertFcmpNode(llvm::FCmpInst&);
  ConstFPNode* insertConstFPNode(llvm::ConstantFP&);

  LoopNode* insertLoopNode(std::unique_ptr<LoopNode>);

  void breakEdge(Node*, Node*, Node*);

  Edge* insertEdge(Edge::EdgeType, Port src, Port dst);
  bool edgeExist(Port _node_src, Port _node_dst);
  void removeEdge(Node*, Node*);
  Edge* findEdge(const Port _src, const Port _dst) const;
  Edge* findEdge(const Node* _src, const Node* _dst) const;

  Edge* insertMemoryEdge(Edge::EdgeType, Port src, Port dst);

  SplitCallNode*
  getSplitCall() const {
    return split_call.get();
  }

  void
  setOutputNode(Node* _n) {
    out_node = _n;
  }

public:
  // Optimization passes
  void optimizationPasses();
  void groundStoreNodes();
  void groundReattachNode();
  void printMUIR();

protected:
  // General print functions with accepting print type
  void printFunctionArgument(PrintType);
  void printCallIO(PrintType);
  void printMemIO(PrintType);
  void printBasicBlocks(PrintType);
  void printInstructions(PrintType);
  void printConstants(PrintType);
  void printSharedModules(PrintType);
  void printBasickBlockInstructionPredicateEdges(PrintType);
  void printBasickBlockLoopPredicateEdges(PrintType);
  void printBasickBLockInstructionEdges(PrintType);
  void printPhiNodesConnections(PrintType);
  void printMemInsConnections(PrintType _pt);
  void printSharedConnections(PrintType _pt);
  void printDatadependencies(PrintType);
  void printControlDependencies(PrintType);
  void printClosingclass(PrintType);
  void printLoopEndingDependencies(PrintType _pt);
  void printLoopBranchEdges(PrintType);
  void printLoopHeader(PrintType);
  void printLoopDataDependencies(PrintType);
  void printOutPort(PrintType);
  void printParallelConnections(PrintType);

  // Scala specific functions
  void printScalaHeader(std::string json_path);
  void printScalaFunctionHeader();
  void printScalaInputSpliter();
  void printScalaMainClass();
};
}  // namespace dandelion

#endif  // end of DANDDELION_GRAPH_H
