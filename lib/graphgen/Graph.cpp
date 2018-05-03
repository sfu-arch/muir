#define DEBUG_TYPE "graphgen"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Dandelion/Graph.h"
#include "Dandelion/Node.h"
#include "luacpptemplater/LuaTemplater.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace llvm;
using namespace dandelion;

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
            printBasicBlocks(PrintType::Scala);
            printInstructions(PrintType::Scala);
            printBasickBlockPredicateEdges(PrintType::Scala);
            printBasickBLockInstructionEdges(PrintType::Scala);
            printPhiNodesConnections(PrintType::Scala);

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
                outCode << ins_node->printDefinition(PrintType::Scala);

                // if (auto _binary_ins =
                // dyn_cast<BinaryOperatorNode>(&ins_node))
                // outCode << _binary_ins->PrintDefinition(PrintType::Scala);
                // else if (auto _icmp_ins = dyn_cast<IcmpNode>(&ins_node))
                // outCode << _icmp_ins->PrintDefinition(PrintType::Scala);
                // else if (auto _br_ins = dyn_cast<BranchNode>(&ins_node))
                // outCode << _br_ins->PrintDefinition(PrintType::Scala);
                // else if (auto _phi_ins = dyn_cast<PhiSelectNode>(&ins_node))
                // outCode << _phi_ins->PrintDefinition(PrintType::Scala);
                // else if (auto _ret_ins = dyn_cast<ReturnNode>(&ins_node))
                // outCode << _ret_ins->PrintDefinition(PrintType::Scala);
                // else {
                ////ins_node.getInstruction()->dump();
                // auto _ins = dyn_cast<InstructionNode>(&ins_node);
                //_ins->getInstruction()->dump();
                // assert(!"Not supported instruction!");
                //}
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
            outCode << memory_unit.printDefinition(PrintType::Scala);
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

                    // Iterating over datainput of PHI node
                    for (auto _phi_input_it = _phi_ins->inputDataport_begin();
                         _phi_input_it != _phi_ins->inputDataport_end();
                         _phi_input_it++) {
                        auto _phi_input_node = dyn_cast<Node>(*_phi_input_it);

                        auto _input_index = std::distance(
                            _phi_ins->inputDataport_begin(), _phi_input_it);

                        this->outCode << "  "
                                      << _phi_ins->printInputData(
                                             PrintType::Scala, _input_index)
                                      << " <> "
                                      << _phi_input_node->printOutputData(
                                             PrintType::Scala, 0)
                                      << "\n\n";
                    }

                    // Adding phi node mask
                    auto _input_index = std::distance(
                        _s_node->phi_begin(), _phi_it);
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

void Graph::printScalaInputSpliter() {
    this->outCode << split_call.printDefinition(PrintType::Scala);
}

/**
 * Print the basicblock definition
 */
void Graph::printScalaFunctionHeader() {
    // print the header
    this->outCode << helperScalaPrintHeader("Printing ports definition");

    LuaTemplater ins_template;
    string final_command;
    string command =
        "abstract class {{module_name}}DFIO"
        "(implicit val p: Parameters) extends Module with CoreParams {\n"
        "  val io = IO(new Bundle {\n";
    ins_template.set("module_name", this->graph_info.Name);
    final_command.append(ins_template.render(command));

    // Print input call parameters
    command = "    val in = Flipped(new CallDecoupled(List(";
    final_command.append(ins_template.render(command));
    for (uint32_t c = 0; c < this->arg_list.size(); c++) {
        command = "32,";
        ins_template.set("index", static_cast<int>(c));
        final_command.append(ins_template.render(command));
    }
    final_command.pop_back();
    command = ")))\n";
    final_command.append(ins_template.render(command));

    // Print sub-function call interface
    uint32_t c = 0;
    for (auto &_ins : this->inst_list) {
        if (auto _fc = dyn_cast<CallNode>(_ins.get())) {
            // Call arguments to subroutine
            command = "    val {{call}}_out = new CallDecoupled(List(";
            ins_template.set("call", _ins->getName());
            final_command.append(ins_template.render(command));
            // TODO: Make sure there is no inconsistancy here
            for (auto &ag : _fc->getInstruction()->getFunction()->args()) {
                command = "32,";
                ins_template.set("index", static_cast<int>(c++));
                final_command.append(ins_template.render(command));
            }
            final_command.pop_back();
            command = "))\n";
            final_command.append(ins_template.render(command));
            // Return values from sub-routine.
            // Only supports a single 32 bit data bundle for now
            command =
                "    val {{call}}_in = Flipped(new CallDecoupled(List(32)))\n";
            ins_template.set("call", _fc->getName());
            final_command.append(ins_template.render(command));
        }
    }

    // Print global memory interface
    c = 0;
    for (uint32_t c = 0; c < this->glob_list.size(); c++) {
        command =
            "    val glob_{{index}} = Flipped(Decoupled(new "
            "DataBundle))\n";
        ins_template.set("index", static_cast<int>(c++));
        final_command.append(ins_template.render(command));
        break;
    }

    // Print cache memory interface
    final_command.append(
        "    val CacheResp = Flipped(Valid(new CacheRespT))\n"
        "    val CacheReq = Decoupled(new CacheReq)\n");

    // Print output (return) parameters
    if (!function_ptr->getReturnType()->isVoidTy()) {
        final_command.append("    val out = new CallDecoupled(List(32))\n");
    }

    final_command.append(
        "  })\n"
        "}\n\n");

    // Printing Abstract
    outCode << ins_template.render(final_command);

    final_command =
        "class {{module_name}}DF(implicit p: Parameters)"
        " extends {{module_name}}DFIO()(p) {\n";
    ins_template.set("module_name", graph_info.Name);

    helperScalaPrintHeader("Printing Module Definition");
    outCode << ins_template.render(final_command);
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
        NodeInfo(inst_list.size(), I.getName().str()), &I));

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
        NodeInfo(inst_list.size(), I.getName().str()), &I));

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
    arg_list.push_back(
        ArgumentNode(NodeInfo(arg_list.size(), AR.getName().str()), &AR));

    auto ff = std::find_if(arg_list.begin(), arg_list.end(),
                           [&AR](ArgumentNode &arg) -> bool {
                               return arg.getArgumentValue() == &AR;
                           });
    return &*ff;
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
    edge_list.push_back(Edge(_typ, _node_src, _node_dst));
    auto ff = std::find_if(edge_list.begin(), edge_list.end(),
                           [_node_src, _node_dst](Edge &e) -> bool {
                               return (e.ReturnSrc() == _node_src) &&
                                      (e.ReturnTar() == _node_dst);
                           });
    return &*ff;
}

/**
 * Inserting memory edges
 */
Edge *Graph::insertMemoryEdge(Edge::EdgeType _edge_type, Node *_node_src,
                              Node *_node_dst) {
    edge_list.push_back(Edge(_edge_type, _node_src, _node_dst));
    auto ff = std::find_if(edge_list.begin(), edge_list.end(),
                           [_node_src, _node_dst](Edge &e) -> bool {
                               return (e.ReturnSrc() == _node_src) &&
                                      (e.ReturnTar() == _node_dst);
                           });

    return &*ff;
}

/**
 * Insert a new Store node
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

/**
 * Set function pointer
 */
void Graph::setFunction(Function *_fn) { this->function_ptr = _fn; }
