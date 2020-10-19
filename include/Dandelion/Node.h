#ifndef DANDELION_NODE_H
#define DANDELION_NODE_H
#include <stdint.h>
#include <list>
#include <map>

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

#include "Common.h"
#include "iterator_range.h"

#define XLEN 32
#define LOOPCONTROL 2

using namespace llvm;
using common::FloatingPointIEEE754;
using common::GepInfo;

extern cl::opt<char> HWoptLevel;

namespace dandelion {

class Graph;
class Node;
class SuperNode;
class ContainerNode;
class InstructionNode;
class LoopNode;
class MemoryNode;
class ScratchpadNode;
class PhiSelectNode;
class SelectNode;
class SplitCallNode;
class ArgumentNode;
class CallNode;
class CallInNode;
class CallOutNode;
class ConstIntNode;
class ConstFPNode;
class AllocaNode;

enum PrintType { Scala = 0, Dot, Json };
struct PortID {
    uint32_t ID;

    // Default value for ID is equalt to zero
    PortID() : ID(0) {}
    PortID(uint32_t _id) : ID(_id) {}

    uint32_t getID() { return ID; }
    void setID(uint32_t _id) { ID = _id; }

    bool operator==(const PortID &rhs) const { return this->ID == rhs.ID; }
};

using PortEntry = std::pair<Node *, PortID>;

struct DataPort {
    std::list<PortEntry> data_input_port;
    std::list<PortEntry> data_output_port;
};

struct ControlPort {
    std::list<PortEntry> control_input_port;
    std::list<PortEntry> control_output_port;
};

struct MemoryPort {
    std::list<PortEntry> memory_req_port;
    std::list<PortEntry> memory_resp_port;
};

struct NodeInfo {
    uint32_t ID;
    std::string Name;

    NodeInfo(uint32_t _id, std::string _n) : ID(_id), Name(_n){};
    NodeInfo(std::string _n, uint32_t _id) : ID(_id), Name(_n){};
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
        ConstFPTy,
        MemoryUnitTy,
        StackUnitTy,
        FloatingPointTy,
        ContainerTy,
        UnkonwTy

    };

    enum DataType { IntegerType = 0, FloatType, PointerType, UknownType };

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
    Node(NodeType _nt, NodeInfo _ni) : node_type(_nt), info(_ni) {}

    NodeInfo getInfo() { return info; }

    PortID returnDataInputPortIndex(Node *);
    PortID returnControlInputPortIndex(Node *);
    PortID returnMemoryReadInputPortIndex(Node *);
    PortID returnMemoryWriteInputPortIndex(Node *);

    PortID returnDataOutputPortIndex(Node *);
    PortID returnControlOutputPortIndex(Node *);
    PortID returnMemoryReadOutputPortIndex(Node *);
    PortID returnMemoryWriteOutputPortIndex(Node *);

    // Node *returnDataOutputPortNode(uint32_t index);
    Node *returnControlOutputPortNode(uint32_t index);

    virtual PortID addDataInputPort(Node *node);
    virtual PortID addDataOutputPort(Node *node);

    virtual PortID addDataInputPort(Node *node, uint32_t id);
    virtual PortID addDataOutputPort(Node *node, uint32_t id);

    PortID addControlInputPort(Node *node);
    PortID addControlOutputPort(Node *node);

    PortID addControlInputPort(Node *node, uint32_t id);
    PortID addControlOutputPort(Node *node, uint32_t id);

    bool existControlInput(Node *node);
    bool existControlOutput(Node *node);

    bool existDataInput(Node *);
    bool existDataOutput(Node *);

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

    uint32_t numReadMemReqPort() {
        return read_port_data.memory_req_port.size();
    }
    uint32_t numReadMemRespPort() {
        return read_port_data.memory_resp_port.size();
    }

    uint32_t numWriteMemReqPort() {
        return write_port_data.memory_req_port.size();
    }
    uint32_t numWriteMemRespPort() {
        return write_port_data.memory_resp_port.size();
    }

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

    // If there are multiple of the same node added to the list, this function
    // only returns the first PortEntry
    std::list<PortEntry>::iterator findDataInputNode(Node *);
    std::list<PortEntry>::iterator findDataOutputNode(Node *);
    std::list<PortEntry>::iterator findControlInputNode(Node *);
    std::list<PortEntry>::iterator findControlOutputNode(Node *);

    // If there are multiple of the same node added to the list, this function
    // only returns a list of PortEntry
    std::list<PortEntry> findDataInputNodeList(Node *);
    std::list<PortEntry> findDataOutputNodeList(Node *);
    std::list<PortEntry> findControlInputNodeList(Node *);
    std::list<PortEntry> findControlOutputNodeList(Node *);

    void removeNodeDataInputNode(Node *);
    void removeNodeDataOutputNode(Node *);
    void removeNodeControlInputNode(Node *);
    void removeNodeControlOutputNode(Node *);

    /// replace two nodes form the control input container
    virtual void replaceControlInputNode(Node *src, Node *tar);

    /// replace two nodes form the control output container
    virtual void replaceControlOutputNode(Node *src, Node *tar);

    /// replace two nodes form the control input container
    virtual void replaceDataInputNode(Node *src, Node *tar);

    /// replace two nodes form the control output container
    virtual void replaceDataOutputNode(Node *src, Node *tar);

