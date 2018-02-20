

#ifndef STATICCALLCOUNTER_H
#define STATICCALLCOUNTER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Common.h"
#include "NodeType.h"

#include <map>
#include <set>
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

struct GlobalInfo {
    std::string name;
    uint32_t id;
};

class DataflowGeneratorPass : public llvm::ModulePass {
    // Default value is standard out
    llvm::raw_ostream &outCode;
    llvm::raw_ostream &outTest;

    // Function name
    // Basicblock info maps
    std::map<llvm::BasicBlock *, BBInfo> basic_block_info;
    std::map<llvm::Instruction *, InsInfo> instruction_info;
    std::map<llvm::Argument *, ArgInfo> argument_info;
    std::map<llvm::GlobalValue *, GlobalInfo> global_info;

    std::vector<llvm::Instruction *> instruction_branch;
    std::vector<llvm::Instruction *> instruction_compute;
    std::vector<llvm::Instruction *> instruction_comp;
    std::vector<llvm::Instruction *> instruction_phi;
    std::vector<llvm::Instruction *> instruction_gep;
    std::vector<llvm::Instruction *> instruction_load;
    std::vector<llvm::Instruction *> instruction_store;
    std::vector<llvm::Instruction *> instruction_alloca;
    std::vector<llvm::Instruction *> instruction_select;
    std::vector<llvm::Instruction *> instruction_call;
#ifdef TAPIR
    std::vector<llvm::Instruction *> instruction_detach;
#endif

    std::map<llvm::Instruction *, uint32_t> instruction_use;
    std::map<llvm::Argument *, uint32_t> argument_use;

    // Function arguments
    std::vector<llvm::Argument *> function_argument;

    // Global values
    std::vector<llvm::GlobalValue *> module_global;

    // Set of each instruction successors
    std::map<llvm::Instruction *, std::vector<llvm::Instruction *>> mem_succ;

    // Set of each instruction predecessors
    std::map<llvm::Instruction *, std::vector<llvm::Instruction *>> mem_pred;

    // All the function's loops
    std::vector<llvm::Loop *> loop_container;

    // Set of each for loop live-ins
    std::map<llvm::Loop *, std::set<llvm::Value *>> loop_liveins;

    // Set of each for loop live-ins and their number of usage inside the for
    // loop
    std::map<llvm::Loop *, std::map<llvm::Value *, uint32_t>>
        loop_liveins_count;

    // Set of each for loop live-outs
    std::map<llvm::Loop *, std::set<llvm::Value *>> loop_liveouts;

    // Set of each for loop live-outs and their number of usage inside the for
    // loop
    std::map<llvm::Loop *, std::map<llvm::Value *, uint32_t>>
        loop_liveouts_count;

    // Set of loops header basicblock
    std::map<BasicBlock *, Loop *> loop_header_bb;

    std::map<llvm::Value *, uint32_t> ins_loop_header_idx;
    std::map<llvm::Value *, uint32_t> ins_loop_end_idx;

    // Edges which we need to connect them too loop latch
    // LoopEdge-> <src, dst>
    std::set<std::pair<llvm::Value *, llvm::Value *>> LoopEdges;

    std::map<llvm::Instruction *, std::pair<llvm::Value *, llvm::Value *>>
        JumpIns;

    llvm::BasicBlock *entry_bb;

    // LoopInfo
    llvm::LoopInfo *LI;

    // Instruction counters
    uint32_t count_ins;
    uint32_t count_binary;
    uint32_t count_branch;
    uint32_t count_node;

    std::string param_name;

    llvm::StringRef FunctionName;

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

    virtual bool doInitialization(llvm::Module &M) override;

   public:
    static char ID;

    DataflowGeneratorPass()
        : llvm::ModulePass(ID), outCode(llvm::outs()), outTest(llvm::outs()) {}

    DataflowGeneratorPass(llvm::raw_ostream &out, llvm::raw_ostream &test,
                          llvm::StringRef name)
        : llvm::ModulePass(ID),
          outCode(out),
          outTest(test),
          FunctionName(name) {}

    virtual bool runOnModule(llvm::Module &m) override;

    void printCode(std::string);
    void printCode(std::string, llvm::raw_ostream &);

    void setOutput(llvm::raw_ostream &);

    void generateFunction(llvm::Function &);

    /**
     * Container functions
     */
    void FillInstructionContainers(llvm::Function &);
    void FillFunctionArg(llvm::Function &);
    void FillGlobalVar(llvm::Module &);
    void FillLoopHeader(llvm::LoopInfo &);

    void PrintHelperObject(llvm::Function &);
    void PrintDatFlowAbstractIO(llvm::Function &);
    void printHeader(string);

    void NamingBasicBlock(llvm::Function &);
    void NamingInstruction(llvm::Function &);

    /**
     * Print functions
     */
    void generateImportSection(llvm::raw_ostream &);
    void HelperPrintBBInit(llvm::Function &);
    void PrintBasicBlockInit(llvm::BasicBlock &);
    void PrintBasicBlockInit(llvm::BasicBlock &, llvm::Loop &);

    void HelperPrintInstInit(llvm::Function &);
    void PrintInstInit(Instruction &);
    void PrintBinaryComparisionIns(Instruction &);
    void PrintBranchIns(Instruction &);
    void PrintPHIIns(Instruction &);
    void PrintGepIns(Instruction &);
    void PrintLoadIns(Instruction &);
    void PrintStoreIns(Instruction &);
    void PrintSextIns(Instruction &);
    void PrintZextIns(Instruction &);
    void PrintBitCastIns(Instruction &);
    void PrintAllocaIns(Instruction &);
    void PrintRetIns(Instruction &);
    void PrintCallIns(Instruction &);

    //void PrintDFBinaryComparisionIns(Instruction &, uint32_t, RightSide);
#ifdef TAPIR
    void PrintDetachIns(Instruction &);
    void PrintReattachIns(Instruction &);
    void PrintSyncIns(Instruction &);
#endif
    void PrintParamObject();

    void PrintLoopHeader(llvm::Function &);
    // void HelperPrintLoop(llvm::Function &);
    void PrintLoopRegister(llvm::Function &);

    void PrintBasicBlockEnableInstruction(llvm::Function &);
    void HelperPrintBasicBlockPredicate();
    void PrintBranchBasicBlockCon(Instruction &);
    void PrintDetachBasicBlockCon(Instruction &);
    void HelperPrintBasicBlockPhi();
    // void PrintPHIMask(llvm::Instruction &);
    void PrintPHIMask(llvm::Instruction &, uint32_t);
    void PrintPHIMask(llvm::Instruction &,
                      std::map<llvm::BasicBlock *, uint32_t> &);
    void PrintPHICon(llvm::Instruction &);

    void HelperPrintInstructionDF(llvm::Function &);
    void PrintDataFlow(llvm::Instruction &);
    void NewPrintDataFlow(llvm::Instruction &);

    // Printing TESTER function
    void generateTestFunction(llvm::Function &);

    // Get instruction type

    void PrintStackPointer();
    void PrintRegisterFile();
    void PrintCacheMem();
    void PrintInputSplitter(llvm::Function &);

    void printEndingModule(llvm::Function &);

    std::string PrintCallResp(llvm::Instruction &, uint32_t);

    /**
     * Print method gets called right after the pass finishes
     */
    // virtual void print(llvm::raw_ostream &out,
    // llvm::Module const *m) const override;
};
}

#endif
