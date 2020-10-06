
#ifndef DEBUGINFO_H
#define DEBUGINFO_H


#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <stdint.h>

namespace debuginfo{


struct DebugInfo : public llvm::ModulePass {
  static char ID;

  std::map<llvm::Instruction *, llvm::BasicBlock *> inst_bb;

  uint32_t node_id;

  DebugInfo(uint32_t id) : llvm::ModulePass(ID), node_id(id) {}

  bool runOnModule(llvm::Module& m) override;
};


}  // namespace debuginfo


#endif
