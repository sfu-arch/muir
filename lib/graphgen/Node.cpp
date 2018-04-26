#define DEBUG_TYPE "graphgen"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Dandelion/Node.h"
#include "luacpptemplater/LuaTemplater.h"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;
using namespace llvm;
using namespace dandelion;

//===----------------------------------------------------------------------===//
//                           HELPEr FUNCTIONS
//===----------------------------------------------------------------------===//
bool helperReplace(std::string &str, const std::string &from, const std::string &to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos) return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

//===----------------------------------------------------------------------===//
//                            Node Class
//===----------------------------------------------------------------------===//

void Node::addDataInputPort(Node *const n) {
    port_data.data_input_port.emplace_back(n);
}
void Node::addDataOutputPort(Node *const n) {
    port_data.data_output_port.emplace_back(n);
}

void Node::addControlInputPort(Node *const n) {
    port_control.control_input_port.emplace_back(n);
}
void Node::addControlOutputPort(Node *const n) {
    port_control.control_output_port.emplace_back(n);
}

//===----------------------------------------------------------------------===//
//                            SuperNode Class
//===----------------------------------------------------------------------===//

void SuperNode::addInstruction(InstructionNode *node) {
    this->instruction_list.push_back(node);
}

void SuperNode::addPhiInstruction(PhiSelectNode *node) {
    this->phi_list.push_back(node);
}

std::string SuperNode::PrintDefinition(PrintType pt) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "  val $name = Module(new $type(NumInputs = $num_in, NumOuts = $num_out, BID = $bid))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_in", std::to_string(this->numControlInputPort()));
            helperReplace(_text, "$num_out", std::to_string(this->numControlOutputPort()));
            helperReplace(_text, "$bid", std::to_string(this->getID()));
            if(this->getNumPhi())
                helperReplace(_text, "$type", "BasicBlockNoMaskNode");
            else
                helperReplace(_text, "$type", "BasicBlockNode");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
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
