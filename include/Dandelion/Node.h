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

namespace dandelion {

class Graph;
class Node;
class SuperNode;
class ContainerNode;
class InstructionNode;
class LoopNode;
class MemoryNode;
class StackNode;
class PhiSelectNode;
class SelectNode;
class SplitCallNode;
class ArgumentNode;
class CallNode;
class CallInNode;
class CallOutNode;
class ConstIntNode;
class ConstFPNode;

enum PrintType { Scala = 0, Dot, Json };
struct PortID {
    uint32_t ID;

    // Default value for ID is equalt to zero
    PortID() : ID(0) {}
    PortID(uint32_t _id) : ID(_id) {}

    uint32_t getID() { return ID; }
    uint32_t setID(uint32_t _id) { ID = _id; }

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

    std::list<PortEntry>::iterator findDataInputNode(Node *);
    std::list<PortEntry>::iterator findDataOutputNode(Node *);
    std::list<PortEntry>::iterator findControlInputNode(Node *);
    std::list<PortEntry>::iterator findControlOutputNode(Node *);

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
        port_control.control_input_port.push_back(std::make_pair(_n, _id));

        // if (port_control.control_input_port.size() == _id)
        // port_control.control_input_port.push_back(_n);
        // else if (port_control.control_input_port.size() < _id) {
        // port_control.control_input_port.resize(_id);
        // port_control.control_input_port.push_back(_n);
        //}

        // auto it = port_control.control_input_port.begin();
        // std::advance(it, _id);
        // std::replace(port_control.control_input_port.begin(),
        // port_control.control_input_port.end(), *it, _n);
    }

    /**
     * Adding a node to a specific index of control output port
     */
    void addControlOutputPortIndex(Node *_n, uint32_t _id) {
        port_control.control_output_port.push_back(std::make_pair(_n, _id));
        // if (port_control.control_output_port.size() == _id)
        // port_control.control_output_port.push_back(_n);
        // else if (port_control.control_output_port.size() < _id) {
        // port_control.control_output_port.resize(_id);
        // port_control.control_output_port.push_back(_n);
        //}

        // auto it = port_control.control_output_port.begin();
        // std::advance(it, _id);
        // std::replace(port_control.control_output_port.begin(),
        // port_control.control_output_port.end(), *it, _n);
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
    using ConstIntNodeList = std::list<ConstIntNode *>;
    using CosntFPNodeList = std::list<ConstFPNode *>;
    enum SuperNodeType { Mask, NoMask, LoopHead };

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
    virtual std::string printOutputEnable(PrintType, uint32_t) override;
    virtual std::string printMaskOutput(PrintType, uint32_t);
    std::string printActivateEnable(PrintType);
};

class ArgumentNode : public Node {
   public:
    enum ArgumentType { LiveIn = 0, LiveOut, FunctionArgument };

   private:
    ArgumentType arg_type;
    ContainerNode *parent_call_node;
    llvm::Value *parent_argument;

   public:
    explicit ArgumentNode(NodeInfo _ni, ArgumentType _arg_type,
                          ContainerNode *_call_node = nullptr,
                          llvm::Value *_arg = nullptr)
        : Node(Node::FunctionArgTy, _ni),
          arg_type(_arg_type),
          parent_call_node(_call_node),
          parent_argument(_arg) {}

    const llvm::Value *getArgumentValue() { return parent_argument; }

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::FunctionArgTy;
    }

    auto getArgType() { return arg_type; }

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

    auto live_in_begin() { return this->live_in.begin(); }
    auto live_in_end() { return this->live_in.end(); }
    auto live_ins() {
        return helpers::make_range(live_in_begin(), live_in_end());
    }

    auto live_out_begin() { return this->live_out.begin(); }
    auto live_out_end() { return this->live_out.end(); }
    auto live_outs() {
        return helpers::make_range(live_out_begin(), live_out_end());
    }

    Node *findLiveIn(llvm::Value *);
    Node *findLiveOut(llvm::Value *);
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

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printMemReadInput(PrintType, uint32_t) override;
    virtual std::string printMemReadOutput(PrintType, uint32_t) override;
    virtual std::string printMemWriteInput(PrintType, uint32_t) override;
    virtual std::string printMemWriteOutput(PrintType, uint32_t) override;
};

/**
 * Memory unit works as a local memory for each graph
 */
