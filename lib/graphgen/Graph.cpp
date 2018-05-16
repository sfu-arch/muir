#define DEBUG_TYPE "graphgen"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Common.h"
#include "Dandelion/Graph.h"
#include "Dandelion/Node.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace llvm;
using namespace dandelion;
using namespace helpers;

using InstructionList = std::list<InstructionNode>;

/**
 * HELPER FUNCTIONS
 * Printing header part of each section of the code
 */
std::string helperScalaPrintHeader(string header) {
    std::transform(header.begin(), header.end(), header.begin(), ::toupper);
    string tmp_line =
        "   * "
        "================================================================== "
        "*/\n\n";

    uint32_t remain_space = tmp_line.length() - 2 - header.length() - 23;

    // Append space to the string
    string header_final = "";
    for (uint32_t i = 0; i < remain_space - 2; i++) {
        header_final.append(" ");
    }
    header_final.append("*\n");

    tmp_line =
        "\n\n  /* "
        "================================================================== "
        "*\n"
        "   *                   " +
        header + header_final + tmp_line;
    return tmp_line;
}

//===----------------------------------------------------------------------===//
//                           Graph Class
//===----------------------------------------------------------------------===//
//

/**
 * Print function prints the generated graph in the choosen format
 */
void Graph::printGraph(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(dbgs() << "Print Graph information!\n");

            // TODO: pass the corect config path
            printScalaHeader("config.json", "dataflow");

            printScalaFunctionHeader();
            printMemoryModules(PrintType::Scala);
            printScalaInputSpliter();
            PrintLoopHeader(PrintType::Scala);
            printBasicBlocks(PrintType::Scala);
            printInstructions(PrintType::Scala);
            printBasickBlockPredicateEdges(PrintType::Scala);
            printLoopBranchEdges(PrintType::Scala);
            printBasickBLockInstructionEdges(PrintType::Scala);
            printPhiNodesConnections(PrintType::Scala);
            printMemInsConnections(PrintType::Scala);
            printDatadependencies(PrintType::Scala);
            printClosingclass(PrintType::Scala);
            printScalaMainClass();

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

/**
 * Print the function argument
 */
void Graph::printFunctionArgument(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(dbgs() << "\t Print BasicBlocks information\n");
            this->outCode << helperScalaPrintHeader(
                "Printing Function Argument");
            for (auto &_arg : this->args()) {
                outCode << _arg->printDefinition(PrintType::Scala);
            }
            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

/**
 * Print loop headers
 */
void Graph::PrintLoopHeader(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(dbgs() << "\t Print Loop header\n");
            this->outCode << helperScalaPrintHeader("Printing loop headers");
            for (auto &ll : this->loop_nodes) {
                outCode << ll->printDefinition(PrintType::Scala);
            }
            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

/**
 * Print the basicblock definition
 */
void Graph::printBasicBlocks(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(dbgs() << "\t Print BasicBlocks information\n");
            this->outCode << helperScalaPrintHeader(
                "Printing basicblock nodes");
            for (auto &bb_node : this->super_node_list) {
                outCode << bb_node->printDefinition(PrintType::Scala);
            }
            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

/**
 * Print the insturctions definition
 */
void Graph::printInstructions(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            this->outCode << helperScalaPrintHeader(
                "Printing instruction nodes");
            for (auto &ins_node : this->inst_list) {
                // ins_node->getInstruction()->dump();
                this->outCode << "  //";
                ins_node->getInstruction()->print(this->outCode);
                this->outCode << "\n";
                this->outCode << ins_node->printDefinition(PrintType::Scala);
            }
            break;
        default:
            assert(!"We don't support the other types right now");
    }
}

/**
 * Print memory modules definition
 */
void Graph::printMemoryModules(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(dbgs() << "\t Printing Memory modules:\n");
            this->outCode << helperScalaPrintHeader("Printing Memory modules");
            outCode << memory_unit->printDefinition(PrintType::Scala);
            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

/**
 * Print control signals
 */
void Graph::printBasickBlockPredicateEdges(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(dbgs() << "\t Printing Control signals:\n");
            this->outCode << helperScalaPrintHeader(
                "Basicblock -> predicate instruction");
            for (auto &_s_node : super_node_list) {
                for (auto _enable_iterator = _s_node->inputControl_begin();
                     _enable_iterator != _s_node->inputControl_end();
                     _enable_iterator++) {
                    auto _input_node = dyn_cast<Node>(*_enable_iterator);
                    auto _input_index = std::distance(
                        _s_node->inputControl_begin(), _enable_iterator);

                    // Finding super node
                    auto ff = std::find_if(_input_node->outputControl_begin(),
                                           _input_node->outputControl_end(),
                                           [&_s_node](auto &arg) -> bool {
                                               return _s_node.get() == &*arg;
                                           });

                    auto _output_index =
                        std::distance(_input_node->outputControl_begin(), ff);

                    if (ff == _input_node->outputControl_end())
                        assert(!"Couldn't find the control edge\n");

                    this->outCode << "  "
                                  << _s_node->printInputEnable(PrintType::Scala,
                                                               _input_index)
                                  << " <> "
                                  << _input_node->printOutputEnable(
                                         PrintType::Scala, _output_index)
                                  << "\n\n";
                }

                // Print activate signal if the super node is loopNode
                if (_s_node->getNodeType() ==
                    SuperNode::SuperNodeType::LoopHead) {
                    auto _input_node = _s_node->getActivateNode();

                    errs() << "TEST\n";

                    this->outCode
                        << "  "
                        << _s_node->printActivateEnable(PrintType::Scala)
                        << " <> "
                        << _input_node->printOutputEnable(PrintType::Scala)
                        << "\n\n";
                }
            }

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

/**
 * Print control signals
 */
void Graph::printBasickBLockInstructionEdges(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(dbgs() << "\t Printing Control signals:\n");
            this->outCode << helperScalaPrintHeader(
                "Basicblock -> enable instruction");
            for (auto &_s_node : super_node_list) {
                for (auto _ins_iterator = _s_node->ins_begin();
                     _ins_iterator != _s_node->ins_end(); _ins_iterator++) {
                    auto _output_node = dyn_cast<Node>(*_ins_iterator);

                    auto _output_index =
                        std::distance(_s_node->ins_begin(), _ins_iterator);

                    // Finding super node
                    auto ff = std::find_if(_output_node->inputControl_begin(),
                                           _output_node->inputControl_end(),
                                           [&_s_node](auto &arg) -> bool {
                                               return _s_node.get() == &*arg;
                                           });

                    auto _input_index =
                        std::distance(_output_node->inputControl_begin(), ff);

                    if (ff == _output_node->inputControl_end())
                        assert(!"Couldn't find the control edge\n");

                    this->outCode
                        << "  "
                        << _output_node->printInputEnable(PrintType::Scala)
                        << " <> "
                        << _s_node->printOutputEnable(PrintType::Scala,
                                                      _output_index)
                        << "\n\n";
                }
                this->outCode << "\n";
            }

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

/**
 * Print control signals
 */
void Graph::printPhiNodesConnections(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(dbgs() << "\t Printing phi nodes\n");
            this->outCode << helperScalaPrintHeader("Connecting phi nodes");
            for (auto &_s_node : super_node_list) {
                // Adding Phi node inputs
                for (auto _phi_it = _s_node.get()->phi_begin();
                     _phi_it != _s_node.get()->phi_end(); _phi_it++) {
                    auto _phi_ins = dyn_cast<PhiSelectNode>(*_phi_it);

                    // Adding phi node mask
                    auto _input_index =
                        std::distance(_s_node->phi_begin(), _phi_it);
                    this->outCode << "  "
                                  << _phi_ins->printMaskInput(PrintType::Scala)
                                  << " <> "
                                  << _phi_ins->getMaskNode()->printMaskOutput(
                                         PrintType::Scala, _input_index)
                                  << "\n\n";
                }
            }

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

/**
 * Print data dependencies
 */
void Graph::printDatadependencies(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(dbgs() << "\t Data dependencies\n");
            this->outCode << helperScalaPrintHeader(
                "Connecting data dependencies");
            for (auto &_data_edge : edge_list) {
                if (_data_edge->getType() == Edge::DataTypeEdge) {
                    this->outCode
                        << "  "
                        << _data_edge->getTar()->printInputData(
                               PrintType::Scala,
                               _data_edge->getTar()->returnDataInputPortIndex(
                                   _data_edge->getSrc()))
                        << " <> "
                        << _data_edge->getSrc()->printOutputData(
                               PrintType::Scala,
                               _data_edge->getSrc()->returnDataOutputPortIndex(
                                   _data_edge->getTar()))
                        << "\n\n";
                }
            }

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

/**
 * Print memory connections
 */
void Graph::printMemInsConnections(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(dbgs() << "\t Memory to instructions dependencies\n");
            this->outCode << helperScalaPrintHeader(
                "Connecting memory connections");
            for (auto &_mem_edge : edge_list) {
                if (_mem_edge->getType() == Edge::MemoryReadTypeEdge) {
                    this->outCode << "  "
                                  << _mem_edge->getTar()->printMemReadInput(
                                         PrintType::Scala,
                                         _mem_edge->getTar()
                                             ->returnMemoryReadInputPortIndex(
                                                 _mem_edge->getSrc()))
                                  << " <> "
                                  << _mem_edge->getSrc()->printMemReadOutput(
                                         PrintType::Scala,
                                         _mem_edge->getSrc()
                                             ->returnMemoryReadOutputPortIndex(
                                                 _mem_edge->getTar()))
                                  << "\n\n";
                } else if (_mem_edge->getType() == Edge::MemoryWriteTypeEdge) {
                    this->outCode << "  "
                                  << _mem_edge->getTar()->printMemWriteInput(
                                         PrintType::Scala,
                                         _mem_edge->getTar()
                                             ->returnMemoryWriteInputPortIndex(
                                                 _mem_edge->getSrc()))
                                  << " <> "
                                  << _mem_edge->getSrc()->printMemWriteOutput(
                                         PrintType::Scala,
                                         _mem_edge->getSrc()
                                             ->returnMemoryWriteOutputPortIndex(
                                                 _mem_edge->getTar()))
                                  << "\n\n";
                }
            }

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

/**
 * Print data closing class
 */
void Graph::printClosingclass(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(dbgs() << "\t Data dependencies\n");
            this->outCode << "}\n\n";
            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

/**
 * Print the main class of chisel module
 */
void Graph::printScalaMainClass() {
    // Printing Tests class
    string _command =
        "import java.io.{File, FileWriter}\n"
        "object $class_nameMain extends App {\n"
        "  val dir = new File(\"RTL/$class_name\") ; dir.mkdirs\n"
        "  implicit val p = config.Parameters.root((new "
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

void Graph::printScalaInputSpliter() {
    this->outCode << split_call->printDefinition(PrintType::Scala);
}

/**
 * Print the basicblock definition
 */
void Graph::printScalaFunctionHeader() {
    // print the header
    this->outCode << helperScalaPrintHeader("Printing ports definition");

    string _final_command;
    string _command =
        "abstract class $module_nameDFIO"
        "(implicit val p: Parameters) extends Module with CoreParams {\n"
        "  val io = IO(new Bundle {\n";
    helperReplace(_command, "$module_name", this->graph_info.Name.c_str());
    _final_command.append(_command);

    // Print input call parameters
    _command = "    val in = Flipped(new CallDecoupled(List(";
    _final_command.append((_command));
    for (uint32_t c = 0; c < this->arg_list.size(); c++) {
        _command = "32,";
        helperReplace(_command, "$index", c);
        _final_command.append(_command);
    }
    _final_command.pop_back();
    _command = ")))\n";
    _final_command.append(_command);

    // Print sub-function call interface
    uint32_t c = 0;
    for (auto &_ins : this->inst_list) {
        if (auto _fc = dyn_cast<CallNode>(_ins.get())) {
            // Call arguments to subroutine
            _command = "    val $call_out = new CallDecoupled(List(";
            helperReplace(_command, "$call", _ins->getName());
            _final_command.append(_command);
            // TODO: Make sure there is no inconsistancy here
            for (auto &ag : _fc->getInstruction()->getFunction()->args()) {
                _command = "32,";
                helperReplace(_command, "$index", c++);
                _final_command.append(_command);
            }
            _final_command.pop_back();
            _command = "))\n";
            _final_command.append(_command);
            // Return values from sub-routine.
            // Only supports a single 32 bit data bundle for now
            _command =
                "    val $call_in = Flipped(new CallDecoupled(List(32)))\n";
            helperReplace(_command, "$call", _fc->getName());
            _final_command.append(_command);
        }
    }

    // Print global memory interface
    c = 0;
    for (uint32_t c = 0; c < this->glob_list.size(); c++) {
        _command =
            "    val glob_$index = Flipped(Decoupled(new "
            "DataBundle))\n";
        helperReplace(_command, "$index", static_cast<int>(c++));
        _final_command.append(_command);
        break;
    }

    // Print cache memory interface
    _final_command.append(
        "    val MemResp = Flipped(Valid(new MemRespT))\n"
        "    val MemReq = Decoupled(new MemReq)\n");

    // Print output (return) parameters
    if (!function_ptr->getReturnType()->isVoidTy()) {
        _final_command.append("    val out = new CallDecoupled(List(32))\n");
    }

    _final_command.append(
        "  })\n"
        "}\n\n");

    // Printing Abstract
    outCode << _final_command;

    _final_command =
        "class $module_nameDF(implicit p: Parameters)"
        " extends $module_nameDFIO()(p) {\n";
    helperReplace(_final_command, "$module_name", graph_info.Name);

    helperScalaPrintHeader("Printing Module Definition");
    outCode << _final_command;
}

/**
 * Print specific scala header files
 */
void Graph::printScalaHeader(string config_path, string package_name) {
    std::ifstream _in_file(config_path);
    Json::Value _root_json;

    _in_file >> _root_json;

    assert(!_root_json["import"].empty() && "Config should contain import key");

    // TODO add one level of package to the config json file
    outCode << "package " << package_name << "\n\n";

    for (auto _it_obj = _root_json["import"].begin();
         _it_obj != _root_json["import"].end(); _it_obj++) {
        if (_it_obj->isArray()) {
            outCode << "import " << _it_obj.key().asString() << "._\n";
            for (auto &elem : *_it_obj) {
                outCode << "import " << _it_obj.key().asString() << "."
                        << elem.asString() << "._\n";
            }
        }
    }
}

/**
 * Returning instruction list
 */
// InstructionList Graph::getInstructionList() { return &this->inst_list; }

/**
 * Insert a new instruction
 */
// void Graph::insertInstruction(llvm::Instruction &ins) {}

/**
 * Insert a new basic block
 */
SuperNode *Graph::insertSuperNode(BasicBlock &BB) {
    super_node_list.push_back(std::make_unique<SuperNode>(
        NodeInfo(super_node_list.size(), BB.getName().str()), &BB));
    auto ff = std::find_if(
        super_node_list.begin(), super_node_list.end(),
        [&BB](auto &arg) -> bool { return arg.get()->getBasicBlock() == &BB; });

    return ff->get();
}

/**
 * Insert a new computation instruction
 */
InstructionNode *Graph::insertBinaryOperatorNode(BinaryOperator &I) {
    inst_list.push_back(std::make_unique<BinaryOperatorNode>(
        NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(
        inst_list.begin(), inst_list.end(),
        [&I](auto &arg) -> bool { return arg.get()->getInstruction() == &I; });
    ff->get()->printDefinition(PrintType::Scala);

    return ff->get();
}

/**
 * Insert a new computation instruction
 */
InstructionNode *Graph::insertIcmpOperatorNode(ICmpInst &I) {
    inst_list.push_back(std::make_unique<IcmpNode>(
        NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(
        inst_list.begin(), inst_list.end(),
        [&I](auto &arg) -> bool { return arg.get()->getInstruction() == &I; });
    return ff->get();
}

/**
 * Insert a new computation Branch
 */
InstructionNode *Graph::insertBranchNode(BranchInst &I) {
    if (I.getName().str() == "")
        inst_list.push_back(std::make_unique<BranchNode>(
            NodeInfo(inst_list.size(),
                     "br_" + std::to_string(inst_list.size())),
            &I));
    else
        inst_list.push_back(std::make_unique<BranchNode>(
            NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(
        inst_list.begin(), inst_list.end(),
        [&I](auto &arg) -> bool { return arg.get()->getInstruction() == &I; });
    return ff->get();
}

/**
 * Insert a new computation PhiNode
 */
InstructionNode *Graph::insertPhiNode(PHINode &I) {
    inst_list.push_back(std::make_unique<PhiSelectNode>(
        NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(
        inst_list.begin(), inst_list.end(),
        [&I](auto &arg) -> bool { return arg.get()->getInstruction() == &I; });
    return ff->get();
}

/**
 * Insert a new Alloca node
 */
InstructionNode *Graph::insertAllocaNode(AllocaInst &I) {
    inst_list.push_back(std::make_unique<AllocaNode>(
        NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(
        inst_list.begin(), inst_list.end(),
        [&I](auto &arg) -> bool { return arg.get()->getInstruction() == &I; });
    return ff->get();
}

/**
 * Insert a new GEP node
 */
InstructionNode *Graph::insertGepNode(GetElementPtrInst &I) {
    inst_list.push_back(std::make_unique<GEPNode>(
        NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(
        inst_list.begin(), inst_list.end(),
        [&I](auto &arg) -> bool { return arg.get()->getInstruction() == &I; });
    return ff->get();
}

/**
 * Insert a new Load node
 */
InstructionNode *Graph::insertLoadNode(LoadInst &I) {
    inst_list.push_back(std::make_unique<LoadNode>(
        NodeInfo(inst_list.size(), "ld_" + std::to_string(inst_list.size())),
        &I, this->getMemoryUnit()));

    auto ff = std::find_if(
        inst_list.begin(), inst_list.end(),
        [&I](auto &arg) -> bool { return arg.get()->getInstruction() == &I; });
    return ff->get();
}

/**
 * Insert a new Store node
 */
InstructionNode *Graph::insertStoreNode(StoreInst &I) {
    inst_list.push_back(std::make_unique<StoreNode>(
        NodeInfo(inst_list.size(), "st_" + std::to_string(inst_list.size())),
        &I, this->getMemoryUnit()));
    // NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(
        inst_list.begin(), inst_list.end(),
        [&I](auto &arg) -> bool { return arg.get()->getInstruction() == &I; });
    return ff->get();
}

/**
 * Insert a new Call node
 */
InstructionNode *Graph::insertCallNode(CallInst &I) {
    inst_list.push_back(std::make_unique<CallNode>(
        NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(
        inst_list.begin(), inst_list.end(),
        [&I](auto &arg) -> bool { return arg.get()->getInstruction() == &I; });
    return ff->get();
}

/**
 * Insert a new Store node
 */
InstructionNode *Graph::insertReturnNode(ReturnInst &I) {
    if (I.getName().str() == "")
        inst_list.push_back(std::make_unique<ReturnNode>(
            NodeInfo(inst_list.size(),
                     "ret_" + std::to_string(inst_list.size())),
            &I));
    else
        inst_list.push_back(std::make_unique<ReturnNode>(
            NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(
        inst_list.begin(), inst_list.end(),
        [&I](auto &arg) -> bool { return arg.get()->getInstruction() == &I; });
    return ff->get();
}

/**
 * Insert a new function argument
 */
ArgumentNode *Graph::insertFunctionArgument(Argument &AR) {
    arg_list.push_back(std::make_unique<ArgumentNode>(
        NodeInfo(arg_list.size(), AR.getName().str() + "_arg"),
        this->getSplitCall(), &AR));

    auto ff = std::find_if(arg_list.begin(), arg_list.end(),
                           [&AR](auto &arg) -> bool {
                               return arg.get()->getArgumentValue() == &AR;
                           });
    return ff->get();
}

/**
 * Insert a new Store node
 */
GlobalValueNode *Graph::insertFunctionGlobalValue(GlobalValue &G) {
    glob_list.push_back(
        GlobalValueNode(NodeInfo(glob_list.size(), G.getName().str()), &G));

    auto ff = std::find_if(glob_list.begin(), glob_list.end(),
                           [&G](GlobalValueNode &gl) -> bool {
                               return gl.getGlobalValue() == &G;
                           });
    return &*ff;
}

/**
 * Insert a new Edge
 */
Edge *Graph::insertEdge(Edge::EdgeType _typ, Node *_node_src, Node *_node_dst) {
    edge_list.push_back(std::make_unique<Edge>(_typ, _node_src, _node_dst));
    auto ff = std::find_if(
        edge_list.begin(), edge_list.end(),
        [_node_src, _node_dst](std::unique_ptr<Edge> &e) -> bool {
            return (e->getSrc() == _node_src) && (e->getTar() == _node_dst);
        });
    return ff->get();
}

bool Graph::edgeExist(Node *_node_src, Node *_node_dst) {
    auto ff = std::find_if(
        edge_list.begin(), edge_list.end(),
        [_node_src, _node_dst](std::unique_ptr<Edge> &e) -> bool {
            return (e->getSrc() == _node_src) && (e->getTar() == _node_dst);
        });
    return ff != edge_list.end();
}

/**
 * Find an edge
 */


/**
 * Inserting memory edges
 */
Edge *Graph::insertMemoryEdge(Edge::EdgeType _edge_type, Node *_node_src,
                              Node *_node_dst) {
    edge_list.push_back(
        std::make_unique<Edge>(_edge_type, _node_src, _node_dst));
    auto ff = std::find_if(
        edge_list.begin(), edge_list.end(),
        [_node_src, _node_dst](std::unique_ptr<Edge> &e) -> bool {
            return (e->getSrc() == _node_src) && (e->getTar() == _node_dst);
        });

    return ff->get();
}

/**
 * Insert a new const node
 */
ConstIntNode *Graph::insertConstIntNode(ConstantInt &C) {
    const_list.push_back(
        ConstIntNode(NodeInfo(const_list.size(),
                              "const" + std::to_string(const_list.size())),
                     &C));

    auto ff = std::find_if(const_list.begin(), const_list.end(),
                           [&C](ConstIntNode &cs) -> bool {
                               return cs.getConstantParent() == &C;
                           });
    return &*ff;
}

LoopNode *Graph::insertLoopNode(std::unique_ptr<LoopNode> _ln) {
    auto _node_p = _ln.get();
    loop_nodes.push_back(std::move(_ln));

    auto ff = std::find_if(loop_nodes.begin(), loop_nodes.end(),
                           [_node_p](std::unique_ptr<LoopNode> &l) -> bool {
                               return l.get() == _node_p;
                           });

    return ff->get();
}

/**
 * Set function pointer
 */
void Graph::setFunction(Function *_fn) { this->function_ptr = _fn; }

void Graph::removeEdge(Node *_src, Node *_dest) {
    this->edge_list.remove_if([_src, _dest](std::unique_ptr<Edge> &_e) -> bool {
                       return (_e->getSrc() == _src) && (_e->getTar() == _dest);
                   });
}

void Graph::breakEdge(Node *_src, Node *_tar, Node *_new_node) {
    // Update the control dependencies
    /**
     * The function does three important tasks:
     * 1) Remove destination node form the source data port list
     * 2) Remove source node from the destination data port list
     * 3) Remove the edge between source and destination node
     */
    _tar->removeNodeControlInputNode(_src);
    _src->removeNodeControlOutputNode(_tar);

    // Add loop control signals
    this->insertEdge(Edge::ControlTypeEdge, _src, _new_node);
    this->insertEdge(Edge::ControlTypeEdge, _new_node, _tar);
    _src->addControlOutputPortBack(_new_node);
    _new_node->addControlInputPort(_src);

    _new_node->addControlOutputPortBack(_tar);

    if (isa<LoopNode>(_new_node))
        dyn_cast<SuperNode>(_tar)->setActivateInput(_new_node);
    else
        _tar->addControlOutputPortBack(_new_node);
}

/**
 * Print loop control signals
 */
void Graph::printLoopBranchEdges(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(dbgs() << "\t Printing Control signals:\n");
            this->outCode << helperScalaPrintHeader(
                "Loop -> predicate instruction");
            for (auto &_l_node : loop_nodes) {
                for (auto _enable_iterator = _l_node->inputControl_begin();
                     _enable_iterator != _l_node->inputControl_end();
                     _enable_iterator++) {
                    auto _input_node = dyn_cast<Node>(*_enable_iterator);
                    auto _input_index =
                        _l_node->returnControlInputPortIndex(_input_node);
                    auto _output_index =
                        _input_node->returnControlOutputPortIndex(
                            _l_node.get());

                    this->outCode << "  "
                                  << _l_node->printInputEnable(PrintType::Scala,
                                                               _input_index)
                                  << " <> "
                                  << _input_node->printOutputEnable(
                                         PrintType::Scala, _output_index)
                                  << "\n\n";
                }
            }

            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}
