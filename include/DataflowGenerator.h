

#ifndef STATICCALLCOUNTER_H
#define STATICCALLCOUNTER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"

#include "Common.h"
#include "NodeType.h"

#include <map>
#include <string>
#include <vector>

namespace codegen {

struct BBInfo {
    std::string name;
    uint32_t id;
};

struct InsInfo {
    std::string name;
    uint32_t id;
};

struct ArgInfo {
    std::string name;
    uint32_t id;
};

class DataflowGeneratorPass : public llvm::ModulePass {

    // Default value is standard out
    llvm::raw_ostream &outCode;

    // Function name
    // Basicblock info maps
    std::map<llvm::BasicBlock *, BBInfo> basic_block_info;
    std::map<llvm::Instruction *, InsInfo> instruction_info;
    std::map<llvm::Argument *, ArgInfo> argument_info;

    std::vector<llvm::Instruction *> instruction_branch;
    std::vector<llvm::Instruction *> instruction_compute;
    std::vector<llvm::Instruction *> instruction_comp;
    std::vector<llvm::Instruction *> instruction_phi;
    std::vector<llvm::Instruction *> instruction_gep;
    std::vector<llvm::Instruction *> instruction_load;
    std::vector<llvm::Instruction *> instruction_store;
    std::vector<llvm::Instruction *> instruction_alloca;

    std::map<llvm::Instruction *, uint32_t> instruction_use;

    std::vector<llvm::Argument *> function_argument;

    std::map<llvm::Instruction *, std::vector<llvm::Instruction *>> mem_succ;
    std::map<llvm::Instruction *, std::vector<llvm::Instruction *>> mem_pred;

    llvm::BasicBlock *entry_bb;

    // Instruction counters
    uint32_t count_ins;
    uint32_t count_binary;
    uint32_t count_branch;
    uint32_t count_node;

    std::string param_name;

    llvm::StringRef FunctionName;

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;



   public:

    static char ID;

    DataflowGeneratorPass() : llvm::ModulePass(ID), outCode(llvm::outs()) {}

    DataflowGeneratorPass(llvm::raw_ostream &out, llvm::StringRef name)
        : llvm::ModulePass(ID), outCode(out), FunctionName(name) {}

    virtual bool runOnModule(llvm::Module &m) override;

    void printCode(std::string code);

    void setOutput(llvm::raw_ostream &);

    void generateFunction(llvm::Function &);

    /**
     * Container functions
     */
    void FillInstructionContainers(llvm::Function &);
    void FillFunctionArg(llvm::Function &);

    void PrintHelperObject(llvm::Function &);
    void PrintDatFlowAbstractIO(llvm::Function &);
    void printHeader(string );

    void NamingBasicBlock(llvm::Function &);
    void NamingInstruction(llvm::Function &);

    /**
     * Print functions
     */
    void generateImportSection();
    void HelperPrintBBInit(llvm::Function &);
    void PrintBasicBlockInit(llvm::BasicBlock &);

    void HelperPrintInistInit(llvm::Function &);
    void PrintInstInit(Instruction &);
    void PrintBinaryComparisionIns(Instruction &);
    void PrintBranchIns(Instruction &);
    void PrintPHIIns(Instruction &);
    void PrintGepIns(Instruction &);
    void PrintLoadIns(Instruction &);
    void PrintStoreIns(Instruction &);
    void PrintSextIns(Instruction &);
    void PrintZextIns(Instruction &);
    void PrintAllocaIns(Instruction &);

    void PrintParamObject();

    void HelperPrintLoop(llvm::Function &);

    void HelperPrintBasicBlockPredicate();
    void PrintBranchBasicBlockCon(Instruction &);
    void HelperPrintBasicBlockPhi();
    void PrintPHIMask(llvm::Instruction &);

    void HelperPrintInstructionDF(llvm::Function &);
    void PrintDataFlow(llvm::Instruction &);
    // Get instruction type

    void PrintStackPointer();
    void PrintRegisterFile();
    void PrintCacheMem();

    /**
     * Print method gets called right after the pass finishes
     */
    // virtual void print(llvm::raw_ostream &out,
    // llvm::Module const *m) const override;
};
}

#endif
