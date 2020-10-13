#define DEBUG_TYPE "graphgen"

#include "llvm/Support/Debug.h"
//#include "llvm/Support/raw_ostream.h"

#include "Common.h"
#include "Dandelion/Node.h"

#include <algorithm>
#include <experimental/iterator>
#include <iostream>
#include <regex>
#include <sstream>

#define MEM_SIZE 32
#define BASE_SIZE 2

#define DATA_SIZE 64

using namespace std;
using namespace llvm;
using namespace dandelion;
using namespace helpers;
using namespace std::placeholders;

std::string printFloatingPointIEEE754(FloatingPointIEEE754 _number) {
    // TODO: Enable if the following lines if it's needed
    // auto sign = std::bitset<1>(_number.raw.sign);
    // auto exponent = std::bitset<8>(_number.raw.exponent);
    // auto mantissa = std::bitset<23>(_number.raw.mantissa);

    std::stringstream _output;
    _output << "0x" << std::hex << _number.bits;
    return _output.str();
}

//===----------------------------------------------------------------------===//
//                            Node Class
//===----------------------------------------------------------------------===//

PortID Node::addDataInputPort(Node *n) {
    auto _port_info = PortID(port_data.data_input_port.size());
    port_data.data_input_port.emplace_back(std::make_pair(n, _port_info));
    return _port_info;
}

PortID Node::addDataInputPort(Node *n, uint32_t _id) {
    auto _port_info = PortID(_id);
    port_data.data_input_port.emplace_back(std::make_pair(n, _port_info));
    return _port_info;
}

PortID Node::addDataOutputPort(Node *n) {
    auto _port_info = PortID(port_data.data_output_port.size());
    port_data.data_output_port.emplace_back(std::make_pair(n, _port_info));
    return _port_info;
}

PortID Node::addDataOutputPort(Node *n, uint32_t _id) {
    auto _port_info = PortID(_id);
    port_data.data_output_port.emplace_back(std::make_pair(n, _port_info));
    return _port_info;
}

PortID Node::addControlInputPort(Node *n) {
    auto _port_info = PortID(port_control.control_input_port.size());
    port_control.control_input_port.emplace_back(std::make_pair(n, _port_info));
    return _port_info;
}

PortID Node::addControlInputPort(Node *n, uint32_t _id) {
    auto _port_info = PortID(_id);
    port_control.control_input_port.emplace_back(std::make_pair(n, _port_info));
    return _port_info;
}

PortID Node::addControlOutputPort(Node *n) {
    auto _port_info = PortID(port_control.control_output_port.size());
    port_control.control_output_port.emplace_back(
        std::make_pair(n, _port_info));
    return _port_info;
}

PortID Node::addControlOutputPort(Node *n, uint32_t _id) {
    auto _port_info = PortID(_id);
    port_control.control_output_port.emplace_back(
        std::make_pair(n, _port_info));
    return _port_info;
}

PortID Node::addReadMemoryReqPort(Node *const n) {
    auto _port_info = PortID(read_port_data.memory_req_port.size());
    read_port_data.memory_req_port.emplace_back(std::make_pair(n, _port_info));
    return _port_info;
}

PortID Node::addReadMemoryRespPort(Node *const n) {
    auto _port_info = PortID(read_port_data.memory_resp_port.size());
    read_port_data.memory_resp_port.emplace_back(std::make_pair(n, _port_info));
    return _port_info;
}

PortID Node::addWriteMemoryReqPort(Node *const n) {
    auto _port_info = PortID(write_port_data.memory_req_port.size());
    write_port_data.memory_req_port.emplace_back(std::make_pair(n, _port_info));
    return _port_info;
}

PortID Node::addWriteMemoryRespPort(Node *const n) {
    auto _port_info = PortID(write_port_data.memory_resp_port.size());
    write_port_data.memory_resp_port.emplace_back(
        std::make_pair(n, _port_info));
    return _port_info;
}

Node *Node::returnControlOutputPortNode(uint32_t index) {
    auto node = port_control.control_output_port.begin();
    std::advance(node, index);
    return node->first;
}

PortID Node::returnDataOutputPortIndex(Node *_node) {
    auto ff = std::find_if(
        this->port_data.data_output_port.begin(),
        this->port_data.data_output_port.end(),
        [&_node](auto &arg) -> bool { return arg.first == _node; });
    if (ff == this->port_data.data_output_port.end())
        assert(!"Node doesn't exist\n");

    return find_if(this->port_data.data_output_port.begin(),
                   this->port_data.data_output_port.end(),
                   [&_node](auto &arg) -> bool { return arg.first == _node; })
        ->second;
}

PortID Node::returnDataInputPortIndex(Node *_node) {
    auto ff = std::find_if(
        this->port_data.data_input_port.begin(),
        this->port_data.data_input_port.end(),
        [&_node](auto &arg) -> bool { return arg.first == _node; });

    if (ff == this->port_data.data_input_port.end())
        assert(!"Node doesn't exist\n");

    return find_if(this->port_data.data_input_port.begin(),
                   this->port_data.data_input_port.end(),
                   [&_node](auto &arg) -> bool { return arg.first == _node; })
        ->second;
}

PortID Node::returnControlOutputPortIndex(Node *_node) {
    auto ff = std::find_if(
        this->port_control.control_output_port.begin(),
        this->port_control.control_output_port.end(),
        [&_node](auto &arg) -> bool { return arg.first == _node; });

    if (ff == this->port_control.control_output_port.end())
        assert(!"Node doesn't exist\n");

    return ff->second;
}

PortID Node::returnControlInputPortIndex(Node *_node) {
    auto ff =
        find_if(this->port_control.control_input_port.begin(),
                this->port_control.control_input_port.end(),
                [&_node](auto &arg) -> bool { return arg.first == _node; });

    if (ff == this->port_control.control_input_port.end())
        throw std::runtime_error("Input node doesn't exist");

    return ff->second;
}

bool Node::existControlInput(Node *_node) {
    return find_if(this->port_control.control_input_port.begin(),
                   this->port_control.control_input_port.end(),
                   [&_node](auto &arg) -> bool {
                       return arg.first == _node;
                   }) != this->port_control.control_input_port.end();
}

bool Node::existControlOutput(Node *_node) {
    return find_if(this->port_control.control_output_port.begin(),
                   this->port_control.control_output_port.end(),
                   [&_node](auto &arg) -> bool {
                       return arg.first == _node;
                   }) != this->port_control.control_output_port.end();
}

bool Node::existDataInput(Node *_node) {
    return find_if(this->port_data.data_input_port.begin(),
                   this->port_data.data_input_port.end(),
                   [&_node](auto &arg) -> bool {
                       return arg.first == _node;
                   }) != this->port_data.data_input_port.end();
}

bool Node::existDataOutput(Node *_node) {
    return find_if(this->port_data.data_output_port.begin(),
                   this->port_data.data_output_port.end(),
                   [&_node](auto &arg) -> bool {
                       return arg.first == _node;
                   }) != this->port_data.data_output_port.end();
}

// Return memory indexes
PortID Node::returnMemoryReadInputPortIndex(Node *_node) {
    return find_if(this->read_port_data.memory_req_port.begin(),
                   this->read_port_data.memory_req_port.end(),
                   [&_node](auto &arg) -> bool { return arg.first == _node; })
        ->second;
}

PortID Node::returnMemoryReadOutputPortIndex(Node *_node) {
    return find_if(this->read_port_data.memory_resp_port.begin(),
                   this->read_port_data.memory_resp_port.end(),
                   [&_node](auto &arg) -> bool { return arg.first == _node; })
        ->second;
}

PortID Node::returnMemoryWriteInputPortIndex(Node *_node) {
    return find_if(this->write_port_data.memory_req_port.begin(),
                   this->write_port_data.memory_req_port.end(),
                   [&_node](auto &arg) -> bool { return arg.first == _node; })
        ->second;
}

PortID Node::returnMemoryWriteOutputPortIndex(Node *_node) {
    return find_if(this->write_port_data.memory_resp_port.begin(),
                   this->write_port_data.memory_resp_port.end(),
                   [&_node](auto &arg) -> bool { return arg.first == _node; })
        ->second;
}

