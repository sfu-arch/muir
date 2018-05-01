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

using namespace llvm;

namespace dandelion {

class Node;
class SuperNode;
class LoopNode;
class MemoryNode;
class InstructionNode;
class PhiSelectNode;

enum PrintType { Scala = 0, Dot, Json };

//enum MemoryMode { Cache = 0, Reg };

struct DataPort {
    std::list<Node *const> data_input_port;
    std::list<Node *const> data_output_port;
};

struct ControlPort {
    std::list<Node *const> control_input_port;
    std::list<Node *const> control_output_port;
};

struct DependencyPort {
    std::list<Node *const> depen_input_port;
    std::list<Node *const> depen_output_port;
};

struct MemoryPort {
    std::list<Node *const> memory_req_port;
    std::list<Node *const> memory_resp_port;
};

struct NodeInfo {
    uint32_t ID;
    std::string Name;

    NodeInfo(uint32_t _id, std::string _n) : ID(_id), Name(_n){};
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
        SplitCallTy,
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

    // List of Dependency port
    DependencyPort port_depen;

   public:  // Public methods
    Node(NodeType _nt, NodeInfo _ni) : info(_ni), node_type(_nt) {}

    uint32_t returnDataInputPortIndex(Node &);
    uint32_t returnControlInputPortIndex(Node &);
    uint32_t returnMemoryInputPortIndex(Node &);

    uint32_t returnDataOutputPortIndex(Node &);
    uint32_t returnControlOutputPortIndex(Node &);
    uint32_t returnMemoryOutputPortIndex(Node &);

    void addDataInputPort(Node *);
    void addDataOutputPort(Node *);

    void addControlInputPort(Node *const);
    void addControlOutputPort(Node *);

    uint32_t numDataInputPort() { return port_data.data_input_port.size(); }
    uint32_t numDataOutputPort() { return port_data.data_output_port.size(); }
    uint32_t numControlInputPort() {
        return port_control.control_input_port.size();
    }
    uint32_t numControlOutputPort() {
        return port_control.control_output_port.size();
    }

    uint32_t getID() { return info.ID; }
    std::string getName() { return info.Name; }

    // TODO how to define virtual functions?
    // virtual void printInitilization() {}

    uint32_t getType() const { return node_type; }
    virtual std::string PrintDefinition() { return std::string(); }

   protected:  // Private methods
               // virtual void PrintDataflow();
               // virtual void PrintControlFlow();
               // virtual void PrintMemory();
};

/**
 * Super node is actual implimetation of our basic blocks
 */
class SuperNode : public Node {
   public:
    // List of the instructions
    using PhiNodeList = std::list<PhiSelectNode *>;

   private:
    llvm::BasicBlock *basic_block;

    llvm::SmallVector<InstructionNode *, 16> instruction_list;
    PhiNodeList phi_list;

   public:
    explicit SuperNode(NodeInfo _nf, llvm::BasicBlock *_bb = nullptr)
        : Node(Node::SuperNodeTy, _nf), basic_block(_bb) {}

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::SuperNodeTy;
    }

    llvm::BasicBlock *getBasicBlock();
    void addInstruction(InstructionNode *);
    void addPhiInstruction(PhiSelectNode *);

    bool hasPhi() { return !phi_list.empty(); }
    uint32_t getNumPhi() const { return phi_list.size(); }
    const PhiNodeList &getPhiList() const { return phi_list; }

    std::string PrintDefinition(PrintType);
};

/**
 * Memory unit works as a local memory for each graph
 */
class MemoryUnitNode: public Node {
   private:
    MemoryPort read_port_data;
    MemoryPort write_port_data;

   public:
    explicit MemoryUnitNode(NodeInfo _nf) : Node(Node::MemoryUnitTy, _nf) {}

    // Restrict access to data input ports
    void addDataInputPort(Node *) = delete;
    void addDataOutputPort(Node *) = delete;
    uint32_t numDataInputPort() = delete;
    uint32_t numDataOutputPort() = delete;

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::MemoryUnitTy;
    }

    std::string PrintDefinition(PrintType);

    void addReadMemoryReqPort(Node *);
    void addReadMemoryRespPort(Node *);
    void addWriteMemoryReqPort(Node *);
    void addWriteMemoryRespPort(Node *);
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
};

/**
 * SplitCall node
 */
class SplitCallNode: public Node {
   private:
       uint32_t num_input;

   public:
    explicit SplitCallNode(NodeInfo _nf)
        : Node(Node::SplitCallTy, _nf),num_input(0){}

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::SuperNodeTy;
    }

    void setNumInput(uint32_t _n){ num_input =_n; }

    std::string PrintDefinition();
};




/**
 * LoopNode contains all the instructions and useful information about the loops
 */
class LoopNode : public Node {
   public:
    using PhiNodeList = std::list<PhiSelectNode *>;

   private:
    llvm::Loop *loop;

