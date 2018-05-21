#ifndef DANDELION_NODE_H
#define DANDELION_NODE_H
#include <stdint.h>
#include <list>

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

#include "iterator_range.h"

#define XLEN 32
#define LOOPCONTROL 2

using namespace llvm;

namespace dandelion {

class Node;
class SuperNode;
class ContainerNode;
class InstructionNode;
class LoopNode;
class MemoryNode;
class PhiSelectNode;
class SplitCallNode;
class ArgumentNode;

enum PrintType { Scala = 0, Dot, Json };

struct DataPort {
    std::list<Node *> data_input_port;
    std::list<Node *> data_output_port;
};

struct ControlPort {
    std::list<Node *> control_input_port;
    std::list<Node *> control_output_port;
};

struct MemoryPort {
    std::list<Node *> memory_req_port;
    std::list<Node *> memory_resp_port;
};

struct NodeInfo {
    uint32_t ID;
    std::string Name;

    NodeInfo(uint32_t _id, std::string _n) : ID(_id), Name(_n){};
};

struct PortID {
    uint32_t ID;

    // Default value for ID is equalt to zero
    PortID() : ID(0) {}
    PortID(uint32_t _id) : ID(_id) {}

    uint32_t getID(){ return ID; }

    bool operator==(const PortID &rhs) const { return this->ID == rhs.ID; }
};

class Node {
   public:
    enum NodeType {
        SuperNodeTy = 0,
        LoopNodeTy,
        InstructionNodeTy,
        FunctionArgTy,
        GlobalValueTy,
        ConstIntTy,
        MemoryUnitTy,
        ContainerTy,
        UnkonwTy

    };

   private:
    // Type of the Node
    NodeType node_type;
    // Node information
    NodeInfo info;

    // List of data ports
    DataPort port_data;

    // List of Control ports
    ControlPort port_control;

    // Memory ports
    MemoryPort read_port_data;
    MemoryPort write_port_data;

   public:  // Public methods
    Node(NodeType _nt, NodeInfo _ni) : info(_ni), node_type(_nt) {}

    PortID returnDataInputPortIndex(Node *);
    PortID returnControlInputPortIndex(Node *);
    PortID returnMemoryReadInputPortIndex(Node *);
    PortID returnMemoryWriteInputPortIndex(Node *);

    PortID returnDataOutputPortIndex(Node *);
    PortID returnControlOutputPortIndex(Node *);
    PortID returnMemoryReadOutputPortIndex(Node *);
    PortID returnMemoryWriteOutputPortIndex(Node *);

    PortID addDataInputPort(Node *);
    PortID addDataOutputPort(Node *);

    PortID addControlInputPort(Node *);
    PortID addControlOutputPort(Node *);

    void resizeControlInputPort(uint32_t _s) {
        this->port_control.control_input_port.resize(_s);
    }
    void resizeControlOutputPort(uint32_t _s) {
        this->port_control.control_output_port.resize(_s);
    }

    PortID addReadMemoryReqPort(Node *);
    PortID addReadMemoryRespPort(Node *);

    PortID addWriteMemoryReqPort(Node *);
    PortID addWriteMemoryRespPort(Node *);

    uint32_t numDataInputPort() { return port_data.data_input_port.size(); }
    uint32_t numDataOutputPort() { return port_data.data_output_port.size(); }

    uint32_t numControlInputPort() {
        return port_control.control_input_port.size();
    }
    uint32_t numControlOutputPort() {
        return port_control.control_output_port.size();
    }

    uint32_t numReadDataInputPort() {
        return read_port_data.memory_req_port.size();
    }
    uint32_t numReadDataOutputPort() {
        return read_port_data.memory_resp_port.size();
    }
    uint32_t numWriteDataInputPort() {
        return write_port_data.memory_req_port.size();
    }
    uint32_t numWriteDataOutputPort() {
        return write_port_data.memory_resp_port.size();
    }

    std::list<Node *>::iterator findDataInputNode(Node *);
    std::list<Node *>::iterator findDataOutputNode(Node *);
    std::list<Node *>::iterator findControlInputNode(Node *);
    std::list<Node *>::iterator findControlOutputNode(Node *);

    void removeNodeDataInputNode(Node *);
    void removeNodeDataOutputNode(Node *);
    void removeNodeControlInputNode(Node *);
    void removeNodeControlOutputNode(Node *);

    /// replace two nodes form the control input container
    void replaceControlInputNode(Node *src, Node *tar);

    /// replace two nodes form the control output container
    void replaceControlOutputNode(Node *src, Node *tar);

    /// replace two nodes form the control input container
    void replaceDataInputNode(Node *src, Node *tar);