std::list<PortEntry>::iterator Node::findDataInputNode(Node *_node) {
    return find_if(this->port_data.data_input_port.begin(),
                   this->port_data.data_input_port.end(),
                   [_node](auto &arg) -> bool { return arg.first == _node; });
}

std::list<PortEntry>::iterator Node::findDataOutputNode(Node *_node) {
    return find_if(this->port_data.data_output_port.begin(),
                   this->port_data.data_output_port.end(),
                   [_node](auto &arg) -> bool { return arg.first == _node; });
}

std::list<PortEntry>::iterator Node::findControlInputNode(Node *_node) {
    return find_if(this->port_control.control_input_port.begin(),
                   this->port_control.control_input_port.end(),
                   [_node](auto &arg) -> bool { return arg.first == _node; });
}

std::list<PortEntry>::iterator Node::findControlOutputNode(Node *_node) {
    return find_if(this->port_control.control_output_port.begin(),
                   this->port_control.control_output_port.end(),
                   [_node](auto &arg) -> bool { return arg.first == _node; });
}

std::list<PortEntry> Node::findDataInputNodeList(Node *_node) {
    std::list<PortEntry> result;
    copy_if(this->port_data.data_input_port.begin(),
            this->port_data.data_input_port.end(), std::back_inserter(result),
            [_node](auto &arg) -> bool { return arg.first == _node; });
    return result;
}

std::list<PortEntry> Node::findDataOutputNodeList(Node *_node) {
    std::list<PortEntry> result;
    copy_if(this->port_data.data_output_port.begin(),
            this->port_data.data_output_port.end(), std::back_inserter(result),
            [_node](auto &arg) -> bool { return arg.first == _node; });
    return result;
}

std::list<PortEntry> Node::findControlInputNodeList(Node *_node) {
    std::list<PortEntry> result;
    copy_if(this->port_control.control_input_port.begin(),
            this->port_control.control_input_port.end(),
            std::back_inserter(result),
            [_node](auto &arg) -> bool { return arg.first == _node; });
    return result;
}

std::list<PortEntry> Node::findControlOutputNodeList(Node *_node) {
    std::list<PortEntry> result;
    copy_if(this->port_control.control_output_port.begin(),
            this->port_control.control_output_port.end(),
            std::back_inserter(result),
            [_node](auto &arg) -> bool { return arg.first == _node; });

    return result;
}

void Node::removeNodeDataInputNode(Node *_node) {
    this->port_data.data_input_port.remove_if(
        [_node](auto &arg) -> bool { return arg.first == _node; });
}

void Node::removeNodeDataOutputNode(Node *_node) {
    this->port_data.data_output_port.remove_if(
        [_node](auto &arg) -> bool { return arg.first == _node; });

    // Updating portIDs
    uint32_t _id = 0;
    for (auto &_out_node : this->port_data.data_output_port) {
        _out_node.second.setID(_id++);
    }
}

void Node::removeNodeControlInputNode(Node *_node) {
    this->port_control.control_input_port.remove_if(
        [_node](auto &arg) -> bool { return arg.first == _node; });
}

void Node::removeNodeControlOutputNode(Node *_node) {
    this->port_control.control_output_port.remove_if(
        [_node](auto &arg) -> bool { return arg.first == _node; });
}

void Node::replaceControlInputNode(Node *src, Node *tar) {
    // std::replace(this->port_control.control_input_port.begin(),
    // this->port_control.control_input_port.end(), src, tar);
    auto count =
        std::count_if(port_control.control_input_port.begin(),
                      port_control.control_input_port.end(),
                      [src](auto &arg) -> bool { return arg.first == src; });

    assert(count == 1 &&
           "Can not have multiple edge from one node to another!");

    auto _src_node =
        std::find_if(port_control.control_input_port.begin(),
                     port_control.control_input_port.end(),
                     [src](auto &arg) -> bool { return arg.first == src; });
    _src_node->first = tar;
}

void Node::replaceControlOutputNode(Node *src, Node *tar) {
    // We can't replace if we have multiple edge from src to dst
    // with different port numbers
    // std::replace(port_control.control_output_port.begin(),
    // port_control.control_output_port.end(), src, tar);
    auto count =
        std::count_if(port_control.control_output_port.begin(),
                      port_control.control_output_port.end(),
                      [src](auto &arg) -> bool { return arg.first == src; });

    assert(count == 1 &&
           "Can not have multiple edge from one node to another!");

    auto _src_node =
        std::find_if(port_control.control_output_port.begin(),
                     port_control.control_output_port.end(),
                     [src](auto &arg) -> bool { return arg.first == src; });
    _src_node->first = tar;
}

void Node::replaceDataInputNode(Node *src, Node *tar) {
    auto count = std::count_if(
        port_data.data_input_port.begin(), port_data.data_input_port.end(),
        [src](auto &arg) -> bool { return arg.first == src; });

    assert(count == 1 &&
           "Can not have multiple edge from one node to another!");

    auto _src_node = std::find_if(
        port_data.data_input_port.begin(), port_data.data_input_port.end(),
        [src](auto &arg) -> bool { return arg.first == src; });
    _src_node->first = tar;
}

void Node::replaceDataOutputNode(Node *src, Node *tar) {
    // std::replace(port_data.data_output_port.begin(),
    // port_data.data_output_port.end(), src, tar);
    auto count = std::count_if(
        port_data.data_output_port.begin(), port_data.data_output_port.end(),
        [src](auto &arg) -> bool { return arg.first == src; });

    assert(count == 1 &&
           "Can not have multiple edge from one node to another!");

    auto _src_node = std::find_if(
        port_data.data_output_port.begin(), port_data.data_output_port.end(),
        [src](auto &arg) -> bool { return arg.first == src; });
    _src_node->first = tar;
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

void SuperNode::addconstIntNode(ConstIntNode *node) {
    this->const_int_list.push_back(node);
}

void SuperNode::addconstFPNode(ConstFPNode *node) {
    this->const_fp_list.push_back(node);
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

            switch (this->getNodeType()) {
                case SuperNodeType::NoMask:
                    helperReplace(_text, "$type",
                                  HWoptLevel == '1' ? "BasicBlockNoMaskFastNode"
                                                    : "BasicBlockNoMaskNode");
                    break;
                case SuperNodeType::Mask:
                    _text =
                        "  val $name = Module(new $type("
                        "NumInputs = $num_in, "
                        "NumOuts = "
                        "$num_out, NumPhi = $num_phi, BID = $bid))\n\n";
                    helperReplace(_text, "$type", "BasicBlockNode");
                    break;
            }

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_in", this->numControlInputPort());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numControlOutputPort()));
            helperReplace(_text, "$bid", this->getID());
            helperReplace(_text, "$num_phi", this->getNumPhi());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string SuperNode::printInputEnable(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.predicateIn($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);
            break;
        default:
            break;
    }

    return _text;
}

std::string SuperNode::printInputEnable(PrintType pt,
                                        std::pair<Node *, PortID> _node) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            if (this->getNodeType() == SuperNode::Mask)
                _text = "$name.io.predicateIn($id)";
            else
                _text = HWoptLevel == '1' ? "$name.io.predicateIn($id)"
                                          : "$name.io.predicateIn";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _node.second.getID());

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

std::string SuperNode::printOutputEnable(PrintType pt,
                                         std::pair<Node *, PortID> _node) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _node.second.getID());

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
        case PrintType::Scala: {
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  //Cache\n"
                "  val $name = Module(new $module_type(ID = $id, NumRead = "
                "$num_rd, NumWrite = $num_wr))\n"
                "\n"
                "  io.MemReq <> $name.io.cache.MemReq\n"
                "  $name.io.cache.MemResp <> io.MemResp\n\n";
            ;
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$module_type", "CacheMemoryEngine");
            helperReplace(_text, "$id", std::to_string(this->getID()));
            helperReplace(_text, "$num_rd", this->numReadDataInputPort());
            helperReplace(_text, "$num_wr", this->numWriteDataInputPort());

        } break;
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
            _text = "$name.io.rd.mem($mid).MemReq";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$mid", _id);
            // TODO add mid
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
            _text = "$name.io.rd.mem($mid).MemResp";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$mid", _id);
            // TODO add mid
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
            _text = "$name.io.wr.mem($mid).MemReq";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$mid", _id);
            // TODO add mid
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
            _text = "$name.io.wr.mem($mid).MemResp";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$mid", _id);
            // TODO add mid
            break;
        default:
            break;
    }

    return _text;
}