    // Iterator over input data edges
    auto inputDataport_begin() {
        return this->port_data.data_input_port.begin();
    }
    auto inputDataport_end() { return this->port_data.data_input_port.end(); }

    auto input_data_range() {
        return helpers::make_range(inputDataport_begin(), inputDataport_end());
    }

    // Iterator over output data edges
    auto outputDataport_begin() {
        return this->port_data.data_output_port.begin();
    }
    auto outputDataport_end() { return this->port_data.data_output_port.end(); }
    auto output_data_range() {
        return helpers::make_range(outputDataport_begin(),
                                   outputDataport_end());
    }

    // Iterator over input control edges
    auto inputControl_begin() {
        return this->port_control.control_input_port.begin();
    }
    auto inputControl_end() {
        return this->port_control.control_input_port.end();
    }
    auto input_control_range() {
        return helpers::make_range(inputControl_begin(), inputControl_end());
    }

    // Iterator over output control edges
    auto outputControl_begin() {
        return this->port_control.control_output_port.begin();
    }
    auto outputControl_end() {
        return this->port_control.control_output_port.end();
    }
    auto output_control_range() {
        return helpers::make_range(outputControl_begin(), outputControl_end());
    }

    // Iterator over output control edges
    auto read_req_begin() {
        return this->read_port_data.memory_req_port.begin();
    }

    auto read_req_end() { return this->read_port_data.memory_req_port.end(); }
    auto read_req_range() {
        return helpers::make_range(read_req_begin(), read_req_end());
    }

    // Iterator over output control edges
    auto write_req_begin() {
        return this->write_port_data.memory_req_port.begin();
    }

    auto write_req_end() { return this->write_port_data.memory_req_port.end(); }
    auto write_req_range() {
        return helpers::make_range(write_req_begin(), write_req_end());
    }

    // Iterator over output control edges
    auto read_resp_begin() {
        return this->read_port_data.memory_resp_port.begin();
    }

    auto read_resp_end() { return this->read_port_data.memory_resp_port.end(); }
    auto read_resp_range() {
        return helpers::make_range(read_resp_begin(), read_resp_end());
    }

    // Iterator over output control edges
    auto write_resp_begin() {
        return this->write_port_data.memory_resp_port.begin();
    }

    auto write_resp_end() {
        return this->write_port_data.memory_resp_port.end();
    }
    auto write_resp_range() {
        return helpers::make_range(write_resp_begin(), write_resp_end());
    }

    uint32_t getID() { return info.ID; }
    std::string getName() { return info.Name; }

    // TODO how to define virtual functions?
    // virtual void printInitilization() {}

    uint32_t getType() const { return node_type; }

    // protected:
    /**
     * Adding a node to a specific index of control input port
     */
    void addControlInputPortIndex(Node *_n, uint32_t _id) {
        port_control.control_input_port.push_back(std::make_pair(_n, _id));
    }

    /**
     * Adding a node to a specific index of control output port
     */
    void addControlOutputPortIndex(Node *_n, uint32_t _id) {
        port_control.control_output_port.push_back(std::make_pair(_n, _id));
    }

   public:
    virtual std::string printDefinition(PrintType) {
        return this->info.Name + std::string(" Definition is Not defined!");
    }