    llvm::SmallVector<InstructionNode *, 16> instruction_list;
    PhiNodeList phi_list;

   public:
    explicit LoopNode(NodeInfo _nf, llvm::Loop *_ll = nullptr)
        : Node(Node::LoopNodeTy, _nf), loop(_ll) {}

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::LoopNodeTy;
    }

    llvm::BasicBlock *getBasicBlock();
    void addInstruction(InstructionNode *);
    void addPhiInstruction(PhiSelectNode *);

    bool hasPhi() { return !phi_list.empty(); }
    uint32_t getNumPhi() const { return phi_list.size(); }
    const PhiNodeList &getPhiList() const { return phi_list; }

    std::string PrintDefinition(PrintType);
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
    InstructionNode(NodeType _nd, NodeInfo _ni, InstType _ins_t,
                    llvm::Instruction *_ins = nullptr)
        : Node(Node::InstructionNodeTy, _ni),
          ins_type(_ins_t),
          parent_instruction(_ins) {}

    llvm::Instruction *getInstruction();

    uint32_t getOpCode() const { return ins_type; }

    bool isBinaryOp() const { return ins_type == BinaryInstructionTy; }

    static bool classof(const Node *T) {
        return T->getType() == Node::InstructionNodeTy;
    }

    std::string PrintDefinition(PrintType);
};

class BinaryOperatorNode : public InstructionNode {
   public:
    BinaryOperatorNode(NodeInfo _ni, llvm::BinaryOperator *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy, _ni,
                          InstructionNode::BinaryInstructionTy, _ins) {}

    // Overloading isa<>, dyn_cast from llvm
    static bool classof(const InstructionNode *I) {
        return I->getOpCode() == InstType::BinaryInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    std::string PrintDefinition(PrintType);
};

class IcmpNode : public InstructionNode {
   public:
    IcmpNode(NodeInfo _ni, llvm::ICmpInst *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy, _ni,
                          InstructionNode::IcmpInstructionTy, _ins) {}

    static bool classof(const InstructionNode *I) {
        return I->getOpCode() == InstType::IcmpInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    std::string PrintDefinition(PrintType);
};

class BranchNode : public InstructionNode {
   public:
    BranchNode(NodeInfo _ni, llvm::BranchInst *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy, _ni,
                          InstType::BranchInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::BranchInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    std::string PrintDefinition(PrintType);
};

class PhiSelectNode : public InstructionNode {
   public:
    PhiSelectNode(NodeInfo _ni, llvm::PHINode *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy, _ni,
                          InstType::PhiInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::PhiInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    std::string PrintDefinition(PrintType);
};

class AllocaNode : public InstructionNode {
   public:
    AllocaNode(NodeInfo _ni, llvm::AllocaInst *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy, _ni,
                          InstructionNode::AllocaInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::AllocaInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
};

class GEPNode : public InstructionNode {
   public:
    GEPNode(NodeInfo _ni, llvm::GetElementPtrInst *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy, _ni,
                          InstructionNode::GetElementPtrInstTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::GetElementPtrInstTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
};

class LoadNode : public InstructionNode {
   private:
    MemoryPort read_port_data;

   public:
    LoadNode(NodeInfo _ni, llvm::LoadInst *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy, _ni,
                          InstructionNode::LoadInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::LoadInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    void addReadMemoryReqPort(Node *);
    void addReadMemoryRespPort(Node *);

};

class StoreNode : public InstructionNode {
   private:
    MemoryPort write_port_data;

   public:
    StoreNode(NodeInfo _ni, llvm::StoreInst *_ins = nullptr,
              NodeType _nd = UnkonwTy)
        : InstructionNode(Node::InstructionNodeTy, _ni,
                          InstructionNode::StoreInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::StoreInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
};

class ReturnNode : public InstructionNode {
   public:
    ReturnNode(NodeInfo _ni, llvm::ReturnInst *_ins = nullptr,
               NodeType _nd = UnkonwTy)
        : InstructionNode(Node::InstructionNodeTy, _ni,
                          InstructionNode::ReturnInstrunctionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::ReturnInstrunctionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }

    std::string PrintDefinition(PrintType);
};

class CallNode : public InstructionNode {
   public:
    CallNode(NodeInfo _ni, llvm::CallInst *_ins = nullptr,
             NodeType _nd = UnkonwTy)
        : InstructionNode(Node::InstructionNodeTy, _ni,
                          InstructionNode::CallInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::CallInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
};

class ArgumentNode : public Node {
   private:
    llvm::Argument *parent_argument;

   public:
    ArgumentNode(NodeInfo _ni, llvm::Argument *_arg = nullptr)
        : Node(Node::FunctionArgTy, _ni), parent_argument(_arg) {}

    llvm::Argument *getArgumentValue();
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
};
}  // namespace dandelion

#endif  // end of DANDDELION_NODE_H
