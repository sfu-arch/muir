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

#include "NodeType.h"

#include <map>
#include <sstream>
#include <string>

#define WARNING(x)                                                 \
    do {                                                           \
        std::cout << "\033[1;31m[WARNING] \033[0m"                 \
                  << "\033[1;33m" << x << " \033[0m" << std::endl; \
    } while (0)

#define ASSERTION(x)                                 \
    do {                                             \
        std::cout << "\033[1;35m" << x << "\033[0m"; \
    } while (0)  //<< "\033[1;33m" << x << " \033[0m" << std::endl; \
    } while (0)

#define PURPLE(x) "\033[1;35m" << x << "\033[0m";

using namespace std;
using namespace llvm;

namespace common {

/**
 * Implimentaiton of FloatingPointIEEE754
 */
union FloatingPointIEEE754 {
    struct ieee754 {
        ieee754() : mantissa(0), exponent(0), sign(0) {}
        unsigned int mantissa : 23;
        unsigned int exponent : 8;
        unsigned int sign : 1;
    };
    ieee754 raw;
    unsigned int bits;
    float f;

    FloatingPointIEEE754() : f(0) {}
};

// Structures
struct GepInfo {
    uint32_t overall_size;
    std::vector<uint32_t> element_size;

    GepInfo() : overall_size(0) { element_size.clear(); }

    GepInfo(std::vector<uint32_t> _input_elements)
        : element_size(_input_elements) {
        overall_size = _input_elements.back();
    }
};

// Functions
void optimizeModule(llvm::Module *);

void PrintFunctionDFG(llvm::Module &);

InstructionType getLLVMOpcodeName(uint32_t OpCode);
}

namespace helpers {

/**
 * Print helper function
 */
bool helperReplace(std::string &, const std::string &, const std::string &);
bool helperReplace(std::string &, const std::string &,
                   std::vector<const std::string> &, const std::string &);
bool helperReplace(std::string &, const std::string &, std::vector<uint32_t>,
                   const std::string &);
bool helperReplace(std::string &, const std::string &, const uint32_t);
bool helperReplace(std::string &, const std::string &, const int);
bool helperReplace(std::string &, const std::string &,
                   std::vector<const uint32_t> &);
bool helperReplace(std::string &, const std::string &,
                   std::list<std::pair<uint32_t, uint32_t>> &,
                   const std::string &);

/**
 * FUNCTIONS
 */

void printAlloca(llvm::Function &);

void printStruct(llvm::Module &);

void printDFG(llvm::Function &);

void printDFG(llvm::Module &);

void PDGPrinter(llvm::Function &);

void UIDLabel(Function &);

void FunctionUIDLabel(llvm::Function &);

/**
 * CLSSES
 */

/**
 * pdgDump class dumps PDG of the given funciton
 */
struct pdgDump : public llvm::FunctionPass {
    static char ID;

    pdgDump() : FunctionPass(ID) {}

    virtual bool runOnFunction(llvm::Function &F);
};

class DFGPrinter : public llvm::FunctionPass,
                   public llvm::InstVisitor<DFGPrinter> {
    friend class InstVisitor<DFGPrinter>;

    void visitFunction(llvm::Function &F);

    void visitBasicBlock(llvm::BasicBlock &BB);

    void visitInstruction(llvm::Instruction &I);

    stringstream dot;
    std::map<llvm::Value *, uint64_t> nodes;
    uint64_t counter_ins;
    uint64_t counter_bb;
    uint64_t counter_cnst;
    uint64_t counter_arg;

   public:
    static char ID;

    DFGPrinter()
        : FunctionPass(ID),
          counter_ins(0),
          counter_bb(0),
          counter_cnst(0),
          counter_arg(0) {}

    bool doInitialization(llvm::Module &) override;

    bool doFinalization(llvm::Module &) override;

    bool runOnFunction(llvm::Function &) override;

    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        AU.setPreservesAll();
    }
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

class GepInformation : public ModulePass, public InstVisitor<GepInformation> {
    friend class InstVisitor<GepInformation>;

    void visitGetElementPtrInst(llvm::GetElementPtrInst &I);

   public:
    static char ID;

    // Gep containers
    std::map<llvm::Instruction *, common::GepInfo> GepAddress;

    // Function name
    llvm::StringRef function_name;

    GepInformation(llvm::StringRef FN) : ModulePass(ID), function_name(FN) {}

    bool doInitialization(Module &) override { return false; };

    bool doFinalization(Module &) override { return true; };

    bool runOnModule(Module &) override;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
        AU.setPreservesAll();
    }
};

class InstCounter : public llvm::ModulePass {
   public:
    static char ID;

    // Function name
    llvm::StringRef function_name;

    std::map<llvm::BasicBlock *, uint64_t> BasicBlockCnt;

    InstCounter(llvm::StringRef fn) : llvm::ModulePass(ID), function_name(fn) {}

    bool doInitialization(llvm::Module &) override;
    bool doFinalization(llvm::Module &) override;
    bool runOnModule(llvm::Module &) override;

    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        AU.setPreservesAll();
    }
};

class CallInstSpliter : public ModulePass, public InstVisitor<CallInstSpliter> {
    friend class InstVisitor<CallInstSpliter>;

   private:
    llvm::SmallVector<llvm::CallInst *, 10> call_container;

   public:
    static char ID;

    // Function name
    llvm::StringRef function_name;

    CallInstSpliter() : llvm::ModulePass(ID), function_name("") {}
    CallInstSpliter(llvm::StringRef fn)
        : llvm::ModulePass(ID), function_name(fn) {}

    bool doInitialization(llvm::Module &) override;
    bool doFinalization(llvm::Module &) override;
    bool runOnModule(llvm::Module &) override;

    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        AU.setPreservesAll();
    }

    void visitCallInst(llvm::CallInst &Inst);
};
}

#endif
