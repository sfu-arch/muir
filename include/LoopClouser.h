#ifndef EPPPATHPRINTER_H
#define EPPPATHPRINTER_H

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"

#include <set>
#include <fstream>
#include <string>


namespace loopclouser {

struct LoopClouser: public llvm::ModulePass {
    static char ID;

    uint32_t lid;
    LoopClouser() : llvm::ModulePass(ID), lid(0) {}

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        AU.addRequiredID(llvm::BreakCriticalEdgesID);
        AU.addRequiredID(llvm::LoopSimplifyID);
        AU.addRequired<llvm::LoopInfoWrapperPass>();
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
        AU.setPreservesAll();
    }

    virtual bool runOnModule(llvm::Module&) override;
    bool doInitialization(llvm::Module&) override;
    bool doFinalization(llvm::Module&) override;

};


}

#endif
