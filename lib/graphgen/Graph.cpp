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

    std::copy(bb_list.begin(), bb_list.end(), _bb_ll.begin());
    std::copy(inst_list.begin(), inst_list.end(), _ins_ll.begin());
    std::copy(arg_list.begin(), arg_list.end(), _arg_ll.begin());
    std::copy(glob_list.begin(), glob_list.end(), _glb_ll.begin());
    std::copy(const_list.begin(), const_list.end(), _con_ll.begin());
    std::copy(edge_list.begin(), edge_list.end(), _edge.begin());

    graph_empty = true;
}
