#define DEBUG_TYPE "dandelion-debug"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/AsmParser/Parser.h"

#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"

// Support for LLVM 4.0+
#define LLVM_VERSION_GE(major, minor) \
    (LLVM_VERSION_MAJOR > (major) ||  \
     LLVM_VERSION_MAJOR == (major) && LLVM_VERSION_MINOR >= (minor))
#define LLVM_VERSION_EQ(major, minor) \
    (LLVM_VERSION_MAJOR == (major) && LLVM_VERSION_MINOR == (minor))
#define LLVM_VERSION_LE(major, minor) \
    (LLVM_VERSION_MAJOR < (major) ||  \
     LLVM_VERSION_MAJOR == (major) && LLVM_VERSION_MINOR <= (minor))
#if LLVM_VERSION_GE(3, 7)

#include "llvm/IR/LegacyPassManager.h"

#else
#include "llvm/PassManager.h"
#endif
#if LLVM_VERSION_GE(4, 0)

#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"

#else
#include "llvm/Bitcode/ReaderWriter.h"
#endif

//#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/CodeGen/LinkAllAsmWriterComponents.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
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
#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/TailRecursionElimination.h"

#include <memory>
#include <string>

#include "AliasMem.h"
#include "Common.h"
#include "GEPSplitter.h"
#include "GraphGeneratorPass.h"
#include "TargetLoopExtractor.h"

using namespace llvm;
using std::string;
using std::unique_ptr;
using std::vector;
using llvm::sys::ExecuteAndWait;
using llvm::sys::findProgramByName;
using llvm::legacy::PassManager;

static cl::OptionCategory dandelionCategory{"dandelion options"};

cl::opt<string> inPath(cl::Positional, cl::desc("<Module to analyze>"),
                       cl::value_desc("bitcode filename"), cl::init(""),
                       cl::Required, cl::cat{dandelionCategory});

cl::opt<string> XKETCHName("fn-name", cl::desc("Target function name"),
                           cl::value_desc("Function name"), cl::Required,
                           cl::cat{dandelionCategory});

cl::opt<string> config_path("config", cl::desc("Target function name"),
                            cl::value_desc("config_file"), cl::Required,
                            cl::cat{dandelionCategory});

cl::opt<bool> aaTrace("aa-trace", cl::desc("Alias analysis trace"),
                      cl::value_desc("T/F {default = true}"), cl::init(false),
                      cl::cat{dandelionCategory}, cl::cat{dandelionCategory});

cl::opt<bool> lExtract("l-ex", cl::desc("Extracting loops"),
                       cl::value_desc("T/F {default = false}"), cl::init(false),
                       cl::cat{dandelionCategory});

cl::opt<bool> testCase("test-file", cl::desc("Printing Test file"),
                       cl::value_desc("T/F {default = True}"), cl::init(true),
                       cl::cat{dandelionCategory});

cl::opt<string> outFile("o", cl::desc("tapas output file"),
                        cl::value_desc("filename"), cl::init(""), cl::cat{dandelionCategory});

static cl::opt<char> optLevel(
    "O", cl::desc("Optimization level. [-O0, -O1, -O2, or -O3] "
                  "(default = '-O2')"),
    cl::Prefix, cl::ZeroOrMore, cl::init('2'));

cl::list<string> libPaths("L", cl::Prefix,
                          cl::desc("Specify a library search path"),
                          cl::value_desc("directory"));

cl::list<string> libraries("l", cl::Prefix,
                           cl::desc("Specify libraries to link to"),
                           cl::value_desc("library prefix"));

static void compile(Module &m, string outputPath) {
    string err;

    Triple triple = Triple(m.getTargetTriple());
    Target const *target = TargetRegistry::lookupTarget(MArch, triple, err);
    if (!target) {
        report_fatal_error("Unable to find target:\n " + err);
    }

    CodeGenOpt::Level level = CodeGenOpt::Default;
    switch (optLevel) {
        default:
            report_fatal_error("Invalid optimization level.\n");
        // No fall through
        case '0':
            level = CodeGenOpt::None;
            break;
        case '1':
            level = CodeGenOpt::Less;
            break;
        case '2':
            level = CodeGenOpt::Default;
            break;
        case '3':
            level = CodeGenOpt::Aggressive;
            break;
    }

    string FeaturesStr;
    TargetOptions options = InitTargetOptionsFromCodeGenFlags();
    unique_ptr<TargetMachine> machine(
        target->createTargetMachine(triple.getTriple(), MCPU, FeaturesStr,
                                    options, getRelocModel(), CMModel, level));
    assert(machine.get() && "Could not allocate target machine!");

    if (FloatABIForCalls != FloatABI::Default) {
        options.FloatABIType = FloatABIForCalls;
    }

    std::error_code errc;
    auto out = std::make_unique<tool_output_file>(outputPath.c_str(), errc,
                                                  sys::fs::F_None);
    if (!out) {
        report_fatal_error("Unable to create file:\n " + errc.message());
    }

    // Build up all of the passes that we want to do to the module.
    legacy::PassManager pm;

    // Add target specific info and transforms
    TargetLibraryInfoImpl tlii(Triple(m.getTargetTriple()));
    pm.add(new TargetLibraryInfoWrapperPass(tlii));

    m.setDataLayout(machine->createDataLayout());

    {  // Bound this scope
        raw_pwrite_stream *os(&out->os());

        FileType = TargetMachine::CGFT_ObjectFile;
        std::unique_ptr<buffer_ostream> bos;
        if (!out->os().supportsSeeking()) {
            bos = std::make_unique<buffer_ostream>(*os);
            os = bos.get();
        }

        // Ask the target to add backend passes as necessary.
        if (machine->addPassesToEmitFile(pm, *os, FileType)) {
            report_fatal_error(
                "target does not support generation "
                "of this file type!\n");
        }

        // Before executing passes, print the final values of the LLVM options.
        cl::PrintOptionValues();

        pm.run(m);
    }

    // Keep the output binary if we've been successful to this point.
    out->keep();
}

