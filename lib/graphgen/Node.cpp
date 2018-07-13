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

std::string printFloatingPointIEEE754(FloatingPointIEEE754 _number) {
  auto sign = std::bitset<1>(_number.raw.sign);
  auto exponent = std::bitset<8>(_number.raw.exponent);
  auto mantissa = std::bitset<23>(_number.raw.mantissa);

  std::stringstream _output;
  _output << "0x" << std::hex << _number.bits;
  return _output.str();
}

//===----------------------------------------------------------------------===//
//                            Node Class
//===----------------------------------------------------------------------===//

PortID Node::addDataInputPort(Node *n) {
  auto _port_info = PortID(port_data.data_input_port.size());
  port_data.data_input_port.emplace_back(n);
  return _port_info;
}
PortID Node::addDataOutputPort(Node *n) {
  auto _port_info = PortID(port_data.data_output_port.size());
  port_data.data_output_port.emplace_back(n);
  return _port_info;
}

PortID Node::addControlInputPort(Node *n) {
  auto _port_info = PortID(port_control.control_input_port.size());
  port_control.control_input_port.emplace_back(n);
  return _port_info;
}
PortID Node::addControlOutputPort(Node *n) {
  auto _port_info = PortID(port_control.control_output_port.size());
  port_control.control_output_port.emplace_back(n);
  return _port_info;
}

PortID Node::addReadMemoryReqPort(Node *const n) {
  auto _port_info = PortID(read_port_data.memory_req_port.size());
  read_port_data.memory_req_port.emplace_back(n);
  return _port_info;
}
PortID Node::addReadMemoryRespPort(Node *const n) {
  auto _port_info = PortID(read_port_data.memory_resp_port.size());
  read_port_data.memory_resp_port.emplace_back(n);
  return _port_info;
}

PortID Node::addWriteMemoryReqPort(Node *const n) {
  auto _port_info = PortID(write_port_data.memory_req_port.size());
  write_port_data.memory_req_port.emplace_back(n);
  return _port_info;
}
PortID Node::addWriteMemoryRespPort(Node *const n) {
  auto _port_info = PortID(write_port_data.memory_resp_port.size());
  write_port_data.memory_resp_port.emplace_back(n);
  return _port_info;
}

PortID Node::returnDataOutputPortIndex(Node *_node) {
  auto ff = std::find_if(this->port_data.data_output_port.begin(),
                         this->port_data.data_output_port.end(),
                         [&_node](auto &arg) -> bool { return arg == _node; });
  if (ff == this->port_data.data_output_port.end())
    assert(!"Node doesn't exist\n");

  return std::distance(this->port_data.data_output_port.begin(),
                       find(this->port_data.data_output_port.begin(),
                            this->port_data.data_output_port.end(), _node));
}

PortID Node::returnDataInputPortIndex(Node *_node) {
  auto ff = std::find_if(this->port_data.data_input_port.begin(),
                         this->port_data.data_input_port.end(),
                         [&_node](auto &arg) -> bool { return arg == _node; });

  if (ff == this->port_data.data_input_port.end())
    assert(!"Node doesn't exist\n");
  return std::distance(this->port_data.data_input_port.begin(),
                       find(this->port_data.data_input_port.begin(),
                            this->port_data.data_input_port.end(), _node));
}

PortID Node::returnControlOutputPortIndex(Node *_node) {
  return std::distance(this->port_control.control_output_port.begin(),
                       find(this->port_control.control_output_port.begin(),
                            this->port_control.control_output_port.end(),
                            _node));
}

PortID Node::returnControlInputPortIndex(Node *_node) {
  return std::distance(this->port_control.control_input_port.begin(),
                       find(this->port_control.control_input_port.begin(),
                            this->port_control.control_input_port.end(),
                            _node));
}

bool Node::existControlInput(Node *_n) {
  return find(this->port_control.control_input_port.begin(),
              this->port_control.control_input_port.end(),
              _n) != this->port_control.control_input_port.end();
}

