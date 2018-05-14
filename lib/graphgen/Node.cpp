#define DEBUG_TYPE "graphgen"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Common.h"
#include "Dandelion/Node.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#define MEM_SIZE 32
#define BASE_SIZE 2

using namespace std;
using namespace llvm;
using namespace dandelion;
using namespace helpers;

//===----------------------------------------------------------------------===//
//                            Node Class
//===----------------------------------------------------------------------===//

void Node::addDataInputPort(Node *n) {
    port_data.data_input_port.emplace_back(n);
}
void Node::addDataOutputPort(Node *n) {
    port_data.data_output_port.emplace_back(n);
}

void Node::addControlInputPort(Node *n) {
    port_control.control_input_port.emplace_back(n);
}
void Node::addControlOutputPort(Node *n) {
    port_control.control_output_port.emplace_back(n);
}

void Node::addReadMemoryReqPort(Node *const n) {
    read_port_data.memory_req_port.emplace_back(n);
}
void Node::addReadMemoryRespPort(Node *const n) {
    read_port_data.memory_resp_port.emplace_back(n);
}

void Node::addWriteMemoryReqPort(Node *const n) {
    write_port_data.memory_req_port.emplace_back(n);
}
void Node::addWriteMemoryRespPort(Node *const n) {
    write_port_data.memory_resp_port.emplace_back(n);
}

uint32_t Node::returnDataOutputPortIndex(Node *_node) {
    return std::distance(this->port_data.data_output_port.begin(),
                         find(this->port_data.data_output_port.begin(),
                              this->port_data.data_output_port.end(), _node));
}

uint32_t Node::returnDataInputPortIndex(Node *_node) {
    return std::distance(this->port_data.data_input_port.begin(),
                         find(this->port_data.data_input_port.begin(),
                              this->port_data.data_input_port.end(), _node));
}

uint32_t Node::returnControlOutputPortIndex(Node *_node) {
    return std::distance(
        this->port_control.control_output_port.begin(),
        find(this->port_control.control_output_port.begin(),
             this->port_control.control_output_port.end(), _node));
}

uint32_t Node::returnControlInputPortIndex(Node *_node) {
    return std::distance(
        this->port_control.control_input_port.begin(),
        find(this->port_control.control_input_port.begin(),
             this->port_control.control_input_port.end(), _node));
}

//Return memory indexes
uint32_t Node::returnMemoryReadInputPortIndex(Node *_node) {
    return std::distance(
        this->read_port_data.memory_req_port.begin(),
        find(this->read_port_data.memory_req_port.begin(),
             this->read_port_data.memory_req_port.end(), _node));
}

uint32_t Node::returnMemoryReadOutputPortIndex(Node *_node) {
    return std::distance(
        this->read_port_data.memory_resp_port.begin(),
        find(this->read_port_data.memory_resp_port.begin(),
             this->read_port_data.memory_resp_port.end(), _node));
}


uint32_t Node::returnMemoryWriteInputPortIndex(Node *_node) {
    return std::distance(
        this->write_port_data.memory_req_port.begin(),
        find(this->write_port_data.memory_req_port.begin(),
             this->write_port_data.memory_req_port.end(), _node));
}

uint32_t Node::returnMemoryWriteOutputPortIndex(Node *_node) {
    return std::distance(
        this->write_port_data.memory_resp_port.begin(),
        find(this->write_port_data.memory_resp_port.begin(),
             this->write_port_data.memory_resp_port.end(), _node));
}

std::list<Node *>::const_iterator Node::findDataInputNode(Node *_node) {
    return find(this->port_data.data_input_port.begin(),
                this->port_data.data_input_port.end(), _node);
}

std::list<Node *>::const_iterator Node::findDataOutputNode(Node *_node) {
    return find(this->port_data.data_output_port.begin(),
                this->port_data.data_output_port.end(), _node);
}

std::list<Node *>::const_iterator Node::findControlInputNode(Node *_node) {
    return find(this->port_control.control_input_port.begin(),
                this->port_control.control_input_port.end(), _node);
}

std::list<Node *>::const_iterator Node::findControlOutputNode(Node *_node) {
    return find(this->port_control.control_output_port.begin(),
                this->port_control.control_output_port.end(), _node);
}

void Node::removeNodeDataInputNode(Node *_node) {
    this->port_data.data_input_port.remove(_node);
}

void Node::removeNodeDataOutputNode(Node *_node) {
    this->port_data.data_output_port.remove(_node);
}

