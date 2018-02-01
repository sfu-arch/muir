#ifndef DANDELION_NODE_H
#define DANDELION_NODE_H
#include <stdint.h>
#include <list>

#include "llvm/IR/Argument.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"

namespace dandelion {

class SuperNode;
class MemoryNode;

class Node {
   private:
    // List of data ports
    std::list<Node *> data_input_port;
    std::list<Node *> data_output_port;
    // List of Control ports
    std::list<SuperNode *> control_input_port;
    std::list<SuperNode *> control_output_port;
    // List of Memory ports
    std::list<MemoryNode *> memory_input_port;
    std::list<MemoryNode *> memory_output_port;

   public:
    Node() {}

    uint32_t ReturnDataInputPortIndex(Node &);
    uint32_t ReturnControlInputPortIndex(Node &);
    uint32_t ReturnMemoryInputPortIndex(Node &);

    uint32_t ReturnDataOutputPortIndex(Node &);
    uint32_t ReturnControlOutputPortIndex(Node &);
    uint32_t ReturnMemoryOutputPortIndex(Node &);

    //TODO how to define virtual functions?
    //virtual void PrintInitilization();
    //virtual void PrintDataflow();
    //virtual void PrintControlFlow();
    //virtual void PrintMemory();
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
    explicit SuperNode(llvm::BasicBlock *_bb = nullptr) : basic_block(_bb) {}
    explicit SuperNode(llvm::SmallVector<llvm::Instruction *, 16> _bb_ins_list,
                       llvm::BasicBlock *_bb = nullptr)
        : basic_block(_bb) {
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
    InstructionNode(llvm::Instruction *_ins = nullptr)
        : parent_instruction(_ins) {}
};

class ArgumentNode : public Node {
   private:
    llvm::Argument *parent_argument;

   public:
    ArgumentNode(llvm::Argument *_arg = nullptr) : parent_argument(_arg) {}
};
}

#endif  // end of DANDDELION_NODE_H
