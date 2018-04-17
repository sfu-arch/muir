#define DEBUG_TYPE "graphgen"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Dandelion/Graph.h"
#include "Dandelion/Node.h"
#include "luacpptemplater/LuaTemplater.h"

#include <iostream>

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
void Graph::init(BasicBlockList &_bb_ll, InstructionList &_ins_ll,
                 ArgumentList &_arg_ll, GlobalValueList &_glb_ll,
                 ConstIntList &_con_ll, EdgeList &_edge) {
    //std::copy(_bb_ll.begin(), _bb_ll.end(), std::back_inserter(_bb_ll));
    //std::copy(bb_list.begin(), bb_list.end(), _bb_ll.begin());
    std::copy(inst_list.begin(), inst_list.end(), _ins_ll.begin());
    std::copy(arg_list.begin(), arg_list.end(), _arg_ll.begin());
    std::copy(glob_list.begin(), glob_list.end(), _glb_ll.begin());
    std::copy(const_list.begin(), const_list.end(), _con_ll.begin());
    std::copy(edge_list.begin(), edge_list.end(), _edge.begin());

    outs() << "Init the graph\n";
    outs() << "Size" << bb_list.size() << "\n";

    graph_empty = true;
}

/**
 * Print function prints the generated graph in the choosen format
 */
void Graph::printGraph(PrintType _pt) {
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(outs() << "Print Graph information!\n");
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
void Graph::printBasicBlocks(PrintType _pt){
    switch (_pt) {
        case PrintType::Scala:
            DEBUG(outs() << "\t Print BasicBlocks information!\n");
            DEBUG(outs() << "Size: " << bb_list.size() << "\n");
            for(auto &bb_node : this->bb_list){
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
 * Returning instruction list
 */
const InstructionList Graph::getInstructionList(){
    return this->inst_list;
}

/**
 * Insert a new instruction
 */
const Node* Graph::insertInstruction(llvm::Instruction){
}

/**
 * Insert a new instruction
 */
const SuperNode* Graph::insertSuperNode(BasicBlock &BB){
    super_node_list.push_back(SuperNode(&BB));
    auto ff = std::find_if(
        super_node_list.begin(), super_node_list.end(),
        [&BB](SuperNode &arg) -> bool { return arg.getBasicBlock() == &BB; });

    return &*ff;
}














