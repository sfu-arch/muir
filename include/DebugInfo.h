
#ifndef DEBUGINFO_H
#define DEBUGINFO_H

#include <stdint.h>

#include <string>
#include <map>
#include <vector>
#include <set>

#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

namespace debuginfo {

struct DebugInfo : public llvm::ModulePass {
  static char ID;

  std::map<llvm::Instruction*, llvm::BasicBlock*> inst_bb;
  std::map<llvm::Instruction*, std::set<uint32_t>> node_operands;

  std::string function_name;
  int node_id;

  bool print_values;

  DebugInfo(std::string function_name, uint32_t id)
    : llvm::ModulePass(ID),
      function_name(function_name),
      node_id(id),
      print_values(false) {}

  DebugInfo(std::string function_name, bool print_verbose)
    : llvm::ModulePass(ID),
      function_name(function_name),
      node_id(-1),
      print_values(print_verbose) {}

  bool runOnModule(llvm::Module& m) override;
  bool doFinalization(llvm::Module& M) override;
};

}  // namespace debuginfo

#endif