    virtual std::string printInputEnable(PrintType, uint32_t) {
        return this->info.Name +
               std::string(" EnableInput with ID Not defined!");
    }
    virtual std::string printInputEnable(PrintType, std::pair<Node *, PortID>) {
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
    virtual std::string printOutputEnable(PrintType, PortEntry) {
        return this->info.Name +
               std::string(" EnableInput with ID Not defined!");
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

    void dump() { outs() << info.Name << "\n"; }
};

/**
 * Super node is actual implimetation of our basic blocks
 */
class SuperNode : public Node {
   public:
    // List of the instructions
    using PhiNodeList = std::list<PhiSelectNode *>;
    using ConstIntNodeList = std::list<ConstIntNode *>;
    using CosntFPNodeList = std::list<ConstFPNode *>;
    enum SuperNodeType { Mask, NoMask };

   private:
    llvm::BasicBlock *basic_block;

    std::list<InstructionNode *> instruction_list;
    PhiNodeList phi_list;
    ConstIntNodeList const_int_list;
    CosntFPNodeList const_fp_list;

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
    void addconstIntNode(ConstIntNode *);
    void addconstFPNode(ConstFPNode *);

    bool hasPhi() { return !phi_list.empty(); }
    uint32_t getNumPhi() const { return phi_list.size(); }

    auto phi_begin() { return this->phi_list.begin(); }
    auto phi_end() { return this->phi_list.end(); }
    auto phis() { return helpers::make_range(phi_begin(), phi_end()); }

    auto ins_begin() const { return this->instruction_list.begin(); }
    auto ins_end() const { return this->instruction_list.end(); }
    auto instructions() { return helpers::make_range(ins_begin(), ins_end()); }

    auto const_int_begin() const { return this->const_int_list.begin(); }
    auto const_int_end() const { return this->const_int_list.end(); }
    auto cints() {
        return helpers::make_range(const_int_begin(), const_int_end());
    }

    auto const_fp_begin() const { return this->const_fp_list.begin(); }
    auto const_fp_end() const { return this->const_fp_list.end(); }
    auto cfps() {
        return helpers::make_range(const_fp_begin(), const_fp_end());
    }

    const SuperNodeType getNodeType() { return type; }
    void setNodeType(SuperNodeType _t) { this->type = _t; }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printInputEnable(PrintType,
                                         std::pair<Node *, PortID>) override;
    virtual std::string printOutputEnable(PrintType, uint32_t) override;
    virtual std::string printOutputEnable(PrintType,
                                          std::pair<Node *, PortID>) override;
    virtual std::string printMaskOutput(PrintType, uint32_t);
    std::string printActivateEnable(PrintType);
};

class ArgumentNode : public Node {
   public:
    enum ArgumentType {
        LiveIn = 0,
        LiveOut,
        LoopLiveIn,
        LoopLiveOut,
        CarryDependency
    };

   private:
    ArgumentType arg_type;
    DataType data_type;
    ContainerNode *parent_call_node;
    llvm::Value *parent_argument;

   public:
    explicit ArgumentNode(NodeInfo _ni, ArgumentType _arg_type,
                          DataType _d_type, ContainerNode *_call_node = nullptr,
                          llvm::Value *_arg = nullptr)
        : Node(Node::FunctionArgTy, _ni),
          arg_type(_arg_type),
          data_type(_d_type),
          parent_call_node(_call_node),
          parent_argument(_arg) {}

    const llvm::Value *getArgumentValue() { return parent_argument; }

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::FunctionArgTy;
    }

    auto getArgType() { return arg_type; }
    auto getDataArgType() { return data_type; }

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
    using RegisterList = std::list<std::shared_ptr<ArgumentNode>>;

   protected:
    ContainType con_type;
    // ptrs and vals are fore split call
    RegisterList live_in_ptrs;
    RegisterList live_in_vals;

    // live_in is for loop nodes
    RegisterList live_in;

    RegisterList live_out;
    RegisterList carry_depen;

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

    ArgumentNode *insertLiveInArgument(llvm::Value *Value,
                                       ArgumentNode::ArgumentType Type);
    ArgumentNode *insertLiveOutArgument(llvm::Value *Value,
                                        ArgumentNode::ArgumentType Type);
    ArgumentNode *insertCarryDepenArgument(llvm::Value *Value,
                                           ArgumentNode::ArgumentType Type);

    Node *findLiveInNode(llvm::Value *_val);
    Node *findLiveOutNode(llvm::Value *_val);
    Node *findCarryDepenNode(llvm::Value *_val);

    uint32_t findLiveInArgumentIndex(ArgumentNode *);
    uint32_t findLiveOutArgumentIndex(ArgumentNode *);
    uint32_t findCarryDepenArgumentIndex(ArgumentNode *);

    uint32_t numLiveInArgList(ArgumentNode::ArgumentType type,
                              ArgumentNode::DataType dtype);
    uint32_t numLiveOutArgList(ArgumentNode::ArgumentType type);
    uint32_t numCarryDepenArgList(ArgumentNode::ArgumentType type);

    auto live_in_begin() { return this->live_in.begin(); }
    auto live_in_end() { return this->live_in.end(); }

    auto live_in_ptrs_begin() { return this->live_in_ptrs.begin(); }
    auto live_in_ptrs_end() { return this->live_in_ptrs.end(); }

    auto live_in_vals_begin() { return this->live_in_vals.begin(); }
    auto live_in_vals_end() { return this->live_in_vals.end(); }

    auto live_in_lists() {
        return helpers::make_range(live_in_begin(), live_in_end());
    }

    auto live_in_ptrs_lists() {
        return helpers::make_range(live_in_ptrs_begin(), live_in_ptrs_end());
    }

    auto live_in_vals_lists() {
        return helpers::make_range(live_in_vals_begin(), live_in_vals_end());
    }

    auto live_out_begin() { return this->live_out.begin(); }
    auto live_out_end() { return this->live_out.end(); }
    auto live_out_lists() {
        return helpers::make_range(live_out_begin(), live_out_end());
    }

    auto carry_depen_begin() { return this->carry_depen.begin(); }
    auto carry_depen_end() { return this->carry_depen.end(); }
    auto carry_depen_lists() {
        return helpers::make_range(carry_depen_begin(), carry_depen_end());
    }

    // Node *findArg(llvm::Value *);
};

/**
 * Memory unit works as a local memory for each graph
 */
class MemoryNode : public Node {
   public:
    explicit MemoryNode(NodeInfo _nf) : Node(Node::MemoryUnitTy, _nf) {}

    // Restrict access to data input ports
    virtual PortID addDataInputPort(Node *) override {
        assert(!"You are not supposed to call this function!");
        return PortID();
    }
    virtual PortID addDataOutputPort(Node *) override {
        assert(!"You are not supposed to call this function!");
        return PortID();
    };
    uint32_t numDataInputPort() = delete;
    uint32_t numDataOutputPort() = delete;

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::MemoryUnitTy;
    }

    bool isInitilized() {
        // return true;
        return (this->numReadMemReqPort() || this->numWriteMemReqPort());
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printMemReadInput(PrintType, uint32_t) override;
    virtual std::string printMemReadOutput(PrintType, uint32_t) override;
    virtual std::string printMemWriteInput(PrintType, uint32_t) override;
    virtual std::string printMemWriteOutput(PrintType, uint32_t) override;
    std::string printUninitilizedUnit(PrintType);
};

/**
 * Memory unit works as a local memory for each graph
 */
class ScratchpadNode : public Node {
   public:
    AllocaNode *alloca_node;
    uint32_t size;
    uint32_t num_byte;

