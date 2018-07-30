#ifndef DANDELION_EDGE_H
#define DANDELION_EDGE_H
#include <stdint.h>
#include <list>

#include "llvm/IR/Argument.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

#include "Dandelion/Node.h"

using namespace llvm;

namespace dandelion {

class Edge {
   public:
    using Port = std::pair<Node *, PortID>;

    enum EdgeType {
        DataTypeEdge = 0,
        ControlTypeEdge,
        MaskTypeEdge,
        MemoryReadTypeEdge,
        MemoryWriteTypeEdge,
        UknownType
    };

   private:
    EdgeType edge_type;
    Port src;
    Port tar;

   public:
    explicit Edge(EdgeType _ty) : edge_type(_ty) {}
    explicit Edge(Port _src, Port _tar)
        : edge_type(UknownType), src(_src), tar(_tar) {}
    explicit Edge(EdgeType _ty = UknownType, Port _src = {nullptr, 0},
                  Port _tar = {nullptr, 0})
        : edge_type(_ty), src(_src), tar(_tar) {}

    uint32_t getType() const { return edge_type; }

    Port getSrc() const { return src; }
    Port getTar() const { return tar; }

    bool operator == (const Edge &rhs) const {
        return this->getSrc() == rhs.getSrc();
    }
};
}

#endif  // end of DANDDELION_EDGE_H
