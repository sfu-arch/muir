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


namespace lx {

struct TargetLoopExtractor : public llvm::ModulePass {
    static char ID;

    std::set<std::pair<std::string, int>> Locations;

    std::set<llvm::Function*> ExtractedLoopFunctions; 
    std::ofstream LoopLocationDumpFile;

    TargetLoopExtractor() : llvm::ModulePass(ID) {}

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        AU.addRequiredID(llvm::BreakCriticalEdgesID);
        AU.addRequiredID(llvm::LoopSimplifyID);
        AU.addRequired<llvm::LoopInfoWrapperPass>();
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
    }

    virtual bool runOnModule(llvm::Module&) override;
    bool doInitialization(llvm::Module&) override;
    bool doFinalization(llvm::Module&) override;
    bool extractLoop(llvm::Loop*, llvm::LoopInfo&, 
            llvm::DominatorTree&, std::string);

};


}

#endif