    explicit ScratchpadNode(NodeInfo _nf, AllocaNode *alloca, uint32_t mem_size,
                            uint32_t mem_byte)
        : Node(Node::StackUnitTy, _nf),
          alloca_node(alloca),
          size(mem_size),
          num_byte(mem_byte) {}

    // Restrict access to data input ports
    virtual PortID addDataInputPort(Node *) override {
        assert(!"You are not supposed to call this function!");
        return PortID();
    }
    virtual PortID addDataOutputPort(Node *) override {
        assert(!"You are not supposed to call this function!");
        return PortID();
    };

    AllocaNode *getAllocaNode() { return alloca_node; }

    uint32_t getMemSize() { return size; }
    uint32_t getMemByte() { return num_byte; }

    uint32_t numDataInputPort() = delete;
    uint32_t numDataOutputPort() = delete;

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::StackUnitTy;
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printMemReadInput(PrintType, uint32_t) override;
    virtual std::string printMemReadOutput(PrintType, uint32_t) override;
    virtual std::string printMemWriteInput(PrintType, uint32_t) override;
    virtual std::string printMemWriteOutput(PrintType, uint32_t) override;
};

/**
 * Memory unit works as a local memory for each graph
 */
class FloatingPointNode : public Node {
   public:
    explicit FloatingPointNode(NodeInfo _nf) : Node(Node::StackUnitTy, _nf) {}

    // Restrict access to data input ports
    virtual PortID addDataInputPort(Node *) override {
        assert(!"You are not supposed to call this function!");
        return PortID();
    }
    virtual PortID addDataOutputPort(Node *) override {
        assert(!"You are not supposed to call this function!");
        return PortID();
    };
    uint32_t numDataInputPort() = delete;
    uint32_t numDataOutputPort() = delete;

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::FloatingPointTy;
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printMemReadInput(PrintType, uint32_t) override;
    virtual std::string printMemReadOutput(PrintType, uint32_t) override;
};

/**
 * LoopNode contains all the instructions and useful information about the loops
 */
class LoopNode : public ContainerNode {
   public:
    enum PortType {
        Active_Loop_Start = 0,
        Active_Loop_Back,
        Enable,
        LoopBack,
        LoopFinish,
        LoopExit
    };

    using PortList = std::list<std::pair<Node *, PortType>>;

   private:
    std::list<std::pair<Node *, PortType>> port_type;
    LoopNode *parent_loop;
    std::list<InstructionNode *> instruction_list;
    InstructionNode * induction_variable;
    std::list<SuperNode *> basic_block_list;
    std::list<InstructionNode *> ending_instructions;

    SuperNode *head_node;
    SuperNode *latch_node;
    std::list<SuperNode *> exit_node;

    // Set auxiliary information for ouput nodes
    PortEntry activate_loop_start;
    PortEntry activate_loop_back;
    std::vector<PortEntry> loop_exits;

    bool outer_loop;

    // Restrict the access to these two functions
    using Node::addControlInputPort;
    using Node::addControlOutputPort;

   public:
    explicit LoopNode(NodeInfo _nf)
        : ContainerNode(_nf, ContainerNode::LoopNodeTy),
          parent_loop(nullptr),
          head_node(nullptr),
          latch_node(nullptr),
          exit_node(std::list<SuperNode *>()),
          outer_loop(false) {
        // Set the size of control input prot to at least two
        // resizeControlInputPort(LOOPCONTROL);
        // resizeControlOutputPort(LOOPCONTROL);
    }

    explicit LoopNode(NodeInfo _nf, LoopNode *_p_l, SuperNode *_hnode,
                      SuperNode *_lnode)
        : ContainerNode(_nf, ContainerNode::LoopNodeTy),
          parent_loop(_p_l),
          head_node(_hnode),
          latch_node(_lnode),
          outer_loop(false) {
        // Set the size of control input prot to at least two
        // resizeControlInputPort(LOOPCONTROL);
        // resizeControlOutputPort(LOOPCONTROL);
    }
    explicit LoopNode(NodeInfo _nf, SuperNode *_hnode, SuperNode *_lnode,
                      std::list<SuperNode *> _ex)
        : ContainerNode(_nf, ContainerNode::LoopNodeTy),
          parent_loop(nullptr),
          head_node(_hnode),
          latch_node(_lnode),
          exit_node(_ex),
          outer_loop(false) {
        // Set the size of control input prot to at least two
        // resizeControlInputPort(LOOPCONTROL);
        // resizeControlOutputPort(LOOPCONTROL);
    }

    auto getParentLoopNode() { return parent_loop; }
    void setOuterLoop() { outer_loop = true; }
    bool isOuterLoop() { return outer_loop; }

    // Define classof function so that we can use dyn_cast function
    // static bool classof(const Node *T) {
    // return T->getType() == Node::LoopNodeTy;
    //}

    static bool classof(const ContainerNode *I) {
        return I->getContainerType() == ContainerNode::LoopNodeTy;
    }

    static bool classof(const Node *T) {
        return isa<ContainerNode>(T) && classof(cast<ContainerNode>(T));
    }

