#include "Dandelion/Node.h"

#include <iostream>

using namespace std;
using namespace llvm;
using namespace dandelion;

//===----------------------------------------------------------------------===//
//                            SuperNode Class
//===----------------------------------------------------------------------===//
void Node::AddDataInputPort(Node *n) { port_data.data_input_port.push_back(n); }
void Node::AddDataOutputPort(Node *n) {
    port_data.data_output_port.push_back(n);
}


/**
 * Returning address of the parent instruction
 */
Instruction * InstructionNode::getInstruction(){
    return this->parent_instruction;
}


ConstantInt* ConstIntNode::getConstantParent(){
    return this->parent_const_int;
}

Argument* ArgumentNode::getArgumentValue(){
    return this->parent_argument;
}

GlobalValue* GlobalValueNode::getGlobalValue(){
    return this->parent_glob;
}