std::string MemoryNode::printUninitilizedUnit(PrintType _pt) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text =
                "  //Remember if there is no mem operation io memreq/memresp "
                "should be grounded\n"
                "  io.MemReq <> DontCare\n"
                "  io.MemResp <> DontCare\n\n";
            break;
        default:
            break;
    }

    return _text;
}

//===----------------------------------------------------------------------===//
//                            ContainerNode Class
//===----------------------------------------------------------------------===//

ArgumentNode *ContainerNode::insertLiveInArgument(
    llvm::Value *_val, ArgumentNode::ArgumentType _type) {
    switch (con_type) {
        case ContainerNode::LoopNodeTy: {
            auto ff = std::find_if(
                live_in.begin(), live_in.end(), [&_val](auto &arg) -> bool {
                    return arg.get()->getArgumentValue() == _val;
                });
            if (ff == live_in.end()) {
                live_in.push_back(std::make_unique<ArgumentNode>(
                    NodeInfo(live_in.size(), _val->getName().str()), _type,
                    Node::PointerType, this, _val));

                ff = std::find_if(
                    live_in.begin(), live_in.end(), [&_val](auto &arg) -> bool {
                        return arg.get()->getArgumentValue() == _val;
                    });
            }

            return ff->get();
        }
        case SplitCallTy: {
            if (_val->getType()->isPointerTy()) {
                auto ff = std::find_if(
                    live_in_ptrs.begin(), live_in_ptrs.end(),
                    [&_val](auto &arg) -> bool {
                        return arg.get()->getArgumentValue() == _val;
                    });
                if (ff == live_in_ptrs.end()) {
                    live_in_ptrs.push_back(std::make_unique<ArgumentNode>(
                        NodeInfo(live_in_ptrs.size(), _val->getName().str()),
                        _type, Node::PointerType, this, _val));

                    ff = std::find_if(
                        live_in_ptrs.begin(), live_in_ptrs.end(),
                        [&_val](auto &arg) -> bool {
                            return arg.get()->getArgumentValue() == _val;
                        });
                }

                return ff->get();
            } else if (_val->getType()->isStructTy()) {
                assert(!"The input argument type in unkonw!");
            } else {
                auto ff = std::find_if(
                    live_in_vals.begin(), live_in_vals.end(),
                    [&_val](auto &arg) -> bool {
                        return arg.get()->getArgumentValue() == _val;
                    });
                if (ff == live_in_vals.end()) {
                    live_in_vals.push_back(std::make_unique<ArgumentNode>(
                        NodeInfo(live_in_vals.size(), _val->getName().str()),
                        _type, ArgumentNode::IntegerType, this, _val));

                    ff = std::find_if(
                        live_in_vals.begin(), live_in_vals.end(),
                        [&_val](auto &arg) -> bool {
                            return arg.get()->getArgumentValue() == _val;
                        });
                }
                return ff->get();
            }
        }
        default:
            assert(!"Container type is unkonw!");
    }
}

ArgumentNode *ContainerNode::insertLiveOutArgument(
    llvm::Value *_val, ArgumentNode::ArgumentType _type) {
    ArgumentNode::DataType arg_dtype;
    if (_val->getType()->isPointerTy()) {
        arg_dtype = Node::PointerType;
    } else if (_val->getType()->isStructTy()) {
        assert(!"Input argument type is unkonwn");
    } else
        arg_dtype = ArgumentNode::IntegerType;

    auto ff = std::find_if(live_out.begin(), live_out.end(),
                           [&_val](auto &arg) -> bool {
                               return arg.get()->getArgumentValue() == _val;
                           });
    if (ff == live_out.end()) {
        live_out.push_back(std::make_unique<ArgumentNode>(
            NodeInfo(live_out.size(), _val->getName().str()), _type, arg_dtype,
            this, _val));

        ff = std::find_if(live_out.begin(), live_out.end(),
                          [&_val](auto &arg) -> bool {
                              return arg.get()->getArgumentValue() == _val;
                          });
    }

    return ff->get();
}

ArgumentNode *ContainerNode::insertCarryDepenArgument(
    llvm::Value *_val, ArgumentNode::ArgumentType _type) {
    ArgumentNode::DataType arg_dtype;
    if (_val->getType()->isPointerTy()) {
        arg_dtype = Node::PointerType;
    } else if (_val->getType()->isStructTy()) {
        assert(!"Input argument type is unkonwn");
    } else
        arg_dtype = ArgumentNode::IntegerType;

    auto ff = std::find_if(carry_depen.begin(), carry_depen.end(),
                           [&_val](auto &arg) -> bool {
                               return arg.get()->getArgumentValue() == _val;
                           });
    if (ff == carry_depen.end()) {
        carry_depen.push_back(std::make_unique<ArgumentNode>(
            NodeInfo(carry_depen.size(), _val->getName().str()), _type,
            arg_dtype, this, _val));

        ff = std::find_if(carry_depen.begin(), carry_depen.end(),
                          [&_val](auto &arg) -> bool {
                              return arg.get()->getArgumentValue() == _val;
                          });
    }

    return ff->get();
}

Node *ContainerNode::findLiveInNode(llvm::Value *_val) {
    Node *return_ptr = nullptr;

    switch (con_type) {
        case ContainerNode::SplitCallTy: {
            if (_val->getType()->isPointerTy()) {
                auto ff = std::find_if(
                    live_in_ptrs.begin(), live_in_ptrs.end(),
                    [&_val](auto &arg) -> bool {
                        return arg.get()->getArgumentValue() == _val;
                    });
                if (ff == live_in_ptrs.end()) {
                    DEBUG(_val->print(errs(), true));
                    // assert(!"Couldn't find the live-in");
                    return nullptr;
                }

                return_ptr = ff->get();
            } else {
                auto ff = std::find_if(
                    live_in_vals.begin(), live_in_vals.end(),
                    [&_val](auto &arg) -> bool {
                        return arg.get()->getArgumentValue() == _val;
                    });
                if (ff == live_in_vals.end()) {
                    DEBUG(_val->print(errs(), true));
                    // assert(!"Couldn't find the live-in");
                    return nullptr;
                }

                return_ptr = ff->get();
            }
        }
        case ContainerNode::LoopNodeTy: {
            auto ff = std::find_if(
                live_in.begin(), live_in.end(), [&_val](auto &arg) -> bool {
                    return arg.get()->getArgumentValue() == _val;
                });
            if (ff == live_in.end()) {
                DEBUG(_val->print(errs(), true));
                // assert(!"Couldn't find the live-in");
                return nullptr;
            }

            return_ptr = ff->get();
        }
    }

    return return_ptr;
}

Node *ContainerNode::findLiveOutNode(llvm::Value *_val) {
    auto ff = std::find_if(live_out.begin(), live_out.end(),
                           [&_val](auto &arg) -> bool {
                               return arg.get()->getArgumentValue() == _val;
                           });
    if (ff == live_out.end()) {
        DEBUG(_val->print(errs(), true));
        return nullptr;
        // assert(!"Couldn't find the live-in");
    }

    return ff->get();
}

Node *ContainerNode::findCarryDepenNode(llvm::Value *_val) {
    auto ff = std::find_if(carry_depen.begin(), carry_depen.end(),
                           [&_val](auto &arg) -> bool {
                               return arg.get()->getArgumentValue() == _val;
                           });
    if (ff == carry_depen.end()) {
        DEBUG(_val->print(errs(), true));
        return nullptr;
        // assert(!"Couldn't find the live-in");
    }

    return ff->get();
}

uint32_t ContainerNode::findLiveInArgumentIndex(ArgumentNode *_arg_node) {
    auto _arg_type = _arg_node->getArgType();
    auto _arg_data_type = _arg_node->getDataArgType();

    RegisterList _local_list;

    auto find_function = [_arg_type](auto &node) {
        return (node->getArgType() == _arg_type);
    };

    switch (con_type) {
        case ContainerNode::SplitCallTy: {
            if (_arg_data_type == Node::PointerType) {
                std::copy_if(live_in_ptrs.begin(), live_in_ptrs.end(),
                             std::back_inserter(_local_list), find_function);
            } else if (_arg_data_type == Node::IntegerType) {
                std::copy_if(live_in_vals.begin(), live_in_vals.end(),
                             std::back_inserter(_local_list), find_function);
            }
        }
        case ContainerNode::LoopNodeTy: {
            std::copy_if(live_in.begin(), live_in.end(),
                         std::back_inserter(_local_list), find_function);
        }
    }

    auto arg_find = std::find_if(
        _local_list.begin(), _local_list.end(),
        [_arg_node](auto &arg) -> bool { return arg.get() == _arg_node; });

    ptrdiff_t pos = std::distance(_local_list.begin(), arg_find);
    return pos;
}

