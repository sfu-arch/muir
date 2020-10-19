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
#include <iostream>

#include "AliasEdgeWriter.h"
#include "Common.h"
#include "GEPSplitter.h"
#include "GraphGeneratorPass.h"
//#include "LoopClouser.h"
#include "TargetLoopExtractor.h"

#define DATA_SIZE 64

using namespace llvm;
using llvm::legacy::PassManager;
using llvm::sys::ExecuteAndWait;
using llvm::sys::findProgramByName;
using std::string;
using std::unique_ptr;
using std::vector;

using namespace helpers;

template <class T>
std::vector<T*>
getInstList(Function* func) {
  std::vector<T*> return_list;
  for (auto& _node : llvm::instructions(func)) {
    if (auto cast_node = dyn_cast<T>(&_node))
      return_list.push_back(cast_node);
  }
  return return_list;
}

static cl::OptionCategory dandelionCategory{"dandelion options"};

cl::opt<string> inPath(cl::Positional,
                       cl::desc("<Module to analyze>"),
                       cl::value_desc("bitcode filename"),
                       cl::init(""),
                       cl::Required,
                       cl::cat{dandelionCategory});

cl::opt<string> target_fn("fn-name",
                          cl::desc("Target function name"),
                          cl::value_desc("Function name"),
                          cl::Required,
                          cl::cat{dandelionCategory});

cl::opt<string> config_path("config",
                            cl::desc("Target function name"),
                            cl::value_desc("config_file"),
                            cl::Required,
                            cl::cat{dandelionCategory});

cl::opt<bool> aaTrace("aa-trace",
                      cl::desc("Alias analysis trace"),
                      cl::value_desc("T/F {default = true}"),
                      cl::init(false),
                      cl::cat{dandelionCategory},
                      cl::cat{dandelionCategory});

cl::opt<bool> lExtract("l-ex",
                       cl::desc("Extracting loops"),
                       cl::value_desc("T/F {default = false}"),
                       cl::init(false),
                       cl::cat{dandelionCategory});

cl::opt<bool> testCase("test-file",
                       cl::desc("Printing Test file"),
                       cl::value_desc("T/F {default = True}"),
                       cl::init(true),
                       cl::cat{dandelionCategory});

cl::opt<bool> dump_muir("dump-muir",
                        cl::desc("Dumping muIR representation"),
                        cl::value_desc("T/F {default = false}"),
                        cl::init(false),
                        cl::cat{dandelionCategory});

cl::opt<bool> verbose("verbose",
                      cl::desc("Verbose execution"),
                      cl::value_desc("T/F {default = false}"),
                      cl::init(false),
                      cl::cat{dandelionCategory});

cl::opt<string> outFile("o",
                        cl::desc("tapas output file"),
                        cl::value_desc("filename"),
                        cl::init(""),
                        cl::cat{dandelionCategory});

cl::opt<char> HWoptLevel("H",
                         cl::desc("Optimization level. [-H0, -H1, -H2, or -H3] "
                                  "(default = '-H1')"),
                         cl::Prefix,
                         cl::ZeroOrMore,
                         cl::init('1'));

static cl::opt<char> optLevel("O",
                              cl::desc("Optimization level. [-O0, -O1, -O2, or -O3] "
                                       "(default = '-O2')"),
                              cl::Prefix,
                              cl::ZeroOrMore,
                              cl::init('2'));

cl::list<string> libPaths("L",
                          cl::Prefix,
                          cl::desc("Specify a library search path"),
                          cl::value_desc("directory"));

cl::list<string> libraries("l",
                           cl::Prefix,
                           cl::desc("Specify libraries to link to"),
                           cl::value_desc("library prefix"));

static void
compile(Module& m, string outputPath) {
  string err;

  Triple triple        = Triple(m.getTargetTriple());
  Target const* target = TargetRegistry::lookupTarget(MArch, triple, err);
  if (!target) {
    report_fatal_error("Unable to find target:\n " + err);
  }

  CodeGenOpt::Level level = CodeGenOpt::Default;
  switch (optLevel) {
    default: report_fatal_error("Invalid optimization level.\n");
    // No fall through
    case '0': level = CodeGenOpt::None; break;
    case '1': level = CodeGenOpt::Less; break;
    case '2': level = CodeGenOpt::Default; break;
    case '3': level = CodeGenOpt::Aggressive; break;
  }

  string FeaturesStr;
  TargetOptions options = InitTargetOptionsFromCodeGenFlags();
  unique_ptr<TargetMachine> machine(target->createTargetMachine(triple.getTriple(),
                                                                MCPU,
                                                                FeaturesStr,
                                                                options,
                                                                getRelocModel(),
                                                                CMModel.getValue(),
                                                                level));
  assert(machine.get() && "Could not allocate target machine!");

  if (FloatABIForCalls != FloatABI::Default) {
    options.FloatABIType = FloatABIForCalls;
  }

  std::error_code errc;
  auto out = std::make_unique<ToolOutputFile>(outputPath.c_str(), errc, sys::fs::F_None);
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
    raw_pwrite_stream* os(&out->os());

    FileType = TargetMachine::CGFT_ObjectFile;
    std::unique_ptr<buffer_ostream> bos;
    if (!out->os().supportsSeeking()) {
      bos = std::make_unique<buffer_ostream>(*os);
      os  = bos.get();
    }

    // Ask the target to add backend passes as necessary.
    if (machine->addPassesToEmitFile(pm, *os, FileType)) {
      report_fatal_error("target does not support generation "
                         "of this file type!\n");
    }

    // Before executing passes, print the final values of the LLVM options.
    cl::PrintOptionValues();

    pm.run(m);
  }

  // Keep the output binary if we've been successful to this point.
  out->keep();
}