    // Iterator over instucrion list
    auto ins_begin() { return instruction_list.begin(); }
    auto ins_end() { return instruction_list.end(); }
    auto instructions() { return helpers::make_range(ins_begin(), ins_end()); }

    // Iterator over basic block list
    auto bb_begin() { return basic_block_list.begin(); }
    auto bb_end() { return basic_block_list.end(); }
    auto bblocks() { return helpers::make_range(bb_begin(), bb_end()); }

    // Iterator over ending instructions
    auto ending_begin() { return ending_instructions.begin(); }
    auto ending_end() { return ending_instructions.end(); }
    auto endings() { return helpers::make_range(ending_begin(), ending_end()); }

    // Iterator over input edges

    void setHeadNode(SuperNode *_n) { head_node = _n; }
    void setLatchNode(SuperNode *_n) { latch_node = _n; }

    void setIndeuctionVariable(InstructionNode *I) { this->induction_variable = I; }
    /**
     * Make sure that loop enable signal is always set to index 0
     */
    void setInputControlLoopSignal(Node *_n) {
        addControlInputPort(_n);
        port_type.push_back(std::make_pair(_n, PortType::Enable));
    }

    /**
     * Make sure the loop enable signal is always set to index 0
     */
    void setActiveOutputLoopSignal(Node *_n) {
        auto _port_info = PortID(this->numControlOutputPort());
        addControlOutputPort(_n);
        port_type.push_back(std::make_pair(_n, PortType::Active_Loop_Start));

        // Seting activate_loop_start
        activate_loop_start = std::make_pair(_n, _port_info);
    }

    void setActiveBackSignal(Node *_n) {
        auto _port_info = PortID(this->numControlOutputPort());
        addControlOutputPort(_n);
        port_type.push_back(std::make_pair(_n, PortType::Active_Loop_Back));

        // Seting activate_loop_start
        activate_loop_back = std::make_pair(_n, _port_info);
    }

    void setActiveExitSignal(Node *_n) {
        auto _port_info = PortID(this->numControlOutputPort());
        addControlOutputPort(_n);
        port_type.push_back(std::make_pair(_n, PortType::LoopExit));

        // Seting activate_loop_start
        loop_exits.push_back(std::make_pair(_n, _port_info));
    }

    /**
     * Push supernode to the super node container
     */
    void pushSuperNode(SuperNode *_n) { basic_block_list.push_back(_n); }

    /**
     * Push instrucitions
     */
    void pushInstructionNode(InstructionNode *_n) {
        instruction_list.push_back(_n);
    }

    /**
     * Make sure that loop latch enable signal is always fix to index 1
     */
    void addLoopBackEdge(Node *_n) {
        addControlInputPort(_n);
        port_type.push_back(std::make_pair(_n, PortType::LoopBack));
    }

    /**
     * Make sure that loop end enable signal is always fix to index 1
     */
    void setLoopEndEnable(Node *_n) {
        addControlOutputPort(_n);
        port_type.push_back(std::make_pair(_n, PortType::LoopFinish));
    }
    // void setLoopEndEnable(Node *_n, uint32_t i) {
    // addControlOutputPortIndex(_n, i);
    //}

    /**
     * Make sure that loop exit points are always starting from index 2
     */
    PortID pushLoopExitLatch(Node *_n) {
        assert(numControlInputPort() > 1 && "Error in loop control signal!");
        port_type.push_back(std::make_pair(_n, PortType::LoopExit));
        return addControlInputPort(_n);
    }

    // TODO the function should move to private section and get calls inside the
    // init fuctions
    void setEndingInstructions();
    std::list<InstructionNode *> findEndingInstructions();

    /**
     * Print functions
     */
    virtual std::string printDefinition(PrintType) override;
    virtual std::string printOutputEnable(PrintType) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printOutputEnable(PrintType, uint32_t) override;
    virtual std::string printOutputEnable(PrintType, PortEntry) override;
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
        GetElementPtrArrayInstTy,
        GetElementPtrStructInstTy,
        GetElementPtrInstTy,
        LoadInstructionTy,
        StoreInstructionTy,
        SextInstructionTy,
        ZextInstructionTy,
        BitCastInstructionTy,
        STIoFPInstructionTy,
        FPToUIInstructionTy,
        TruncInstructionTy,
        SelectInstructionTy,

        // Floating point operations

        FaddInstructionTy,
        FsubInstructionTy,
        FmulInstructionTy,
        FdiveInstructionTy,
        FremInstructionTy,
        FcmpInstructionTy,

#ifdef TAPIR
        DetachInstructionTy,
        ReattachInstructionTy,
        SyncInstructionTy,
#endif
        ReturnInstrunctionTy,
        CallInstructionTy,
        CallInInstructionTy,
        CallOutInstructionTy
    };

   private:
    InstType ins_type;
    DataType data_type;
    llvm::Instruction *parent_instruction;

   public:
    InstructionNode(NodeInfo _ni, InstType _ins_t,
                    llvm::Instruction *_ins = nullptr)
        : Node(Node::InstructionNodeTy, _ni),
          ins_type(_ins_t),
          data_type(UknownType),
          parent_instruction(_ins) {}

    InstructionNode(NodeInfo _ni, InstType _ins_t, DataType _dtype,
                    llvm::Instruction *_ins = nullptr)
        : Node(Node::InstructionNodeTy, _ni),
          ins_type(_ins_t),
          data_type(_dtype),
          parent_instruction(_ins) {}