    /// replace two nodes form the control output container
    void replaceDataOutputNode(Node *src, Node *tar);

    // Iterator over input data edges
    auto inputDataport_begin() {
        return this->port_data.data_input_port.cbegin();
    }
    auto inputDataport_end() { return this->port_data.data_input_port.cend(); }

    auto input_data_range() {
        return helpers::make_range(inputDataport_begin(), inputDataport_end());
    }

    // Iterator over output data edges
    auto outputDataport_begin() {
        return this->port_data.data_output_port.cbegin();
    }
    auto outputDataport_end() {
        return this->port_data.data_output_port.cend();
    }
    auto output_data_range() {
        return helpers::make_range(outputDataport_begin(),
                                   outputDataport_end());
    }

    // Iterator over input control edges
    auto inputControl_begin() {
        return this->port_control.control_input_port.cbegin();
    }
    auto inputControl_end() {
        return this->port_control.control_input_port.cend();
    }
    auto input_control_range() {
        return helpers::make_range(inputControl_begin(), inputControl_end());
    }

    // Iterator over output control edges
    auto outputControl_begin() {
        return this->port_control.control_output_port.cbegin();
    }
    auto outputControl_end() {
        return this->port_control.control_output_port.cend();
    }
    auto output_control_range() {
        return helpers::make_range(outputControl_begin(), outputControl_end());
    }

    uint32_t getID() { return info.ID; }
    std::string getName() { return info.Name; }

    // TODO how to define virtual functions?
    // virtual void printInitilization() {}

    uint32_t getType() const { return node_type; }

   protected:
    /**
     * Adding a node to a specific index of control input port
     */
    void addControlInputPortIndex(Node *_n, uint32_t _id) {
        auto it = port_control.control_input_port.begin();
        std::advance(it, _id);
        std::replace(port_control.control_input_port.begin(),
                     port_control.control_input_port.end(), *it, _n);
    }

    /**
     * Adding a node to a specific index of control output port
     */
    void addControlOutputPortIndex(Node *_n, uint32_t _id) {
        auto it = port_control.control_output_port.begin();
        std::advance(it, _id);
        std::replace(port_control.control_output_port.begin(),
                     port_control.control_output_port.end(), *it, _n);
    }

   public:
    virtual std::string printDefinition(PrintType) {
        return this->info.Name + std::string(" Definition is Not defined!");
    }
    virtual std::string printInputEnable(PrintType, uint32_t) {
        return this->info.Name +
               std::string(" EnableInput with ID Not defined!");
    }
    virtual std::string printInputEnable(PrintType) {
        return this->info.Name + std::string(" EnableInput Not defined!");
    }
    virtual std::string printOutputEnable(PrintType) {
        return this->info.Name + std::string(" EnableOutput Not defined!");
    }
    virtual std::string printOutputEnable(PrintType, uint32_t) {
        return this->info.Name +
               std::string(" -> EnableOutput with ID Not defined!");
    }
    virtual std::string printInputData(PrintType) {
        return this->info.Name + std::string(" -> DataInput Not defined!");
    }
    virtual std::string printInputData(PrintType, uint32_t) {
        return this->info.Name +
               std::string(" -> DataInput with ID Not defined!");
    }
    virtual std::string printOutputData(PrintType) {
        return this->info.Name + std::string(" -> DataOutput Not defined!");
    }
    virtual std::string printOutputData(PrintType, uint32_t) {
        return this->info.Name +
               std::string(" -> DataOutput with ID Not defined!");
    }

    virtual std::string printMemReadInput(PrintType, uint32_t) {
        return this->info.Name +
               std::string(" -> MemInput with ID Not defined!");
    }

    virtual std::string printMemReadOutput(PrintType, uint32_t) {
        return this->info.Name +
               std::string(" -> MemOutput with ID Not defined!");
    }

    virtual std::string printMemWriteInput(PrintType, uint32_t) {
        return this->info.Name +
               std::string(" -> MemInput with ID Not defined!");
    }

    virtual std::string printMemWriteOutput(PrintType, uint32_t) {
        return this->info.Name +
               std::string(" -> MemOutput with ID Not defined!");
    }
};

/**
 * Super node is actual implimetation of our basic blocks
 */
class SuperNode : public Node {
   public:
    // List of the instructions
    using PhiNodeList = std::list<PhiSelectNode *>;
    enum SuperNodeType { Mask, NoMask, LoopHead };

   private:
    llvm::BasicBlock *basic_block;

    std::list<InstructionNode *> instruction_list;
    PhiNodeList phi_list;

    SuperNodeType type;