void Node::removeNodeControlInputNode(Node *_node) {
    this->port_control.control_input_port.remove(_node);
}

void Node::removeNodeControlOutputNode(Node *_node) {
    this->port_control.control_output_port.remove(_node);
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

std::string SuperNode::printDefinition(PrintType pt) {
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
            switch (this->getNodeType()) {
                case SuperNodeType::NoMask:
                    helperReplace(_text, "$type", "BasicBlockNoMaskNode");
                    break;
                case SuperNodeType::Mask:
                    helperReplace(_text, "$type", "BasicBlockNode");
                    break;
                case SuperNodeType::LoopHead:
                    helperReplace(_text, "$type", "LoopHead");
                    break;
            }

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string SuperNode::printInputEnable(PrintType pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.predicateIn($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string SuperNode::printOutputEnable(PrintType pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string SuperNode::printMaskOutput(PrintType pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.MaskBB($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string SuperNode::printActivateEnable(PrintType pt) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.activate";
            helperReplace(_text, "$name", _name.c_str());
            // helperReplace(_text, "$id", _id);

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


std::string MemoryNode::printDefinition(PrintType pt) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $reg_type(ID=$id, Size=$size, "
                "NReads=$num_read, NWrites=$num_write))\n"
                "\t\t (WControl=new "
                "WriteMemoryController(NumOps=$write_num_op, "
                "BaseSize=$read_base_size, NumEntries=$read_num_entries))\n"
                "\t\t (RControl=new ReadMemoryController(NumOps=$read_num_op, "
                "BaseSize=$write_base_size, "
                "NumEntries=$write_num_entries)))\n\n"
                "  io.MemReq <> $name.MemReq\n"
                "  $name.io.MemResp <> io.MemResp\n\n";
            ;
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$reg_type", "UnifiedController");
            helperReplace(_text, "$id", std::to_string(this->getID()));

            // TODO this part can be parametrize using config file
            helperReplace(_text, "$size", MEM_SIZE);
            helperReplace(_text, "$num_read",
                          std::to_string(this->numReadDataInputPort()));
            helperReplace(_text, "$num_write",
                          std::to_string(this->numWriteDataInputPort()));
            helperReplace(_text, "$read_num_op",
                          std::to_string(this->numReadDataInputPort()));
            helperReplace(_text, "$read_base_size", BASE_SIZE);
            helperReplace(_text, "$read_num_entries", BASE_SIZE);
            helperReplace(_text, "$write_num_op",
                          std::to_string(this->numWriteDataOutputPort()));
            helperReplace(_text, "$write_base_size", BASE_SIZE);
            helperReplace(_text, "$write_num_entries", BASE_SIZE);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string MemoryNode::printMemReadInput(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text =
                "$name.io.ReadIn($mid)";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$mid", _id);
            //TODO add mid
            break;
        default:
            break;
    }

    return _text;
}


std::string MemoryNode::printMemReadOutput(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text =
                "$name.io.ReadOut($mid)";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$mid", _id);
            //TODO add mid
            break;
        default:
            break;
    }

    return _text;
}


std::string MemoryNode::printMemWriteInput(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text =
                "$name.io.WriteIn($mid)";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$mid", _id);
            //TODO add mid
            break;
        default:
            break;
    }

    return _text;
}


std::string MemoryNode::printMemWriteOutput(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text =
                "$name.io.WriteOut($mid)";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$mid", _id);
            //TODO add mid
            break;
        default:
            break;
    }

    return _text;
}



//===----------------------------------------------------------------------===//
//                            CallSpliter Class
//===----------------------------------------------------------------------===//

ArgumentNode *SplitCallNode::insertArgument(llvm::Argument &_f_arg) {
    fun_arg_list.push_back(std::make_unique<ArgumentNode>(
        NodeInfo(fun_arg_list.size(), _f_arg.getName().str()), this, &_f_arg));

    auto ff = std::find_if(fun_arg_list.begin(), fun_arg_list.end(),
                           [&_f_arg](auto &arg) -> bool {
                               return arg.get()->getArgumentValue() == &_f_arg;
                           });
    ff->get()->printDefinition(PrintType::Scala);

    return ff->get();
}

std::string SplitCallNode::printDefinition(PrintType _pt) {
    string _text("");
    string _name(this->getName());

    auto make_argument_port = [](SplitCallNode::FunctionArgumentList &_list) {
        std::vector<uint32_t> _arg_count;
        for (auto &l : _list)
            _arg_count.push_back(l->getArgumentValue()->getNumUses());
        return _arg_count;
    };

    switch (_pt) {
        case PrintType::Scala:

            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(List($<input_vector>)))\n"
                "  $name.io.In <> io.in\n\n";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$type", "SplitCall");
            helperReplace(_text, "$id", std::to_string(this->getID()));
            helperReplace(_text, "$<input_vector>",
                          make_argument_port(this->fun_arg_list), ",");
            // std::vector<uint32_t>(this->numDataOutputPort(), XLEN), ",");
            // TODO: uncomment if you update the list shape.
            // this->num_ports, ",");

            break;
        default:
            assert(!"Don't support!");
    }
    return _text;
}

std::string SplitCallNode::printOutputEnable(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.Out.enable";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string SplitCallNode::printOutputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out.data(\"field$id\")";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _idx);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}


uint32_t SplitCallNode::findArgumentIndex(ArgumentNode *_arg_node){
    uint32_t c = 0;
    for(auto &_a : fun_arg_list){
        if(_a->getName() == _arg_node->getName())
            return c;
        c++;
    }
    return c;
}

//===----------------------------------------------------------------------===//
//                            BranchNode Class
//===----------------------------------------------------------------------===//

std::string BranchNode::printOutputEnable(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);
            break;
        default:
            break;
    }

    return _text;
}