class StackNode : public Node {
   public:
    explicit StackNode(NodeInfo _nf) : Node(Node::StackUnitTy, _nf) {}

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
        return T->getType() == Node::StackUnitTy;
    }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printMemReadInput(PrintType, uint32_t) override;
    virtual std::string printMemReadOutput(PrintType, uint32_t) override;
    // virtual std::string printMemWriteInput(PrintType, uint32_t) override;
    // virtual std::string printMemWriteOutput(PrintType, uint32_t) override;
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
   private:
    enum PortType { Active = 0, Enable, EndEnable, LatchEnable, LoopExit };

    std::list<std::pair<Node *, PortType>> port_type;
    LoopNode *parent_loop;
    std::list<InstructionNode *> instruction_list;
    std::list<SuperNode *> basic_block_list;
    std::list<InstructionNode *> ending_instructions;

    SuperNode *head_node;
    SuperNode *latch_node;
    std::list<SuperNode *> exit_node;

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
    static bool classof(const Node *T) {
        return T->getType() == Node::LoopNodeTy;
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

    /**
     * Make sure that loop enable signal is always set to index 0
     */
    void setEnableLoopSignal(Node *_n) {
        addControlInputPort(_n);
        port_type.push_back(std::make_pair(_n, PortType::Enable));
    }

    /**
     * Make sure the loop enable signal is always set to index 0
     */
    void setActiveOutputLoopSignal(Node *_n) {
        addControlOutputPort(_n);
        port_type.push_back(std::make_pair(_n, PortType::Active));
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
    void setLoopLatchEnable(Node *_n) {
        addControlInputPort(_n);
        port_type.push_back(std::make_pair(_n, PortType::LatchEnable));
    }

    /**
     * Make sure that loop end enable signal is always fix to index 1
     */
    void setLoopEndEnable(Node *_n) {
        addControlOutputPort(_n);
        port_type.push_back(std::make_pair(_n, PortType::EndEnable));
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
   public:
    FdiveOperatorNode(NodeInfo _ni, llvm::Instruction *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::FdiveInstructionTy, _ins) {}

    // Overloading isa<>, dyn_cast from llvm
    static bool classof(const InstructionNode *I) {
        return I->getOpCode() == InstType::FdiveInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

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
   public:
    FcmpNode(NodeInfo _ni, llvm::FCmpInst *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::FcmpInstructionTy, _ins) {}

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
   public:
    enum PredicateResult { True = 0, False };
    using PrintedNode = std::pair<Node *, PredicateResult>;

    list<std::pair<Node *, PredicateResult>> output_predicate;
    BranchNode(NodeInfo _ni, llvm::BranchInst *_ins = nullptr)
        : InstructionNode(_ni, InstType::BranchInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::BranchInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

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
   private:
    uint32_t size;
    uint32_t num_byte;
    uint32_t route_id;

   public:
    AllocaNode(NodeInfo _ni, llvm::AllocaInst *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::AllocaInstructionTy, _ins),
          size(1),
          num_byte(0) {}
    AllocaNode(NodeInfo _ni, uint32_t _num_byte, uint32_t _size = 1,
               uint32_t rid = 0, llvm::AllocaInst *_ins = nullptr)
        : InstructionNode(_ni, InstructionNode::AllocaInstructionTy, _ins),
          size(_size),
          num_byte(_num_byte),
          route_id(rid) {}

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

    auto getRouteID() { return route_id; }
    void setRouteID(uint32_t _id) { route_id = _id; }

    virtual std::string printDefinition(PrintType) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
    virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    virtual std::string printMemReadInput(PrintType, uint32_t) override;
    virtual std::string printMemReadOutput(PrintType, uint32_t) override;

    std::string printOffset(PrintType);
};

// class GepArrayNode : public InstructionNode {
// private:
// GepArrayInfo gep_info;

// public:
// explicit GepArrayNode(NodeInfo _ni, llvm::GetElementPtrInst *_ins = nullptr)
//: InstructionNode(_ni, InstructionNode::GetElementPtrArrayInstTy,
//_ins) {}
// explicit GepArrayNode(NodeInfo _ni, GepArrayInfo _info,
// llvm::GetElementPtrInst *_ins = nullptr)
//: InstructionNode(_ni, InstructionNode::GetElementPtrArrayInstTy, _ins),
// gep_info(_info) {}

// static bool classof(const InstructionNode *T) {
// return T->getOpCode() == InstructionNode::GetElementPtrArrayInstTy;
//}
// static bool classof(const Node *T) {
// return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
//}

// virtual std::string printDefinition(PrintType) override;
// virtual std::string printInputEnable(PrintType) override;
// virtual std::string printInputEnable(PrintType, uint32_t) override;
// virtual std::string printInputData(PrintType, uint32_t) override;
// virtual std::string printOutputData(PrintType, uint32_t) override;
//};

// class GepStructNode : public InstructionNode {
// private:
// GepStructInfo gep_info;

// public:
// explicit GepStructNode(NodeInfo _ni,
// llvm::GetElementPtrInst *_ins = nullptr)
//: InstructionNode(_ni, InstructionNode::GetElementPtrArrayInstTy,
//_ins) {}
// explicit GepStructNode(NodeInfo _ni, GepStructInfo _info,
// llvm::GetElementPtrInst *_ins = nullptr)
//: InstructionNode(_ni, InstructionNode::GetElementPtrArrayInstTy, _ins),
// gep_info(_info) {}

// static bool classof(const InstructionNode *T) {
// return T->getOpCode() == InstructionNode::GetElementPtrArrayInstTy;
//}
// static bool classof(const Node *T) {
// return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
//}

// virtual std::string printDefinition(PrintType) override;
// virtual std::string printInputEnable(PrintType) override;
// virtual std::string printInputEnable(PrintType, uint32_t) override;
// virtual std::string printInputData(PrintType, uint32_t) override;
// virtual std::string printOutputData(PrintType, uint32_t) override;
//};

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
        value.f = parent_const_fp->getValueAPF().convertToDouble();
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
    // virtual std::string printInputEnable(PrintType) override;
    // virtual std::string printOutputData(PrintType, uint32_t) override;
    // virtual std::string printOutputData(PrintType) override;
    // virtual std::string printInputData(PrintType, uint32_t) override;
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
    // virtual std::string printOutputEnable(PrintType, uint32_t) override;
    // virtual std::string printInputEnable(PrintType) override;
    // virtual std::string printInputEnable(PrintType, uint32_t) override;
    virtual std::string printInputEnable(PrintType) override;
    virtual std::string printOutputData(PrintType, uint32_t) override;
    // virtual std::string printOutputData(PrintType) override;
    virtual std::string printInputData(PrintType, uint32_t) override;
};

}  // namespace dandelion

#endif  // end of DANDDELION_NODE_H