   public:
    explicit SuperNode(NodeInfo _nf, llvm::BasicBlock *_bb = nullptr)
        : Node(Node::SuperNodeTy, _nf),
          basic_block(_bb),
          type(SuperNodeType::NoMask) {}

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::SuperNodeTy;
    }

    llvm::BasicBlock *getBasicBlock();
    void addInstruction(InstructionNode *);
    void addPhiInstruction(PhiSelectNode *);

    bool hasPhi() { return !phi_list.empty(); }
    uint32_t getNumPhi() const { return phi_list.size(); }

    auto phi_begin() { return this->phi_list.cbegin(); }
    auto phi_end() { return this->phi_list.cend(); }
    auto phis() { return helpers::make_range(phi_begin(), phi_end()); }

    auto ins_begin() const { return this->instruction_list.begin(); }
    auto ins_end() const { return this->instruction_list.end(); }
    auto instructions() { return helpers::make_range(ins_begin(), ins_end()); }

    const SuperNodeType getNodeType() { return type; }
    void setNodeType(SuperNodeType _t) { this->type = _t; }
    // void setActivateInput(Node *_n) { this->activate_input = _n; }
    // void setExitInput(Node *_n) { this->exit_input = _n; }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printOutputEnable(PrintType, uint32_t) override;
    virtual std::string printMaskOutput(PrintType, uint32_t);
    std::string printActivateEnable(PrintType);
};

class ArgumentNode : public Node {
   private:
    ContainerNode *parent_call_node;
    llvm::Value *parent_argument;

   public:
    ArgumentNode(NodeInfo _ni, ContainerNode *_call_node = nullptr,
                 llvm::Argument *_arg = nullptr)
        : Node(Node::FunctionArgTy, _ni),
          parent_call_node(_call_node),
          parent_argument(_arg) {}

    const llvm::Value *getArgumentValue() { return parent_argument; }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
};

/**
 * Container node
 */
class ContainerNode : public Node {
   public:
    enum ContainType { LoopNodeTy = 0, SplitCallTy };
    using RegisterList = std::list<std::unique_ptr<ArgumentNode>>;

   private:
    ContainType con_type;
    RegisterList live_in;
    RegisterList live_out;

   public:
    explicit ContainerNode(NodeInfo _nf)
        : Node(Node::ContainerTy, _nf), con_type(ContainType::LoopNodeTy) {}

    explicit ContainerNode(NodeInfo _nf, ContainType _cn_type)
        : Node(Node::ContainerTy, _nf), con_type(_cn_type) {}

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::ContainerTy;
    }

    uint32_t getContainerType() const { return con_type; }

    ArgumentNode *insertLiveInArgument(llvm::Value *);
    ArgumentNode *insertLiveOutArgument(llvm::Value *);

    uint32_t findLiveInIndex(ArgumentNode *);
    uint32_t findLiveOutIndex(ArgumentNode *);

    uint32_t numLiveIn() { return live_in.size(); }
    uint32_t numLiveOut() { return live_out.size(); }

    auto live_in_begin() { return this->live_in.cbegin(); }
    auto live_in_end() { return this->live_in.cend(); }
    auto live_ins() {
        return helpers::make_range(live_in_begin(), live_in_end());
    }

    auto live_out_begin() { return this->live_out.cbegin(); }
    auto live_out_end() { return this->live_out.cend(); }
    auto live_outs() {
        return helpers::make_range(live_out_begin(), live_out_end());
    }
};

/**
 * Memory unit works as a local memory for each graph
 */
class MemoryNode : public Node {
   public:
    explicit MemoryNode(NodeInfo _nf) : Node(Node::MemoryUnitTy, _nf) {}

    // Restrict access to data input ports
    void addDataInputPort(Node *) = delete;
    void addDataOutputPort(Node *) = delete;
    uint32_t numDataInputPort() = delete;
    uint32_t numDataOutputPort() = delete;

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::MemoryUnitTy;
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printMemReadInput(PrintType, uint32_t) override;
    virtual std::string printMemReadOutput(PrintType, uint32_t) override;
    virtual std::string printMemWriteInput(PrintType, uint32_t) override;
    virtual std::string printMemWriteOutput(PrintType, uint32_t) override;
};

/**
 * LoopNode contains all the instructions and useful information about the loops
 */
class LoopNode : public ContainerNode {
   private:
    std::list<InstructionNode *> instruction_list;
    std::list<SuperNode *> basic_block_list;

    SuperNode *head_node;
    SuperNode *latch_node;
    SuperNode *exit_node;

    // Restrict the access to these two functions
    using Node::addControlInputPort;
    using Node::addControlOutputPort;

