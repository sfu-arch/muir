//===----------------------------------------------------------------------===//
//
// This file declares the Node class.
//
//===----------------------------------------------------------------------===//

#ifndef XKETCH_NODE_H
#define XKETCH_NODE_H

#include"Xketch/Node/Type.h"

#include<map>
#include<list>

namespace dandelion{

    class Node{
        private:
            std::list<Node*> node_predecessors;
            std::list<Node*> node_sucessors;

        public:
            virtual Type ReturnType();

    };

//template <typename NodeTy> class SymbolTableListTraits;

} // End dandelion namespace

#endif // XKETCH_NODE_H
