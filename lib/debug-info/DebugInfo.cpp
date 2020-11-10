#define DEBUG_TYPE "debug-info"


#include <features.h>

#include "llvm/Analysis/CFG.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"

#include "DebugInfo.h"

#include <map>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <set>


using namespace llvm;
using debuginfo::DebugInfo;

namespace debuginfo {

char DebugInfo::ID = 0;
// static RegisterPass<DebugInfo> X("debug-info", "Getting debug info");
}  // namespace debuginfo

static uint32_t
getUID(Instruction* I) {
  auto* N = I->getMetadata("UID");
  if (N == nullptr)
    return 0;
  auto* S = dyn_cast<MDString>(N->getOperand(0));
  return stoi(S->getString().str());
}

static uint32_t
getARGID(Argument* ARG) {
  auto* N = ARG->getParent()->getMetadata("ARGID_" + ARG->getName().str());
  if (N == nullptr)
    return 0;
  auto* S = dyn_cast<MDString>(N->getOperand(0));
  return stoi(S->getString().str());
}

/// This function recursively search for the operand nodes, and if the operands are in the
/// same basicblock and are not PHI, it adds them under node_operands map
static void
visitPhiOperands(llvm::Instruction* ins,
                 std::set<uint32_t>& node_operands,
                 std::set<BasicBlock*>& visited_bb) {
  auto find_bb = visited_bb.find(ins->getParent());
  if (find_bb != visited_bb.end())
    return;
  // If intruction is PHI find all the child's dependencies
  for (auto& op : ins->operands()) {
    if (auto op_ins = dyn_cast<llvm::Instruction>(op.get())) {
      if (op_ins->getParent() == ins->getParent()) {
        node_operands.insert(getUID(op_ins));
      }
    } else {
      // TODO: Enable this part after adding UIDs to
      // function arguments
      continue;
    }
  }
}


/// This function recursively search for the operand nodes, and if the operands are in the
/// same basicblock and are not PHI, it adds them under node_operands map
static void
visitOperands(llvm::Instruction* ins,
              std::set<uint32_t>& node_operands,
              std::set<BasicBlock*>& visited_bb) {
  auto find_bb = visited_bb.find(ins->getParent());
  if (find_bb != visited_bb.end())
    return;
  // If intruction is PHI find all the child's dependencies
  if (auto phi_ins = dyn_cast<llvm::PHINode>(ins)) {
    for (auto& op : phi_ins->operands()) {
      if (auto op_ins = dyn_cast<llvm::Instruction>(op.get())) {
        visited_bb.insert(ins->getParent());
        visitPhiOperands(op_ins, node_operands, visited_bb);
      } else if (auto arg = dyn_cast<Argument>(op.get())) {
        arg->print(outs());
        node_operands.insert(getARGID(arg));
      } else {
        // The value is constant
        continue;
      }
    }
  } else {
    for (auto& op : ins->operands()) {
      if (auto op_ins = dyn_cast<llvm::Instruction>(op.get())) {
        if (op_ins->getParent() == ins->getParent()) {
          node_operands.insert(getUID(op_ins));
          visitOperands(op_ins, node_operands, visited_bb);
          visited_bb.insert(ins->getParent());
        }
      } else {
        // TODO: Enable this part after adding UIDs to
        // function arguments
        continue;
      }
    }
  }
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
        std::set<BasicBlock*> visited_bb;
        if (int(this->node_id) != -1) {
          if (inst_uid == this->node_id) {
            std::set<uint32_t> parent_ids;
            visitOperands(&ins, parent_ids, visited_bb);
            this->node_operands[&ins] = parent_ids;
          }
        } else {
          std::set<uint32_t> parent_ids;
          visitOperands(&ins, parent_ids, visited_bb);
          this->node_operands[&ins] = parent_ids;
        }
      }
    }
  }

  return false;
}


bool
DebugInfo::doFinalization(llvm::Module& M) {
  if (this->print_values) {
    outs() << "Print debug nodes:\n";
    outs() << "node[ " << this->node_id << " ]: ";
    for (auto op : this->node_operands) {
      for (auto ids : op.second) {
        outs() << ids << ", ";
      }
    }
    outs() << "\n";
  }

  return false;
}