uint32_t ContainerNode::findLiveOutArgumentIndex(ArgumentNode *_arg_node) {
    auto _arg_type = _arg_node->getArgType();
    RegisterList _local_list;

    auto find_function = [_arg_type](auto &node) {
        return (node->getArgType() == _arg_type);
    };

    std::copy_if(live_out.begin(), live_out.end(),
                 std::back_inserter(_local_list), find_function);

    auto arg_find = std::find_if(
        _local_list.begin(), _local_list.end(),
        [_arg_node](auto &arg) -> bool { return arg.get() == _arg_node; });

    ptrdiff_t pos = std::distance(_local_list.begin(), arg_find);
    return pos;
}

uint32_t ContainerNode::findCarryDepenArgumentIndex(ArgumentNode *_arg_node) {
    auto _arg_type = _arg_node->getArgType();
    RegisterList _local_list;

    auto find_function = [_arg_type](auto &node) {
        return (node->getArgType() == _arg_type);
    };

    std::copy_if(carry_depen.begin(), carry_depen.end(),
                 std::back_inserter(_local_list), find_function);

    auto arg_find = std::find_if(
        _local_list.begin(), _local_list.end(),
        [_arg_node](auto &arg) -> bool { return arg.get() == _arg_node; });

    ptrdiff_t pos = std::distance(_local_list.begin(), arg_find);
    return pos;
}

uint32_t ContainerNode::numLiveInArgList(ArgumentNode::ArgumentType type,
                                         ArgumentNode::DataType dtype) {
    RegisterList _local_list;

    auto find_function = [type](auto &node) {
        return (node->getArgType() == type);
    };

    switch (con_type) {
        case ContainerNode::SplitCallTy: {
            if (dtype == ArgumentNode::PointerType) {
                std::copy_if(live_in_ptrs.begin(), live_in_ptrs.end(),
                             std::back_inserter(_local_list), find_function);
            } else if (dtype == ArgumentNode::IntegerType) {
                std::copy_if(live_in_vals.begin(), live_in_vals.end(),
                             std::back_inserter(_local_list), find_function);
            }
        }
        case ContainerNode::LoopNodeTy: {
            std::copy_if(live_in.begin(), live_in.end(),
                         std::back_inserter(_local_list), find_function);
        }
    }

    return _local_list.size();
}

uint32_t ContainerNode::numLiveOutArgList(ArgumentNode::ArgumentType type) {
    RegisterList _local_list;

    auto find_function = [type](auto &node) {
        return (node->getArgType() == type);
    };

    std::copy_if(live_out.begin(), live_out.end(),
                 std::back_inserter(_local_list), find_function);

    return _local_list.size();
}

uint32_t ContainerNode::numCarryDepenArgList(ArgumentNode::ArgumentType type) {
    RegisterList _local_list;

    auto find_function = [type](auto &node) {
        return (node->getArgType() == type);
    };

    std::copy_if(carry_depen.begin(), carry_depen.end(),
                 std::back_inserter(_local_list), find_function);

    return _local_list.size();
}

//===----------------------------------------------------------------------===//
//                            CallSpliter Class
//===----------------------------------------------------------------------===//

