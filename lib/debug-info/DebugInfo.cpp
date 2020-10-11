#include "DebugInfo.h"

#include <features.h>

#include <map>

#include "llvm/Analysis/CFG.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;
using debuginfo::DebugInfo;

namespace debuginfo {

char DebugInfo::ID = 0;

}

uint32_t
getUID(Instruction* I) {
  auto* N = I->getMetadata("UID");
  if (N == nullptr)
    return 0;
  auto* S = dyn_cast<MDString>(N->getOperand(0));
  return stoi(S->getString().str());
}

// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool
DebugInfo::runOnModule(Module& m) {
  for (auto& f : m) {
    // If function is not target, continue
    if (f.getName() != this->function_name)
      continue;

    for (auto& bb : f) {
      // for( auto it = llvm::pred_begin(&bb), et = llvm::pred_end(&bb);
      // it != et; it++){ BasicBlock *test = *it; outs() << "BasicBlock:
      // \n"; test->print(outs());
      //}
      for (auto& ins : bb) {
        // getting instruction's UID
        auto inst_uid = getUID(&ins);

        if (inst_uid == this->node_id) {
          // If intruction is PHI find all the child's dependencies
          if (auto phi_ins = dyn_cast<llvm::PHINode>(&ins)) {
            for (auto& op : phi_ins->operands()) {
              if (auto op_ins = dyn_cast<llvm::Instruction>(op.get())) {
                this->node_operands[&ins].push_back(getUID(op_ins));
                for (auto& op_op_phi : op_ins->operands()) {
                  if (auto op_op_ins = dyn_cast<llvm::Instruction>(op_op_phi.get())) {
                    if (op_op_ins->getParent() == op_ins->getParent()) {
                      this->node_operands[&ins].push_back(getUID(op_op_ins));
                      op->print(outs());
                      outs() << "\n";
                    }
                  }
                }
              } else {
                // TODO: Enable this part after adding UIDs to
                // function arguments
                continue;
              }
            }

          } else {
            outs() << "\n Instruction (" << this->node_id << "): \n";
            ins.print(outs());
            outs() << "\n";
            for (auto& op : ins.operands()) {
              if (auto op_ins = dyn_cast<llvm::Instruction>(op.get())) {
                if (op_ins->getParent() == ins.getParent()) {
                  this->node_operands[&ins].push_back(getUID(op_ins));
                  for (auto& op_op_ins : op_ins->operands()) {
                    if (auto opins_op = dyn_cast<llvm::Instruction>(op_op_ins)) {
                      if (opins_op->getParent() == ins.getParent()) {
                        this->node_operands[&ins].push_back(getUID(opins_op));
                      }
                    }
                  }
                  op->print(outs());
                  outs() << "\n";
                }
              } else {
                // TODO: Enable this part after adding UIDs to
                // function arguments
                continue;
              }
            }
            inst_bb.insert({&ins, &bb});
          }
        }
      }
    }
  }

  return false;
}