    llvm::Instruction *getInstruction();

    DataType getDataType() const { return data_type; }
    bool isPointerType() const { return data_type == Node::PointerType; }
    bool isIntegerType() const { return data_type == Node::IntegerType; }
    bool isFloatType() const { return data_type == Node::FloatType; }

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

class FaddOperatorNode : public InstructionNode {
   public:
    FaddOperatorNode(NodeInfo _ni, llvm::Instruction *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::FaddInstructionTy, _ins) {}

    // Overloading isa<>, dyn_cast from llvm
    static bool classof(const InstructionNode *I) {
        return I->getOpCode() == InstType::FaddInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
};

class FdiveOperatorNode : public InstructionNode {
   private:
    uint32_t route_id;

   public:
    FdiveOperatorNode(NodeInfo _ni, llvm::Instruction *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::FdiveInstructionTy, _ins),
          route_id(0) {}

    // Overloading isa<>, dyn_cast from llvm
    static bool classof(const InstructionNode *I) {
        return I->getOpCode() == InstType::FdiveInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    auto getRouteID() { return route_id; }
    void setRouteID(uint32_t _id) { route_id = _id; }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printMemReadInput(PrintType, uint32_t) override;
    virtual std::string printMemReadOutput(PrintType, uint32_t) override;
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

class FcmpNode : public InstructionNode {
    std::map<std::string, std::string> op_codes;

   public:
    FcmpNode(NodeInfo _ni, llvm::FCmpInst *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::FcmpInstructionTy, _ins) {
        op_codes = {{"ogt", ">GT"}, {"olt", "<LT"}, {"oeq", "=EQ"}};
    }

    static bool classof(const InstructionNode *I) {
        return I->getOpCode() == InstType::FcmpInstructionTy;
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
   private:
    bool ending_loop;

   public:
    enum PredicateResult { True = 0, False };
    using PrintedNode = std::pair<Node *, PredicateResult>;

    list<std::pair<Node *, PredicateResult>> output_predicate;

    BranchNode(NodeInfo _ni, llvm::BranchInst *_ins = nullptr)
        : InstructionNode(_ni, InstType::BranchInstructionTy, _ins),
          ending_loop(false) {}

    BranchNode(NodeInfo _ni, bool _loop, llvm::BranchInst *_ins = nullptr)
        : InstructionNode(_ni, InstType::BranchInstructionTy, _ins),
          ending_loop(_loop) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::BranchInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    void setEndingLoopBranch() { ending_loop = true; }
    bool getEndingLoopBranch() { return ending_loop; }

    map<PrintedNode, uint32_t> printed_predicate;
    /**
     * Because each index is fixed for branch node the user
     * can not remove any node from the control port
     * he only can swap
     */
    void removeNodeControlInputNode(Node *) = delete;
    void removeNodeControlOutputNode(Node *) = delete;

    void addTrueBranch(Node *_n) {
        this->output_predicate.push_back(
            std::make_pair(_n, PredicateResult::True));
        this->addControlOutputPort(_n);
    }
    void addFalseBranch(Node *_n) {
        this->output_predicate.push_back(
            std::make_pair(_n, PredicateResult::False));
        this->addControlOutputPort(_n);
    }

    /// replace two nodes form the control output container
    virtual void replaceControlOutputNode(Node *src, Node *tar) override;

    /**
     * Overloaded print functions
     */
    virtual std::string printDefinition(PrintType) override;
    virtual std::string printOutputEnable(PrintType, uint32_t) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
};

class SelectNode : public InstructionNode {
   private:
    SuperNode *mask_node;

   public:
    SelectNode(NodeInfo _ni, llvm::SelectInst *_ins = nullptr,
               SuperNode *_parent = nullptr)
        : InstructionNode(_ni, InstType::SelectInstructionTy, _ins) {}

    SuperNode *getMaskNode() const { return mask_node; }

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::SelectInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    void setParentNode(SuperNode *_parent) { this->mask_node = _parent; }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
};

class PhiSelectNode : public InstructionNode {
   private:
    SuperNode *mask_node;
    bool reverse;

   public:
    PhiSelectNode(NodeInfo _ni, llvm::PHINode *_ins = nullptr,
                  SuperNode *_parent = nullptr)
        : InstructionNode(_ni, InstType::PhiInstructionTy, _ins),
          reverse(false) {}

    PhiSelectNode(NodeInfo _ni, bool _rev, llvm::PHINode *_ins = nullptr,
                  SuperNode *_parent = nullptr)
        : InstructionNode(_ni, InstType::PhiInstructionTy, _ins),
          reverse(_rev) {}

    PhiSelectNode(NodeInfo _ni, DataType _type, bool _rev,
                  llvm::PHINode *_ins = nullptr, SuperNode *_parent = nullptr)
        : InstructionNode(_ni, InstType::PhiInstructionTy, _type, _ins),
          reverse(_rev) {}

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
   private:
    uint32_t size;
    uint32_t num_byte;

   public:
    AllocaNode(NodeInfo _ni, llvm::AllocaInst *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::AllocaInstructionTy, _ins),
          size(1),
          num_byte(0) {}
    AllocaNode(NodeInfo _ni, uint32_t _num_byte, uint32_t _size = 1,
               uint32_t rid = 0, llvm::AllocaInst *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::AllocaInstructionTy, _ins),
          size(_size),
          num_byte(_num_byte) {}

    AllocaNode(NodeInfo _ni, DataType _type, uint32_t _num_byte,
               uint32_t _size = 1, uint32_t rid = 0,
               llvm::AllocaInst *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::AllocaInstructionTy, _type,
                          _ins),
          size(_size),
          num_byte(_num_byte) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::AllocaInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    auto getNumByte() { return num_byte; }
    void setNumByte(uint32_t _n) { num_byte = _n; }

    auto getSize() { return size; }
    void setSize(uint32_t _n) { size = _n; }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printMemReadInput(PrintType, uint32_t) override;
    virtual std::string printMemReadOutput(PrintType, uint32_t) override;

    std::string printOffset(PrintType);
};

class GepNode : public InstructionNode {
   private:
    GepInfo gep_info;

   public:
    explicit GepNode(NodeInfo _ni, llvm::GetElementPtrInst *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::GetElementPtrInstTy, _ins) {}
    explicit GepNode(NodeInfo _ni, common::GepInfo _info,
                     llvm::GetElementPtrInst *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::GetElementPtrInstTy, _ins),
          gep_info(_info) {}

    explicit GepNode(NodeInfo _ni, DataType _type, common::GepInfo _info,
                     llvm::GetElementPtrInst *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::GetElementPtrInstTy, _type,
                          _ins),
          gep_info(_info) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::GetElementPtrInstTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
};

class LoadNode : public InstructionNode {
   private:
    MemoryNode *mem_unit;
    uint32_t route_id;

   public:
    LoadNode(NodeInfo _ni, llvm::LoadInst *_ins = nullptr,
             MemoryNode *_node = nullptr, uint32_t _id = 0)
        : InstructionNode(_ni, InstructionNode::LoadInstructionTy, _ins),
          mem_unit(_node),
          route_id(_id) {}

    LoadNode(NodeInfo _ni, DataType _type, llvm::LoadInst *_ins = nullptr,
             MemoryNode *_node = nullptr, uint32_t _id = 0)
        : InstructionNode(_ni, InstructionNode::LoadInstructionTy, _type, _ins),
          mem_unit(_node),
          route_id(_id) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::LoadInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    void setMemoryUnit(MemoryNode *_node) { mem_unit = _node; }
    void setRouteID(uint32_t _id) { route_id = _id; }
    auto getRouteID() { return route_id; }

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
    uint32_t route_id;
    bool ground;

   public:
    StoreNode(NodeInfo _ni, llvm::StoreInst *_ins = nullptr,
              MemoryNode *_mem = nullptr, uint32_t _id = 0)
        : InstructionNode(_ni, InstructionNode::StoreInstructionTy, _ins),
          mem_node(_mem),
          route_id(_id),
          ground(false) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::StoreInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    void setRouteID(uint32_t _id) { route_id = _id; }
    auto getRouteID() { return route_id; }

    auto isGround() {
        if (this->numDataOutputPort() > 0) unsetGround();
        return ground;
    }
    void setGround() { ground = true; }
    void unsetGround() { ground = false; }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printOutputEnable(PrintType, uint32_t) override;
    virtual std::string printOutputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printMemWriteInput(PrintType, uint32_t) override;
    virtual std::string printMemWriteOutput(PrintType, uint32_t) override;

    std::string printGround(PrintType);
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
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
};

/**
 * For ease of use for now we use callnode as wraper around two other nodes
 * the first node is CallIn and the second one is CallOut
 * The reason behind having wraper node is that we are still maintaining a
 * direct
 * maping from LLVM values to each node and it's hard to have two nodes instead
 * of one node
 * and keep having the direct mapping, in the future we have get ride of the
 * restriction
 */
class CallNode : public InstructionNode {
   private:
    std::unique_ptr<CallInNode> call_in;
    std::unique_ptr<CallOutNode> call_out;

   public:
    CallNode(NodeInfo _ni, llvm::CallInst *_ins = nullptr,
             NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::CallInstructionTy, _ins) {
        call_in = std::make_unique<CallInNode>(
            NodeInfo(_ni.Name + "_in", _ni.ID), this, _ins);
        call_out = std::make_unique<CallOutNode>(
            NodeInfo(_ni.Name + "_out", _ni.ID), this, _ins);
    }

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::CallInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    CallInNode *getCallIn() { return call_in.get(); }
    CallOutNode *getCallOut() { return call_out.get(); }

    // virtual PortID addDataInputPort(Node *_node) override;
    // virtual PortID addDataOutputPort(Node *_node) override;

    void setCallOutEnable(Node *_n);

    // virtual std::string printDefinition(PrintType) override;
};

class CallInNode : public InstructionNode {
   private:
    CallNode *parent_node;
    Graph *parent_graph;

   public:
    CallInNode(NodeInfo _ni, CallNode *_parent, llvm::CallInst *_ins = nullptr,
               NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::CallInInstructionTy, _ins),
          parent_node(_parent) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::CallInInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    void setParent(Graph *_g) { parent_graph = _g; }
    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printOutputEnable(PrintType) override;
    virtual std::string printOutputEnable(PrintType, uint32_t) override;
};

class CallOutNode : public InstructionNode {
   private:
    CallNode *parent_node;
    Graph *parent_graph;

   public:
    CallOutNode(NodeInfo _ni, CallNode *_node, llvm::CallInst *_ins = nullptr,
                NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::CallOutInstructionTy, _ins),
          parent_node(_node) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::CallOutInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
    void setParent(Graph *_g) { parent_graph = _g; }
    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
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
    // uint32_t value;
    int value;

   public:
    // Else part handels Undef cases.
    // We pass nullptr to ConstIntNode when the operand type is
    // undef
    ConstIntNode(NodeInfo _ni, llvm::ConstantInt *_cint = nullptr)
        : Node(Node::ConstIntTy, _ni), parent_const_int(_cint) {
        if (parent_const_int)
            value = parent_const_int->getSExtValue();
        else
            value = 0;
    }

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::ConstIntTy;
    }

    // uint32_t getValue() { return value; }
    int getValue() { return value; }

    llvm::ConstantInt *getConstantParent();
    virtual std::string printDefinition(PrintType) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printInputEnable(PrintType) override;
};

class ConstFPNode : public Node {
   private:
    llvm::ConstantFP *parent_const_fp;
    FloatingPointIEEE754 value;

   public:
    ConstFPNode(NodeInfo _ni, llvm::ConstantFP *_cfp = nullptr)
        : Node(Node::ConstFPTy, _ni), parent_const_fp(_cfp) {
        // TODO Check the actual value of f, right now I only make sure
        // the tool doesn't break in cases that I don't support
        if (parent_const_fp->getValueAPF().isZero())
            value.f = 0;
        else if (parent_const_fp->getValueAPF().isNegative())
            value.f = 0;
        else if (parent_const_fp->getValueAPF().isInteger()) {
            value.f = parent_const_fp->getValueAPF().convertToFloat();
        } else{
            value.f = parent_const_fp->getValueAPF().convertToDouble();
        }
        // value.f = 0;
    }

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::ConstFPTy;
    }

    float getValue() { return value.f; }
    unsigned int getBits() { return value.bits; }
    FloatingPointIEEE754 getFloatIEEE() { return value; }

    llvm::ConstantFP *getConstantParent();
    virtual std::string printDefinition(PrintType) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printInputEnable(PrintType) override;
};

/**
 * SplitCall node
 */
class SplitCallNode : public ContainerNode {
   public:
    enum ArgType { Ptrs = 0, Vals };

