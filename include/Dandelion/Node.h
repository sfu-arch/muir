#ifndef DANDELION_NODE_H
#define DANDELION_NODE_H
#include <stdint.h>
#include <list>

#include "llvm/IR/Argument.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"

namespace dandelion {

class Node;
class SuperNode;
class MemoryNode;

struct DataPort {
    std::list<Node *> data_input_port;
    std::list<Node *> data_output_port;
};

struct ControlPort {
    std::list<SuperNode *> control_input_port;
    std::list<SuperNode *> control_output_port;
};

struct MemoryPort {
    std::list<MemoryNode *> memory_input_port;
    std::list<MemoryNode *> memory_output_port;
};

enum NodeType {
    BasicBlockTy = 0,
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
    CallInstructionTy,
    FunctionArgTy,
    GlobalValueTy,
    UnkonwTy
};

class Node {
   private:
    // List of data ports
    DataPort port_data;
    // List of Control ports
    ControlPort port_control;
    // List of Memory ports
    MemoryPort port_memory;

    // Type of the Node
    NodeType node_type;

   public:
    Node(NodeType _nt = UnkonwTy) : node_type(_nt) {}

    uint32_t ReturnDataInputPortIndex(Node &);
    uint32_t ReturnControlInputPortIndex(Node &);
    uint32_t ReturnMemoryInputPortIndex(Node &);

    uint32_t ReturnDataOutputPortIndex(Node &);
    uint32_t ReturnControlOutputPortIndex(Node &);
    uint32_t ReturnMemoryOutputPortIndex(Node &);

    void AddDataInputPort(Node *);
    void AddDataOutputPort(Node *);

    uint32_t NumDataInputPort() { return port_data.data_input_port.size(); }
    uint32_t NumDataOutputPort() { return port_data.data_output_port.size(); }

    // TODO how to define virtual functions?
    virtual void PrintInitilization() {}

    uint32_t ReturnType() { return node_type; }
    // virtual void PrintDataflow();
    // virtual void PrintControlFlow();
    // virtual void PrintMemory();
};

/**
 * Super node is actual implimetation of our basic blocks
 */
class SuperNode : public Node {
   private:
    llvm::BasicBlock *basic_block;

    // List of the instructions
    llvm::SmallVector<llvm::Instruction *, 16> instruction_list;

   public:
    explicit SuperNode(llvm::BasicBlock *_bb = nullptr, NodeType _nd = UnkonwTy)
        : Node(_nd), basic_block(_bb) {}
    explicit SuperNode(llvm::SmallVector<llvm::Instruction *, 16> _bb_ins_list,
                       llvm::BasicBlock *_bb = nullptr, NodeType _nd = UnkonwTy)
        : Node(_nd), basic_block(_bb) {
        std::copy(_bb_ins_list.begin(), _bb_ins_list.end(),
                  instruction_list.begin());
    }
};

/**
 * This class is basic implementation of Instruction nodes
 * It inheretens from Node class and it has a pointer to original bitcode IR
 * the pointer can be NULL that means this is a new insturction type
 */
class InstructionNode : public Node {
   private:
    llvm::Instruction *parent_instruction;

   public:
    InstructionNode(llvm::Instruction *_ins = nullptr, NodeType _nd = UnkonwTy)
        : Node(_nd), parent_instruction(_ins) {}

