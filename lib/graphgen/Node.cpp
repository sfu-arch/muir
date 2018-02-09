#define DEBUG_TYPE "graphgen"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Dandelion/Node.h"
#include "luacpptemplater/LuaTemplater.h"

#include <iostream>

using namespace std;
using namespace llvm;
using namespace dandelion;

//===----------------------------------------------------------------------===//
//                            Node Class
//===----------------------------------------------------------------------===//

void Node::addDataInputPort(Node *n) { port_data.data_input_port.push_back(n); }
void Node::addDataOutputPort(Node *n) {
    port_data.data_output_port.push_back(n);
}

void Node::addControlInputPort(Node *n) {
    port_control.control_input_port.push_back(n);
}
void Node::addControlOutputPort(Node *n) {
    port_control.control_output_port.push_back(n);
}

//===----------------------------------------------------------------------===//
//                            SuperNode Class
//===----------------------------------------------------------------------===//

void SuperNode::addInstruction(InstructionNode *node) {
    this->instruction_list.push_back(node);
}

void SuperNode::addPhiInstruction(PhiNode *node) {
    this->phi_list.push_back(node);
}

void SuperNode::PrintDefinition(Node::PrintType pt) {
    switch (pt) {
        case PrintType::Scala:
            DEBUG(errs() << "Test print scala!\n");
            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

//===----------------------------------------------------------------------===//
//                            Instruction Class
//===----------------------------------------------------------------------===//
//
/**
 * Returning address of the parent instruction
 */
Instruction *InstructionNode::getInstruction() {
    return this->parent_instruction;
}

ConstantInt *ConstIntNode::getConstantParent() {
    return this->parent_const_int;
}

Argument *ArgumentNode::getArgumentValue() { return this->parent_argument; }

GlobalValue *GlobalValueNode::getGlobalValue() { return this->parent_glob; }

BasicBlock *SuperNode::getBasicBlock() { return this->basic_block; }
