#define DEBUG_TYPE "dataflow_common"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"

#include "Common.h"
#include "NodeType.h"

using namespace llvm;
using namespace std;


extern cl::opt<string> XKETCHName;

namespace common {

InstructionInfo::InstructionName InstructionInfo::instruction_name_type = {
    {TBinaryOperator, "ComputeNode"},
    {TICmpInst, "IcmpNode"},
    {TUBranchInst, "UBranchNode"},
    {TCBranchInst, "CBranchNode"},
    {TPHINode, "PhiNode"},
    {TGEP, "GepNode"},
    {TLoad, "UnTypLoad"},
    {TStore, "UnTypStor"},
    {TSEXT, "SEXTNode"},
    {TBitCast, "TBitCastNode"},
    {TTrunc, "TruncNode"},
    {TAlloca, "AllocaNode"},
#ifdef TAPIR
    {TDetach, "Detach"},
    {TReattach, "Reattach"},
    {TSync, "Sync"},
#endif
    {TReturnInst, "RetNode"}};

void optimizeModule(Module *Mod) {
    PassManagerBuilder PMB;
    PMB.OptLevel = 2;
    PMB.SLPVectorize = false;
    PMB.BBVectorize = false;
    legacy::PassManager PM;
    PMB.populateModulePassManager(PM);
    PM.run(*Mod);
}

void PrintFunctionDFG(llvm::Module &M) {
    for (auto &F : M) {
         if (F.isDeclaration()) continue;

         if (F.getName() == XKETCHName) {
         stripDebugInfo(F);
         DEBUG(dbgs() << "FUNCTION FOUND\n");
         //Making a function pass
         helpers::printDFG(F);
         
        }
    }
}

}