std::string SplitCallNode::printDefinition(PrintType _pt) {
    string _text("");
    string _name(this->getName());

    auto make_argument_port = [](const auto &_list) {
        std::vector<uint32_t> _arg_count;
        for (auto &l : _list) _arg_count.push_back(l->numDataOutputPort());
        return _arg_count;
    };

    switch (_pt) {
        case PrintType::Scala: {
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(ptrsArgTypes = "
                "List($<ptrs_input_vector>), valsArgTypes = "
                "List($<vals_input_vector>)))\n"
                "  $name.io.In <> io.in\n\n";

            helperReplace(_text, "$name", _name);
            helperReplace(_text, "$type", "SplitCallDCR");
            helperReplace(_text, "$id", std::to_string(this->getID()));

            // TODO make a list of liveins first
            auto find_function = [](auto &node) {
                return (node->getArgType() == ArgumentNode::LiveIn);
            };
            RegisterList _local_list_ptrs;
            RegisterList _local_list_vals;
            std::copy_if(live_in_ptrs_begin(), live_in_ptrs_end(),
                         std::back_inserter(_local_list_ptrs), find_function);
            std::copy_if(live_in_vals_begin(), live_in_vals_end(),
                         std::back_inserter(_local_list_vals), find_function);

            helperReplace(_text, "$<ptrs_input_vector>",
                          make_argument_port(_local_list_ptrs), ", ");
            helperReplace(_text, "$<vals_input_vector>",
                          make_argument_port(_local_list_vals), ", ");

            break;
        }

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

//===----------------------------------------------------------------------===//
//                            BranchNode Class
//===----------------------------------------------------------------------===//

void BranchNode::replaceControlOutputNode(Node *src, Node *tar) {
    // We can't replace if we have multiple edge from src to dst
    // with different port numbers
    // std::replace(port_control.control_output_port.begin(),
    // port_control.control_output_port.end(), src, tar);
    auto count =
        std::count_if(outputControl_begin(), outputControl_end(),
                      [src](auto &arg) -> bool { return arg.first == src; });

    assert(count == 1 &&
           "Can not have multiple edge from one node to another!");

    auto _src_node =
        std::find_if(outputControl_begin(), outputControl_end(),
                     [src](auto &arg) -> bool { return arg.first == src; });
    _src_node->first = tar;

    auto _src_predicate =
        std::find_if(output_predicate.begin(), output_predicate.end(),
                     [src](auto &arg) -> bool { return arg.first == src; });
    _src_predicate->first = tar;

    // Make sure ordering works
    output_predicate.splice(output_predicate.end(), output_predicate,
                            _src_predicate);
}

std::string BranchNode::printInputEnable(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.PredOp($id)";
            helperReplace(_text, "$name", _name.c_str());
            // std::cout << _id << "\n";
            // std::cout << this->numControlInputPort() << "\n";
            helperReplace(_text, "$id", _id - 1);
            break;
        default:
            break;
    }

    return _text;
}

std::string BranchNode::printOutputEnable(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            // The branch is UBranch and there is no true and false outptu
            if (this->numDataInputPort() == 0) {
                _text = "$name.io.Out($id)";
                helperReplace(_text, "$name", _name.c_str());
                helperReplace(_text, "$id", _id);
            } else {
                // The branch is CBranch and there is true and false outptut
                auto node = this->returnControlOutputPortNode(_id);
                uint32_t false_index = 0;
                uint32_t true_index = 0;
                for (auto pr : output_predicate) {
                    if (pr.first == node) {
                        auto result =
                            printed_predicate.insert(std::make_pair(pr, 1));
                        if (result.second == false) {
                            if (pr.second == BranchNode::PredicateResult::False)
                                false_index++;
                            else
                                true_index++;
                            continue;
                        } else {
                            if (pr.second ==
                                BranchNode::PredicateResult::True) {
                                _text = "$name.io.TrueOutput($id)";
                                helperReplace(_text, "$name", _name.c_str());
                                helperReplace(_text, "$id", true_index);
                            } else if (pr.second ==
                                       BranchNode::PredicateResult::False) {
                                _text = "$name.io.FalseOutput($id)";
                                helperReplace(_text, "$name", _name.c_str());
                                helperReplace(_text, "$id", false_index);
                            } else {
                                _text = "$name.io.CONDITIONAL?";
                                helperReplace(_text, "$name", _name.c_str());
                            }
                            break;
                        }
                    }

                    if (pr.second == BranchNode::PredicateResult::False)
                        false_index++;
                    else
                        true_index++;
                }
            }
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
            _text = "$name.io.CmpIO";
            helperReplace(_text, "$name", _name.c_str());
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
        case PrintType::Scala: {
            switch (this->getArgType()) {
                case ArgumentNode::LiveIn: {
                    std::replace(_name.begin(), _name.end(), '.', '_');
                    _text = "$call.io.In($id)";
                    helperReplace(_text, "$call",
                                  this->parent_call_node->getName());
                    helperReplace(_text, "$id", _idx);

                    break;
                }
                case ArgumentNode::LiveOut: {
                    std::replace(_name.begin(), _name.end(), '.', '_');
                    _text = "$call.io.InLiveOut($id)";
                    helperReplace(_text, "$call",
                                  this->parent_call_node->getName());
                    helperReplace(_text, "$id", _idx);
                    break;
                }
                case ArgumentNode::LoopLiveIn: {
                    std::replace(_name.begin(), _name.end(), '.', '_');
                    _text = "$call.io.InLiveIn($id)";
                    helperReplace(_text, "$call",
                                  this->parent_call_node->getName());
                    helperReplace(_text, "$id", _idx);

                    break;
                }
                case ArgumentNode::LoopLiveOut: {
                    std::replace(_name.begin(), _name.end(), '.', '_');
                    _text = "$call.io.InLiveOut($id)";
                    helperReplace(_text, "$call",
                                  this->parent_call_node->getName());
                    helperReplace(_text, "$id", _idx);
                    break;
                }
                case ArgumentNode::CarryDependency: {
                    std::replace(_name.begin(), _name.end(), '.', '_');
                    _text = "$call.io.CarryDepenIn($id)";
                    helperReplace(_text, "$call",
                                  this->parent_call_node->getName());
                    helperReplace(_text, "$id", _idx);
                    break;
                }

                default:
                    assert(!"Unrecognized argument node type!");
                    break;
            }

            break;
        }
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
        case PrintType::Scala: {
            switch (this->getArgType()) {
                case ArgumentNode::LiveIn: {
                    std::replace(_name.begin(), _name.end(), '.', '_');
                    _text = "$call.io.$out.$data.elements(\"field$num\")($id)";
                    helperReplace(_text, "$call",
                                  this->parent_call_node->getName());
                    helperReplace(
                        _text, "$num",
                        this->parent_call_node->findLiveInArgumentIndex(this));
                    helperReplace(_text, "$out", "Out");

                    helperReplace(_text, "$id", _idx);

                    if (this->getDataArgType() == ArgumentNode::PointerType)
                        helperReplace(_text, "$data", "dataPtrs");
                    else if (this->getDataArgType() ==
                             ArgumentNode::IntegerType)
                        helperReplace(_text, "$data", "dataVals");

                    break;
                }
                case ArgumentNode::LiveOut: {
                    std::replace(_name.begin(), _name.end(), '.', '_');
                    _text = "$call.io.$out($id)";
                    helperReplace(_text, "$call",
                                  this->parent_call_node->getName());
                    helperReplace(
                        _text, "$num",
                        this->parent_call_node->findLiveOutArgumentIndex(this));
                    helperReplace(_text, "$out", "Out");

                    helperReplace(_text, "$id", _idx);
                    break;
                }
                case ArgumentNode::LoopLiveIn: {
                    std::replace(_name.begin(), _name.end(), '.', '_');
                    _text = "$call.io.$out.elements(\"field$num\")($id)";
                    helperReplace(_text, "$call",
                                  this->parent_call_node->getName());
                    helperReplace(
                        _text, "$num",
                        this->parent_call_node->findLiveInArgumentIndex(this));
                    if (this->parent_call_node->getContainerType() ==
                        ContainerNode::LoopNodeTy)
                        helperReplace(_text, "$out", "OutLiveIn");
                    else
                        helperReplace(_text, "$out", "Out");

                    helperReplace(_text, "$id", _idx);

                    break;
                }
                case ArgumentNode::LoopLiveOut: {
                    std::replace(_name.begin(), _name.end(), '.', '_');
                    _text = "$call.io.$out.elements(\"field$num\")($id)";
                    helperReplace(_text, "$call",
                                  this->parent_call_node->getName());
                    helperReplace(
                        _text, "$num",
                        this->parent_call_node->findLiveOutArgumentIndex(this));
                    helperReplace(_text, "$out", "OutLiveOut");

                    helperReplace(_text, "$id", _idx);
                    break;
                }
                case ArgumentNode::CarryDependency: {
                    std::replace(_name.begin(), _name.end(), '.', '_');
                    _text = "$call.io.$out.elements(\"field$num\")($id)";
                    helperReplace(_text, "$call",
                                  this->parent_call_node->getName());
                    helperReplace(
                        _text, "$num",
                        this->parent_call_node->findCarryDepenArgumentIndex(
                            this));
                    helperReplace(_text, "$out", "CarryDepenOut");

                    helperReplace(_text, "$id", _idx);
                    break;
                }
                default:
                    assert(!"Unrecognized type of node\n");
                    break;
            }

            break;
        }
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
                "$num_out, ID = $id, opCode = \"$opcode\")(sign = false, Debug "
                "= false))\n\n";
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
            if (this->getOpCodeName() == "ashr")
                WARNING("Make sure shift ordering is correct");

            std::replace(_name.begin(), _name.end(), '.', '_');
            if (_idx == 0)
                _text = "$name.io.LeftIO";
            else
                _text = "$name.io.RightIO";
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
//                            FloatAddNode Class
//===----------------------------------------------------------------------===//

std::string FaddOperatorNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumOuts = "
                "$num_out, ID = $id, opCode = \"$opcode\")(fType))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "FPComputeNode");
            helperReplace(_text, "$opcode", this->getOpCodeName());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string FaddOperatorNode::printInputEnable(PrintType _pt) {
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

std::string FaddOperatorNode::printOutputData(PrintType _pt, uint32_t _id) {
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

std::string FaddOperatorNode::printInputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            if (_idx == 0)
                _text = "$name.io.LeftIO";
            else
                _text = "$name.io.RightIO";
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
//                            FloatDiveNode Class
//===----------------------------------------------------------------------===//

std::string FdiveOperatorNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumOuts = "
                "$num_out, ID = $id, RouteID = $route_id, opCode = "
                "\"$opcode\")(fType))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$route_id", this->getRouteID());
            helperReplace(_text, "$type", "FPDivSqrtNode");
            helperReplace(_text, "$opcode", this->getOpCodeName());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string FdiveOperatorNode::printInputEnable(PrintType _pt) {
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

std::string FdiveOperatorNode::printOutputData(PrintType _pt, uint32_t _id) {
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

std::string FdiveOperatorNode::printInputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            if (_idx == 0)
                _text = "$name.io.a";
            else
                _text = "$name.io.b";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string FdiveOperatorNode::printMemReadInput(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.FUResp";
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

std::string FdiveOperatorNode::printMemReadOutput(PrintType _pt,
                                                  uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.FUReq";
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

//===----------------------------------------------------------------------===//
//                            FCMPNode Class
//===----------------------------------------------------------------------===//

std::string FcmpNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumOuts = "
                "$num_out, ID = $id, opCode = \"$opcode\")(fType))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "FPCompareNode");
            helperReplace(_text, "$opcode",
                          this->op_codes[llvm::ICmpInst::getPredicateName(
                              dyn_cast<llvm::FCmpInst>(this->getInstruction())
                                  ->getPredicate())]);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string FcmpNode::printInputEnable(PrintType _pt) {
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

std::string FcmpNode::printInputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            if (_idx == 0)
                _text = "$name.io.LeftIO";
            else
                _text = "$name.io.RightIO";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string FcmpNode::printOutputData(PrintType _pt, uint32_t _id) {
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
//                            ConstantFPNode Class
//===----------------------------------------------------------------------===//

ConstantFP *ConstFPNode::getConstantParent() { return this->parent_const_fp; }

std::string ConstFPNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(value = $valL"
                ", ID = $id))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            // helperReplace(_text, "$num_out",
            // std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "ConstFastNode");
            helperReplace(_text, "$val",
                          printFloatingPointIEEE754(this->getFloatIEEE()));

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ConstFPNode::printOutputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            //_text = "$name.io.Out($id)";
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

std::string ConstFPNode::printInputEnable(PrintType pt) {
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
                "$num_out, ID = $id, opCode = \"$opcode\")(sign = $sign, Debug "
                "= false))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "ComputeNode");
            helperReplace(_text, "$opcode",
                          llvm::ICmpInst::getPredicateName(
                              dyn_cast<llvm::ICmpInst>(this->getInstruction())
                                  ->getSignedPredicate()));

            if (dyn_cast<llvm::ICmpInst>(this->getInstruction())->isSigned())
                helperReplace(_text, "$sign", "true");
            else
                helperReplace(_text, "$sign", "false");

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
                _text = "$name.io.LeftIO";
            else
                _text = "$name.io.RightIO";
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
            if (this->numControlInputPort() > 1 &&
                this->numControlOutputPort() == 1 &&
                this->numDataInputPort() == 0)
                _text =
                    "  val $name = Module(new $type(NumPredOps=$npo, ID = "
                    "$id))\n\n";
            else if (this->numControlInputPort() > 1 &&
                     this->numControlOutputPort() > 1 &&
                     this->numDataInputPort() == 0)
                _text =
                    "  val $name = Module(new $type(NumPredOps=$npo, "
                    "NumOuts=$nout, ID = "
                    "$id))\n\n";
            else if (this->numControlInputPort() == 1 &&
                     this->numControlOutputPort() > 1 &&
                     this->numDataInputPort() == 0)
                _text =
                    "  val $name = Module(new $type(NumOuts=$nout, ID = "
                    "$id))\n\n";
            else
                _text =
                    "  val $name = Module(new $type(ID = "
                    "$id))\n\n";

            if (this->numDataInputPort() > 0) {
                _text =
                    "  val $name = Module(new $type(NumTrue = $true, NumFalse "
                    "= $false, NumPredecessor = $pred, ID = "
                    "$id))\n\n";

                // Getting port index
                uint32_t p_true_index = 0;
                uint32_t p_false_index = 0;
                for (auto _p : output_predicate) {
                    if (_p.second == this->PredicateResult::False)
                        p_false_index++;
                    else if (_p.second == this->PredicateResult::True)
                        p_true_index++;
                }
                helperReplace(_text, "$type",
                              (this->getEndingLoopBranch())
                                  ? "CBranchNodeVariableLoop"
                                  : "CBranchNodeVariable");
                helperReplace(_text, "$false", p_false_index);
                helperReplace(_text, "$true", p_true_index);
                helperReplace(_text, "$pred", this->numControlInputPort() - 1);

            } else
                helperReplace(_text, "$type", "UBranchNode");
            helperReplace(_text, "$nout", this->numControlOutputPort());
            helperReplace(_text, "$npo", this->numControlInputPort() - 1);
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
//                            SelectNode Class
//===----------------------------------------------------------------------===//

std::string SelectNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type("
                "NumOuts = $num_out, ID = $id)(fast = false))\n\n";
            helperReplace(_text, "$type", "SelectNode");
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

std::string SelectNode::printInputEnable(PrintType _pt) {
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

std::string SelectNode::printInputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            if (_id == 0)
                _text = "$name.io.Select";
            else if (_id == 1)
                _text = "$name.io.InData1";
            else if (_id == 2)
                _text = "$name.io.InData2";
            else
                assert(!"Select nod can not have more than three inputs! (select, "
                        "input1, input2)");

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

std::string SelectNode::printOutputData(PrintType _pt, uint32_t _id) {
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
                "NumOutputs = $num_out, ID = $id, Res = $reverse))\n\n";

            helperReplace(_text, "$type", "PhiFastNode");
            helperReplace(_text, "$num_in",
                          std::to_string(this->numDataInputPort()));
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$reverse", this->reverse ? "true" : "false");

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
                "  val $name = Module(new $type(retTypes = "
                "List($<input_list>), "
                "ID = $id))\n\n";
            helperReplace(_text, "$type", "RetNode2");
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$<input_list>",
                          std::vector<uint32_t>(this->numDataInputPort(), 32),
                          ", ");

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
            _text = "$name.io.In.enable";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}


std::string ReturnNode::printInputEnable(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.In.Succ($id)";
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



std::string ReturnNode::printInputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.In.data(\"field$id\")";
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

std::string ReturnNode::printOutputData(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out";
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
//                            LoadNode Class
//===----------------------------------------------------------------------===//

std::string LoadNode::printDefinition(PrintType _pt) {
    string _text("");
    string _name(this->getName());

    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumPredOps = $npo, "
                "NumSuccOps = $nso, "
                "NumOuts = $num_out, ID = $id, RouteID = $rid))\n\n";
            helperReplace(_text, "$type", "UnTypLoadCache");

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$rid", this->getRouteID());
            helperReplace(_text, "$num_out", this->numDataOutputPort());
            helperReplace(_text, "$npo", this->numControlInputPort() - 1);
            helperReplace(_text, "$nso", this->numControlOutputPort());

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
            _text = "$name.io.PredOp($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id - 1);

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
            _text = "$name.io.Out($id)";
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
            _text = "$name.io.GepAddr";

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
            _text = "$name.io.MemResp";

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
            _text = "$name.io.MemReq";
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
                "  val $name = Module(new $type(NumPredOps = $npo, "
                "NumSuccOps = $nso, "
                "ID = $id, RouteID = $rid))\n\n";
            helperReplace(_text, "$type", "UnTypStoreCache");

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$rid", this->getRouteID());
            helperReplace(_text, "$npo", this->numControlInputPort() - 1);
            helperReplace(_text, "$nso", this->numControlOutputPort());

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
            _text = "$name.io.PredOp($id)";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id - 1);

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

std::string StoreNode::printOutputEnable(PrintType pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.SuccOp($id)";
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

std::string StoreNode::printOutputEnable(PrintType pt) {
    string _text;
    string _name(this->getName());
    switch (pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.SuccOp($id)";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
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
                _text = "$name.io.inData";
            else
                _text = "$name.io.GepAddr";
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
            _text = "$name.io.MemResp";

            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string StoreNode::printGround(PrintType _pt) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.Out(0).ready := true.B";

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
            _text = "$name.io.MemReq";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string StoreNode::printOutputData(PrintType _pt, uint32_t _id) {
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

//===----------------------------------------------------------------------===//
//                            ConstantNode Class
//===----------------------------------------------------------------------===//

std::string ConstIntNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(value = $val"
                ", ID = $id))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "ConstFastNode");
            helperReplace(_text, "$val", this->getValue());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ConstIntNode::printOutputData(PrintType _pt, uint32_t _id) {
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

std::string ConstIntNode::printInputEnable(PrintType pt) {
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

//===----------------------------------------------------------------------===//
//                            SextNode Class
//===----------------------------------------------------------------------===//

std::string SextNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "  val $name = Module(new $type(NumOuts = $num_out))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$type", "SextNode");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string SextNode::printInputData(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.Input";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string SextNode::printOutputData(PrintType _pt, uint32_t _id) {
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

std::string SextNode::printInputEnable(PrintType pt) {
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

//===----------------------------------------------------------------------===//
//                            ZextNode Class
//===----------------------------------------------------------------------===//

std::string ZextNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "  val $name = Module(new $type(NumOuts = $num_out))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$type", "ZextNode");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ZextNode::printInputData(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.Input";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string ZextNode::printOutputData(PrintType _pt, uint32_t _id) {
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

std::string ZextNode::printInputEnable(PrintType pt) {
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

//===----------------------------------------------------------------------===//
//                            TruncNode Class
//===----------------------------------------------------------------------===//

std::string TruncNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "  val $name = Module(new $type(NumOuts = $num_out))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$type", "TruncNode");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string TruncNode::printInputData(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.Input";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string TruncNode::printOutputData(PrintType _pt, uint32_t _id) {
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

std::string TruncNode::printInputEnable(PrintType pt) {
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

//===----------------------------------------------------------------------===//
//                            STIoFPNode Class
//===----------------------------------------------------------------------===//

std::string STIoFPNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "  val $name = Module(new $type(NumOuts = $num_out))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$type", "STIoFPNode");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string STIoFPNode::printInputData(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.Input";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string STIoFPNode::printOutputData(PrintType _pt, uint32_t _id) {
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

std::string STIoFPNode::printInputEnable(PrintType pt) {
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

//===----------------------------------------------------------------------===//
//                            FPToUINode Class
//===----------------------------------------------------------------------===//

std::string FPToUINode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "  val $name = Module(new $type(NumOuts = $num_out))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));
            helperReplace(_text, "$type", "FPToUINode");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string FPToUINode::printInputData(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.Input";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string FPToUINode::printOutputData(PrintType _pt, uint32_t _id) {
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

std::string FPToUINode::printInputEnable(PrintType pt) {
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

//===----------------------------------------------------------------------===//
//                            GetElementPtrStruct Class
//===----------------------------------------------------------------------===//
std::string GepNode::printDefinition(PrintType _pt) {
    string _text("");
    string _name(this->getName());

    switch (_pt) {
        case PrintType::Scala: {
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumIns = $num_ins, "
                "NumOuts = $num_out, "
                "ID = $id)(ElementSize = $size, ArraySize = $array))\n\n";
            helperReplace(_text, "$type", "GepNode");
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", std::to_string(this->getID()));
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));

            // The first input is always baseaddress
            helperReplace(_text, "$num_ins",
                          std::to_string(this->numDataInputPort() - 1));

            stringstream _array;
            std::copy(this->gep_info.element_size.begin(),
                      std::prev(this->gep_info.element_size.end()),
                      std::experimental::make_ostream_joiner(_array, ", "));

            /**
             * TODO: Currently our simulation framework only supports 64bit data
             * hence, after initialization we only write 64bit data to the
             * memory even the actual data size 32bit. We need to address this
             * limitation from simulation and then uncomment the follwoing line
             * to use proper elementSize instead of hard-coded version.
             */
            // helperReplace(_text, "$size",
            //*std::prev(this->gep_info.element_size.end()));
            helperReplace(_text, "$size", 8);

            helperReplace(_text, "$array", "List(" + _array.str() + ")");

            break;
        }
        default:
            assert(!"Don't support!");
    }
    return _text;
}

std::string GepNode::printInputEnable(PrintType pt, uint32_t _id) {
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

std::string GepNode::printInputEnable(PrintType pt) {
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

std::string GepNode::printOutputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
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

std::string GepNode::printInputData(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            if (_id == 0) _text = "$name.io.baseAddress";
            // else if (_id == 1){
            //    if(this->numDataInputPort() == 2)
            //        _text = "$name.io.idx";
            //    else
            //        _text = "$name.io.idx1";
            //}
            else
                _text = "$name.io.idx($ix)";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$ix", _id - 1);
            break;
        default:
            break;
    }

    return _text;
}

//===----------------------------------------------------------------------===//
//                            LoopNode Class
//===----------------------------------------------------------------------===//

/**
 * Finding the ending insturctions inside the loop
 */
void LoopNode::setEndingInstructions() {
    // Iterate over the supernodes and then find the store nodes
    // outs() << "AMIRALI\n";
    for (auto &_s_node : this->bblocks()) {
        for (auto &_ins_node : _s_node->instructions()) {
            if (isa<StoreNode>(&*_ins_node)) {
                _ins_node->getInstruction()->dump();
                ending_instructions.push_back(&*_ins_node);
            }
        }
    }
}

std::string LoopNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());

    auto make_argument_port = [](const auto &_list) {
        std::vector<uint32_t> _arg_count;
        for (auto &l : _list) _arg_count.push_back(l->numDataOutputPort());
        // if (_arg_count.size() == 0)
        //_arg_count.push_back(0);
        return _arg_count;
    };

    switch (_pt) {
        case PrintType::Scala: {
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumIns = "
                "List($<input_vector>), "
                "NumOuts = List($<num_out>), "
                "NumCarry = List($<num_carry>), "
                "NumExits = $num_exit, ID = $id))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "LoopBlockNode");
            helperReplace(_text, "$num_exit",
                          static_cast<uint32_t>(this->loop_exits.size()));

            auto live_in_args = make_argument_port(live_in_lists());
            helperReplace(_text, "$<input_vector>", live_in_args, ", ");

            helperReplace(_text, "$<num_out>",
                          make_argument_port(live_out_lists()), ", ");

            helperReplace(_text, "$<num_carry>",
                          make_argument_port(carry_depen_lists()), ", ");

            break;
        }
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
            _text = "$name.io.Out.endEnable";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string LoopNode::printOutputEnable(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    auto node = this->returnControlOutputPortNode(_id);
    auto node_t =
        find_if(port_type.begin(), port_type.end(),
                [node](auto _nt) -> bool { return _nt.first == node; });

    switch (_pt) {
        case PrintType::Scala:
            if (node_t->second == PortType::LoopFinish)
                _text = "$name.io.loopfinish";
            else if (node_t->second == PortType::Active_Loop_Start)
                _text = "$name.io.activate_loop_start";
            else if (node_t->second == PortType::Enable)
                _text = "$name.io.???";
            //_text = "UKNOWN";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string LoopNode::printOutputEnable(PrintType _pt, PortEntry _port) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;

    auto port_equal = [](auto port_1, auto port_2) -> bool {
        return ((port_1.first == port_2.first) &&
                (port_1.second.getID() == port_2.second.getID()));
    };

    switch (_pt) {
        case PrintType::Scala:
            if (port_equal(this->activate_loop_start, _port))
                _text = "$name.io.activate_loop_start";
            else if (port_equal(this->activate_loop_back, _port))
                _text = "$name.io.activate_loop_back";
            else {
                auto out_port =
                    find_if(this->loop_exits.begin(), this->loop_exits.end(),
                            std::bind(port_equal, _1, _port));

                if (out_port != this->loop_exits.end()) {
                    _text = "$name.io.loopExit($id)";
                    uint32_t pos =
                        std::distance(this->loop_exits.begin(), out_port);
                    helperReplace(_text, "$id", pos);

                } else
                    _text = "$name.io.XXX";
            }

            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string LoopNode::printInputEnable(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            if (_id == 0)
                _text = "$name.io.enable";
            else if (_id == 1)
                _text = "$name.io.loopBack(0)";
            else if (_id >= 2)
                _text = "$name.io.loopFinish($id)";
            else
                _text = "XXXXXX";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id - 2);

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            ReattachNode Class
//===----------------------------------------------------------------------===//
std::string ReattachNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumPredOps= "
                "$num_out, ID = $id))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            if (this->numDataInputPort() == 0)
                helperReplace(_text, "$num_out", std::to_string(1));
            else
                helperReplace(_text, "$num_out",
                              std::to_string(this->numDataInputPort()));
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "Reattach");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ReattachNode::printInputEnable(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.enable.enq(ControlBundle.active())";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ReattachNode::printInputEnable(PrintType _pt, uint32_t _id) {
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

std::string ReattachNode::printOutputEnable(PrintType _pt, uint32_t _id) {
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

std::string ReattachNode::printInputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
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

std::string ReattachNode::printGround(PrintType _pt) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.predicateIn(0).enq(DataBundle.active(1.U))";

            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

//===----------------------------------------------------------------------===//
//                            DeatachNode Class
//===----------------------------------------------------------------------===//
std::string DetachNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "  val $name = Module(new $type(ID = $id))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "Detach");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string DetachNode::printInputEnable(PrintType _pt) {
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

std::string DetachNode::printOutputEnable(PrintType pt, uint32_t _id) {
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

//===----------------------------------------------------------------------===//
//                            DeattachNode Class
//===----------------------------------------------------------------------===//
std::string SyncNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(ID = $id, NumInc=$num_inc, "
                "NumDec=$num_dec, NumOuts=$num_out))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "SyncTC");

            // TODO add special port for increase and decrease
            helperReplace(_text, "$num_inc", 1);
            helperReplace(_text, "$num_dec", 1);

            if (this->numDataOutputPort() == 0)
                helperReplace(_text, "$num_out", 1);
            else
                helperReplace(_text, "$num_out", this->numDataOutputPort());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string SyncNode::printInputEnable(PrintType _pt) {
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

std::string SyncNode::printInputEnable(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            if (_id == 1)
                _text = "$name.io.incIn(0)";
            else if (_id == 2)
                _text = "$name.io.decIn(0)";
            else
                assert(
                    !"Sync node can not have more than three control inputs!");
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string SyncNode::printOutputEnable(PrintType pt, uint32_t _id) {
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

//===----------------------------------------------------------------------===//
//                            AllocaNode Class
//===----------------------------------------------------------------------===//
//
std::string AllocaNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumOuts=$num_out, ID = $id"
                "))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$num_out", this->numDataOutputPort());
            helperReplace(_text, "$type", "AllocaConstNode");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string AllocaNode::printInputData(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.CmpIO";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

std::string AllocaNode::printOutputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
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

std::string AllocaNode::printInputEnable(PrintType _pt) {
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

std::string AllocaNode::printInputEnable(PrintType _pt, uint32_t _id) {
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

std::string AllocaNode::printMemReadInput(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.allocaRespIO";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string AllocaNode::printMemReadOutput(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.allocaReqIO";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string AllocaNode::printOffset(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  $name.io.allocaInputIO.bits.size      := "
                "$size.U\n"
                "  $name.io.allocaInputIO.bits.numByte   := "
                "$num_byte.U\n"
                "  $name.io.allocaInputIO.bits.predicate := "
                "true.B\n"
                "  $name.io.allocaInputIO.bits.valid     := "
                "true.B\n"
                "  $name.io.allocaInputIO.valid          := "
                "true.B\n\n";

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$num_byte", getNumByte());
            helperReplace(_text, "$size", getSize());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

//===----------------------------------------------------------------------===//
//                            CallNode Class
//===----------------------------------------------------------------------===//

void CallNode::setCallOutEnable(Node *_n) { call_out->addControlInputPort(_n); }

//===----------------------------------------------------------------------===//
//                            CallInNode Class
//===----------------------------------------------------------------------===//
std::string CallInNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());

    auto make_argument_port = [](const auto &_list) {
        std::vector<uint32_t> _arg_count;
        for (auto &l : _list) {
            _arg_count.push_back(64);
            DEBUG(errs() << "WRONG WRONG\n");
        }
        return _arg_count;
    };

    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(ID = $id"
                ", argTypes = List($<output_vector>)))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "CallInNode");
            helperReplace(_text, "$<output_vector>",
                          make_argument_port(this->output_data_range()), ",");

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string CallInNode::printOutputData(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out.data(\"field$id\")";
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

std::string CallInNode::printOutputEnable(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out.enable.ready := true.B";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string CallInNode::printOutputEnable(PrintType _pt, uint32_t _id) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out.enable";
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
//                            CallOutNode Class
//===----------------------------------------------------------------------===//
std::string CallOutNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());

    auto make_argument_port = [](const auto &_list, Node::DataType type) {
        std::vector<uint32_t> _arg_count;
        for (auto &l : _list) {
            _arg_count.push_back(DATA_SIZE);
        }
        return _arg_count;
    };

    switch (_pt) {
        case PrintType::Scala: {
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(ID = $id"
                ", NumSuccOps = $num_succ, PtrsTypes = "
                "List($<input_ptr_vector>), "
                "ValsTypes = List($<input_val_vector>)))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$type", "CallOutDCRNode");
            helperReplace(_text, "$num_succ", this->numControlOutputPort());

            uint32_t num_ptrs = 0;
            uint32_t num_vals = 0;

            for (auto ins : this->input_data_range()) {
                if (auto instruction_node =
                        dyn_cast<InstructionNode>(ins.first)) {
                    if (instruction_node->getType() == Node::IntegerType ||
                        Node::FloatType)
                        num_vals++;
                    else if (instruction_node->getType() == Node::PointerType)
                        num_ptrs++;
                    else {
                        std::cout << instruction_node->getName() << "\n";
                        throw std::runtime_error("Input datatype is Uknown");
                    }
                }
            }

            helperReplace(_text, "$<input_ptr_vector>",
                          std::vector<uint32_t>(num_ptrs, DATA_SIZE), ",");
            helperReplace(_text, "$<input_val_vector>",
                          std::vector<uint32_t>(num_vals, DATA_SIZE), ",");

            // helperReplace(_text, "$num_out", this->numDataOutputPort());

            break;
        }
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string CallOutNode::printInputEnable(PrintType pt) {
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

std::string CallOutNode::printInputData(PrintType _pt, uint32_t _id) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala: {
            auto iter = this->inputDataport_begin();
            std::advance(iter, _id);
            if (auto instr = dyn_cast<InstructionNode>(&*iter->first)) {
                if (instr->getDataType() == Node::PointerType)
                    _text = "$name.io.inPtrs.elements(\"field$id\")";
                else
                    _text = "$name.io.inVals.elements(\"field$id\")";
            } else if (auto arg = dyn_cast<ArgumentNode>(&*iter->first)) {
                if (arg->getDataArgType() == Node::PointerType)
                    _text = "$name.io.inPtrs.elements(\"field$id\")";
                else
                    _text = "$name.io.inVals.elements(\"field$id\")";
            } else {
                std::cout << instr->getName() << "\n";
                throw std::runtime_error("Input datatype is Uknown");
            }

            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$id", _id);
            break;
        }
        default:
            break;
    }

    return _text;
}

std::string CallOutNode::printOutputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Out($id)";
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

//===----------------------------------------------------------------------===//
//                            CallInNode Class
//===----------------------------------------------------------------------===//
std::string CallInNode::printInputEnable(PrintType pt) {
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

std::string CallInNode::printInputData(PrintType _pt) {
    string _name(this->getName());
    std::replace(_name.begin(), _name.end(), '.', '_');
    string _text;
    switch (_pt) {
        case PrintType::Scala:
            _text = "$name.io.In";
            helperReplace(_text, "$name", _name.c_str());
            break;
        default:
            break;
    }

    return _text;
}

//===----------------------------------------------------------------------===//
//                            ScratchpadNode Class
//===----------------------------------------------------------------------===//
std::string ScratchpadNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  //$name"
                "\n  val $name = Module(new CacheMemoryEngine(ID = $id, "
                "NumRead = $num_read, NumWrite = $num_write))\n\n"
                "  $alloca_mem_req <> $name.io.cache.MemReq\n"
                "  $name.io.cache.MemResp <> $alloca_mem_resp\n\n";

            helperReplace(_text, "$id", this->getID());
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$alloca", this->getAllocaNode()->getName());
            helperReplace(_text, "$num_read", this->numReadDataInputPort());
            helperReplace(_text, "$num_write", this->numWriteDataInputPort());
            helperReplace(_text, "$size", this->getMemSize());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string ScratchpadNode::printMemReadInput(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.rd.mem($id).MemReq";
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

std::string ScratchpadNode::printMemReadOutput(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.rd.mem($id).MemResp";
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

std::string ScratchpadNode::printMemWriteInput(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.wr.mem($id).MemReq";
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

std::string ScratchpadNode::printMemWriteOutput(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.wr.mem($id).MemResp";
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

//===----------------------------------------------------------------------===//
//                            FloatingPointNode Class
//===----------------------------------------------------------------------===//
std::string FloatingPointNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val SharedFPU = Module(new SharedFPU(NumOps = $op, "
                "PipeDepth = 32)(fType))\n\n";
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$op", this->numReadDataInputPort());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string FloatingPointNode::printMemReadInput(PrintType _pt, uint32_t _idx) {
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

std::string FloatingPointNode::printMemReadOutput(PrintType _pt,
                                                  uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.OutData($id)";
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

//===----------------------------------------------------------------------===//
//                            BitCastNode Class
//===----------------------------------------------------------------------===//

std::string BitcastNode::printDefinition(PrintType _pt) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text =
                "  val $name = Module(new $type(NumOuts = "
                "$num_out, ID = $id))\n\n";
            helperReplace(_text, "$id", std::to_string(this->getID()));
            helperReplace(_text, "$name", _name.c_str());
            helperReplace(_text, "$type", "BitCastNode");
            helperReplace(_text, "$num_out",
                          std::to_string(this->numDataOutputPort()));

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}

std::string BitcastNode::printInputEnable(PrintType _pt) {
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

std::string BitcastNode::printOutputData(PrintType _pt, uint32_t _id) {
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

std::string BitcastNode::printInputData(PrintType _pt, uint32_t _idx) {
    string _text;
    string _name(this->getName());
    switch (_pt) {
        case PrintType::Scala:
            std::replace(_name.begin(), _name.end(), '.', '_');
            _text = "$name.io.Input";
            helperReplace(_text, "$name", _name.c_str());

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
    return _text;
}
