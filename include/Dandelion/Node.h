#ifndef DANDELION_NODE_H
#define DANDELION_NODE_H
#include <stdint.h>
#include <list>

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
class MemoryNode;
class InstructionNode;
class PhiNode;

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
    std::list<MemoryNode *> memory_input_port;
    std::list<MemoryNode *> memory_output_port;
};

class Node {
   public:
    enum NodeType {
        SuperNodeTy = 0,
        InstructionNodeTy,
        FunctionArgTy,
        GlobalValueTy,
        ConstIntTy,
        UnkonwTy

    };

   private:
    // List of data ports
    DataPort port_data;
    // List of Control ports
    ControlPort port_control;
    // List of Memory ports
    MemoryPort port_memory;

    // Type of the Node
    NodeType node_type;

   public:  // Public methods
    Node(NodeType _nt) : node_type(_nt) {}

    uint32_t returnDataInputPortIndex(Node &);
    uint32_t returnControlInputPortIndex(Node &);
    uint32_t returnMemoryInputPortIndex(Node &);

    uint32_t returnDataOutputPortIndex(Node &);
    uint32_t returnControlOutputPortIndex(Node &);
    uint32_t returnMemoryOutputPortIndex(Node &);

    void addDataInputPort(Node *);
    void addDataOutputPort(Node *);

    void addControlInputPort(Node *);
    void addControlOutputPort(Node *);

    uint32_t numDataInputPort() { return port_data.data_input_port.size(); }
    uint32_t numDataOutputPort() { return port_data.data_output_port.size(); }

    // TODO how to define virtual functions?
    // virtual void printInitilization() {}

    uint32_t getType() const { return node_type; }
    virtual void printDefinition() {}

   protected:  // Private methods
    // virtual void PrintDataflow();
    // virtual void PrintControlFlow();
    // virtual void PrintMemory();
    //
};

/**
 * Super node is actual implimetation of our basic blocks
 */
class SuperNode : public Node {
   public:
    // List of the instructions
    using PhiNodeList = std::list<PhiNode *>;

   private:
    llvm::BasicBlock *basic_block;

    llvm::SmallVector<InstructionNode *, 16> instruction_list;
    PhiNodeList phi_list;

   public:
    explicit SuperNode(llvm::BasicBlock *_bb = nullptr)
        : Node(Node::SuperNodeTy), basic_block(_bb) {}

    // Define classof function so that we can use dyn_cast function
    static bool classof(const Node *T) {
        return T->getType() == Node::SuperNodeTy;
    }

    llvm::BasicBlock *getBasicBlock();
    void addInstruction(InstructionNode *);
    void addPhiInstruction(PhiNode *);

    bool hasPhi() { return !phi_list.empty(); }
    uint32_t getNumPhi() { return phi_list.size(); }
    const PhiNodeList &getPhiList() const { return phi_list; }

    void PrintDefinition(Node::PrintType);
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
    InstructionNode(NodeType _nd, InstType _ins_t,
                    llvm::Instruction *_ins = nullptr)
        : Node(Node::InstructionNodeTy),
          ins_type(_ins_t),
          parent_instruction(_ins) {}

    llvm::Instruction *getInstruction();

    uint32_t getOpCode() const { return ins_type; }

    bool isBinaryOp() const { return ins_type == BinaryInstructionTy; }

    static bool classof(const Node *T) {
        return T->getType() == Node::InstructionNodeTy;
    }
};

class BinaryOperatorNode : public InstructionNode {
   public:
    BinaryOperatorNode(llvm::BinaryOperator *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy,
                          InstructionNode::BinaryInstructionTy, _ins) {}

    // Overloading isa<>, dyn_cast from llvm
    static bool classof(const InstructionNode *I) {
        return I->getOpCode() == InstType::BinaryInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
};

class IcmpNode : public InstructionNode {
   public:
    IcmpNode(llvm::ICmpInst *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy,
                          InstructionNode::IcmpInstructionTy, _ins) {}

    static bool classof(const InstructionNode *I) {
        return I->getOpCode() == InstType::IcmpInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
};

class BranchNode : public InstructionNode {
   public:
    BranchNode(llvm::BranchInst *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy,
                          InstType::BranchInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::BranchInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
};

class PhiNode : public InstructionNode {
   public:
    PhiNode(llvm::PHINode *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy, InstType::PhiInstructionTy,
                          _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::PhiInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
};

class AllocaNode : public InstructionNode {
   public:
    AllocaNode(llvm::AllocaInst *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy,
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
    GEPNode(llvm::GetElementPtrInst *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy,
                          InstructionNode::GetElementPtrInstTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::GetElementPtrInstTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
};

class LoadNode : public InstructionNode {
   public:
    LoadNode(llvm::LoadInst *_ins = nullptr)
        : InstructionNode(Node::InstructionNodeTy,
                          InstructionNode::LoadInstructionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::LoadInstructionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
};

class StoreNode : public InstructionNode {
   public:
    StoreNode(llvm::StoreInst *_ins = nullptr, NodeType _nd = UnkonwTy)
        : InstructionNode(Node::InstructionNodeTy,
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
    ReturnNode(llvm::ReturnInst *_ins = nullptr, NodeType _nd = UnkonwTy)
        : InstructionNode(Node::InstructionNodeTy,
                          InstructionNode::ReturnInstrunctionTy, _ins) {}

    static bool classof(const InstructionNode *T) {
        return T->getOpCode() == InstructionNode::ReturnInstrunctionTy;
    }
    static bool classof(const Node *T) {
        return isa<InstructionNode>(T) && classof(cast<InstructionNode>(T));
    }
};

class CallNode : public InstructionNode {
   public:
    CallNode(llvm::CallInst *_ins = nullptr, NodeType _nd = UnkonwTy)
        : InstructionNode(Node::InstructionNodeTy,
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
    ArgumentNode(llvm::Argument *_arg = nullptr)
        : Node(Node::FunctionArgTy), parent_argument(_arg) {}

    llvm::Argument *getArgumentValue();
};

class GlobalValueNode : public Node {
   private:
    llvm::GlobalValue *parent_glob;

   public:
    GlobalValueNode(llvm::GlobalValue *_glb = nullptr)
        : Node(Node::GlobalValueTy), parent_glob(_glb) {}

    llvm::GlobalValue *getGlobalValue();
};

class ConstIntNode : public Node {
   private:
    llvm::ConstantInt *parent_const_int;

   public:
    ConstIntNode(llvm::ConstantInt *_cint = nullptr)
        : Node(Node::ConstIntTy), parent_const_int(_cint) {}

    llvm::ConstantInt *getConstantParent();
};
}

#endif  // end of DANDDELION_NODE_H