   private:
    std::map<ArgumentNode *, ArgType> arg_types;

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

class DetachNode : public InstructionNode {
   public:
    DetachNode(NodeInfo _ni, llvm::DetachInst *_ins = nullptr,
               NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::DetachInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::DetachInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printOutputEnable(PrintType, uint32_t) override;
    virtual std::string printInputEnable(PrintType) override;
    // virtual std::string printInputEnable(PrintType) override;
    // virtual std::string printOutputData(PrintType, uint32_t) override;
    // virtual std::string printOutputData(PrintType) override;
    // virtual std::string printInputData(PrintType, uint32_t) override;
};

/**
 * SextNode
 */
class SextNode : public InstructionNode {
   public:
    SextNode(NodeInfo _ni, llvm::SExtInst *_ins = nullptr,
             NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::SextInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::SextInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
};

/**
 * Zero extension node
 */
class ZextNode : public InstructionNode {
   public:
    ZextNode(NodeInfo _ni, llvm::ZExtInst *_ins = nullptr,
             NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::ZextInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::ZextInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
};

/**
 * truncate node
 */
class TruncNode : public InstructionNode {
   public:
    TruncNode(NodeInfo _ni, llvm::TruncInst *_ins = nullptr,
              NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::TruncInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::TruncInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
};

/**
 * stiofp node
 */
class STIoFPNode : public InstructionNode {
   public:
    STIoFPNode(NodeInfo _ni, llvm::SIToFPInst *_ins = nullptr,
               NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::STIoFPInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::STIoFPInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
};

/**
 * stiofp node
 */
class FPToUINode : public InstructionNode {
   public:
    FPToUINode(NodeInfo _ni, llvm::FPToUIInst *_ins = nullptr,
               NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::FPToUIInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::FPToUIInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
};

class ReattachNode : public InstructionNode {
   private:
    bool ground;

   public:
    ReattachNode(NodeInfo _ni, llvm::ReattachInst *_ins = nullptr,
                 NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::ReattachInstructionTy, _ins),
          ground(false) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::ReattachInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    auto isGround() {
        if (this->numDataOutputPort() > 0) unsetGround();
        return ground;
    }
    void setGround() { ground = true; }
    void unsetGround() { ground = false; }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printOutputEnable(PrintType, uint32_t) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    // virtual std::string printOutputData(PrintType, uint32_t) override;
    // virtual std::string printOutputData(PrintType) override;
    std::string printGround(PrintType);
};

class SyncNode : public InstructionNode {
   public:
    SyncNode(NodeInfo _ni, llvm::SyncInst *_ins = nullptr,
             NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::SyncInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::SyncInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printOutputEnable(PrintType, uint32_t) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
};

class BitcastNode : public InstructionNode {
   public:
    BitcastNode(NodeInfo _ni, llvm::BitCastInst *_ins = nullptr,
                NodeType _nd = UnkonwTy)
        : InstructionNode(_ni, InstructionNode::BitCastInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::BitCastInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
};

}  // namespace dandelion

#endif  // end of DANDDELION_NODE_H
