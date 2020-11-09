#include <jsoncpp/json/value.h>
#define DEBUG_TYPE "graphgen"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Common.h"
#include "Dandelion/Graph.h"
#include "Dandelion/Node.h"

#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <sstream>

#define DATA_SIZE 64

using namespace std;
using namespace llvm;
using namespace dandelion;
using namespace helpers;

using InstructionList = std::list<InstructionNode>;

static uint32_t
getUID(Instruction* I) {
  auto* N = I->getMetadata("UID");
  if (N == nullptr)
    return 0;
  auto* S = dyn_cast<MDString>(N->getOperand(0));
  return stoi(S->getString().str());
}

static uint32_t
getUID(BasicBlock* BB) {
  auto* N = BB->getTerminator()->getMetadata("BB_UID");
  if (N == nullptr)
    return 0;
  auto* S = dyn_cast<MDString>(N->getOperand(0));
  return stoi(S->getString().str());
}

/**
 * HELPER FUNCTIONS
 * Printing header part of each section of the code
 */
std::string
helperScalaPrintHeader(string header) {
  std::transform(header.begin(), header.end(), header.begin(), ::toupper);
  string tmp_line = "   * "
                    "================================================================== "
                    "*/\n\n";

  uint32_t remain_space = tmp_line.length() - 2 - header.length() - 23;

  // Append space to the string
  string header_final = "";
  for (uint32_t i = 0; i < remain_space - 2; i++) {
    header_final.append(" ");
  }
  header_final.append("*\n");

  tmp_line = "\n\n  /* "
             "================================================================== "
             "*\n"
             "   *                   "
             + header + header_final + tmp_line;
  return tmp_line;
}

template <class T>
Node*
findParallelNode(Graph* _graph) {
  for (auto& _node : _graph->instructions()) {
    if (isa<T>(&*_node))
      return &*_node;
  }
  return nullptr;
}

template <class T>
std::vector<T*>
getNodeList(Graph* _graph) {
  std::vector<T*> return_list;
  for (auto& _node : _graph->instructions()) {
    if (auto cast_node = dyn_cast<T>(&*_node))
      return_list.push_back(cast_node);
  }
  return return_list;
}

//===----------------------------------------------------------------------===//
//                           Graph Class
//===----------------------------------------------------------------------===//
//

/**
 * Print function prints the generated graph in the choosen format
 */
void
Graph::printGraph(PrintType _pt, std::string json_path) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "Print Graph information!\n");

      // TODO: pass the corect config path
      printScalaHeader(json_path);

      doInitialization();

      printScalaFunctionHeader();
      printCallIO(PrintType::Scala);
      printMemIO(PrintType::Scala);
      printSharedModules(PrintType::Scala);
      printScalaInputSpliter();
      printLoopHeader(PrintType::Scala);
      printBasicBlocks(PrintType::Scala);
      printInstructions(PrintType::Scala);
      printConstants(PrintType::Scala);
      printBasickBlockInstructionPredicateEdges(PrintType::Scala);
      printBasickBlockLoopPredicateEdges(PrintType::Scala);
      printParallelConnections(PrintType::Scala);
      printLoopBranchEdges(PrintType::Scala);
      printLoopEndingDependencies(PrintType::Scala);
      printLoopDataDependencies(PrintType::Scala);
      printBasickBLockInstructionEdges(PrintType::Scala);
      printPhiNodesConnections(PrintType::Scala);
      printMemInsConnections(PrintType::Scala);
      printSharedConnections(PrintType::Scala);
      printDatadependencies(PrintType::Scala);
      printControlDependencies(PrintType::Scala);
      printOutPort(PrintType::Scala);
      printClosingclass(PrintType::Scala);
      // printScalaMainClass();

      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print the function argument
 */
