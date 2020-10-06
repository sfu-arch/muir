#define DEBUG_TYPE "dandelion-debug"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/CodeGen/CommandFlags.def"
#include "llvm/CodeGen/LinkAllAsmWriterComponents.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Linker/Linker.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"

#include <experimental/iterator>
#include <memory>
#include <string>

#include "Common.h"

using namespace llvm;
using llvm::legacy::PassManager;
using llvm::sys::ExecuteAndWait;
using llvm::sys::findProgramByName;
using std::string;
using std::unique_ptr;
using std::vector;

using namespace helpers;

//template <class T>
//std::vector<T *> getInstList(Function *func) {
    //std::vector<T *> return_list;
    //for (auto &_node : llvm::instructions(func)) {
        //if (auto cast_node = dyn_cast<T>(&_node))
            //return_list.push_back(cast_node);
    //}
    //return return_list;
//}

static cl::OptionCategory ddebugCategory{"dandelion options"};

cl::opt<string> inPath(cl::Positional, cl::desc("<Module to analyze>"),
                       cl::value_desc("bitcode filename"), cl::init(""),
                       cl::Required, cl::cat{ddebugCategory});

cl::opt<string> function_name("function", cl::desc("Target function name"),
                          cl::value_desc("Function name"), cl::Required,
                          cl::cat{ddebugCategory});

cl::opt<string> instruction_id("id", cl::desc("Instruction ID"),
                          cl::value_desc("ID of the instruction"), cl::Required,
                          cl::cat{ddebugCategory});

cl::opt<string> outFile("o", cl::desc("tapas output file"),
                        cl::value_desc("filename"), cl::init(""),
                        cl::cat{ddebugCategory});

static void saveModule(Module &m, StringRef const filename) {
    std::error_code errc;
    raw_fd_ostream out(filename.data(), errc, sys::fs::F_None);

    if (errc) {
        report_fatal_error("error saving llvm module to '" + filename +
                           "': \n" + errc.message());
    }
    WriteBitcodeToFile(&m, out);
}

static void extractLoops(Module &m) {
    // Test function to print loops informations
    legacy::PassManager pm;
    pm.add(new llvm::AssumptionCacheTracker());
    pm.add(llvm::createBasicAAWrapperPass());
    pm.add(createTypeBasedAAWrapperPass());
    pm.add(createBreakCriticalEdgesPass());
    pm.add(createLoopSimplifyPass());
    pm.add(new LoopInfoWrapperPass());
    //pm.add(new DominatorTreeWrapperPass());
    // pm.add(new lx::TargetLoopExtractor());
    // pm.add(new loopclouser::LoopClouser());
    pm.add(createVerifierPass());
    pm.run(m);

    // Save the new bc file
    auto replaceExt = [](string &s, const string &newExt) {
        string::size_type i = s.rfind('.', s.length());
        if (i != string::npos) {
            s.replace(i + 1, newExt.length(), newExt);
        }
    };

    replaceExt(inPath, "lx.bc");
    saveModule(m, inPath);
}

/**
 * Running UIDLabel pss
 */
void labelFunctions(Module &M) {
    for (auto &F : M) {
        if (F.isDeclaration()) continue;
        helpers::FunctionUIDLabel(F);
    }
}

int main(int argc, char **argv) {
    // This boilerplate provides convenient stack traces and clean LLVM exit
    // handling. It also initializes the built in support for convenient
    // command line option handling.
    sys::PrintStackTraceOnErrorSignal(argv[0]);
    llvm::PrettyStackTraceProgram X(argc, argv);
    llvm_shutdown_obj shutdown;
    cl::HideUnrelatedOptions(ddebugCategory);
    cl::ParseCommandLineOptions(argc, argv);

    // Construct an IR file from the filename passed on the command line.
    SMDiagnostic err;
    LLVMContext context;
    unique_ptr<Module> module = parseIRFile(inPath.getValue(), err, context);

    if (!module.get()) {
        errs() << "Error reading bitcode file: " << inPath << "\n";
        err.print(argv[0], errs());
        return -1;
    }

    labelFunctions(*module);

    outs() << function_name.getValue() << "\n";
    outs() << instruction_id.getValue() << "\n";

    saveModule(*module, function_name.getValue() + ".final.bc");

    return 0;
}