bool Node::existControlOutput(Node *_n) {
  return find(this->port_control.control_output_port.begin(),
              this->port_control.control_output_port.end(),
              _n) != this->port_control.control_output_port.end();
}

bool Node::existDataInput(Node *_n) {
  return find(this->port_data.data_input_port.begin(),
              this->port_data.data_input_port.end(),
              _n) != this->port_data.data_input_port.end();
}

bool Node::existDataOutput(Node *_n) {
  return find(this->port_data.data_output_port.begin(),
              this->port_data.data_output_port.end(),
              _n) != this->port_data.data_output_port.end();
}

// Return memory indexes
PortID Node::returnMemoryReadInputPortIndex(Node *_node) {
  return std::distance(this->read_port_data.memory_req_port.begin(),
                       find(this->read_port_data.memory_req_port.begin(),
                            this->read_port_data.memory_req_port.end(), _node));
}

PortID Node::returnMemoryReadOutputPortIndex(Node *_node) {
  return std::distance(this->read_port_data.memory_resp_port.begin(),
                       find(this->read_port_data.memory_resp_port.begin(),
                            this->read_port_data.memory_resp_port.end(),
                            _node));
}

PortID Node::returnMemoryWriteInputPortIndex(Node *_node) {
  return std::distance(this->write_port_data.memory_req_port.begin(),
                       find(this->write_port_data.memory_req_port.begin(),
                            this->write_port_data.memory_req_port.end(),
                            _node));
}

PortID Node::returnMemoryWriteOutputPortIndex(Node *_node) {
  return std::distance(this->write_port_data.memory_resp_port.begin(),
                       find(this->write_port_data.memory_resp_port.begin(),
                            this->write_port_data.memory_resp_port.end(),
                            _node));
}

std::list<Node *>::iterator Node::findDataInputNode(Node *_node) {
  return find(this->port_data.data_input_port.begin(),
              this->port_data.data_input_port.end(), _node);
}

std::list<Node *>::iterator Node::findDataOutputNode(Node *_node) {
  return find(this->port_data.data_output_port.begin(),
              this->port_data.data_output_port.end(), _node);
}

std::list<Node *>::iterator Node::findControlInputNode(Node *_node) {
  return find(this->port_control.control_input_port.begin(),
              this->port_control.control_input_port.end(), _node);
}

