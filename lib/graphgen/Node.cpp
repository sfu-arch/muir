#define DEBUG_TYPE "graphgen"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Common.h"
#include "Dandelion/Node.h"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;
using namespace llvm;
using namespace dandelion;
using namespace helpers;

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
            _text =
                "  val $name = Module(new $type(NumInputs = $num_in, NumOuts = "
                "$num_out, BID = $bid))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_in", this->numControlInputPort());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numControlOutputPort()));
            helperReplace(_text, "$bid", this->getID());
            if (this->getNumPhi())
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
//                            MemoryUnitNode Class
//===----------------------------------------------------------------------===//

void MemoryUnitNode::addReadMemoryReqPort(Node *const n) {
    read_port_data.memory_req_port.emplace_back(n);
}
void MemoryUnitNode::addReadMemoryRespPort(Node *const n) {
    read_port_data.memory_resp_port.emplace_back(n);
}

void MemoryUnitNode::addWriteMemoryReqPort(Node *const n) {
    write_port_data.memory_req_port.emplace_back(n);
}
void MemoryUnitNode::addWriteMemoryRespPort(Node *const n) {
    write_port_data.memory_resp_port.emplace_back(n);
}

std::string MemoryUnitNode::PrintDefinition(PrintType pt) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $reg_type(ID=$id, Size=$size, "
                "NReads=$num_read, NWrites=$num_write))\n"
                "\t\t (WControl=new WriteMemoryController(NumOps=$read_num_op, "
                "BaseSize=$read_base_size, NumEntries=$read_num_entries))\n"
                "\t\t (RControl=new ReadMemoryController(NumOps=$write_num_op, "
                "BaseSize=$write_base_size, "
                "NumEntries=$write_num_entries)))\n\n"
                "  io.MemReq <> $name.MemReq\n"
                "  $name.io.MemResp <> io.MemResp\n\n";
            ;
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$reg_type", "UnifiedController");
            helperReplace(_text, "$id", std::to_string(this->getID()));

            // TODO this part can be parametrize using config file
            helperReplace(_text, "$size", std::to_string(32));
            helperReplace(_text, "$num_read",
                          std::to_string(this->numReadDataInputPort()));
            helperReplace(_text, "$num_write",
                          std::to_string(this->numWriteDataOutputPort()));
            helperReplace(_text, "$read_num_op",
                          std::to_string(this->numReadDataInputPort()));
            helperReplace(_text, "$read_base_size", std::to_string(2));
            helperReplace(_text, "$read_num_entries",
                          std::to_string(this->numReadDataInputPort()));
            helperReplace(_text, "$write_num_op",
                          std::to_string(this->numWriteDataOutputPort()));
            helperReplace(_text, "$write_base_size", std::to_string(2));
            helperReplace(_text, "$write_num_entries",
                          std::to_string(this->numReadDataInputPort()));

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            CallSpliter Class
//===----------------------------------------------------------------------===//

std::string SplitCallNode::PrintDefinition() {
    string _name(this->getName());

    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text =
        "  val $name = Module(new $type(List($<input_vector>)))\n"
        "  $name.io.In <> io.in\n\n";

    std::vector<uint32_t> _t_input(2,32);

    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$type", "SplitCall");
    helperReplace(_text, "$id", std::to_string(this->getID()));
    helperReplace(_text, "$<input_vector>", std::vector<uint32_t>({32,32}), "," );

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

//===----------------------------------------------------------------------===//
//                            LoadNode Class
//===----------------------------------------------------------------------===//

void LoadNode::addReadMemoryReqPort(Node *const _nd) {
    this->read_port_data.memory_req_port.push_back(_nd);
}
void LoadNode::addReadMemoryRespPort(Node *const _nd) {
    this->read_port_data.memory_resp_port.push_back(_nd);
}