static void
link(string const& objectFile, string const& outputFile) {
  auto clang = findProgramByName("clang++");
  string opt("-O");
  opt += optLevel;

  if (!clang) {
    report_fatal_error("Unable to find clang.");
  }
  vector<string> args{clang.get(), opt, "-o", outputFile, objectFile};

  for (auto& libPath : libPaths) {
    args.push_back("-L" + libPath);
  }

  for (auto& library : libraries) {
    args.push_back("-l" + library);
  }

  vector<char const*> charArgs;
  for (auto& arg : args) {
    charArgs.push_back(arg.c_str());
  }
  charArgs.push_back(nullptr);

  for (auto& arg : args) {
    outs() << arg.c_str() << " ";
  }
  outs() << "\n";

  string err;
  if (-1 == ExecuteAndWait(clang.get(), &charArgs[0], nullptr, {}, 0, 0, &err)) {
    report_fatal_error("Unable to link output file.");
  }
}

static void
generateBinary(Module& m, string const& outputFilename) {
  // Compiling to native should allow things to keep working even when the
  // version of clang on the system and the version of LLVM used to compile
  // the tool don't quite match up.
  string objectFile = outputFilename + ".o";
  compile(m, objectFile);
  link(objectFile, outputFilename);
}

static void
saveModule(Module& m, StringRef const filename) {
  std::error_code errc;
  raw_fd_ostream out(filename.data(), errc, sys::fs::F_None);

  if (errc) {
    report_fatal_error("error saving llvm module to '" + filename + "': \n"
                       + errc.message());
  }
  WriteBitcodeToFile(&m, out);
}

static void
extractLoops(Module& m) {
  // Test function to print loops informations
  legacy::PassManager pm;
  pm.add(new llvm::AssumptionCacheTracker());
  pm.add(llvm::createBasicAAWrapperPass());
  pm.add(createTypeBasedAAWrapperPass());
  pm.add(createBreakCriticalEdgesPass());
  pm.add(createLoopSimplifyPass());
  pm.add(new LoopInfoWrapperPass());
  pm.add(new DominatorTreeWrapperPass());
  // pm.add(new lx::TargetLoopExtractor());
  // pm.add(new loopclouser::LoopClouser());
  pm.add(createVerifierPass());
  pm.run(m);

  // Save the new bc file
  auto replaceExt = [](string& s, const string& newExt) {
    string::size_type i = s.rfind('.', s.length());
    if (i != string::npos) {
      s.replace(i + 1, newExt.length(), newExt);
    }
  };

  replaceExt(inPath, "lx.bc");
  saveModule(m, inPath);
}

static void
AApassTest(Module& m) {
  legacy::PassManager pm;
  pm.add(createBasicAAWrapperPass());
  pm.add(llvm::createTypeBasedAAWrapperPass());
  pm.add(createGlobalsAAWrapperPass());
  pm.add(createSCEVAAWrapperPass());
  pm.add(createScopedNoAliasAAWrapperPass());
  pm.add(createCFLAndersAAWrapperPass());
  pm.add(createAAResultsWrapperPass());
  pm.add(new aew::AliasEdgeWriter());
  pm.add(createVerifierPass());
  pm.run(m);
}

