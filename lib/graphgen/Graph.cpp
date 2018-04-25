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

//===----------------------------------------------------------------------===//
//                           Graph Class
//===----------------------------------------------------------------------===//
//

/**
 * init function accepts graph elements and build a new graph containing all the
 * elements
 */
// void Graph::init(BasicBlockList &_bb_ll, InstructionList &_ins_ll,
// ArgumentList &_arg_ll, GlobalValueList &_glb_ll,
// ConstIntList &_con_ll, EdgeList &_edge) {
//// std::copy(_bb_ll.begin(), _bb_ll.end(), std::back_inserter(_bb_ll));
//// std::copy(bb_list.begin(), bb_list.end(), _bb_ll.begin());
// std::copy(inst_list.begin(), inst_list.end(), _ins_ll.begin());
// std::copy(arg_list.begin(), arg_list.end(), _arg_ll.begin());
// std::copy(glob_list.begin(), glob_list.end(), _glb_ll.begin());
// std::copy(const_list.begin(), const_list.end(), _con_ll.begin());
// std::copy(edge_list.begin(), edge_list.end(), _edge.begin());

// outs() << "Init the graph\n";
// outs() << "Size" << super_node_list.size() << "\n";

// graph_empty = true;
//}

/**
 * Print function prints the generated graph in the choosen format
 */
void Graph::printGraph(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(outs() << "Print Graph information!\n");
            printScalaHeader();
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
            DEBUG(outs() << "\t Print BasicBlocks information!\n");
            DEBUG(outs() << "\t Number of BB: " << super_node_list.size()
                         << "\n");
            for (auto &bb_node : this->super_node_list) {
                bb_node.PrintDefinition(PrintType::Scala);
            }
            break;
        case PrintType::Dot:
            assert(!"Dot file format is not supported!");
        default:
            assert(!"Uknown print type!");
    }
}

/**
 * Print specific scala header files
 */
void Graph::printScalaHeader(){

    std::ifstream _in_file("config.json");
    Json::Value _root_json;

    _in_file >> _root_json;

    for(auto _it_obj =  _root_json["import"].begin(); _it_obj != _root_json["import"].end(); _it_obj++){

        if(_it_obj->isArray()){
            outs() << _it_obj.key().asString() << "\n";
            for(auto &elem : *_it_obj){
                outs() << elem.asString() << "\n";
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
 * Insert a new Store node
 */
ConstIntNode *const Graph::insertConstIntNode(ConstantInt &C) {
    const_list.push_back(
            ConstIntNode(
                NodeInfo(const_list.size(),"const" + std::to_string(const_list.size())),&C));

    auto ff = std::find_if(const_list.begin(), const_list.end(),
                           [&C](ConstIntNode &cs) -> bool {
                               return cs.getConstantParent() == &C;
                           });
    return &*ff;
}
