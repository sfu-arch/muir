#ifndef COMMON_H
#define COMMON_H

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"

#include <map>
#include <sstream>
#include <string>

// using namespace llvm;
using namespace std;
using namespace llvm;

namespace helpers {

// Functions
void printAlloca(llvm::Function &);

void printStruct(llvm::Module &);

void printDFG(llvm::Function &);

void printDFG(llvm::Module &);

void PDGPrinter(llvm::Function &);

void UIDLabel(Function &);

struct pdgDump : public llvm::FunctionPass {
    static char ID;

    pdgDump() : FunctionPass(ID) {}

    virtual bool runOnFunction(llvm::Function &F);
};

class LabelUID : public FunctionPass, public InstVisitor<LabelUID> {
    friend class InstVisitor<LabelUID>;

    uint64_t counter;

    void visitFunction(Function &F);

    void visitBasicBlock(BasicBlock &BB);

    void visitInstruction(Instruction &I);

    template <typename T>
    void visitGeneric(string, T &);

    map<Value *, uint64_t> values;

   public:
    static char ID;

    LabelUID() : FunctionPass(ID), counter(0) {}

    bool doInitialization(Module &) override {
        counter = 0;
        values.clear();
        return false;
    };

    bool doFinalization(Module &) override { return true; };

    bool runOnFunction(Function &) override;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
        AU.setPreservesAll();
    }
};

void FunctionUIDLabel(llvm::Function &);
}

#endif