void
Graph::printFunctionArgument(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Print BasicBlocks information\n");
      this->outCode << helperScalaPrintHeader("Printing Function Argument");
      for (auto& _arg : this->args()) {
        outCode << _arg->printDefinition(PrintType::Scala);
      }
      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print loop headers
 */
void
Graph::printLoopHeader(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Print Loop header\n");
      this->outCode << helperScalaPrintHeader("Printing loop headers");
      for (auto& ll : this->loop_nodes) {
        outCode << ll->printDefinition(PrintType::Scala);
      }
      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print loop headers
 */
void
Graph::printParallelConnections(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala: {
      DEBUG(dbgs() << "\t Print parallel Connections\n");
      this->outCode << helperScalaPrintHeader("Printing parallel connections");

      if (!findParallelNode<SyncNode>(this))
        return;
      auto _sync_node     = findParallelNode<SyncNode>(this);
      auto _detach_node   = findParallelNode<DetachNode>(this);
      auto _reattach_node = findParallelNode<ReattachNode>(this);

      auto printConnection = [&_sync_node](Node* _node) {
        std::stringstream _output;
        _output << "  "
                << _sync_node->printInputEnable(
                       PrintType::Scala,
                       _sync_node->returnControlInputPortIndex(_node).getID())
                << " <> "
                << _node->printOutputEnable(
                       PrintType::Scala,
                       _node->returnControlOutputPortIndex(_sync_node).getID())
                << "\n\n";

        return _output;
      };

      this->outCode << printConnection(_detach_node).str();
      this->outCode << printConnection(_reattach_node).str();

      break;
    }
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    case PrintType::Json: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print the basicblock definition
 */
void
Graph::printBasicBlocks(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Print BasicBlocks information\n");
      this->outCode << helperScalaPrintHeader("Printing basicblock nodes");
      for (auto& bb_node : this->super_node_list) {
        outCode << bb_node->printDefinition(PrintType::Scala);
      }
      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print the insturctions definition
 */
void
Graph::printInstructions(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      this->outCode << helperScalaPrintHeader("Printing instruction nodes");
      for (auto& ins_node : this->inst_list) {
        auto call_ins = dyn_cast<ReturnNode>(&*ins_node);
        if (ins_node->numDataOutputPort() == 0 && ins_node->numControlOutputPort() == 0
            && call_ins == nullptr)
          continue;
        this->outCode << "  //";
        ins_node->getInstruction()->print(this->outCode);
        this->outCode << "\n";
        if (auto call_node = dyn_cast<CallNode>(&*ins_node)) {
          this->outCode << call_node->getCallOut()->printDefinition(PrintType::Scala);
          this->outCode << call_node->getCallIn()->printDefinition(PrintType::Scala);
        } else
          this->outCode << ins_node->printDefinition(PrintType::Scala);
      }
      break;
    default: assert(!"We don't support the other types right now");
  }
}

/**
 * Print the insturctions definition
 */
void
Graph::printConstants(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      this->outCode << helperScalaPrintHeader("Printing constants nodes");
      for (auto& const_node : this->const_int_list) {
        this->outCode << "  //";
        if (const_node->getConstantParent())
          const_node->getConstantParent()->print(this->outCode);
        else
          this->outCode << "NullPtr";
        this->outCode << "\n";
        this->outCode << const_node->printDefinition(PrintType::Scala);
      }

      for (auto& const_node : this->const_fp_list) {
        this->outCode << "  //";
        const_node->getConstantParent()->print(this->outCode);
        this->outCode << "\n";
        this->outCode << const_node->printDefinition(PrintType::Scala);
      }

      break;
    default: assert(!"We don't support the other types right now");
  }
}

/**
 * Print memory modules definition
 */
void
Graph::printSharedModules(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Printing Memory modules:\n");
      this->outCode << helperScalaPrintHeader("Printing Memory modules");
      if (memory_unit->isInitilized())
        outCode << memory_unit->printDefinition(PrintType::Scala);
      else
        outCode << memory_unit->printUninitilizedUnit(PrintType::Scala);

      // Printing local memories
      for (auto& mem : scratchpad_memories) {
        outCode << mem->printDefinition(PrintType::Scala);
      }
      if (floating_point_unit->numReadDataInputPort() > 0)
        outCode << floating_point_unit->printDefinition(PrintType::Scala);
      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print control signals instruction
 */
void
Graph::printBasickBlockInstructionPredicateEdges(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Printing Control signals:\n");
      this->outCode << helperScalaPrintHeader("Basicblock -> predicate instruction");
      for (auto& _s_node : super_node_list) {
        // if (dyn_cast<LoopNode>(&_s_node)) continue;
        for (auto _enable_iterator : _s_node->input_control_range()) {
          // We will print loop connections later
          if (isa<LoopNode>(_enable_iterator.first)) {
            continue;
          }
          auto _input_node   = dyn_cast<Node>(_enable_iterator.first);
          auto _output_index = _input_node->returnControlOutputPortIndex(_s_node.get());

          this->outCode << "  "
                        << _s_node->printInputEnable(PrintType::Scala, _enable_iterator)
                        << " <> "
                        << _input_node->printOutputEnable(PrintType::Scala,
                                                          _output_index.getID())
                        << "\n\n";
        }
      }

      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print control signals for loops
 */
void
Graph::printBasickBlockLoopPredicateEdges(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Printing Control signals:\n");
      this->outCode << helperScalaPrintHeader("Basicblock -> predicate loop");
      for (auto& _s_node : super_node_list) {
        std::set<Node*> unique_loop_nodes;
        for (auto& node : _s_node->input_control_range()) {
          if (auto loop = dyn_cast<LoopNode>(node.first))
            unique_loop_nodes.insert(loop);
        }
        for (auto _l : unique_loop_nodes) {
          auto _list_input_enable  = _s_node->findControlInputNodeList(_l);
          auto _list_output_enable = _l->findControlOutputNodeList(_s_node.get());

          assert(_list_input_enable.size() == _list_output_enable.size()
                 && "Size of the input and output enable signals should "
                    "be the same always!");

          auto _input_it  = _list_input_enable.begin();
          auto _output_it = _list_output_enable.begin();

          for (uint32_t i = 0; i < _list_input_enable.size(); ++i) {
            std::advance(_input_it, i);
            std::advance(_output_it, i);
            this->outCode << "  "
                          << _output_it->first->printInputEnable(
                                 PrintType::Scala, _input_it->second.getID())
                          << " <> "
                          << _input_it->first->printOutputEnable(PrintType::Scala,
                                                                 *_output_it)
                          << "\n\n";
          }
        }
        // for(auto &_arg : _s_node->input_control_range()){
        // unique_control_nodes.insert(_arg.first);
        //}

        // for (auto _enable_iterator : _s_node->input_control_range())
        // {
        // if (auto loop_node =
        // dyn_cast<LoopNode>(_enable_iterator.first)) {
        // auto _input_node =
        // dyn_cast<Node>(_enable_iterator.first);
        // auto _output_index =
        //_input_node->returnControlOutputPortIndex(
        //_s_node.get());

        // this->outCode
        //<< "  "
        //<< _s_node->printInputEnable(PrintType::Scala,
        //_enable_iterator)
        //<< " <> FIXME"
        ////<< _enable_iterator.first->printOutputEnable(
        //// PrintType::Scala, _s_node)
        //<< "\n\n";
        //}
        //}
      }

      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print control signals
 */
void
Graph::printBasickBLockInstructionEdges(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Printing Control signals:\n");
      this->outCode << helperScalaPrintHeader("Basicblock -> enable instruction");
      for (auto& _s_node : super_node_list) {
        for (auto& _const_iterator : _s_node->cfps()) {
          this->outCode
              << "  " << _const_iterator->printInputEnable(PrintType::Scala) << " <> "
              << _s_node->printOutputEnable(
                     PrintType::Scala,
                     _s_node->returnControlOutputPortIndex(&*_const_iterator).getID())
              << "\n\n";
        }
        for (auto& _const_iterator : _s_node->cints()) {
          this->outCode
              << "  " << _const_iterator->printInputEnable(PrintType::Scala) << " <> "
              << _s_node->printOutputEnable(
                     PrintType::Scala,
                     _s_node->returnControlOutputPortIndex(&*_const_iterator).getID())
              << "\n\n";
        }
        for (auto _ins_iterator = _s_node->ins_begin();
             _ins_iterator != _s_node->ins_end();
             _ins_iterator++) {
          auto _output_node = dyn_cast<Node>(*_ins_iterator);

          if (auto detach = dyn_cast<ReattachNode>(_output_node)) {
            if (detach->numControlInputPort() == 0)
              this->outCode << "  " << _output_node->printInputEnable(PrintType::Scala)
                            << "\n\n";
            continue;
          }

          if (auto _call_node = dyn_cast<CallNode>(_output_node)) {
            auto ff_out = std::find_if(
                _call_node->getCallOut()->inputControl_begin(),
                _call_node->getCallOut()->inputControl_end(),
                [&_s_node](auto& arg) -> bool { return _s_node.get() == &*arg.first; });

            if (ff_out == _call_node->getCallOut()->inputControl_end())
              assert(!"Couldn't find the control edge\n");

            auto ff_in = std::find_if(
                _call_node->getCallIn()->inputControl_begin(),
                _call_node->getCallIn()->inputControl_end(),
                [&_s_node](auto& arg) -> bool { return _s_node.get() == &*arg.first; });

            if (ff_in == _call_node->getCallIn()->inputControl_end())
              assert(!"Couldn't find the control edge\n");

            this->outCode
                << "  " << _call_node->getCallIn()->printInputEnable(PrintType::Scala)
                << " <> "
                << _s_node->printOutputEnable(
                       PrintType::Scala,
                       _s_node->returnControlOutputPortIndex(_call_node->getCallIn())
                           .getID())
                << "\n\n";

            this->outCode
                << "  " << _call_node->getCallOut()->printInputEnable(PrintType::Scala)
                << " <> "
                << _s_node->printOutputEnable(
                       PrintType::Scala,
                       _s_node->returnControlOutputPortIndex(_call_node->getCallOut())
                           .getID())
                << "\n\n";

          } else {
            // Finding super node
            auto ff = std::find_if(
                _output_node->inputControl_begin(),
                _output_node->inputControl_end(),
                [&_s_node](auto& arg) -> bool { return _s_node.get() == &*arg.first; });

            if (ff == _output_node->inputControl_end())
              assert(!"Couldn't find the control edge\n");

            this->outCode
                << "  " << _output_node->printInputEnable(PrintType::Scala) << " <> "
                << _s_node->printOutputEnable(
                       PrintType::Scala,
                       _s_node->returnControlOutputPortIndex(_output_node).getID())
                << "\n\n";
          }
          this->outCode << "\n";
        }
      }

      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print control signals
 */
void
Graph::printPhiNodesConnections(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Printing phi nodes\n");
      this->outCode << helperScalaPrintHeader("Connecting phi nodes");
      for (auto& _s_node : super_node_list) {
        // Adding Phi node inputs
        for (auto _phi_it = _s_node.get()->phi_begin();
             _phi_it != _s_node.get()->phi_end();
             _phi_it++) {
          auto _phi_ins = dyn_cast<PhiSelectNode>(*_phi_it);

          // Adding phi node mask
          auto _input_index = std::distance(_s_node->phi_begin(), _phi_it);
          this->outCode << "  " << _phi_ins->printMaskInput(PrintType::Scala) << " <> "
                        << _phi_ins->getMaskNode()->printMaskOutput(PrintType::Scala,
                                                                    _input_index)
                        << "\n\n";
        }
      }

      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print data dependencies
 */
void
Graph::printDatadependencies(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Data dependencies\n");
      this->outCode << helperScalaPrintHeader("Connecting data dependencies");

      for (auto& _data_edge : edge_list) {
        if (_data_edge->getType() == Edge::DataTypeEdge) {
          this->outCode << "  "
                        << _data_edge->getTar().first->printInputData(
                               PrintType::Scala, _data_edge->getTar().second.getID())
                        << " <> "
                        << _data_edge->getSrc().first->printOutputData(
                               PrintType::Scala, _data_edge->getSrc().second.getID())
                        << "\n\n";
        }
      }

      // Print ground ndoes
      for (auto _st_node : getNodeList<StoreNode>(this)) {
        if (_st_node->numDataOutputPort() == 0)
          this->outCode << "  " << _st_node->printGround(PrintType::Scala) << "\n\n";
      }

      // Print ground ndoes
      for (auto _ra_node : getNodeList<ReattachNode>(this)) {
        if (_ra_node->numDataOutputPort() == 0)
          this->outCode << "  " << _ra_node->printGround(PrintType::Scala) << "\n\n";
      }

      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

void
Graph::printControlDependencies(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Control dependencies\n");
      this->outCode << helperScalaPrintHeader("Connecting data dependencies");

      // Print ground ndoes
      for (auto _st_node : getNodeList<StoreNode>(this)) {
        for (auto _cn_node : _st_node->output_control_range()) {
          if (auto branch_node = dyn_cast<BranchNode>(_cn_node.first)) {
            this->outCode
                << "  "
                << branch_node->printInputEnable(
                       PrintType::Scala,
                       branch_node->returnControlInputPortIndex(_st_node).getID())
                << " <> "
                << _st_node->printOutputEnable(
                       PrintType::Scala,
                       _st_node->findControlOutputNode(_cn_node.first)->second.getID())
                << "\n\n";
          } else if (auto ret_node = dyn_cast<ReturnNode>(_cn_node.first)) {
            this->outCode
                << "  "
                << ret_node->printInputEnable(
                       PrintType::Scala,
                       ret_node->returnControlInputPortIndex(_st_node).getID())
                << " <> "
                << _st_node->printOutputEnable(
                       PrintType::Scala,
                       _st_node->findControlOutputNode(_cn_node.first)->second.getID())
                << "\n\n";
          } else
            assert(!"Uknown ground node!\n");
        }
      }

      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

void
Graph::printSharedConnections(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala: {
      this->outCode << helperScalaPrintHeader("Print shared connections");
      auto fdiv_list = getNodeList<FdiveOperatorNode>(this);
      for (auto _fd_node : fdiv_list) {
        this->outCode
            << "  "
            << this->getFPUNode()->printMemReadInput(
                   PrintType::Scala,
                   this->getFPUNode()->returnMemoryReadInputPortIndex(_fd_node).getID())
            << " <> "
            << _fd_node->printMemReadOutput(
                   PrintType::Scala,
                   _fd_node->returnMemoryReadOutputPortIndex(this->getFPUNode()).getID())
            << "\n  "
            << _fd_node->printMemReadInput(
                   PrintType::Scala,
                   _fd_node->returnMemoryReadInputPortIndex(this->getFPUNode()).getID())
            << " <> "
            << this->getFPUNode()->printMemReadOutput(
                   PrintType::Scala,
                   this->getFPUNode()->returnMemoryReadOutputPortIndex(_fd_node).getID())
            << "\n\n";
      }
      break;
    }
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print memory connections
 */
void
Graph::printMemInsConnections(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala: {
      DEBUG(dbgs() << "\t Memory to instructions dependencies\n");
      this->outCode << helperScalaPrintHeader("Connecting memory connections");
      auto cache = this->getMemoryUnit();
      for (auto mem : cache->read_req_range()) {
        this->outCode << "  "
                      << cache->printMemReadInput(
                             PrintType::Scala,
                             cache->returnMemoryReadInputPortIndex(mem.first).getID())
                      << " <> "
                      << mem.first->printMemReadOutput(
                             PrintType::Scala,
                             mem.first->returnMemoryReadOutputPortIndex(cache).getID())
                      << "\n";

        this->outCode << "  "
                      << mem.first->printMemReadInput(
                             PrintType::Scala,
                             mem.first->returnMemoryReadInputPortIndex(cache).getID())
                      << " <> "
                      << cache->printMemReadOutput(
                             PrintType::Scala,
                             cache->returnMemoryReadOutputPortIndex(mem.first).getID())
                      << "\n";
      }

      for (auto mem : cache->write_req_range()) {
        this->outCode << "  "
                      << cache->printMemWriteInput(
                             PrintType::Scala,
                             cache->returnMemoryWriteInputPortIndex(mem.first).getID())
                      << " <> "
                      << mem.first->printMemWriteOutput(
                             PrintType::Scala,
                             mem.first->returnMemoryWriteOutputPortIndex(cache).getID())
                      << "\n";

        this->outCode << "  "
                      << mem.first->printMemWriteInput(
                             PrintType::Scala,
                             mem.first->returnMemoryWriteInputPortIndex(cache).getID())
                      << " <> "
                      << cache->printMemWriteOutput(
                             PrintType::Scala,
                             cache->returnMemoryWriteOutputPortIndex(mem.first).getID())
                      << "\n\n";
      }

      // Print local buffers
      //
      for (auto& scratchpad : this->scratchpad_memories) {
        for (auto mem : scratchpad->read_req_range()) {
          this->outCode
              << "  "
              << scratchpad->printMemReadInput(
                     PrintType::Scala,
                     scratchpad->returnMemoryReadInputPortIndex(mem.first).getID())
              << " <> "
              << mem.first->printMemReadOutput(
                     PrintType::Scala,
                     mem.first->returnMemoryReadOutputPortIndex(scratchpad.get()).getID())
              << "\n";

          this->outCode
              << "  "
              << mem.first->printMemReadInput(
                     PrintType::Scala,
                     mem.first->returnMemoryReadInputPortIndex(scratchpad.get()).getID())
              << " <> "
              << scratchpad->printMemReadOutput(
                     PrintType::Scala,
                     scratchpad->returnMemoryReadOutputPortIndex(mem.first).getID())
              << "\n\n";
        }

        for (auto& scratchpad : this->scratchpad_memories) {
          for (auto mem : scratchpad->write_req_range()) {
            this->outCode
                << "  "
                << scratchpad->printMemWriteInput(
                       PrintType::Scala,
                       scratchpad->returnMemoryWriteInputPortIndex(mem.first).getID())
                << " <> "
                << mem.first->printMemWriteOutput(
                       PrintType::Scala,
                       mem.first->returnMemoryWriteOutputPortIndex(scratchpad.get())
                           .getID())
                << "\n";

            this->outCode
                << "  "
                << mem.first->printMemWriteInput(
                       PrintType::Scala,
                       mem.first->returnMemoryWriteInputPortIndex(scratchpad.get())
                           .getID())
                << " <> "
                << scratchpad->printMemWriteOutput(
                       PrintType::Scala,
                       scratchpad->returnMemoryWriteOutputPortIndex(mem.first).getID())
                << "\n";
          }
        }
      }

    }

    break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print data closing class
 */
void
Graph::printClosingclass(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Data dependencies\n");
      this->outCode << "}\n\n";
      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print the main class of chisel module
 */
void
Graph::printScalaMainClass() {
  // Printing Tests class
  string _command = "import java.io.{File, FileWriter}\n\n"
                    "object $class_nameTop extends App {\n"
                    "  val dir = new File(\"RTL/$class_nameTop\");\n"
                    "  dir.mkdirs\n"
                    "  implicit val p = Parameters.root((new "
                    "MiniConfig).toInstance)\n"
                    "  val chirrtl = firrtl.Parser.parse(chisel3.Driver.emit(() => new "
                    "$module_name()))\n\n"
                    "  val verilogFile = new File(dir, s\"${chirrtl.main}.v\")\n"
                    "  val verilogWriter = new FileWriter(verilogFile)\n"
                    "  val compileResult = (new "
                    "firrtl.VerilogCompiler).compileAndEmit(firrtl.CircuitState(chirrtl, "
                    "firrtl.ChirrtlForm))\n"
                    "  val compiledStuff = compileResult.getEmittedCircuit\n"
                    "  verilogWriter.write(compiledStuff.value)\n"
                    "  verilogWriter.close()\n}\n";
  helperReplace(_command, "$class_name", this->graph_info.Name);
  helperReplace(_command, "$module_name", this->graph_info.Name + "DF");

  this->outCode << _command;
}

void
Graph::printScalaInputSpliter() {
  this->outCode << split_call->printDefinition(PrintType::Scala);
}

/**
 * Print the basicblock definition
 */
void
Graph::printScalaFunctionHeader() {
  // print the header
  // this->outCode << helperScalaPrintHeader("Printing ports definition");

  string _final_command;
  _final_command = "\n\nclass $module_nameDF(PtrsIn: Seq[Int] = "
                   "List($<input_vector_ptrs>), ValsIn: Seq[Int] = "
                   "List($<input_vector_vals>), "
                   "Returns: Seq[Int] = List($<output_vector>))\n"
                   "\t\t\t(implicit p: Parameters)"
                   " extends DandelionAccelDCRModule(PtrsIn, ValsIn, Returns){\n";
  helperReplace(_final_command, "$module_name", graph_info.Name);
  auto num_in_args_ptrs = this->getSplitCall()->numLiveInArgList(
      ArgumentNode::LiveIn, ArgumentNode::PointerType);
  auto num_in_args_vals = this->getSplitCall()->numLiveInArgList(
      ArgumentNode::LiveIn, ArgumentNode::IntegerType);
  uint32_t num_out_args = (!function_ptr->getReturnType()->isVoidTy()) ? 1 : 0;
  std::vector<uint32_t> _input_args_ptrs(num_in_args_ptrs, DATA_SIZE);
  std::vector<uint32_t> _input_args_vals(num_in_args_vals, DATA_SIZE);
  std::vector<uint32_t> _output_args(num_out_args, DATA_SIZE);

  helperReplace(_final_command, "$<input_vector_ptrs>", _input_args_ptrs, ", ");
  helperReplace(_final_command, "$<input_vector_vals>", _input_args_vals, ", ");
  helperReplace(_final_command, "$<output_vector>", _output_args, ", ");

  helperScalaPrintHeader("Printing Module Definition");
  outCode << _final_command;
}

void
Graph::printCallIO(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala: {
      auto call_node_list = getNodeList<CallNode>(this);
      if (call_node_list.size()) {
        this->outCode << "  /**\n    * Call Interfaces\n    */\n";
        string _final_command;
        for (auto& call_node : call_node_list) {
          auto call_in  = call_node->getCallIn();
          auto call_out = call_node->getCallOut();

          _final_command = "  val $<name_out>_io = IO(Decoupled(new "
                           "CallDCR(ptrsArgTypes = List($<input_vector_ptrs>), "
                           "valsArgTypes = List($<input_vector_vals>))))\n";
          _final_command += "  val $<name_in>_io = IO(Flipped(Decoupled(new "
                            "Call(List($<output_vector>)))))";
          helperReplace(_final_command, "$<name_out>", call_out->getName());
          helperReplace(_final_command, "$<name_in>", call_in->getName());

          uint32_t num_in_vals = 0;
          uint32_t num_in_ptrs = 0;
          for (auto input_ins : call_out->input_data_range()) {
            if (auto instruction_node = dyn_cast<InstructionNode>(&*input_ins.first)) {
              if (instruction_node->isPointerType())
                num_in_ptrs++;
              else if (instruction_node->isIntegerType()
                       || instruction_node->isFloatType())
                num_in_vals++;
              else {
                std::cout << instruction_node->getName() << "\n";
                DEBUG(dbgs() << instruction_node->getDataType() << "\n");
                assert(!"Input datatype is Uknonw");
              }
            }
          }
          std::vector<uint32_t> _input_vals(num_in_vals, DATA_SIZE);
          std::vector<uint32_t> _input_ptrs(num_in_ptrs, DATA_SIZE);
          std::vector<uint32_t> _output_vector(call_in->numDataOutputPort(), DATA_SIZE);

          helperReplace(_final_command, "$<input_vector_ptrs>", _input_ptrs, ", ");
          helperReplace(_final_command, "$<input_vector_vals>", _input_vals, ", ");
          helperReplace(_final_command, "$<output_vector>", _output_vector, ", ");
        }
        this->outCode << _final_command;
      }
      break;
    }
    default: assert(!"We don't support the other types right now");
  }
}

void
Graph::printMemIO(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala: {
      auto alloca_node_list = getNodeList<AllocaNode>(this);
      if (alloca_node_list.size()) {
        this->outCode << "\n  /**\n    * Memory Interfaces\n    */\n";
        string _final_command;
        for (auto& alloca_node : alloca_node_list) {
          _final_command = "  val $<name>_mem_req = IO(Decoupled(new MemReq))\n"
                           "  val $<name>_mem_resp = IO(Flipped(Valid(new "
                           "MemResp)))\n\n";
          helperReplace(_final_command, "$<name>", alloca_node->getName());
        }
        this->outCode << _final_command;
      }
      break;
    }
    default: assert(!"We don't support the other types right now");
  }
}

/**
 * Print specific scala header files
 */
void
Graph::printScalaHeader(string config_path) {
  std::ifstream _in_file(config_path);
  Json::Value _root_json;

  _in_file >> _root_json;

  assert(!_root_json["import"].empty() && "Config should contain import key");

  // TODO add one level of package to the config json file
  auto package_name = _root_json["package-name"];
  outCode << "package " << package_name.asString() << "\n\n";

  for (auto _it_obj = _root_json["import"].begin(); _it_obj != _root_json["import"].end();
       _it_obj++) {
    if (_it_obj->isArray()) {
      outCode << "import " << _it_obj.key().asString() << "._\n";
      if (_it_obj->isArray()) {
        for (auto& elem : *_it_obj) {
          outCode << "import " << _it_obj.key().asString() << "." << elem.asString()
                  << "._\n";
        }
      }
    } else if (_it_obj->isString()) {
      outCode << "import " << _it_obj.key().asString() << "._\n";
    }
  }
}

/**
 * Returning instruction list
 */
// InstructionList Graph::getInstructionList() { return &this->inst_list; }

/**
 * Insert a new basic block
 */
SuperNode*
Graph::insertSuperNode(BasicBlock& BB) {
  string fix_name = BB.getName().str();
  std::replace(fix_name.begin(), fix_name.end(), '-', '_');
  fix_name = std::regex_replace(fix_name, std::regex("^\\."), "");
  auto uid = getUID(&BB);
  super_node_list.push_back(
      std::make_unique<SuperNode>(NodeInfo(uid, "bb_" + fix_name + to_string(uid)), &BB));
  auto ff = std::find_if(
      super_node_list.begin(), super_node_list.end(), [&BB](auto& arg) -> bool {
        return arg.get()->getBasicBlock() == &BB;
      });

  return ff->get();
}

/**
 * Insert a new computation instruction
 */
InstructionNode*
Graph::insertBinaryOperatorNode(BinaryOperator& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<BinaryOperatorNode>(
      NodeInfo(uid, "binaryOp_" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new computation instruction
 */
InstructionNode*
Graph::insertFaddNode(BinaryOperator& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<FaddOperatorNode>(
      NodeInfo(uid, "FP_" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new computation instruction
 */
InstructionNode*
Graph::insertFsubNode(BinaryOperator& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<FaddOperatorNode>(
      NodeInfo(uid, "FP_" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new computation instruction
 */
InstructionNode*
Graph::insertFmulNode(BinaryOperator& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<FaddOperatorNode>(
      NodeInfo(uid, "FP_" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new computation instruction
 */
InstructionNode*
Graph::insertFdiveNode(BinaryOperator& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<FdiveOperatorNode>(
      NodeInfo(uid, "FP_" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new computation instruction
 */
InstructionNode*
Graph::insertFcmpNode(FCmpInst& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<FcmpNode>(
      NodeInfo(uid, "FPCMP_" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new computation instruction
 */
InstructionNode*
Graph::insertDetachNode(DetachInst& I) {
  inst_list.push_back(std::make_unique<DetachNode>(
      NodeInfo(inst_list.size(),
               "detach_" + I.getName().str() + std::to_string(inst_list.size())),
      &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new Reattach node
 */
InstructionNode*
Graph::insertReattachNode(ReattachInst& I) {
  inst_list.push_back(std::make_unique<ReattachNode>(
      NodeInfo(inst_list.size(),
               "reattach_" + I.getName().str() + std::to_string(inst_list.size())),
      &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new computation instruction
 */
InstructionNode*
Graph::insertSyncNode(SyncInst& I) {
  inst_list.push_back(std::make_unique<SyncNode>(
      NodeInfo(inst_list.size(),
               "sync_" + I.getName().str() + std::to_string(inst_list.size())),
      &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new computation instruction
 */
InstructionNode*
Graph::insertIcmpOperatorNode(ICmpInst& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<IcmpNode>(
      NodeInfo(uid, "icmp_" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  return ff->get();
}

/**
 * Insert a new computation Branch
 */
InstructionNode*
Graph::insertBranchNode(BranchInst& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<BranchNode>(
      NodeInfo(uid, "br_" + I.getName().str() + std::to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  return ff->get();
}

/**
 * Insert a new computation PhiNode
 */
InstructionNode*
Graph::insertPhiNode(PHINode& I) {
  /**
   * This is a hack, the ordering phi nodes are sometimes different from
   * the branches order to the parent basic block
   * In this way I'm catching if I need to have reverse ordering or not.
   * it has to fixed properly
   */

  bool reverse = false;
  for (int i = 0; i < I.llvm::User::getNumOperands(); ++i) {
    BasicBlock* _op = I.getIncomingBlock(i);
    int j           = 0;
    for (auto _bb : llvm::predecessors(I.getParent())) {
      if ((_op != _bb) && (i == j)) {
        reverse = true;
      }
      j++;
    }
  }

  auto uid = getUID(&I);

  if (I.getType()->isPointerTy()) {
    inst_list.push_back(std::make_unique<PhiSelectNode>(
        NodeInfo(uid, "phi" + I.getName().str() + to_string(uid)),
        Node::DataType::PointerType,
        reverse,
        &I));
  } else if (I.getType()->isIntegerTy()) {
    inst_list.push_back(std::make_unique<PhiSelectNode>(
        NodeInfo(uid, "phi" + I.getName().str() + to_string(uid)),
        Node::DataType::IntegerType,
        reverse,
        &I));
  } else if (I.getType()->isFloatTy() || I.getType()->isDoubleTy()) {
    inst_list.push_back(std::make_unique<PhiSelectNode>(
        NodeInfo(uid, "phi" + I.getName().str() + to_string(uid)),
        Node::DataType::FloatType,
        reverse,
        &I));
  }
  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  return ff->get();
}

/**
 * Insert a new select node
 */
InstructionNode*
Graph::insertSelectNode(SelectInst& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<SelectNode>(
      NodeInfo(uid, "select_" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  return ff->get();
}

/**
 * Insert a new Alloca node
 */
AllocaNode*
Graph::insertAllocaNode(AllocaInst& I, uint32_t size, uint32_t num_byte) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<AllocaNode>(
      NodeInfo(uid, "alloca_" + I.getName().str() + to_string(uid)),
      AllocaNode::DataType::PointerType,
      num_byte,
      size,
      inst_list.size(),
      &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  return dyn_cast<AllocaNode>(ff->get());
}

/**
 * Insert a new GEP node
 */
InstructionNode*
Graph::insertGepNode(GetElementPtrInst& I, GepInfo _info) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<GepNode>(
      NodeInfo(uid, "Gep_" + I.getName().str() + to_string(uid)),
      BinaryOperatorNode::DataType::PointerType,
      _info,
      &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  return ff->get();
}

InstructionNode*
Graph::insertBitcastNode(BitCastInst& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<BitcastNode>(
      NodeInfo(uid, "bitcast_" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  return ff->get();
}

/**
 * Insert a new Load node
 */
InstructionNode*
Graph::insertLoadNode(LoadInst& I) {
  auto _load_list = getNodeList<LoadNode>(this);
  auto uid        = getUID(&I);
  if (I.getType()->isIntegerTy()) {
    inst_list.push_back(
        std::make_unique<LoadNode>(NodeInfo(uid, "ld_" + std::to_string(uid)),
                                   Node::DataType::IntegerType,
                                   &I,
                                   this->getMemoryUnit()));
  } else if (I.getType()->isPointerTy()) {
    inst_list.push_back(
        std::make_unique<LoadNode>(NodeInfo(uid, "ld_" + std::to_string(uid)),
                                   Node::DataType::PointerType,
                                   &I,
                                   this->getMemoryUnit()));
  } else if (I.getType()->isFloatTy() || I.getType()->isDoubleTy()) {
    inst_list.push_back(
        std::make_unique<LoadNode>(NodeInfo(uid, "ld_" + std::to_string(uid)),
                                   Node::DataType::FloatType,
                                   &I,
                                   this->getMemoryUnit()));
  } else if (I.getType()->isArrayTy()) {
    if (I.getType()->getArrayElementType()->isIntegerTy()) {
      inst_list.push_back(
          std::make_unique<LoadNode>(NodeInfo(uid, "ld_" + std::to_string(uid)),
                                     Node::DataType::IntegerType,
                                     &I,
                                     this->getMemoryUnit()));
    } else if (I.getType()->getArrayElementType()->isFloatTy()) {
      inst_list.push_back(
          std::make_unique<LoadNode>(NodeInfo(uid, "ld_" + std::to_string(uid)),
                                     Node::DataType::FloatType,
                                     &I,
                                     this->getMemoryUnit()));
    } else {
      I.getType()->getArrayElementType()->dump();
      assert(!"Uncatched array type for load nodes");
    }
  } else if (I.getType()->isVectorTy()) {
    I.getType()->dump();
    assert(
        !"Load type is vector, "
         "currently we don't support vector loads! "
         "Please make sure you compile the your code "
         "with following options: -fno-vectorize -fno-slp-vectorize -fno-unroll-loops");
  } else {
    I.dump();
    I.getPointerOperandType()->dump();
    assert(!"Uncatch load instruction\n");
  }

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  return ff->get();
}

/**
 * Insert a new Store node
 */
InstructionNode*
Graph::insertStoreNode(StoreInst& I) {
  auto uid         = getUID(&I);
  auto _store_list = getNodeList<StoreNode>(this);
  inst_list.push_back(std::make_unique<StoreNode>(
      NodeInfo(uid, "st_" + std::to_string(uid)), &I, this->getMemoryUnit()));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  return ff->get();
}

/**
 * Insert a new Call node
 */
InstructionNode*
Graph::insertCallNode(CallInst& I) {
  // if (I.getName().str() == "")
  auto uid = getUID(&I);
  inst_list.push_back(
      std::make_unique<CallNode>(NodeInfo(uid, "call_" + std::to_string(uid)), &I));
  // else
  // inst_list.push_back(std::make_unique<CallNode>(
  // NodeInfo(inst_list.size(), I.getName().str()), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });

  auto call_in  = dyn_cast<CallNode>(ff->get())->getCallIn();
  auto call_out = dyn_cast<CallNode>(ff->get())->getCallOut();

  call_in->setParent(this);
  call_out->setParent(this);
  this->pushCallIn(call_in);
  this->pushCallOut(call_out);

  return ff->get();
}

/**
 * Insert a new Store node
 */
InstructionNode*
Graph::insertReturnNode(ReturnInst& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<ReturnNode>(
      NodeInfo(uid, "ret_" + I.getName().str() + std::to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  return ff->get();
}

/**
 * Insert a new Store node
 */
GlobalValueNode*
Graph::insertFunctionGlobalValue(GlobalValue& G) {
  glob_list.push_back(GlobalValueNode(NodeInfo(glob_list.size(), G.getName().str()), &G));

  auto ff =
      std::find_if(glob_list.begin(), glob_list.end(), [&G](GlobalValueNode& gl) -> bool {
        return gl.getGlobalValue() == &G;
      });
  return &*ff;
}

/**
 * Insert a new Edge
 */
Edge*
Graph::insertEdge(Edge::EdgeType _typ, Port _node_src, Port _node_dst) {
  // TODO fix the indexing
  edge_list.push_back(std::make_unique<Edge>(_typ, _node_src, _node_dst));
  auto ff =
      std::find_if(edge_list.begin(),
                   edge_list.end(),
                   [_node_src, _node_dst](std::unique_ptr<Edge>& e) -> bool {
                     return (e->getSrc() == _node_src) && (e->getTar() == _node_dst);
                   });

  return ff->get();
}

bool
Graph::edgeExist(Port _node_src, Port _node_dst) {
  auto ff =
      std::find_if(edge_list.begin(),
                   edge_list.end(),
                   [_node_src, _node_dst](std::unique_ptr<Edge>& e) -> bool {
                     return (e->getSrc() == _node_src) && (e->getTar() == _node_dst);
                   });
  return ff != edge_list.end();
}

/**
 * Find an edge
 */
Edge*
Graph::findEdge(const Port _src, const Port _dst) const {
  auto ff =
      std::find_if(edge_list.begin(), edge_list.end(), [_src, _dst](auto& e) -> bool {
        return (e->getSrc() == _src) && (e->getTar() == _dst);
      });
  if (ff != edge_list.end())
    return ff->get();
  else
    return nullptr;
}

Edge*
Graph::findEdge(const Node* _src, const Node* _dst) const {
  auto ff =
      std::find_if(edge_list.begin(), edge_list.end(), [_src, _dst](auto& e) -> bool {
        return (e->getSrc().first == _src) && (e->getTar().first == _dst);
      });
  if (ff != edge_list.end())
    return ff->get();
  else
    return nullptr;
}

/**
 * Inserting memory edges
 */
Edge*
Graph::insertMemoryEdge(Edge::EdgeType _edge_type, Port _node_src, Port _node_dst) {
  edge_list.push_back(std::make_unique<Edge>(_edge_type, _node_src, _node_dst));
  auto ff =
      std::find_if(edge_list.begin(),
                   edge_list.end(),
                   [_node_src, _node_dst](std::unique_ptr<Edge>& e) -> bool {
                     return (e->getSrc() == _node_src) && (e->getTar() == _node_dst);
                   });

  return ff->get();
}

/**
 * Inserting memory edges
 */
ScratchpadNode*
Graph::createBufferMemory(AllocaNode* alloca, uint32_t size, uint32_t num_byte) {
  scratchpad_memories.push_back(std::make_unique<ScratchpadNode>(
      NodeInfo(scratchpad_memories.size(),
               "buffer_memories_" + std::to_string(scratchpad_memories.size())),
      alloca,
      size,
      num_byte));

  return scratchpad_memories.end()->get();
}

ScratchpadNode*
Graph::returnScratchpadMem(AllocaInst* alloca) {
  auto mem = std::find_if(
      scratchpad_memories.begin(), scratchpad_memories.end(), [alloca](auto& scratch) {
        return scratch->getAllocaNode()->getInstruction() == alloca;
      });

  return mem->get();
}

/**
 * Insert a new const node
 */
ConstIntNode*
Graph::insertConstIntNode(ConstantInt& C) {
  const_int_list.push_back(std::make_unique<ConstIntNode>(
      NodeInfo(const_int_list.size(), "const" + std::to_string(const_int_list.size())),
      &C));

  return const_int_list.back().get();
}

/**
 * Insert a new const node
 */
ConstIntNode*
Graph::insertConstIntNode() {
  const_int_list.push_back(std::make_unique<ConstIntNode>(
      NodeInfo(const_int_list.size(), "const" + std::to_string(const_int_list.size())),
      nullptr));

  return const_int_list.back().get();
}

/**
 * Insert a new sext node
 */
InstructionNode*
Graph::insertTruncNode(TruncInst& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<TruncNode>(
      NodeInfo(uid, "trunc" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new stiofp node
 */
InstructionNode*
Graph::insertSTIoFPNode(SIToFPInst& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<STIoFPNode>(
      NodeInfo(uid, "stiofp" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new fptoui node
 */
InstructionNode*
Graph::insertFPToUINode(FPToUIInst& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<FPToUINode>(
      NodeInfo(uid, "stiofp" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new sext node
 */
InstructionNode*
Graph::insertSextNode(SExtInst& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<SextNode>(
      NodeInfo(uid, "sext" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new sext node
 */
InstructionNode*
Graph::insertZextNode(ZExtInst& I) {
  auto uid = getUID(&I);
  inst_list.push_back(std::make_unique<ZextNode>(
      NodeInfo(uid, "sext" + I.getName().str() + to_string(uid)), &I));

  auto ff = std::find_if(inst_list.begin(), inst_list.end(), [&I](auto& arg) -> bool {
    return arg.get()->getInstruction() == &I;
  });
  ff->get()->printDefinition(PrintType::Scala);

  return ff->get();
}

/**
 * Insert a new const node
 */

ConstFPNode*
Graph::insertConstFPNode(ConstantFP& C) {
  const_fp_list.push_back(std::make_unique<ConstFPNode>(
      NodeInfo(const_fp_list.size(), "constf" + std::to_string(const_fp_list.size())),
      &C));

  return const_fp_list.back().get();
}

LoopNode*
Graph::insertLoopNode(std::unique_ptr<LoopNode> _ln) {
  auto _node_p = _ln.get();
  loop_nodes.push_back(std::move(_ln));

  auto ff = std::find_if(
      loop_nodes.begin(),
      loop_nodes.end(),
      [_node_p](std::unique_ptr<LoopNode>& l) -> bool { return l.get() == _node_p; });

  return ff->get();
}

/**
 * Set function pointer
 */
void
Graph::setFunction(Function* _fn) {
  this->function_ptr = _fn;
}

void
Graph::removeEdge(Node* _src, Node* _dest) {
  this->edge_list.remove_if([_src, _dest](std::unique_ptr<Edge>& _e) -> bool {
    return (_e->getSrc().first == _src) && (_e->getTar().first == _dest);
  });
}

/**
 * Print loop control signals
 */
void
Graph::printLoopBranchEdges(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Printing Control signals:\n");
      this->outCode << helperScalaPrintHeader("Loop -> predicate instruction");
      for (auto& _l_node : loop_nodes) {
        for (auto _enable_iterator : _l_node->input_control_range()) {
          auto _input_node = dyn_cast<Node>(&*_enable_iterator.first);

          auto _output_index = _input_node->returnControlOutputPortIndex(_l_node.get());

          this->outCode << "  "
                        << _l_node->printInputEnable(PrintType::Scala,
                                                     _enable_iterator.second.getID())
                        << " <> "
                        << _input_node->printOutputEnable(PrintType::Scala,
                                                          _output_index.getID())
                        << "\n\n";
        }
      }

      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print loop control signals
 */
void
Graph::printLoopEndingDependencies(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Printing Control signals:\n");
      this->outCode << helperScalaPrintHeader("Ending instructions");
      for (auto& _l_node : loop_nodes) {
        if (!_l_node->isOuterLoop())
          continue;

        for (auto& _ending_ins : _l_node->endings()) {
          for (auto& _cn_dependencies : _ending_ins->output_control_range()) {
            auto _input_index =
                _cn_dependencies.first->returnControlInputPortIndex(_ending_ins);

            auto _output_index =
                _ending_ins->returnControlOutputPortIndex(_cn_dependencies.first);

            this->outCode << "  "
                          << _cn_dependencies.first->printInputEnable(
                                 PrintType::Scala, _input_index.getID())
                          << " <> "
                          << _ending_ins->printOutputEnable(PrintType::Scala,
                                                            _output_index.getID())
                          << "\n\n";
          }
        }
      }

      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print loop data dependencies
 */
void
Graph::printLoopDataDependencies(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala:
      DEBUG(dbgs() << "\t Printing Control signals:\n");
      this->outCode << helperScalaPrintHeader("Loop input Data dependencies");
      for (auto& _l_node : loop_nodes) {
        // TODO remove the counter
        uint32_t c = 0;
        // for (auto &_live_in : _l_node->live_in_ptrs_lists()) {
        // if (_live_in->getArgType() != ArgumentNode::LoopLiveIn)
        // continue;
        // for (auto &_data_in : _live_in->input_data_range()) {
        // this->outCode
        //<< "  "
        //<< _live_in->printInputData(PrintType::Scala, c++)
        //<< " <> "
        //<< _data_in.first->printOutputData(
        // PrintType::Scala,
        //_data_in.first
        //->returnDataOutputPortIndex(
        //_live_in.get())
        //.getID())
        //<< "\n\n";
        //}
        //}

        // c = 0;
        for (auto& _live_in : _l_node->live_in_lists()) {
          if (_live_in->getArgType() != ArgumentNode::LoopLiveIn)
            continue;
          for (auto& _data_in : _live_in->input_data_range()) {
            this->outCode
                << "  " << _live_in->printInputData(PrintType::Scala, c++) << " <> "
                << _data_in.first->printOutputData(
                       PrintType::Scala,
                       _data_in.first->returnDataOutputPortIndex(_live_in.get()).getID())
                << "\n\n";
          }
        }
      }

      this->outCode << helperScalaPrintHeader("Loop Data live-in dependencies");
      for (auto& _l_node : loop_nodes) {
        for (auto& _live_in : _l_node->live_in_lists()) {
          if (_live_in->getArgType() != ArgumentNode::LoopLiveIn)
            continue;
          for (auto& _data_out : _live_in->output_data_range()) {
            if (isa<ArgumentNode>(_data_out.first))
              continue;
            this->outCode
                << "  "
                << _data_out.first->printInputData(
                       PrintType::Scala,
                       _data_out.first->returnDataInputPortIndex(_live_in.get()).getID())
                << " <> "
                << _live_in->printOutputData(
                       PrintType::Scala,
                       _live_in->returnDataOutputPortIndex(_data_out.first).getID())
                << "\n\n";
          }
        }
      }

      this->outCode << helperScalaPrintHeader("Loop Data live-out dependencies");
      for (auto& _l_node : loop_nodes) {
        // for (auto &_live_out : _l_node->live_out_lists()) {
        // if (_live_out->getArgType() != ArgumentNode::LoopLiveOut)
        // continue;
        // for (auto &_data_out : _live_out->input_data_range()) {
        // this->outCode << "  "
        //<< _live_out->printInputData(
        // PrintType::Scala,
        //_live_out
        //->returnDataInputPortIndex(
        //_data_out.first)
        //.getID())
        //<< " <> "
        //<< _data_out.first->printOutputData(
        // PrintType::Scala,
        //_data_out.first
        //->returnDataOutputPortIndex(
        //_live_out.get())
        //.getID())
        //<< "\n\n";
        //}
        //}

        unsigned c = 0;
        for (auto& _live_out : _l_node->live_out_lists()) {
          if (_live_out->getArgType() != ArgumentNode::LoopLiveOut)
            continue;
          for (auto& _data_in : _live_out->input_data_range()) {
            this->outCode
                << "  " << _live_out->printInputData(PrintType::Scala, c++) << " <> "
                << _data_in.first->printOutputData(
                       PrintType::Scala,
                       _data_in.first->returnDataOutputPortIndex(_live_out.get()).getID())
                << "\n\n";
          }
        }
      }

      this->outCode << helperScalaPrintHeader("Loop live out dependencies");
      for (auto& _l_node : loop_nodes) {
        for (auto& _live_out : _l_node->live_out_lists()) {
          if (_live_out->getArgType() != ArgumentNode::LoopLiveOut)
            continue;
          for (auto& _data_out : _live_out->output_data_range()) {
            if (isa<ArgumentNode>(_data_out.first))
              continue;
            this->outCode
                << "  "
                << _data_out.first->printInputData(
                       PrintType::Scala,
                       _data_out.first->returnDataInputPortIndex(_live_out.get()).getID())
                << " <> "
                << _live_out->printOutputData(
                       PrintType::Scala,
                       _live_out->returnDataOutputPortIndex(_data_out.first).getID())
                << "\n\n";
          }
        }
      }

      this->outCode << helperScalaPrintHeader("Loop carry dependencies");
      for (auto& _l_node : loop_nodes) {
        // TODO remove the counter
        uint32_t c = 0;
        for (auto& _carry : _l_node->carry_depen_lists()) {
          if (_carry->getArgType() != ArgumentNode::CarryDependency)
            continue;
          for (auto& _data_in : _carry->input_data_range()) {
            this->outCode
                << "  " << _carry->printInputData(PrintType::Scala, c++) << " <> "
                << _data_in.first->printOutputData(
                       PrintType::Scala,
                       _data_in.first->returnDataOutputPortIndex(_carry.get()).getID())
                << "\n\n";
          }
        }
      }

      this->outCode << helperScalaPrintHeader("Loop Data Carry dependencies");
      for (auto& _l_node : loop_nodes) {
        for (auto& _carry : _l_node->carry_depen_lists()) {
          if (_carry->getArgType() != ArgumentNode::CarryDependency)
            continue;
          for (auto& _data_out : _carry->output_data_range()) {
            if (isa<ArgumentNode>(_data_out.first))
              continue;
            this->outCode
                << "  "
                << _data_out.first->printInputData(
                       PrintType::Scala,
                       _data_out.first->returnDataInputPortIndex(_carry.get()).getID())
                << " <> "
                << _carry->printOutputData(
                       PrintType::Scala,
                       _carry->returnDataOutputPortIndex(_data_out.first).getID())
                << "\n\n";
          }
        }
      }

      break;
    case PrintType::Dot: assert(!"Dot file format is not supported!");
    default: assert(!"Uknown print type!");
  }
}

/**
 * Print the output port
 */
void
Graph::printOutPort(PrintType _pt) {
  switch (_pt) {
    case PrintType::Scala: {
      // Print Call node connections
      auto call_node_list = getNodeList<CallNode>(this);
      for (auto _c_node : call_node_list) {
        this->outCode << helperScalaPrintHeader("Printing callin and callout interface");

        this->outCode << "  " << _c_node->getCallOut()->getName() + "_io"
                      << " <> "
                      << _c_node->getCallOut()->printOutputData(PrintType::Scala, 0)
                      << "\n\n";

        this->outCode << "  " << _c_node->getCallIn()->printInputData(PrintType::Scala)
                      << " <> " << _c_node->getCallIn()->getName() + "_io"
                      << "\n\n";

        if (_c_node->getCallIn()->numControlOutputPort() == 0)
          this->outCode << "  "
                        << _c_node->getCallIn()->printOutputEnable(PrintType::Scala);
        else {
          for (auto _ctrl_node : _c_node->getCallIn()->output_control_range()) {
            this->outCode << "  "
                          << _ctrl_node.first->printInputEnable(
                                 PrintType::Scala,
                                 _ctrl_node.first
                                     ->returnControlInputPortIndex(_c_node->getCallIn())
                                     .getID())
                          << " <> "
                          << _c_node->getCallIn()->printOutputEnable(
                                 PrintType::Scala,
                                 _c_node->getCallIn()
                                     ->returnControlOutputPortIndex(_ctrl_node.first)
                                     .getID())
                          << "\n\n";
          }
        }
      }

      this->outCode << helperScalaPrintHeader("Printing output interface");
      this->outCode << "  io.out <> " << out_node->printOutputData(PrintType::Scala)
                    << "\n\n";
      break;
    }
    default: assert(!"We don't support the other types right now");
  }
}

/**
 * Initializing the graph
 */
void
Graph::doInitialization() {
  // Filling the data dependencies
  //

  for (auto& _node : const_int_list) {
    for (auto& _child : _node->output_data_range()) {
      if (isa<ArgumentNode>(&*_child.first))
        continue;
      this->insertEdge(
          Edge::EdgeType::DataTypeEdge,
          std::make_pair(&*_node, _node->returnDataOutputPortIndex(&*_child.first)),
          std::make_pair(&*_child.first,
                         _child.first->returnDataInputPortIndex(&*_node)));
    }
  }

  for (auto& _node : const_fp_list) {
    for (auto& _child : _node->output_data_range()) {
      if (isa<ArgumentNode>(&*_child.first))
        continue;
      this->insertEdge(
          Edge::EdgeType::DataTypeEdge,
          std::make_pair(&*_node, _node->returnDataOutputPortIndex(&*_child.first)),
          std::make_pair(&*_child.first,
                         _child.first->returnDataInputPortIndex(&*_node)));
    }
  }

  for (auto& _node : inst_list) {
    Node* _ptr = _node.get();
    if (isa<CallNode>(_ptr))
      _ptr = dyn_cast<CallNode>(_ptr)->getCallIn();
    for (auto& _child : _ptr->output_data_range()) {
      if (isa<ArgumentNode>(&*_child.first))
        continue;
      this->insertEdge(
          Edge::EdgeType::DataTypeEdge,
          std::make_pair(_ptr, _ptr->returnDataOutputPortIndex(&*_child.first)),
          std::make_pair(&*_child.first, _child.first->returnDataInputPortIndex(_ptr)));
    }
  }

  for (auto& _arg : this->getSplitCall()->live_in_ptrs_lists()) {
    if (_arg->getArgType() != ArgumentNode::LiveIn)
      continue;
    for (auto& _node : _arg->output_data_range()) {
      if (isa<ArgumentNode>(_node.first))
        continue;
      this->insertEdge(
          Edge::EdgeType::DataTypeEdge,
          std::make_pair(&*_arg, _arg->returnDataOutputPortIndex(&*_node.first)),
          std::make_pair(&*_node.first, _node.first->returnDataInputPortIndex(&*_arg)));
    }
  }

  for (auto& _arg : this->getSplitCall()->live_in_vals_lists()) {
    if (_arg->getArgType() != ArgumentNode::LiveIn)
      continue;
    for (auto& _node : _arg->output_data_range()) {
      if (isa<ArgumentNode>(_node.first))
        continue;
      this->insertEdge(
          Edge::EdgeType::DataTypeEdge,
          std::make_pair(&*_arg, _arg->returnDataOutputPortIndex(&*_node.first)),
          std::make_pair(&*_node.first, _node.first->returnDataInputPortIndex(&*_arg)));
    }
  }
}

/**
 * This function iterate over all the store nodes, and ground their output
 * If the next instruction after store is return the data output is connected
 * to the return data input port
 */
void
Graph::groundStoreNodes() {
  auto _store_nodes  = getNodeList<StoreNode>(this);
  auto _return_nodes = getNodeList<ReturnNode>(this);

  if (_return_nodes.size() > 1)
    assert(!"A function can not have more than one return node!");

  for (auto _st_node : _store_nodes) {
    if (_st_node->numDataOutputPort() == 0)
      _st_node->setGround();
  }
}

/**
 * This function iterate over all the reattachnodes, and ground their output
 * If the next instruction after store is return the data output is connected
 * to the return data input port
 */
void
Graph::groundReattachNode() {
  auto _reattach_nodes = getNodeList<ReattachNode>(this);

  for (auto _st_node : _reattach_nodes) {
    if (_st_node->numDataOutputPort() == 0)
      _st_node->setGround();
  }
}

//===----------------------------------------------------------------------===//
//                          Optmization passes
//===----------------------------------------------------------------------===//
//

void
Graph::optimizationPasses() {
  groundStoreNodes();
}

void
Graph::printMUIR() {
  std::ofstream _out_file(this->graph_info.Name + ".muir.json");

  Json::Value _root_json;

  _root_json["module"]["name"] = this->graph_info.Name;

  // Printing list of basic blocks
  for (auto& bb : this->super_node_list) {
    Json::Value _node_entry;
    auto _name = bb->getInfo().Name;
    std::replace(_name.begin(), _name.end(), '.', '_');

    // Getting list of nodes
    Json::Value _node_array;
    int i = 0;
    for (auto _node : bb->instructions()) {
      _node_array[i] = _node->getID();
      i++;
    }
    _node_entry["name"]  = _name;
    _node_entry["id"]    = bb->getInfo().ID;
    _node_entry["debug"] = "false";
    _node_entry["nodes"] = _node_array;

    Json::Value _node_phi;
    i = 0;
    for (auto phi_node : bb->phis()) {
      _node_phi[i] = phi_node->getID();
      i++;
    }
    _node_entry["phis"] = _node_phi;
    _root_json["module"]["super_node"].append(_node_entry);
  }

  // Printing list of instructions
  for (auto& node : this->inst_list) {
    Json::Value _node_entry;
    auto _name = node->getInfo().Name;
    std::replace(_name.begin(), _name.end(), '.', '_');
    std::string _node_instruction;
    llvm::raw_string_ostream ss(_node_instruction);
    node->getInstruction()->print(ss);
    _node_entry["name"]        = _name;
    _node_entry["id"]          = node->getInfo().ID;
    _node_entry["debug"]       = "false";
    _node_entry["instruction"] = _node_instruction.c_str();
    _node_entry["parent_bb"]   = node->getParentNode()->getID();

    // Extract instruction type
    std::string node_type;
    switch (node->getOpCode()) {
      case InstructionNode::BinaryInstructionTy: node_type = "Binary"; break;
      case InstructionNode::IcmpInstructionTy: node_type = "Icmp"; break;
      case InstructionNode::BranchInstructionTy: node_type = "Branch"; break;
      case InstructionNode::PhiInstructionTy: node_type = "Phi"; break;
      case InstructionNode::LoadInstructionTy: node_type = "Load"; break;
      case InstructionNode::StoreInstructionTy: node_type = "Store"; break;
      case InstructionNode::ReturnInstrunctionTy: node_type = "Return"; break;
      case InstructionNode::GetElementPtrInstTy: node_type = "Gep"; break;
      case InstructionNode::GetElementPtrArrayInstTy: node_type = "Gep_Array"; break;
      case InstructionNode::GetElementPtrStructInstTy: node_type = "Gep_Struct"; break;
      default: node_type = "Uknown"; break;
    }

    _node_entry["type"] = node_type;

    function<string(Node*)> print_id;
    print_id = [&print_id](Node* node) -> string {
      if (auto const_node = dyn_cast<ConstIntNode>(node)) {
        return "CINT_" + to_string(const_node->getValue());
      } else if (auto const_node = dyn_cast<ConstFPNode>(node)) {
        return "CFLOAT_" + to_string(const_node->getValue());
      } else if (auto arg_node = dyn_cast<ArgumentNode>(node)) {
        if (arg_node->getArgType() == ArgumentNode::CarryDependency)
          return "INST_" + to_string(arg_node->getParentNode()->getID());
        else if (arg_node->getArgType() == ArgumentNode::LiveIn) {
          return "ARG_" + to_string(arg_node->getID());
        } else if (arg_node->getArgType() == ArgumentNode::LoopLiveIn) {
          return print_id(arg_node->getParentNode());
        } else if (arg_node->getArgType() == ArgumentNode::LoopLiveOut) {
          return "INST_" + to_string(arg_node->getParentNode()->getID());
        } else {
          errs() << "Node name: " << arg_node->getName() << "\n";
          assert(!"Unhandel node!");
        }
      } else {
        return "INS_" + to_string(node->getID());
      }
    };

    Json::Value _node_ops;
    int i = 0;
    for (auto operand_node : node->input_data_range()) {
      _node_ops[i] = print_id(operand_node.first);
      i++;
    }
    _node_entry["operands"] = _node_ops;


    Json::Value _node_debug_parent;
    i = 0;
    for (auto _d_parent : node->debug_parent_node) {
      _node_debug_parent[i] = _d_parent;
      i++;
    }
    _node_entry["debug_parent_info"] = _node_debug_parent;


    _root_json["module"]["node"].append(_node_entry);
  }

  // Printing list of nodes
  for (auto& loop : this->loops()) {
    Json::Value _loop_entry;
    _loop_entry["name"] = loop->getName();
    _loop_entry["id"]   = to_string(loop->getID());

    if (loop->getParentLoopNode())
      _loop_entry["loop_parent"] = loop->getParentLoopNode()->getID();
    else
      _loop_entry["loop_parent"] = Json::Value::null;

    if (loop->getInductionVariable())
      _loop_entry["induction_id"] = loop->getInductionVariable()->getID();
    else
      _loop_entry["induction_id"] = Json::Value::null;


    // Getting list of blocks
    Json::Value _loop_blocks;
    int i = 0;
    for (auto _node : loop->bblocks()) {
      _loop_blocks[i] = _node->getID();
      i++;
    }

    _loop_entry["basic_blocks"] = _loop_blocks;


    // Loop carry dependencies
    //
    Json::Value _loop_carries;
    i = 0;
    for (auto& _carry : loop->carry_depen_lists()) {
      if (_carry->getArgType() != ArgumentNode::CarryDependency)
        continue;
      for (auto& _data_in : _carry->input_data_range()) {
        _loop_carries[i] = _data_in.first->getID();
        i++;
      }
    }
    _loop_entry["carries"] = _loop_carries;

    _root_json["module"]["loop"].append(_loop_entry);
  }


  Json::Value _arg_entry;
  for (auto& _arg : this->getSplitCall()->live_in_vals_lists()) {
    _arg_entry["name"] = _arg->getName();
    _arg_entry["id"]   = to_string(_arg->getID());
    _arg_entry["type"] = "Value";

    _root_json["module"]["argument"].append(_arg_entry);
  }

  for (auto& _arg : this->getSplitCall()->live_in_ptrs_lists()) {
    _arg_entry["name"] = _arg->getName();
    _arg_entry["id"]   = to_string(_arg->getID());
    _arg_entry["type"] = "Pointers";

    _root_json["module"]["argument"].append(_arg_entry);
  }


  _out_file << _root_json;
  _out_file.close();
}
