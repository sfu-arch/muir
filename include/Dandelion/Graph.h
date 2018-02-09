#ifndef DANDELION_GRAPH_H
#define DANDELION_GRAPH_H
#include <stdint.h>
#include <list>

#include "llvm/IR/Argument.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"

#include "Dandelion/Edge.h"
#include "Dandelion/Node.h"

namespace dandelion {

using InstructionList = std::list<InstructionNode>;
using ArgumentList = std::list<ArgumentNode>;
using BasicBlockList = std::list<SuperNode>;
using GlobalValueList = std::list<GlobalValueNode>;
using ConstIntList = std::list<ConstIntNode>;
using EdgeList = std::list<Edge>;

class Graph {
   private:
    InstructionList inst_list;
    ArgumentList arg_list;
    BasicBlockList bb_list;
    GlobalValueList glob_list;
    ConstIntList const_list;
    EdgeList edge_list;

    bool graph_empty;

   public:
    explicit Graph():graph_empty(false) {}
    explicit Graph(BasicBlockList _bb_ll, InstructionList &_ins_ll,
                   ArgumentList &_arg_ll, GlobalValueList &_glb_ll,
                   ConstIntList &_con_ll, EdgeList &_edge):graph_empty(true) {
        std::copy(_bb_ll.begin(), _bb_ll.end(), bb_list.begin());
        std::copy(inst_list.begin(), inst_list.end(), _ins_ll.begin());
        std::copy(arg_list.begin(), arg_list.end(), _arg_ll.begin());
        std::copy(glob_list.begin(), glob_list.end(), _glb_ll.begin());
        std::copy(const_list.begin(), const_list.end(), _con_ll.begin());
        std::copy(edge_list.begin(), edge_list.end(), _edge.begin());
    }

    void init(BasicBlockList &, InstructionList &, ArgumentList &,
              GlobalValueList &, ConstIntList &, EdgeList &);

    void printGraph();

    bool isEmpty(){return graph_empty;}
};
}

#endif  // end of DANDDELION_GRAPH_H
