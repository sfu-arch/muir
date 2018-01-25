//===----------------------------------------------------------------------===//
//
// This file declares the Node class.
//
//===----------------------------------------------------------------------===//

#ifndef XKETCH_NODE_NODE_H
#define XKETCH_NODE_NODE_H

#include "Xketch/Node/Function.h"
#include "Xketch/Node/Type.h"

#include <algorithm>
#include <list>
#include <map>
#include <utility>

namespace dandelion {

class Node {
   private:
    uint32_t ID;
    std::string Name;
    std::list<Node *> node_sucessors;
    std::list<Node *> node_predecessors;
    Type *node_type;

   public:
    explicit Node();
    explicit Node(uint32_t _id, std::string _nm, std::list<Node *> _list_pred,
                  std::list<Node *> _list_suc)
        : ID(_id),
          Name(_nm),
          node_sucessors(std::move(_list_suc)),
          node_predecessors(std::move(_list_pred)) {}

    ~Node() = default;

    Type::TypeID ReturnType() const { return node_type->getTypeID(); }

    // TODO define usefule public functions
};

class BasicBlock : public Node {
   private:
    Function *parent_function;

   public:
    explicit BasicBlock(Function *PF)
        : parent_function(PF) {}
};

class Instruction : public Node {
   private:
    Function *parent_function;
    Node *parent_basicblock;

   public:
    explicit Instruction(Function *PF, Node *PB)
        : parent_function(PF), parent_basicblock(PB) {}
};

}  // End dandelion namespace

#endif  // XKETCH_NODE_H