   public:
    explicit LoopNode(NodeInfo _nf, SuperNode *_hnode = nullptr,
                      SuperNode *_lnode = nullptr, SuperNode *_ex = nullptr)
        : ContainerNode(_nf, ContainerNode::LoopNodeTy),
          head_node(_hnode),
          exit_node(_ex),
          latch_node(_lnode) {
        // Set the size of control input prot to at least two
        resizeControlInputPort(LOOPCONTROL);
    }

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::LoopNodeTy;
    }

    // Iterator over instucrion list
    auto ins_begin() { return instruction_list.cbegin(); }
    auto ins_end() { return instruction_list.cend(); }
    auto instructions() { return helpers::make_range(ins_begin(), ins_end()); }

    // Iterator over basic block list
    auto bb_begin() { return basic_block_list.cbegin(); }
    auto bb_end() { return basic_block_list.cend(); }
    auto bblocks() { return helpers::make_range(bb_begin(), bb_end()); }

    // Iterator over input edges

    void setHeadNode(SuperNode *_n) { head_node = _n; }
    void setLatchNode(SuperNode *_n) { latch_node = _n; }

    /**
     * Make sure that loop enable signal is always set to index 0
     */
    void setEnableLoopSignal(Node *_n) { addControlInputPortIndex(_n, 0); }

    /**
     * Make sure the loop enable signal is always set to index 0
     */
    void setActiveOutputLoopSignal(Node *_n) {
        addControlOutputPortIndex(_n, 0);
    }

    /**
     * Make sure that loop latch enable signal is always fix to index 1
     */
    void setLoopLatchEnable(Node *_n) { addControlInputPortIndex(_n, 1); }

    /**
     * Make sure that loop end enable signal is always fix to index 1
     */
    void setLoopEndEnable(Node *_n) { addControlOutputPortIndex(_n, 1); }

    /**
     * Make sure that loop exit points are always starting from index 2
     */
    PortID pushLoopExitLatch(Node *_n) {
        assert(numControlInputPort() > 1 && "Error in loop control signal!");
        return addControlInputPort(_n);
    }

    /**
     * Print functions
     */
    virtual std::string printDefinition(PrintType) override;
    virtual std::string printOutputEnable(PrintType) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printOutputEnable(PrintType, uint32_t) override;
};

/**
 * This class is basic implementation of Instruction nodes
 * It inheretens from Node class and it has a pointer to original bitcode IR
 * the pointer can be NULL that means this is a new insturction type
 */
class InstructionNode : public Node {
   public:
    enum InstType {
        BinaryInstructionTy,
        IcmpInstructionTy,
        BranchInstructionTy,
        PhiInstructionTy,
        AllocaInstructionTy,
        GetElementPtrInstTy,
        LoadInstructionTy,
        StoreInstructionTy,
        SextInstructionTy,
        ZextInstructionTy,
        BitCastInstructionTy,
        TruncInstructionTy,
        SelectInstructionTy,
#ifdef TAPIR
        DetachInstructionTy,
        ReattachInstructionTy,
        SyncInstructionTy,
#endif
        ReturnInstrunctionTy,
        CallInstructionTy
    };

   private:
    InstType ins_type;
    llvm::Instruction *parent_instruction;

   public:
    InstructionNode(NodeInfo _ni, InstType _ins_t,
                    llvm::Instruction *_ins = nullptr)
        : Node(Node::InstructionNodeTy, _ni),
          ins_type(_ins_t),
          parent_instruction(_ins) {}

    llvm::Instruction *getInstruction();

    uint32_t getOpCode() const { return ins_type; }
    const std::string getOpCodeName() {
        return parent_instruction->getOpcodeName();
    }

    bool isBinaryOp() const { return ins_type == BinaryInstructionTy; }

    static bool classof(const Node *T) {
        return T->getType() == Node::InstructionNodeTy;
    }

    virtual std::string printDefinition(PrintType) override {
        return std::string("Not defined instructions\n");
    }
};

class BinaryOperatorNode : public InstructionNode {
   public:
    BinaryOperatorNode(NodeInfo _ni, llvm::BinaryOperator *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::BinaryInstructionTy, _ins) {}

    // Overloading isa<>, dyn_cast from llvm
    static bool classof(const InstructionNode *I) {
        return I->getOpCode() == InstType::BinaryInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
};

class IcmpNode : public InstructionNode {
   public:
    IcmpNode(NodeInfo _ni, llvm::ICmpInst *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::IcmpInstructionTy, _ins) {}