    llvm::Instruction *getInstruction();
};

class BinaryOperatorNode : public InstructionNode {
   public:
    BinaryOperatorNode(llvm::BinaryOperator *_ins = nullptr,
                       NodeType _nd = UnkonwTy)
        : InstructionNode(_ins, _nd) {
        assert(((_nd == BinaryInstructionTy) || (_nd == UnkonwTy)) &&
               " WRONG TYPE: Binaryinstruction can be either "
               "BinaryInstructionTy or UnkonwTy!");
    }
};

class IcmpNode : public InstructionNode {
   public:
    IcmpNode(llvm::ICmpInst *_ins = nullptr, NodeType _nd = UnkonwTy)
        : InstructionNode(_ins, _nd) {
        assert(((_nd == IcmpInstructionTy) || (_nd == UnkonwTy)) &&
               " WRONG TYPE: IcmpInstruction can be either "
               "IcmpInstructionTy or UnkonwTy!");
    }
};

class BranchNode : public InstructionNode {
   public:
    BranchNode(llvm::BranchInst *_ins = nullptr, NodeType _nd = UnkonwTy)
        : InstructionNode(_ins, _nd) {
        assert(((_nd == BranchInstructionTy) || (_nd == UnkonwTy)) &&
               " WRONG TYPE: BranchInstruction can be either "
               "BranchInstructionTy or UnkonwTy!");
    }
};

class PHIGNode : public InstructionNode {
   public:
    PHIGNode(llvm::PHINode *_ins = nullptr, NodeType _nd = UnkonwTy)
        : InstructionNode(_ins, _nd) {
        assert(((_nd == PhiInstructionTy) || (_nd == UnkonwTy)) &&
               " WRONG TYPE: PHIInstruction can be either "
               "PHIInsturctionTy or UnkonwTy!");
    }
};

class AllocaNode : public InstructionNode {
   public:
    AllocaNode(llvm::AllocaInst *_ins = nullptr, NodeType _nd = UnkonwTy)
        : InstructionNode(_ins, _nd) {
        assert(((_nd == AllocaInstructionTy) || (_nd == UnkonwTy)) &&
               " WRONG TYPE: AllocaInstruction can be either "
               "AllocaInstructionTy or UnkonwTy!");
    }
};

class GEPNode : public InstructionNode {
   public:
    GEPNode(llvm::GetElementPtrInst *_ins = nullptr, NodeType _nd = UnkonwTy)
        : InstructionNode(_ins, _nd) {
        assert(((_nd == GetElementPtrInstTy) || (_nd == UnkonwTy)) &&
               " WRONG TYPE: GEPInstruction can be either "
               "GEPInstructionTy or UnkonwTy!");
    }
};

class LoadNode : public InstructionNode {
   public:
    LoadNode(llvm::LoadInst *_ins = nullptr, NodeType _nd = UnkonwTy)
        : InstructionNode(_ins, _nd) {
        assert(((_nd == LoadInstructionTy) || (_nd == UnkonwTy)) &&
               " WRONG TYPE: LoadInstruction can be either "
               "LoadInstructionTy or UnkonwTy!");
    }
};

class StoreNode : public InstructionNode {
   public:
    StoreNode(llvm::StoreInst *_ins = nullptr, NodeType _nd = UnkonwTy)
        : InstructionNode(_ins, _nd) {
        assert(((_nd == StoreInstructionTy) || (_nd == UnkonwTy)) &&
               " WRONG TYPE: StoreInstruction can be either "
               "StoreInstructionTy or UnkonwTy!");
    }
};

class ReturnNode : public InstructionNode {
   public:
    ReturnNode(llvm::ReturnInst *_ins = nullptr, NodeType _nd = UnkonwTy)
        : InstructionNode(_ins, _nd) {
        assert(((_nd == ReturnInstrunctionTy) || (_nd == UnkonwTy)) &&
               " WRONG TYPE: ReturnInstruction can be either "
               "ReturnInstructionTy or UnkonwTy!");
    }
};

class CallNode : public InstructionNode {
   public:
    CallNode(llvm::CallInst *_ins = nullptr, NodeType _nd = UnkonwTy)
        : InstructionNode(_ins, _nd) {
        assert(((_nd == CallInstructionTy) || (_nd == UnkonwTy)) &&
               " WRONG TYPE: CallInstruction can be either "
               "CallInstructionTy or UnkonwTy!");
    }
};

class ArgumentNode : public Node {
   private:
    llvm::Argument *parent_argument;

   public:
    ArgumentNode(llvm::Argument *_arg = nullptr, NodeType _nd = UnkonwTy)
        : Node(_nd), parent_argument(_arg) {
        assert(((_nd == FunctionArgTy) || (_nd == UnkonwTy)) &&
               " WRONG TYPE: ArgumentNode can be either "
               "FunctionArgTy or UnkonwTy!");
    }

    llvm::Argument* getArgumentValue();
};

class GlobalValueNode : public Node {
   private:
    llvm::GlobalValue *parent_glob;

   public:
    GlobalValueNode(llvm::GlobalValue *_glb = nullptr, NodeType _nd = UnkonwTy)
        : Node(_nd), parent_glob(_glb) {
        assert(((_nd == GlobalValueTy) || (_nd == UnkonwTy)) &&
               " WRONG TYPE: GlobalNode can be either "
               "GlobalValueTy or UnkonwTy!");
    }

    llvm::GlobalValue* getGlobalValue();
};

class ConstIntNode : public Node {
   private:
    llvm::ConstantInt* parent_const_int;

   public:
    ConstIntNode(llvm::ConstantInt *_cint = nullptr)
        : parent_const_int(_cint) {}

    llvm::ConstantInt* getConstantParent();
};
}

#endif  // end of DANDDELION_NODE_H
