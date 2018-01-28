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
                  std::list<Node *> _list_suc, Type *_ty)
        : ID(_id),
          Name(_nm),
          node_sucessors(std::move(_list_suc)),
          node_predecessors(std::move(_list_pred)),
          node_type(_ty) {}

    ~Node() = default;

    Type::TypeID ReturnType() const { return node_type->getTypeID(); }

    // TODO define usefule public functions
};

class BasicBlock : public Node {
   private:
    Function *parent_function;

   public:
    using Node::Node;
    explicit BasicBlock(uint32_t _id, std::string _nm,
                        std::list<Node *> _list_pred,
                        std::list<Node *> _list_suc, Type *_ty, Function *PF)
        : Node(_id, _nm, _list_pred, _list_suc, _ty), parent_function(PF) {}

    void setParentFunction(Function *PF);
};

class Instruction : public Node {
   private:
    Function *parent_function;
    BasicBlock *parent_basicblock;

   public:
    using Node::Node;
    explicit Instruction(uint32_t _id, std::string _nm,
                         std::list<Node *> _list_pred,
                         std::list<Node *> _list_suc, Type *_tp, Function *PF,
                         BasicBlock *PB)
        : Node(_id, _nm, _list_pred, _list_suc, _tp),
          parent_function(PF),
          parent_basicblock(PB) {}
};

}  // End dandelion namespace

#endif  // XKETCH_NODE_H