    static bool classof(const InstructionNode *I) {
        return I->getOpCode() == InstType::IcmpInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
};

class BranchNode : public InstructionNode {
   public:
    BranchNode(NodeInfo _ni, llvm::BranchInst *_ins = nullptr)
        : InstructionNode(_ni, InstType::BranchInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::BranchInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    /**
     * Because each index is fixed for branch node the user
     * can not remove any node from the control port
     * he only can swap
     */
    void removeNodeControlInputNode(Node *) = delete;
    void removeNodeControlOutputNode(Node *) = delete;

    /**
     * Overloaded print functions
     */
    virtual std::string printDefinition(PrintType) override;
    virtual std::string printOutputEnable(PrintType, uint32_t) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
};

class PhiSelectNode : public InstructionNode {
   private:
    SuperNode *mask_node;

   public:
    PhiSelectNode(NodeInfo _ni, llvm::PHINode *_ins = nullptr,
                  SuperNode *_parent = nullptr)
        : InstructionNode(_ni, InstType::PhiInstructionTy, _ins) {}

    SuperNode *getMaskNode() const { return mask_node; }

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::PhiInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    void setParentNode(SuperNode *_parent) { this->mask_node = _parent; }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printMaskInput(PrintType);
};

class AllocaNode : public InstructionNode {
   public:
    AllocaNode(NodeInfo _ni, llvm::AllocaInst *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::AllocaInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::AllocaInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
};

class GEPNode : public InstructionNode {
   private:
    std::vector<uint32_t> num_byte;

   public:
    GEPNode(NodeInfo _ni, llvm::GetElementPtrInst *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::GetElementPtrInstTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::GetElementPtrInstTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    void addNumByte(uint32_t _byte) { num_byte.push_back(_byte); }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
};

class LoadNode : public InstructionNode {
   private:
    MemoryNode *mem_unit;

   public:
    LoadNode(NodeInfo _ni, llvm::LoadInst *_ins = nullptr,
             MemoryNode *_node = nullptr)
        : InstructionNode(_ni, InstructionNode::LoadInstructionTy, _ins),
          mem_unit(_node) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::LoadInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    void setMemoryUnit(MemoryNode *_node) { mem_unit = _node; }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printMemReadInput(PrintType, uint32_t) override;
    virtual std::string printMemReadOutput(PrintType, uint32_t) override;
};

class StoreNode : public InstructionNode {
   private:
    MemoryNode *mem_node;

   public:
    StoreNode(NodeInfo _ni, llvm::StoreInst *_ins = nullptr,
              MemoryNode *_mem = nullptr)
        // NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::StoreInstructionTy, _ins),
          mem_node(_mem) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::StoreInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printInputData(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printMemWriteInput(PrintType, uint32_t) override;
    virtual std::string printMemWriteOutput(PrintType, uint32_t) override;
};

class ReturnNode : public InstructionNode {
   public:
    ReturnNode(NodeInfo _ni, llvm::ReturnInst *_ins = nullptr,
               NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::ReturnInstrunctionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::ReturnInstrunctionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
};

class CallNode : public InstructionNode {
   public:
    CallNode(NodeInfo _ni, llvm::CallInst *_ins = nullptr,
             NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::CallInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::CallInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
};

class GlobalValueNode : public Node {
   private:
    llvm::GlobalValue *parent_glob;

   public:
    GlobalValueNode(NodeInfo _ni, llvm::GlobalValue *_glb = nullptr)
        : Node(Node::GlobalValueTy, _ni), parent_glob(_glb) {}

    llvm::GlobalValue *getGlobalValue();
};

class ConstIntNode : public Node {
   private:
    llvm::ConstantInt *parent_const_int;

   public:
    ConstIntNode(NodeInfo _ni, llvm::ConstantInt *_cint = nullptr)
        : Node(Node::ConstIntTy, _ni), parent_const_int(_cint) {}

    llvm::ConstantInt *getConstantParent();
    virtual std::string printOutputData(PrintType, uint32_t);
};

/**
 * SplitCall node
 */
class SplitCallNode : public ContainerNode {
   public:
    explicit SplitCallNode(NodeInfo _nf)
        : ContainerNode(_nf, ContainerNode::SplitCallTy) {}

    static bool classof(const ContainerNode *T) {
        return T->getContainerType() == ContainerNode::SplitCallTy;
    }
    static bool classof(const Node *T) {
        return isa<ContainerNode>(T) && classof(cast<ContainerNode>(T));
    }

    ArgumentNode *insertLiveOutArgument(llvm::Value &) = delete;
    uint32_t findLiveOutIndex(ArgumentNode *) = delete;

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printOutputEnable(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
};

}  // namespace dandelion

#endif  // end of DANDDELION_NODE_H