static void
graphGen(Module& m) {
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
static void
splitGeps(Function& F) {
  legacy::FunctionPassManager FPM(F.getParent());
  FPM.add(new gepsplitter::GEPSplitter());
  FPM.run(F);
}

/**
 * Create root scala file
 */
static void
runRootGraph(llvm::Function& function, SetVector<Function*> call_inst, string file_name) {
  auto print_port =
      [](llvm::Function* function, string& ptrs, string& vals, string& rets) {
        uint32_t num_ptrs = 0;
        uint32_t num_vals = 0;
        for (auto& arg : function->args()) {
          if (arg.getType()->isPointerTy())
            num_ptrs++;
          else
            num_vals++;
        }

        std::vector<uint32_t> input_ptrs(num_ptrs, DATA_SIZE);
        std::vector<uint32_t> input_vals(num_vals, DATA_SIZE);
        std::vector<uint32_t> return_vals(function->getReturnType()->isVoidTy() ? 0 : 1,
                                          DATA_SIZE);

        std::stringstream in_ptrs;
        std::stringstream in_vals;
        std::stringstream ret_vals;
        std::copy(input_ptrs.begin(),
                  input_ptrs.end(),
                  std::experimental::make_ostream_joiner(in_ptrs, ", "));
        std::copy(input_vals.begin(),
                  input_vals.end(),
                  std::experimental::make_ostream_joiner(in_vals, ", "));
        std::copy(return_vals.begin(),
                  return_vals.end(),
                  std::experimental::make_ostream_joiner(ret_vals, ", "));

        rets = ret_vals.str();
        vals = in_vals.str();
        ptrs = in_ptrs.str();
      };
  if (file_name.empty()) {
    errs() << "o command line option must be specified.\n";
    exit(-1);
  }

  std::error_code errc;
  raw_fd_ostream out(file_name + "_root.scala", errc, sys::fs::F_None);

  string ptrs, vals, rets;
  print_port(&function, ptrs, vals, rets);
  out << "package dandelion.generator \n\n"
         "import chipsalliance.rocketchip.config._\n"
         "import chisel3._\n"
         "import dandelion.accel._\n"
         "import dandelion.memory._\n\n"
         "class "
      << file_name << "RootDF("
      << "PtrsIn : Seq[Int] = List (" << ptrs << "), ValsIn : Seq[Int] = List(" << vals
      << "), Returns: Seq[Int] = List(" << rets << "))\n"
      << "                  (implicit p: Parameters) extends "
         "DandelionAccelDCRModule(PtrsIn, ValsIn, Returns) {\n\n"
         "  val NumKernels = "
      << call_inst.size()
      << "\n"
         "  val memory_arbiter = Module(new MemArbiter(NumKernels))\n\n  "
         "/**\n    * Local memories\n    */\n";

  for (auto func : call_inst) {
    auto alloca_list = getInstList<AllocaInst>(func);
    uint32_t mem_cnt = 0;

    for (auto mem : alloca_list) {
      // Getting alloca type size
      // auto alloca_type = mem->getAllocatedType();
      // auto DL = mem->getModule()->getDataLayout();
      // auto num_byte = DL.getTypeAllocSize(alloca_type);

      uint32_t num_elements = mem->getAllocatedType()->getArrayNumElements();

      out << "  val memory_" << mem_cnt
          << " = Module(new ScratchPadMemory(Size = " << num_elements << "))\n";
    }

    out << "\n  /**\n    * Kernel Modules\n    */\n";

    string ptrs, vals, rets;
    print_port(func, ptrs, vals, rets);

    out << "  val " << func->getName() << " = "
        << " Module(new " << func->getName() << "DF(PtrsIn = List(" << ptrs
        << "), ValsIn = List(" << vals << "), Returns = List(" << rets << ")))\n";
  }

  out << "\n  " << function.getName()
      << ".io.in <> io.in\n"
         "  io.out <> "
      << function.getName() << ".io.out\n\n";

  std::map<Instruction*, uint32_t> calls;
  uint32_t inst_id = 0;
  for (auto& ins : llvm::instructions(function)) {
    if (auto _call = dyn_cast<llvm::CallInst>(&ins)) {
      auto called =
          dyn_cast<Function>(CallSite(_call).getCalledValue()->stripPointerCasts());
      if (!called)
        continue;

      // Skip debug function
      if (called->isDeclaration())
        continue;
      calls.insert(std::make_pair(_call, inst_id));
    }
    inst_id++;
  }

  for (auto& _cins : calls) {
    auto called =
        dyn_cast<Function>(CallSite(_cins.first).getCalledValue()->stripPointerCasts());
    out << "  " << called->getName() << ".io.in <> "
        << _cins.first->getFunction()->getName() << "." << _cins.first->getName() << "_"
        << _cins.second
        << "_out_io\n"
           "  "
        << _cins.first->getFunction()->getName() << "." << _cins.first->getName() << "_"
        << _cins.second << "_in_io <> " << called->getName() << ".io.out\n\n";
  }

  for (auto func : call_inst) {
    auto alloca_list = getInstList<AllocaInst>(func);

    uint32_t mem_cnt = 0;
    for (auto mem : alloca_list) {
      out << "  memory_" << mem_cnt << ".io.req <> " << func->getName() << "."
          << mem->getName()
          << "_mem_req\n"
             "  "
          << mem->getName() << "_mem_resp <> "
          << "memory_" << mem_cnt << ".io.resp\n";
    }

    out << "\n";
  }

  uint32_t ind = 0;
  for (auto func : call_inst) {
    out << "  memory_arbiter.io.cpu.MemReq(" << ind << ") <> " << func->getName()
        << ".io.MemReq\n  " << func->getName()
        << ".io.MemResp <> memory_arbiter.io.cpu.MemResp(" << ind << ")\n\n";
    ind++;
  }
  out << "  io.MemReq <> memory_arbiter.io.cache.MemReq\n"
         "  memory_arbiter.io.cache.MemResp <> io.MemResp\n\n"
         "}"
         "\n";
  out.close();
}

/**
 * Function lists
 */
static void
runGraphGen(Module& M, string file_name) {
  // Check wether xketch outpufile name has been specified
  if (file_name.empty()) {
    errs() << "o command line option must be specified.\n";
    exit(-1);
  }

  std::error_code errc;
  raw_fd_ostream out(file_name + ".scala", errc, sys::fs::F_None);

  legacy::PassManager pm;
  // Usefull passes
  // pm.add(new helpers::GEPAddrCalculation(target_fn));
  // pm.add(llvm::createLoopSimplifyPass());
  // pm.add(new helpers::CallInstSpliter(target_fn));
  pm.add(new llvm::AssumptionCacheTracker());
  // pm.add(createBreakCriticalEdgesPass());
  // pm.add(createLoopSimplifyPass());
  pm.add(createLoopSimplifyPass());
  // pm.add(llvm::createCFGSimplificationPass());
  pm.add(new LoopInfoWrapperPass());
  pm.add(new DominatorTreeWrapperPass());
  // pm.add(new loopclouser::LoopClouser());

  // pm.add(createBasicAAWrapperPass());
  // pm.add(llvm::createTypeBasedAAWrapperPass());
  // pm.add(createGlobalsAAWrapperPass());
  // pm.add(createSCEVAAWrapperPass());
  // pm.add(createScopedNoAliasAAWrapperPass());
  // pm.add(createCFLAndersAAWrapperPass());
  // pm.add(createAAResultsWrapperPass());
  // pm.add(new aew::AliasEdgeWriter());

  pm.add((llvm::createStripDeadDebugInfoPass()));
  pm.add(new helpers::GepInformation(file_name));
  pm.add(new graphgen::GraphGeneratorPass(NodeInfo(0, file_name), out, dump_muir.getValue()));
  pm.add(createVerifierPass());
  pm.run(M);

  out.close();
}

void
getCallInst(llvm::Function* F, SetVector<Function*>& call_inst) {
  for (auto& ins : llvm::instructions(F)) {
    if (auto _call = dyn_cast<CallInst>(&ins)) {
      auto called =
          dyn_cast<Function>(CallSite(_call).getCalledValue()->stripPointerCasts());
      if (!called) {
        continue;
      }

      // Skip debug function
      if (called->isDeclaration())
        continue;
      call_inst.insert(called);
      getCallInst(called, call_inst);
    }
  }
  return;
}

/**
 * Running UIDLabel pss
 */
void
labelFunctions(Module& M) {
  for (auto& F : M) {
    if (F.isDeclaration())
      continue;
    helpers::FunctionUIDLabel(F);
  }
}

int
main(int argc, char** argv) {
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
  for (auto& F : *module) {
    if (F.isDeclaration() || F.getName() != target_fn)
      continue;
    else
      splitGeps(F);
  }

  labelFunctions(*module);

  for (auto& F : *module) {
    if (F.isDeclaration())
      continue;
    if (F.getName() == target_fn) {
      SetVector<Function*> call_inst;
      getCallInst(&F, call_inst);

      // Insert root function
      call_inst.insert(&F);
      for (auto ff : call_inst) {
        runGraphGen(*module, ff->getName());
      }

      // Create root scala file
      runRootGraph(F, call_inst, target_fn);
    }
  }

  saveModule(*module, target_fn + ".final.bc");

  if (verbose.getValue()) {
    std::cout
        << "\n=========================== OUTPUT FILES =============================="
           "=====\n";
    std::cout << "Dandelion output for " << target_fn << ":\n";
    std::cout << "\tScala file: " << outFile << ".scala\n";
    std::cout << "\tmuIR intermediate representation: " << target_fn << ".summary.json\n";
    std::cout << "\tLLVM dependence graph: cfg." << target_fn << ".svg\n";
    std::cout << "\tTransformed LLVM bitcode: " << target_fn << ".final.bc\n";
    std::cout
        << "========================================================================="
           "===\n\n";
  }


  return 0;
}