std::list<Node *>::iterator Node::findControlOutputNode(Node *_node) {
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

void Node::replaceControlInputNode(Node *src, Node *tar) {
  std::replace(this->port_control.control_input_port.begin(),
               this->port_control.control_input_port.end(), src, tar);
}

void Node::replaceControlOutputNode(Node *src, Node *tar) {
  std::replace(port_control.control_output_port.begin(),
               port_control.control_output_port.end(), src, tar);
}

void Node::replaceDataInputNode(Node *src, Node *tar) {
  std::replace(this->port_data.data_input_port.begin(),
               this->port_data.data_input_port.end(), src, tar);
}

void Node::replaceDataOutputNode(Node *src, Node *tar) {
  std::replace(port_data.data_output_port.begin(),
               port_data.data_output_port.end(), src, tar);
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
    _text = "  val $name = Module(new $type(NumInputs = $num_in, NumOuts = "
            "$num_out, BID = $bid))\n\n";

    switch (this->getNodeType()) {
    case SuperNodeType::NoMask:
      helperReplace(_text, "$type", "BasicBlockNoMaskNode");
      break;
    case SuperNodeType::Mask:
      _text = "  val $name = Module(new $type("
              "NumInputs = $num_in, "
              "NumOuts = "
              "$num_out, NumPhi=$num_phi, BID = $bid))\n\n";
      helperReplace(_text, "$type", "BasicBlockNode");
      break;
    case SuperNodeType::LoopHead:
      _text = "  val $name = Module(new $type("
              "NumOuts = "
              "$num_out, NumPhi=$num_phi, BID = $bid))\n\n";
      helperReplace(_text, "$type", "LoopHead");
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

std::string SuperNode::printInputEnable(PrintType pt, uint32_t _id) {
  string _text;
  string _name(this->getName());
  switch (pt) {
  case PrintType::Scala:
    std::replace(_name.begin(), _name.end(), '.', '_');
    if (this->getNodeType() == SuperNode::LoopHead && _id == 0)
      _text = "$name.io.activate";
    else if (this->getNodeType() == SuperNode::LoopHead && _id == 1)
      _text = "$name.io.loopBack";
    else if (this->getNodeType() == SuperNode::Mask)
      _text = "$name.io.predicateIn($id)";
    else
      _text = "$name.io.predicateIn";

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
  case PrintType::Scala: {
    std::replace(_name.begin(), _name.end(), '.', '_');
    _text = "  val $name = Module(new $reg_type(ID=$id, Size=$size, "
            "NReads=$num_read, NWrites=$num_write)\n"
            "\t\t (WControl=new "
            "WriteMemoryController(NumOps=$write_num_op, "
            "BaseSize=$read_base_size, NumEntries=$read_num_entries))\n"
            "\t\t (RControl=new ReadMemoryController(NumOps=$read_num_op, "
            "BaseSize=$write_base_size, "
            "NumEntries=$write_num_entries))\n"
            "\t\t (RWArbiter=new ReadWriteArbiter()))"
            "\n\n"
            "  io.MemReq <> $name.io.MemReq\n"
            "  $name.io.MemResp <> io.MemResp\n\n";
    ;
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$reg_type", "UnifiedController");
    helperReplace(_text, "$id", std::to_string(this->getID()));

    auto returnMinimumPort = [](auto _num, uint32_t _base) {
      if (_num > _base)
        return _num;
      else
        return _base;
    };

    // TODO this part can be parametrize using config file
    helperReplace(_text, "$size", MEM_SIZE);
    helperReplace(_text, "$num_read",
                  returnMinimumPort(this->numReadDataInputPort(), BASE_SIZE));
    helperReplace(_text, "$num_write",
                  returnMinimumPort(this->numWriteDataInputPort(), BASE_SIZE));
    helperReplace(_text, "$read_num_op",
                  returnMinimumPort(this->numReadDataInputPort(), BASE_SIZE));
    helperReplace(_text, "$read_base_size", BASE_SIZE);
    helperReplace(_text, "$read_num_entries", BASE_SIZE);
    helperReplace(_text, "$write_num_op",
                  returnMinimumPort(this->numWriteDataOutputPort(), BASE_SIZE));
    helperReplace(_text, "$write_base_size", BASE_SIZE);
    helperReplace(_text, "$write_num_entries", BASE_SIZE);

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
    _text = "$name.io.ReadIn($mid)";

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
    _text = "$name.io.ReadOut($mid)";

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
    _text = "$name.io.WriteIn($mid)";

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
    _text = "$name.io.WriteOut($mid)";

    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$mid", _id);
    // TODO add mid
    break;
  default:
    break;
  }

  return _text;
}

//===----------------------------------------------------------------------===//
//                            ContainerNode Class
//===----------------------------------------------------------------------===//

Node *ContainerNode::findLiveIn(llvm::Value *_val) {
  auto ff =
      std::find_if(live_in.begin(), live_in.end(), [&_val](auto &arg) -> bool {
        return arg.get()->getArgumentValue() == _val;
      });
  if (ff == live_in.end()) {
    DEBUG(_val->print(errs(), true));
    WARNING(!"Couldn't find the live-in");
    return nullptr;
    // assert(!"Couldn't find the live-in");
  }

  return ff->get();
}

Node *ContainerNode::findLiveOut(llvm::Value *_val) {
  auto ff = std::find_if(live_out.begin(), live_out.end(),
                         [&_val](auto &arg) -> bool {
                           return arg.get()->getArgumentValue() == _val;
                         });

  if (ff == live_out.end()) {
    WARNING(!"Couldn't find the live-in");
    return nullptr;
  }
  return ff->get();
}

ArgumentNode *ContainerNode::insertLiveInArgument(llvm::Value *_val) {
  auto ff =
      std::find_if(live_in.begin(), live_in.end(), [&_val](auto &arg) -> bool {
        return arg.get()->getArgumentValue() == _val;
      });
  if (ff == live_in.end()) {
    live_in.push_back(std::make_unique<ArgumentNode>(
        NodeInfo(live_in.size(), _val->getName().str()), ArgumentNode::LiveIn,
        this, _val));

    ff = std::find_if(live_in.begin(), live_in.end(),
                      [&_val](auto &arg) -> bool {
                        return arg.get()->getArgumentValue() == _val;
                      });
  }

  return ff->get();
}

ArgumentNode *ContainerNode::insertLiveOutArgument(llvm::Value *_val) {
  auto ff = std::find_if(live_out.begin(), live_out.end(),
                         [&_val](auto &arg) -> bool {
                           return arg.get()->getArgumentValue() == _val;
                         });
  if (ff == live_out.end()) {
    live_out.push_back(std::make_unique<ArgumentNode>(
        NodeInfo(live_out.size(), _val->getName().str()), ArgumentNode::LiveOut,
        this, _val));

    ff = std::find_if(live_out.begin(), live_out.end(),
                      [&_val](auto &arg) -> bool {
                        return arg.get()->getArgumentValue() == _val;
                      });
  }

  return ff->get();
}

uint32_t ContainerNode::findLiveInIndex(ArgumentNode *_arg_node) {
  uint32_t c = 0;
  for (auto &_a : live_in) {
    if (_a->getName() == _arg_node->getName())
      return c;
    c++;
  }
  return c;
}

uint32_t ContainerNode::findLiveOutIndex(ArgumentNode *_arg_node) {
  uint32_t c = 0;
  for (auto &_a : live_out) {
    if (_a->getName() == _arg_node->getName())
      return c;
    c++;
  }
  return c;
}

//===----------------------------------------------------------------------===//
//                            CallSpliter Class
//===----------------------------------------------------------------------===//

std::string SplitCallNode::printDefinition(PrintType _pt) {
  string _text("");
  string _name(this->getName());

  auto make_argument_port = [](const auto &_list) {
    std::vector<uint32_t> _arg_count;
    for (auto &l : _list)
      _arg_count.push_back(l->numDataOutputPort());
    return _arg_count;
  };

  switch (_pt) {
  case PrintType::Scala:

    std::replace(_name.begin(), _name.end(), '.', '_');
    _text = "  val $name = Module(new $type(List($<input_vector>)))\n"
            "  $name.io.In <> io.in\n\n";

    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$type", "SplitCallNew");
    helperReplace(_text, "$id", std::to_string(this->getID()));
    helperReplace(_text, "$<input_vector>",
                  make_argument_port(this->live_ins()), ",");
    // TODO: uncomment if you update the list shape.

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

//===----------------------------------------------------------------------===//
//                            BranchNode Class
//===----------------------------------------------------------------------===//

std::string BranchNode::printInputEnable(PrintType _pt, uint32_t _id) {
  string _name(this->getName());
  std::replace(_name.begin(), _name.end(), '.', '_');
  string _text;
  switch (_pt) {
  case PrintType::Scala:
    _text = "$name.io.PredOp($id)";
    helperReplace(_text, "$name", _name.c_str());
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
    _text = "  val $name = Module(new $type(NumOuts = "
            "$num_out, ID = $id))\n\n";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$num_out", std::to_string(this->numDataOutputPort()));
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
      helperReplace(_text, "$call", this->parent_call_node->getName());
      helperReplace(_text, "$id", _idx);

      break;
    }
    case ArgumentNode::LiveOut: {
      std::replace(_name.begin(), _name.end(), '.', '_');
      _text = "$call.io.liveOut($id)";
      helperReplace(_text, "$call", this->parent_call_node->getName());
      helperReplace(_text, "$id", _idx);
      break;
    }
    case ArgumentNode::FunctionArgument: {
      std::replace(_name.begin(), _name.end(), '.', '_');
      _text = "$call.io.liveOut($id)";
      helperReplace(_text, "$call", this->parent_call_node->getName());
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
      _text = "$call.io.$out.data(\"field$num\")($id)";
      helperReplace(_text, "$call", this->parent_call_node->getName());
      helperReplace(_text, "$num",
                    this->parent_call_node->findLiveInIndex(this));
      if (this->parent_call_node->getContainerType() ==
          ContainerNode::LoopNodeTy)
        helperReplace(_text, "$out", "liveIn");
      else
        helperReplace(_text, "$out", "Out");

      helperReplace(_text, "$id", _idx);

      break;
    }
    case ArgumentNode::LiveOut: {
      std::replace(_name.begin(), _name.end(), '.', '_');
      _text = "$call.io.$out($id)";
      helperReplace(_text, "$call", this->parent_call_node->getName());
      helperReplace(_text, "$num",
                    this->parent_call_node->findLiveInIndex(this));
      helperReplace(_text, "$out", "Out");

      helperReplace(_text, "$id", _idx);
      break;
    }
    case ArgumentNode::FunctionArgument: {
      std::replace(_name.begin(), _name.end(), '.', '_');
      _text = "$call.io.$out.data(\"field$id\")($id)";
      helperReplace(_text, "$call", this->parent_call_node->getName());
      helperReplace(_text, "$num",
                    this->parent_call_node->findLiveInIndex(this));
      helperReplace(_text, "$out", "Out");

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
    _text = "  val $name = Module(new $type(NumOuts = "
            "$num_out, ID = $id, opCode = \"$opcode\")(sign=false))\n\n";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$num_out", std::to_string(this->numDataOutputPort()));
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
    _text = "  val $name = Module(new $type(NumOuts = "
            "$num_out, ID = $id, opCode = \"$opcode\")(t = S))\n\n";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$num_out", std::to_string(this->numDataOutputPort()));
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
    _text = "  val $name = Module(new $type(NumOuts = "
            "$num_out, ID = $id, opCode = \"$opcode\", $route_id)(t = "
            "S))\n\n";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$num_out", std::to_string(this->numDataOutputPort()));
    helperReplace(_text, "$id", this->getID());
    helperReplace(_text, "$route_id", 0);
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
    _text = "  val $name = Module(new $type(NumOuts = "
            "$num_out, ID = $id, opCode = \"$opcode\")(sign=false))\n\n";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$num_out", std::to_string(this->numDataOutputPort()));
    helperReplace(_text, "$id", this->getID());
    helperReplace(_text, "$type", "FPCompareNode");
    helperReplace(
        _text, "$opcode",
        llvm::ICmpInst::getPredicateName(
            dyn_cast<llvm::FCmpInst>(this->getInstruction())->getPredicate()));

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
    _text = "  val $name = Module(new $type(value = $val"
            ", NumOuts = $num_out, ID = $id))\n\n";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$num_out", std::to_string(this->numDataOutputPort()));
    helperReplace(_text, "$id", this->getID());
    helperReplace(_text, "$type", "ConstNode");
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
    _text = "  val $name = Module(new $type(NumOuts = "
            "$num_out, ID = $id, opCode = \"$opcode\")(sign=false))\n\n";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$num_out", std::to_string(this->numDataOutputPort()));
    helperReplace(_text, "$id", this->getID());
    helperReplace(_text, "$type", "IcmpNode");
    helperReplace(_text, "$opcode",
                  llvm::ICmpInst::getPredicateName(
                      dyn_cast<llvm::ICmpInst>(this->getInstruction())
                          ->getUnsignedPredicate()));

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
    if (this->numControlInputPort() > 1 && this->numControlOutputPort() == 1 &&
        this->numDataInputPort() == 0)
      _text = "  val $name = Module(new $type(NumPredOps=$npo, ID = "
              "$id))\n\n";
    else if (this->numControlInputPort() > 1 &&
             this->numControlOutputPort() > 1 && this->numDataInputPort() == 0)
      _text = "  val $name = Module(new $type(NumPredOps=$npo, "
              "NumOuts=$nout ID = "
              "$id))\n\n";
    else if (this->numControlInputPort() == 1 &&
             this->numControlOutputPort() > 1 && this->numDataInputPort() == 0)
      _text = "  val $name = Module(new $type(NumOuts=$nout, ID = "
              "$id))\n\n";
    else
      _text = "  val $name = Module(new $type(ID = "
              "$id))\n\n";

    if (this->numDataInputPort() > 0) {
      helperReplace(_text, "$type", "CBranchNode");
      helperReplace(_text, "$num_out",
                    std::to_string(this->numControlOutputPort()));
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
    _text = "  val $name = Module(new $type("
            "NumOuts = $num_out, ID = $id))\n\n";
    helperReplace(_text, "$type", "SelectNode");
    helperReplace(_text, "$num_out", std::to_string(this->numDataOutputPort()));

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
    else if(_id == 1)
      _text = "$name.io.InData1";
    else if(_id == 2)
      _text = "$name.io.InData2";
    else
        assert(!"Select nod can not have more than three inputs! (select, input1, input2)");

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
    _text = "  val $name = Module(new $type(NumInputs = $num_in, "
            "NumOuts = $num_out, ID = $id))\n\n";
    helperReplace(_text, "$type", "PhiNode");
    helperReplace(_text, "$num_in", std::to_string(this->numDataInputPort()));
    helperReplace(_text, "$num_out", std::to_string(this->numDataOutputPort()));

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
    _text = "  val $name = Module(new $type(retTypes=List($<input_list>), "
            "ID = $id))\n\n";
    helperReplace(_text, "$type", "RetNode");
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$id", this->getID());
    helperReplace(_text, "$<input_list>",
                  std::vector<uint32_t>(this->numDataInputPort(), 32), ",");

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
    _text = "  val $name = Module(new $type(NumPredOps=$npo, "
            "NumSuccOps=$nso, "
            "NumOuts=$num_out, ID=$id, RouteID=$rid))\n\n";
    helperReplace(_text, "$type", "UnTypLoad");

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
    _text = "$name.io.memResp";

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
    _text = "$name.io.memReq";
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
    _text = "  val $name = Module(new $type(NumPredOps=$npo, "
            "NumSuccOps=$nso, "
            "ID=$id, RouteID=$rid))\n\n";
    helperReplace(_text, "$type", "UnTypStore");

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
    _text = "$name.io.memResp";

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
    _text = "$name.io.memReq";
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
    _text = "  val $name = Module(new $type(value = $val"
            ", NumOuts = $num_out, ID = $id))\n\n";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$num_out", std::to_string(this->numDataOutputPort()));
    helperReplace(_text, "$id", this->getID());
    helperReplace(_text, "$type", "ConstNode");
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
//                            GetElementPtrArray Class
//===----------------------------------------------------------------------===//
std::string GepArrayNode::printDefinition(PrintType _pt) {
  string _text("");
  string _name(this->getName());

  switch (_pt) {
  case PrintType::Scala:
    std::replace(_name.begin(), _name.end(), '.', '_');
    if (this->getInstruction()->getNumOperands() == 2) {
      _text = "  val $name = Module(new $type(NumOuts=$num_out, "
              "ID=$id)(numByte=$nb)(size=$size))\n\n";
      helperReplace(_text, "$type", "GepArrayOneNode");
      helperReplace(_text, "$nb", gep_info.array_size);
      helperReplace(_text, "$size", gep_info.length);
    } else {
      _text = "  val $name = Module(new $type(NumOuts=$num_out, "
              "ID=$id)(numByte=$nb)(size=$size))\n\n";
      helperReplace(_text, "$type", "GepArrayTwoNode");
      helperReplace(_text, "$nb", gep_info.array_size);
      helperReplace(_text, "$size", gep_info.length);
    }

    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$id", std::to_string(this->getID()));
    helperReplace(_text, "$num_out", std::to_string(this->numDataOutputPort()));

    break;
  default:
    assert(!"Don't support!");
  }
  return _text;
}

std::string GepArrayNode::printInputEnable(PrintType pt, uint32_t _id) {
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

std::string GepArrayNode::printInputEnable(PrintType pt) {
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

std::string GepArrayNode::printOutputData(PrintType _pt, uint32_t _idx) {
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

std::string GepArrayNode::printInputData(PrintType _pt, uint32_t _id) {
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
//                            GetElementPtrStruct Class
//===----------------------------------------------------------------------===//
std::string GepStructNode::printDefinition(PrintType _pt) {
  string _text("");
  string _name(this->getName());

  switch (_pt) {
  case PrintType::Scala:
    std::replace(_name.begin(), _name.end(), '.', '_');
    if (this->getInstruction()->getNumOperands() == 2) {
      _text = "  val $name = Module(new $type(NumOuts=$num_out, "
              "ID=$id)(numByte=List($<input_vector>)))\n\n";
      helperReplace(_text, "$type", "GepStructOneNode");
      helperReplace(_text, "$<input_vector>", gep_info.element_size, ",");
      // helperReplace(_text, "$nb1", num_byte[0]);
    } else {
      _text = "  val $name = Module(new $type(NumOuts=$num_out, "
              "ID=$id)(numByte1=$nb1, numByte2=$nb2))\n\n";
      helperReplace(_text, "$type", "GepArrayTwoNode");
      // helperReplace(_text, "$nb1", num_byte[0]);
      // helperReplace(_text, "$nb2", num_byte[1]);
    }

    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$id", std::to_string(this->getID()));
    helperReplace(_text, "$num_out", std::to_string(this->numDataOutputPort()));

    break;
  default:
    assert(!"Don't support!");
  }
  return _text;
}

std::string GepStructNode::printInputEnable(PrintType pt, uint32_t _id) {
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

std::string GepStructNode::printInputEnable(PrintType pt) {
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

std::string GepStructNode::printOutputData(PrintType _pt, uint32_t _idx) {
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

std::string GepStructNode::printInputData(PrintType _pt, uint32_t _id) {
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

/**
 * Finding the ending insturctions inside the loop
 */
void LoopNode::setEndingInstructions() {
  // Iterate over the supernodes and then find the store nodes
  for (auto &_s_node : this->bblocks()) {
    for (auto &_ins_node : _s_node->instructions()) {
      if (isa<StoreNode>(&*_ins_node)) {
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
    for (auto &l : _list)
      _arg_count.push_back(l->numDataOutputPort());
    if (_arg_count.size() == 0)
      _arg_count.push_back(0);
    return _arg_count;
  };

  switch (_pt) {
  case PrintType::Scala:
    std::replace(_name.begin(), _name.end(), '.', '_');
    _text = "  val $name = Module(new $type(NumIns=List($<input_vector>), "
            "NumOuts = "
            "$num_out, NumExits=$num_exit, ID = $id))\n\n";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$id", this->getID());
    helperReplace(_text, "$type", "LoopBlock");
    helperReplace(_text, "$<input_vector>",
                  make_argument_port(this->live_ins()), ",");
    helperReplace(_text, "$num_out", this->numLiveOut());
    helperReplace(_text, "$num_exit", this->numControlInputPort() - 2);

    // TODO update the exit points!
    // helperReplace(_text, "num_exit", 1);

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
  switch (_pt) {
  case PrintType::Scala:
    if (_id == 0)
      _text = "$name.io.activate";
    else if (_id == 1)
      _text = "$name.io.endEnable";

    else
      _text = "UKNOWN";
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
      _text = "$name.io.latchEnable";
    else if (_id >= 2)
      _text = "$name.io.loopExit($id)";

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
    _text = "  val $name = Module(new $type(NumPredOps= "
            "$num_out, ID = $id))\n\n";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$num_out", std::to_string(this->numDataInputPort()));
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
    _text = "  val $name = Module(new $type(ID = $id, NumInc=$num_inc, "
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
      assert(!"Sync node can not have more than three control inputs!");
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
    _text = "  val $name = Module(new $type(NumOuts=$num_out, ID = $id"
            ", RouteID=$rid, NumOuts=$num_out))\n\n";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$id", this->getID());
    helperReplace(_text, "$num_out", this->numDataOutputPort());
    helperReplace(_text, "$type", "AllocaNode");
    helperReplace(_text, "$rid", this->getRouteID());

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
    _text = "  $name.io.allocaInputIO.bits.size      := "
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
    for (auto &l : _list)
      _arg_count.push_back(32);
    return _arg_count;
  };

  switch (_pt) {
  case PrintType::Scala:
    std::replace(_name.begin(), _name.end(), '.', '_');
    _text = "  val $name = Module(new $type(ID = $id"
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

  auto make_argument_port = [](const auto &_list) {
    std::vector<uint32_t> _arg_count;
    // for (auto &l : _list) _arg_count.push_back(l->numDataOutputPort());
    // TODO change 32
    for (auto &l : _list)
      _arg_count.push_back(32);
    return _arg_count;
  };

  switch (_pt) {
  case PrintType::Scala:
    std::replace(_name.begin(), _name.end(), '.', '_');
    _text = "  val $name = Module(new $type(ID = $id"
            ", NumSuccOps = $num_succ, argTypes = "
            "List($<input_vector>)))\n\n";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$id", this->getID());
    helperReplace(_text, "$type", "CallOutNode");
    helperReplace(_text, "$num_succ", this->numControlOutputPort());
    helperReplace(_text, "$<input_vector>",
                  make_argument_port(this->input_data_range()), ",");

    // helperReplace(_text, "$num_out", this->numDataOutputPort());

    break;
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
  case PrintType::Scala:
    _text = "$name.io.In(\"field$id\")";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$id", _id);
    break;
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
//                            StackNode Class
//===----------------------------------------------------------------------===//
std::string StackNode::printDefinition(PrintType _pt) {
  string _text;
  string _name(this->getName());
  switch (_pt) {
  case PrintType::Scala:
    std::replace(_name.begin(), _name.end(), '.', '_');
    _text = "  val StackPointer = Module(new Stack(NumOps = $op))\n\n";
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

std::string StackNode::printMemReadInput(PrintType _pt, uint32_t _idx) {
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

std::string StackNode::printMemReadOutput(PrintType _pt, uint32_t _idx) {
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
//                            FloatingPointNode Class
//===----------------------------------------------------------------------===//
std::string FloatingPointNode::printDefinition(PrintType _pt) {
  string _text;
  string _name(this->getName());
  switch (_pt) {
  case PrintType::Scala:
    std::replace(_name.begin(), _name.end(), '.', '_');
    _text = "  val SharedFPU = Module(new SharedFPU(NumOps = $op, "
            "PipeDepth = 32)(t = S))\n\n";
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
    _text = "  val $name = Module(new $type(NumOuts = "
            "$num_out))\n\n";
    helperReplace(_text, "$name", _name.c_str());
    helperReplace(_text, "$num_out", std::to_string(this->numDataOutputPort()));

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