std::string BranchNode::printInputData(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.CmpIO($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);
            break;
        default:
            break;
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

//===----------------------------------------------------------------------===//
//                            ArgumentNode Class
//===----------------------------------------------------------------------===//
std::string ArgumentNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumOuts = "
                "$num_out, ID = $id))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "ArgumentNode");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ArgumentNode::printInputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.InData($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _idx);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ArgumentNode::printOutputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            //_text = "$name.io.Out($id)";
            _text = "$call.io.Out(\"field$num\")($id)";
            helperReplace(_text, "$call", this->parent_call_node->getName());
            helperReplace(_text, "$num", this->parent_call_node->findArgumentIndex(this));
            helperReplace(_text, "$id", _idx);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}


//===----------------------------------------------------------------------===//
//                            GlobalNode Class
//===----------------------------------------------------------------------===//
GlobalValue *GlobalValueNode::getGlobalValue() { return this->parent_glob; }

BasicBlock *SuperNode::getBasicBlock() { return this->basic_block; }

//===----------------------------------------------------------------------===//
//                            BinaryOperatorNode Class
//===----------------------------------------------------------------------===//

std::string BinaryOperatorNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumOuts = "
                "$num_out, ID = $id, opCode = \"$opcode\")(sign=false))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "ComputeNode");
            helperReplace(_text, "$opcode", this->getOpCodeName());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string BinaryOperatorNode::printInputEnable(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string BinaryOperatorNode::printOutputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string BinaryOperatorNode::printInputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            if (_idx == 0)
                _text = "$name.io.Left";
            else
                _text = "$name.io.Right";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            ICMPNode Class
//===----------------------------------------------------------------------===//

std::string IcmpNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumOuts = "
                "$num_out, ID = $id, opCode = \"$opcode\")(sign=false))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "ComputeNode");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string IcmpNode::printInputEnable(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string IcmpNode::printInputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            if (_idx == 0)
                _text = "$name.io.Left";
            else
                _text = "$name.io.Right";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string IcmpNode::printOutputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            BranchNode Class
//===----------------------------------------------------------------------===//

std::string BranchNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "  val $name = Module(new $type(ID = $id))\n\n";
            if (this->numDataInputPort() > 0) {
                helperReplace(_text, "$type", "CBranchNode");
                helperReplace(_text, "$num_out",
                              std::to_string(this->numControlOutputPort()));
            } else
                helperReplace(_text, "$type", "UBranchNode");

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string BranchNode::printInputEnable(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            PhiSelectNode Class
//===----------------------------------------------------------------------===//

std::string PhiSelectNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumInputs = $num_in, "
                "NumOutputs = $num_out, ID = $id))\n\n";
            helperReplace(_text, "$type", "PhiNode");
            helperReplace(_text, "$num_in",
                          std::to_string(this->numDataInputPort()));
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string PhiSelectNode::printInputEnable(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string PhiSelectNode::printInputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.InData($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string PhiSelectNode::printOutputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string PhiSelectNode::printMaskInput(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Mask";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            ReturnNode Class
//===----------------------------------------------------------------------===//
//
std::string ReturnNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(retTypes=List($<input_list>), "
                "ID = $id))\n\n";
            helperReplace(_text, "$type", "RetNode");
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$<input_list>",
                          std::vector<uint32_t>(this->numDataInputPort(), 32),
                          ",");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ReturnNode::printInputEnable(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ReturnNode::printInputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.in.data(\"field$id\")";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ReturnNode::printOutputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            LoadNode Class
//===----------------------------------------------------------------------===//

std::string LoadNode::printDefinition(PrintType _pt) {
    string _text("");
    string _name(this->getName());

    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumPredOps=$npo, "
                "NumSuccOps=$nso, "
                "NumOuts=$num_out,ID=$id,RouteID=$rid))\n\n";
            helperReplace(_text, "$type", "UnTypLoad");

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$rid", 0);
            helperReplace(_text, "$num_out", this->numDataOutputPort());

            break;
        default:
            assert(!"Don't support!");
    }
    return _text;
}

std::string LoadNode::printInputEnable(PrintType pt) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string LoadNode::printInputEnable(PrintType pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.predicateIn($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string LoadNode::printOutputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out.data($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _idx);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string LoadNode::printInputData(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text =
                "$name.io.GepAddr";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$mem", mem_unit->getName());
            break;
        default:
            break;
    }

    return _text;
}

std::string LoadNode::printMemReadInput(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text =
                "$name.io.memResp";

            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}


std::string LoadNode::printMemReadOutput(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text =
                "$name.io.memReq";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

//===----------------------------------------------------------------------===//
//                            StoreNode Class
//===----------------------------------------------------------------------===//

std::string StoreNode::printDefinition(PrintType _pt) {
    string _text("");
    string _name(this->getName());

    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumPredOps=$npo, "
                "NumSuccOps=$nso, "
                "NumOuts=$num_out,ID=$id,RouteID=$rid))\n\n";
            helperReplace(_text, "$type", "UnTypStore");

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$rid", 0);
            helperReplace(_text, "$num_out", this->numDataOutputPort());

            break;
        default:
            assert(!"Don't support!");
    }
    return _text;
}

std::string StoreNode::printInputEnable(PrintType pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.predicateIn($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string StoreNode::printInputEnable(PrintType pt) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string StoreNode::printInputData(PrintType _pt) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$mem.io.inData";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string StoreNode::printInputData(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            if (_id == 0)
                _text = "$name.io.inData($id)";
            else
                _text =
                    "$name.io.GepAddr";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);
            break;
        default:
            break;
    }

    return _text;
}


std::string StoreNode::printMemWriteInput(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text =
                "$name.io.memResp";

            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}


std::string StoreNode::printMemWriteOutput(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text =
                "$name.io.memReq";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

//===----------------------------------------------------------------------===//
//                            ConstantNode Class
//===----------------------------------------------------------------------===//

std::string ConstIntNode::printOutputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            GetElementPtr Class
//===----------------------------------------------------------------------===//
std::string GEPNode::printDefinition(PrintType _pt) {
    string _text("");
    string _name(this->getName());

    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            if (this->getInstruction()->getNumOperands() == 2) {
                _text =
                    "  val $name = Module(new $type(NumOuts=$num_out, "
                    "ID=$id)(numByte1=$nb1))\n\n";
                helperReplace(_text, "$type", "GepOneNode");
            } else {
                _text =
                    "  val $name = Module(new $type(NumOuts=$num_out, "
                    "ID=$id)(numByte1=$nb1, numByte2=$nb2))\n";
                helperReplace(_text, "$type", "GepTwoNode");
            }

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", std::to_string(this->getID()));
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));

            break;
        default:
            assert(!"Don't support!");
    }
    return _text;
}

std::string GEPNode::printInputEnable(PrintType pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string GEPNode::printInputEnable(PrintType pt) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string GEPNode::printOutputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out.data($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _idx);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string GEPNode::printInputData(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            if (_id == 0)
                _text = "$name.io.baseAddress";
            else if (_id == 1)
                _text = "$name.io.idx1";
            else
                _text = "$name.io.idx2";

            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

//===----------------------------------------------------------------------===//
//                            LoopNode Class
//===----------------------------------------------------------------------===//

std::string LoopNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumIns=$<vec_in>, NumOuts = "
                "$num_out, NumExits=$num_exit, ID = $id))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "LoopBlock");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string LoopNode::printOutputEnable(PrintType _pt) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.Out.activate";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string LoopNode::printInputEnable(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _idx);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}