static void link(string const &objectFile, string const &outputFile) {
    auto clang = findProgramByName("clang++");
    string opt("-O");
    opt += optLevel;

    if (!clang) {
        report_fatal_error("Unable to find clang.");
    }
    vector<string> args{clang.get(), opt, "-o", outputFile, objectFile};

    for (auto &libPath : libPaths) {
        args.push_back("-L" + libPath);
    }

    for (auto &library : libraries) {
        args.push_back("-l" + library);
    }

    vector<char const *> charArgs;
    for (auto &arg : args) {
        charArgs.push_back(arg.c_str());
    }
    charArgs.push_back(nullptr);

    for (auto &arg : args) {
        outs() << arg.c_str() << " ";
    }
    outs() << "\n";

    string err;
    if (-1 == ExecuteAndWait(clang.get(), &charArgs[0], nullptr, nullptr, 0, 0,
                             &err)) {
        report_fatal_error("Unable to link output file.");
    }
}

static void generateBinary(Module &m, string const &outputFilename) {
    // Compiling to native should allow things to keep working even when the
    // version of clang on the system and the version of LLVM used to compile
    // the tool don't quite match up.
    string objectFile = outputFilename + ".o";
    compile(m, objectFile);
    link(objectFile, outputFilename);
}

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
    pm.add(new DominatorTreeWrapperPass());
    pm.add(new LoopInfoWrapperPass());
    pm.add(new lx::TargetLoopExtractor());
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

static void AApassTest(Module &m) {
    legacy::PassManager pm;
    pm.add(createBasicAAWrapperPass());
    pm.add(llvm::createTypeBasedAAWrapperPass());
    pm.add(createGlobalsAAWrapperPass());
    pm.add(createSCEVAAWrapperPass());
    pm.add(createScopedNoAliasAAWrapperPass());
    pm.add(createCFLAndersAAWrapperPass());
    pm.add(createAAResultsWrapperPass());
    pm.add(new amem::AliasMem(XKETCHName));
    pm.add(createVerifierPass());
    pm.run(m);
}

static void graphGen(Module &m) {
    // Check wether xketch outpufile name has been specified
    if (outFile.getValue() == "") {
        errs() << "o command line option must be specified.\n";
        exit(-1);
    }

    std::error_code errc;
    raw_fd_ostream out(outFile + ".scala", errc, sys::fs::F_None);

    // raw_fd_ostream test(outFile+"_test.scala", errc, sys::fs::F_None);

    legacy::PassManager pm;

    pm.add(llvm::createPromoteMemoryToRegisterPass());
    pm.add(createSeparateConstOffsetFromGEPPass());
    pm.add(new graphgen::GraphGeneratorPass());

    pm.add(createVerifierPass());
    pm.run(m);
}

/**
 * Running gepsplitter
 */
static void splitGeps(Function &F) {
    legacy::FunctionPassManager FPM(F.getParent());
    FPM.add(new gepsplitter::GEPSplitter());
    FPM.run(F);
}

/**
 * Function lists
 */
static void runGraphGen(Module &M) {
    // Check wether xketch outpufile name has been specified
    if (outFile.getValue() == "") {
        errs() << "o command line option must be specified.\n";
        exit(-1);
    }

    std::error_code errc;
    raw_fd_ostream out(outFile + ".scala", errc, sys::fs::F_None);

    legacy::PassManager pm;
    // Usefull passes
    // pm.add(llvm::createCFGSimplificationPass());
    // pm.add(new helpers::GEPAddrCalculation(XKETCHName));
    pm.add((llvm::createStripDeadDebugInfoPass()));
    //    pm.add(llvm::createLoopSimplifyPass());
    pm.add(new helpers::GepInformation(XKETCHName));
    pm.add(new LoopInfoWrapperPass());
    // pm.add(new helpers::CallInstSpliter(XKETCHName));
    pm.add(new graphgen::GraphGeneratorPass(NodeInfo(0, XKETCHName), out));
    pm.add(createVerifierPass());
    pm.run(M);
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
    cl::HideUnrelatedOptions(dandelionCategory);
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

    // Simplifing the gep instructions
    for (auto &F : *module) {
        if (F.isDeclaration() || F.getName() != XKETCHName)
            continue;
        else
            splitGeps(F);
    }

    runGraphGen(*module);

    // Generating graph
    // graphGen(*module);

    // saveModule(*module, "final.bc");

    return 0;
}
