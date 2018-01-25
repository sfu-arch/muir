//===----------------------------------------------------------------------===//
//
// This file declares the Function class.
//
//===----------------------------------------------------------------------===//

#ifndef XKETCH_NODE_FUNCTION_H
#define XKETCH_NODE_FUNCTION_H

#include <algorithm>
#include <map>
#include <string>
#include <utility>

#include "Xketch/Node/Node.h"

namespace dandelion {

class Function {
   private:
    uint32_t ID;
    std::string function_name;

    std::list<Node*> function_arguments;
    std::list<Node*> function_basicblocks;
    std::list<Node*> function_instructions;
    std::list<Node*> function_global_variables;

   public:
    Function() = delete;
    explicit Function(uint32_t id, std::string fn)
        : ID(id), function_name(fn) {}
    explicit Function(uint32_t id, std::string fn, std::list<Node*> fn_arg,
                      std::list<Node*> fn_bb, std::list<Node*> fn_ins,
                      std::list<Node*> fn_gv)
        : ID(id),
          function_name(fn),
          function_arguments(std::move(fn_arg)),
          function_basicblocks(std::move(fn_bb)),
          function_instructions(std::move(fn_ins)),
          function_global_variables(std::move(fn_gv)) {}
};

}  // End dandelion namespace

#endif  // XKETCH_NODE_FUNCTION_H
