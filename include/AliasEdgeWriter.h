#ifndef ALIASEDGEWRITER_H
#define ALIASEDGEWRITER_H

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>

namespace aew {

struct AliasEdgeWriter : public llvm::ModulePass {
    static char ID;
    std::map<std::string, uint64_t> Data;
    bool print = false;

    llvm::SmallVector<std::pair<uint32_t, uint32_t>, 16> NaiveAliasEdges;
    llvm::SmallVector<std::pair<uint32_t, uint32_t>, 16> AliasEdges;
    llvm::SmallVector<std::pair<uint32_t, uint32_t>, 16> MayAliasEdges;
    llvm::SmallVector<std::pair<uint32_t, uint32_t>, 16> MustAliasEdges;

    llvm::DenseMap<llvm::Instruction *,
                   llvm::SmallVector<llvm::Instruction *, 16>>
        NaiveAliasEdgesMap;
    llvm::DenseMap<llvm::Instruction *,
                   llvm::SmallVector<llvm::Instruction *, 16>>
        AliasEdgesMap;
    llvm::DenseMap<llvm::Instruction *,
                   llvm::SmallVector<llvm::Instruction *, 16>>
        MayAliasEdgesMap;
    llvm::DenseMap<llvm::Instruction *,
                   llvm::SmallVector<llvm::Instruction *, 16>>
        MustAliasEdgesMap;

    AliasEdgeWriter() : llvm::ModulePass(ID), print(false) {}

    virtual bool runOnModule(llvm::Module &) override;
    virtual bool doInitialization(llvm::Module &M) override;
    virtual bool doFinalization(llvm::Module &M) override;

    void writeEdges(llvm::CallInst *, llvm::Function *);

    llvm::StringRef getPassName() const override { return "AliasEdgeWriter"; }
    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        AU.addRequired<llvm::AAResultsWrapperPass>();
    }
};
}

#endif
