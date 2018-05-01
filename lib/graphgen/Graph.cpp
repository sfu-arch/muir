#define DEBUG_TYPE "graphgen"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Dandelion/Graph.h"
#include "Dandelion/Node.h"
#include "luacpptemplater/LuaTemplater.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace llvm;
using namespace dandelion;

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
            printInputSpliter();
            printBasicBlocks(PrintType::Scala);
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
            for (auto &bb_node : this->super_node_list) {
                outCode << bb_node.PrintDefinition(PrintType::Scala);
            }
            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
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
            outCode << memory_unit.PrintDefinition(PrintType::Scala);
            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

void Graph::printInputSpliter() {
    this->outCode << split_call.PrintDefinition();
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
        if (auto _fc = dyn_cast<CallNode>(&_ins)) {
            // Call arguments to subroutine
            command = "    val {{call}}_out = new CallDecoupled(List(";
            ins_template.set("call", _ins.getName());
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
const InstructionList Graph::getInstructionList() { return this->inst_list; }

/**
 * Insert a new instruction
 */
void Graph::insertInstruction(llvm::Instruction &ins) {}

/**
 * Insert a new basic block
 */
SuperNode *const Graph::insertSuperNode(BasicBlock &BB) {
    super_node_list.push_back(
        SuperNode(NodeInfo(super_node_list.size(), BB.getName().str()), &BB));
    auto ff = std::find_if(
        super_node_list.begin(), super_node_list.end(),
        [&BB](SuperNode &arg) -> bool { return arg.getBasicBlock() == &BB; });

    return &*ff;
}

/**
 * Insert a new computation instruction
 */
InstructionNode *const Graph::insertBinaryOperatorNode(BinaryOperator &I) {
    inst_list.push_back(
        BinaryOperatorNode(NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(inst_list.begin(), inst_list.end(),
                           [&I](InstructionNode &arg) -> bool {
                               return arg.getInstruction() == &I;
                           });
    return &*ff;
}

/**
 * Insert a new computation instruction
 */
InstructionNode *const Graph::insertIcmpOperatorNode(ICmpInst &I) {
    inst_list.push_back(
        IcmpNode(NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(inst_list.begin(), inst_list.end(),
                           [&I](InstructionNode &arg) -> bool {
                               return arg.getInstruction() == &I;
                           });
    return &*ff;
}

/**
 * Insert a new computation Branch
 */
InstructionNode *const Graph::insertBranchNode(BranchInst &I) {
    inst_list.push_back(
        BranchNode(NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(inst_list.begin(), inst_list.end(),
                           [&I](InstructionNode &arg) -> bool {
                               return arg.getInstruction() == &I;
                           });
    return &*ff;
}

/**
 * Insert a new computation PhiNode
 */
InstructionNode *const Graph::insertPhiNode(PHINode &I) {
    inst_list.push_back(
        PhiSelectNode(NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(inst_list.begin(), inst_list.end(),
                           [&I](InstructionNode &arg) -> bool {
                               return arg.getInstruction() == &I;
                           });
    return &*ff;
}

/**
 * Insert a new Alloca node
 */
InstructionNode *const Graph::insertAllocaNode(AllocaInst &I) {
    inst_list.push_back(
        AllocaNode(NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(inst_list.begin(), inst_list.end(),
                           [&I](InstructionNode &arg) -> bool {
                               return arg.getInstruction() == &I;
                           });
    return &*ff;
}

/**
 * Insert a new GEP node
 */
InstructionNode *const Graph::insertGepNode(GetElementPtrInst &I) {
    inst_list.push_back(
        GEPNode(NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(inst_list.begin(), inst_list.end(),
                           [&I](InstructionNode &arg) -> bool {
                               return arg.getInstruction() == &I;
                           });
    return &*ff;
}

/**
 * Insert a new Load node
 */
InstructionNode *const Graph::insertLoadNode(LoadInst &I) {
    inst_list.push_back(
        LoadNode(NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(inst_list.begin(), inst_list.end(),
                           [&I](InstructionNode &arg) -> bool {
                               return arg.getInstruction() == &I;
                           });
    return &*ff;
}

/**
 * Insert a new Store node
 */
InstructionNode *const Graph::insertStoreNode(StoreInst &I) {
    inst_list.push_back(
        StoreNode(NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(inst_list.begin(), inst_list.end(),
                           [&I](InstructionNode &arg) -> bool {
                               return arg.getInstruction() == &I;
                           });
    return &*ff;
}

/**
 * Insert a new Call node
 */
InstructionNode *const Graph::insertCallNode(CallInst &I) {
    inst_list.push_back(
        CallNode(NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(inst_list.begin(), inst_list.end(),
                           [&I](InstructionNode &arg) -> bool {
                               return arg.getInstruction() == &I;
                           });
    return &*ff;
}

/**
 * Insert a new Store node
 */
InstructionNode *const Graph::insertReturnNode(ReturnInst &I) {
    inst_list.push_back(
        ReturnNode(NodeInfo(inst_list.size(), I.getName().str()), &I));

    auto ff = std::find_if(inst_list.begin(), inst_list.end(),
                           [&I](InstructionNode &arg) -> bool {
                               return arg.getInstruction() == &I;
                           });
    return &*ff;
}

/**
 * Insert a new function argument
 */
ArgumentNode *const Graph::insertFunctionArgument(Argument &AR) {
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
GlobalValueNode *const Graph::insertFunctionGlobalValue(GlobalValue &G) {
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
Edge *const Graph::insertEdge(Edge::EdgeType _typ, Node *const _node_src,
                              Node *const _node_dst) {
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
Edge *const Graph::insertMemoryEdge(Edge::EdgeType _edge_type,
                                    Node *const _node_src,
                                    Node *const _node_dst) {
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
ConstIntNode *const Graph::insertConstIntNode(ConstantInt &C) {
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
