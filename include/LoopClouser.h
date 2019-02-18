#ifndef EPPPATHPRINTER_H
#define EPPPATHPRINTER_H

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

#include <fstream>
#include <set>
#include <string>

namespace loopclouser {

struct LoopSummary {
    llvm::Instruction *enable;
    llvm::Instruction *loop_back;
    llvm::SetVector<llvm::Instruction *>loop_finish;

    llvm::BasicBlock *header;
    llvm::SmallVector<llvm::BasicBlock *, 8> exit_blocks;


    LoopSummary() : enable(nullptr), loop_back(nullptr){}
};

struct LoopClouser : public llvm::ModulePass {
    static char ID;
    llvm::DenseMap<llvm::Loop *, LoopSummary> loop_sum;
    llvm::DenseMap<llvm::Instruction *, llvm::BasicBlock *> blacklist_control_edge;

    uint32_t lid;
    LoopClouser() : llvm::ModulePass(ID), lid(0) {}

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        //AU.addRequiredID(llvm::BreakCriticalEdgesID);
        AU.addRequiredID(llvm::LoopSimplifyID);
        AU.addRequired<llvm::LoopInfoWrapperPass>();
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
        AU.setPreservesAll();
    }

    virtual bool runOnModule(llvm::Module &) override;
    bool doInitialization(llvm::Module &) override;
    bool doFinalization(llvm::Module &) override;
    LoopSummary summarizeLoop(llvm::Loop*, llvm::LoopInfo&);
};
}

#endif
