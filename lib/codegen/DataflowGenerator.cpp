#define DEBUG_TYPE "generator_code"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#include "luacpptemplater/LuaTemplater.h"

#include <queue>
#include <set>
#include <string>

#include "AliasMem.h"
#include "Common.h"
#include "DataflowGenerator.h"
#include "NodeType.h"

using namespace llvm;
using namespace std;
using namespace common;
using codegen::DataflowGeneratorPass;

namespace codegen {

char DataflowGeneratorPass::ID = 0;

RegisterPass<DataflowGeneratorPass> X("codegen", "Generating chisel code");
}  // namespace codegen

extern bool isTargetFunction(const Function &f,
                             const cl::list<std::string> &FunctionList);

extern cl::opt<string> outFile;

static SetVector<Loop *> getLoops(LoopInfo &LI) {
    SetVector<Loop *> Loops;

    for (auto &L : LI) {
        for (auto &SL : L->getSubLoops()) {
            Loops.insert(SL);
        }
        Loops.insert(L);
    }
    return Loops;
}

static std::map<BasicBlock *, Loop *> getBBHeader(LoopInfo &LI) {
    std::map<BasicBlock *, Loop *> loop_header_bb;
    // Getting loop header basic blocks
    for (auto &L : getLoops(LI)) {
        loop_header_bb[L->getHeader()] = L;
    }

    return loop_header_bb;
}

static std::map<BasicBlock *, Loop *> getBBEnd(LoopInfo &LI) {
    std::map<BasicBlock *, Loop *> loop_end_bb;
    // Getting loop header basic blocks
    for (auto &L : getLoops(LI)) {
        L->getLoopLatch()->dump();
        // loop_header_bb[L->getLoopLatch()] = L;
    }

    return loop_end_bb;
}

static string getBaseName(string Path) {
    auto Idx = Path.find_last_of('/');
    return Idx == string::npos ? Path : Path.substr(Idx + 1);
}

bool DataflowGeneratorPass::doInitialization(llvm::Module &M) {
    for (auto &F : M) {
        if (F.isDeclaration()) continue;

        if (F.getName() == this->FunctionName) {
            this->LI = &getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
        }
    }

    return false;
}

/// definedInCaller - Return true if the specified value is defined in the
/// function being code extracted, but not in the region being extracted.
/// These values must be passed in as live-ins to the function.
bool definedInCaller(const SetVector<BasicBlock *> &Blocks, Value *V) {
    if (isa<Argument>(V)) return true;
    if (Instruction *I = dyn_cast<Instruction>(V))
        if (!Blocks.count(I->getParent())) return true;
    return false;
}

/// definedInRegion - Return true if the specified value is defined in the
/// extracted region.
bool definedInRegion(const SetVector<BasicBlock *> &Blocks, Value *V) {
    if (Instruction *I = dyn_cast<Instruction>(V))
        if (Blocks.count(I->getParent())) return true;
    return false;
}

/**
 * Printing LLVM instruciton opcodes
 */

inline void PrintLLVMOpcodes() {
    for (uint32_t i = 0; i < 20; i++) {
        outs() << common::getLLVMOpcodeName(i) << "\n";
    }
}

inline uint32_t CountPhiNode(BasicBlock &BB) {
    uint32_t c = 0;
    for (auto &ins : BB) {
        if (dyn_cast<llvm::PHINode>(&ins)) c++;
    }
    return c;
}

/**
 * Counting number or BB's parents
 */
inline uint32_t countPred(BasicBlock &BB) {
    uint32_t count = 0;
    for (auto bb_it = llvm::pred_begin(&BB); bb_it != llvm::pred_end(&BB);
         bb_it++)
        count++;
    return count;
}

/**
 * Counting number of uses an Instruction
 */
inline uint32_t countInsUse(Instruction &Ins) {
    // uint32_t count = 0;
    // for (auto ins_it : Ins.users()) {
    // llvm::CallSite CS(ins_it);
    // if (CS) continue;
    // count++;
    //}

    return Ins.getNumUses();
}

/**
 * Returning type of the LLVM instructions from NodeType.h header
 */
InstructionType InstructionTypeNode(Instruction &ins) {
    // All the LLVM instruction's type are defined here

    // Binary instruction
    if (isa<BinaryOperator>(ins)) return common::TBinaryOperator;

    // Comparision instruction
    else if (isa<llvm::ICmpInst>(ins))
        return common::TICmpInst;

    // Branch instruction <Conditional | Unconditional>
    else if (isa<llvm::BranchInst>(ins)) {
        if (ins.getNumOperands() == 3)
            return common::TCBranchInst;
        else if (ins.getNumOperands() == 1)
            return common::TUBranchInst;
    }

    // PHI instruction
    else if (isa<llvm::PHINode>(ins))
        return common::TPHINode;

    // Alloca instruction
    else if (isa<llvm::AllocaInst>(ins))
        return common::TAlloca;

    else if (isa<llvm::CallInst>(ins))
        return common::TCallInst;

    // Return instruction
    else if (isa<llvm::ReturnInst>(ins))
        return common::TReturnInst;

    // GEP instruction
    else if (isa<llvm::GetElementPtrInst>(ins))
        return common::TGEP;

    // Load instruction
    else if (isa<llvm::LoadInst>(ins))
        return common::TLoad;

    // Store instruction
    else if (isa<llvm::StoreInst>(ins))
        return common::TStore;

    // SEXT instruction
    else if (isa<llvm::SExtInst>(ins))
        return common::TSEXT;

    // ZEXT instruction
    else if (isa<llvm::ZExtInst>(ins))
        return common::TZEXT;

    // PtrToInt instruction
    else if (isa<llvm::PtrToIntInst>(ins))
        return common::TPtrToInt;

    // Bitcast instruction
    else if (isa<llvm::BitCastInst>(ins))
        return common::TBitCast;

    // Truncate instruction
    else if (isa<llvm::TruncInst>(ins))
        return common::TTrunc;

    // Select instruction
    else if (isa<llvm::SelectInst>(ins))
        return common::TSelect;

    else if (isa<llvm::FPExtInst>(ins))
        return common::TFpext;

    else if (isa<llvm::FPTruncInst>(ins))
        return common::TFPTrunc;

#ifdef TAPIR
    // Cilk Detach Instruction
    else if (isa<llvm::DetachInst>(ins))
        return common::TDetach;

    // Cilk Reattach Instruction
    else if (isa<llvm::ReattachInst>(ins))
        return common::TReattach;

    // Cilk Sync Instruction
    else if (isa<llvm::SyncInst>(ins))
        return common::TSync;
#endif
    // Default case
    // TODO: Other type of instructions are note supported for now!
    ins.print(errs(), true);

    assert(!"ERROR: Uknown node type");
    return TNULL;
}

/**
 * Printing header part of each section of the code
 */
void DataflowGeneratorPass::printHeader(string header) {
    std::transform(header.begin(), header.end(), header.begin(), ::toupper);
    string tmp_line =
        "   * "
        "================================================================== "
        "*/\n\n";

    uint32_t remain_space = tmp_line.length() - 2 - header.length() - 23;

    // Append space to the string
    string header_final = "";
    for (uint32_t i = 0; i < remain_space - 2; i++) {
        header_final.append(" ");
    }
    header_final.append("*\n");

    tmp_line =
        "\n\n  /* "
        "================================================================== "
        "*\n"
        "   *                   " +
        header + header_final + tmp_line;
    printCode(tmp_line);
}

// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool DataflowGeneratorPass::runOnModule(Module &M) {
    // std::error_code errc;
    // raw_fd_ostream out("amirali.test", errc, sys::fs::F_None);
    // out << "Amirali";

    for (auto &F : M) {
        if (F.isDeclaration()) continue;

        if (F.getName() == this->FunctionName) {
            stripDebugInfo(F);
            DEBUG(dbgs() << "FUNCTION FOUND\n");

            // Generating XKETCH file
            generateFunction(F);

            // Generating Test function
            generateTestFunction(F);
        }
    }

    return false;
}

/**
 * Set pass dependencies
 */
void DataflowGeneratorPass::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<llvm::LoopInfoWrapperPass>();
    AU.addRequired<helpers::GEPAddrCalculation>();
    AU.addRequired<amem::AliasMem>();
    AU.addRequired<helpers::InstCounter>();
    AU.setPreservesAll();
}

/**
 * Printing the input code
 */
void DataflowGeneratorPass::printCode(string code) {
    this->outCode << code << "\n";
}

/**
 * Printing the input code
 */
void DataflowGeneratorPass::printCode(string code, raw_ostream &out) {
    out << code << "\n";
}

/**
 * Filling the instruction containers
 */
void DataflowGeneratorPass::FillInstructionContainers(llvm::Function &F) {
    for (auto &BB : F) {
        for (auto &Ins : BB) {
            instruction_use[&Ins] = 0;
            auto ins_type = InstructionTypeNode(Ins);
            if (ins_type == TUBranchInst || ins_type == TCBranchInst)
                instruction_branch.push_back(&Ins);
            else if (ins_type == TPHINode)
                instruction_phi.push_back(&Ins);
            else if (ins_type == TGEP)
                instruction_gep.push_back(&Ins);
            else if (ins_type == TLoad)
                instruction_load.push_back(&Ins);
            else if (ins_type == TStore)
                instruction_store.push_back(&Ins);
            else if (ins_type == TCallInst)
                instruction_call.push_back(&Ins);
            else if (ins_type == TAlloca)
                instruction_alloca.push_back(&Ins);
#ifdef TAPIR
            else if (ins_type == TDetach)
                instruction_detach.push_back(&Ins);
#endif
        }
    }
}

/**
 * Iterating over function arguments and fill the target function's argument
 * container
 * with the arguments and their information
 */
void DataflowGeneratorPass::FillFunctionArg(llvm::Function &F) {
    uint32_t c = 0;
    for (auto &f_arg : F.args()) {
        function_argument.push_back(&f_arg);
        ArgInfo temp_arg = {"field" + to_string(c), static_cast<uint32_t>(c)};
        argument_info[&f_arg] = temp_arg;
        c++;
    }
}

void DataflowGeneratorPass::FillLoopHeader(llvm::LoopInfo &LI) {
    uint32_t index = 0;
    for (auto &L : getLoops(LI)) {
        this->loop_header_bb[L->getHeader()] = L;
        this->loop_end_bb[L->getExitBlock()] = L;
        this->loop_index[L] = index++;
    }
}

/**
 * Iterating over function arguments and fill the target function's argument
 * container
 * with the arguments and their information
 */
void DataflowGeneratorPass::FillGlobalVar(llvm::Module &M) {
    uint32_t c = 0;
    for (auto &g_var : M.getGlobalList()) {
        // g_var.dump();
        module_global.push_back(&g_var);
        GlobalInfo temp_glob = {"glob_" + to_string(c),
                                static_cast<uint32_t>(c)};
        global_info[&g_var] = temp_glob;
        c++;
    }
}

/**
 * Naming each basic block of the target function
 * starting from index 0
 */
void DataflowGeneratorPass::NamingBasicBlock(Function &F) {
    uint32_t id_count = 0;
    uint32_t c = 0;
    this->entry_bb = &F.getEntryBlock();
    for (auto &BB : F) {
        string tmp_name = "bb_" + BB.getName().str();

        // If the basic block doesn't have name we add "unknown" prefix
        if (tmp_name.empty()) tmp_name = "unknown." + to_string(c++);

        std::replace(tmp_name.begin(), tmp_name.end(), '.', '_');
        BBInfo t_info = {tmp_name, id_count++};
        basic_block_info[&BB] = t_info;
    }
}

/**
 * Naming instruction of the target function
 * Starting from index 0
 * The prefix is m_XXX
 */
void DataflowGeneratorPass::NamingInstruction(llvm::Function &F) {
    // Initializing the counter to zero
    this->count_ins = 0;
    this->count_binary = 0;
    this->count_node = 0;
    this->count_branch = 0;

    // Append instruction global id to its name:
    // name: m_XXX
    uint32_t counter = 0;
    for (auto &BB : F) {
        for (auto &INS : BB) {
            string name = INS.getOpcodeName();
            instruction_info[&INS] = {name + to_string(this->count_ins++),
                                      counter++};
            LLVMContext &C = INS.getContext();
            MDNode *N =
                MDNode::get(C, MDString::get(C, instruction_info[&INS].name));
            INS.setMetadata("ScalaLabel", N);
        }
    }
}

/**
 * This function generate header part of chisel files.
 * It should contain all the packages which are needed for
 * generating a new Xketch file.
 * TODO:
 * \todo: make the imports configurable so that it easy to add a new library
 */
void DataflowGeneratorPass::generateImportSection(raw_ostream &out) {
    string command =
        "package dataflow\n"
        "\n"
        "import chisel3._\n"
        "import chisel3.util._\n"
        "import chisel3.Module\n"
        "import chisel3.testers._\n"
        "import chisel3.iotesters._\n"
        "import org.scalatest.{FlatSpec, Matchers}\n"
        "import muxes._\n"
        "import config._\n"
        "import control._\n"
        "import util._\n"
        "import interfaces._\n"
        "import regfile._\n"
        "import memory._\n"
        "import stack._\n"
        "import arbiters._\n"
        "import loop._\n"
        "import accel._\n"
        "import node._\n"
        "import junctions._\n\n";

    // Print to the OUTPUT
    printCode(command, out);
}

/**
 * This function dumps helper object which maps all
 * the instructions, basic block and arguments to their indexes
 */
void DataflowGeneratorPass::PrintHelperObject(llvm::Function &F) {
    LuaTemplater ins_template;

    string comment =
        "/**\n"
        "  * This Object should be initialized at the first step\n"
        "  * It contains all the transformation from indices to their "
        "module's name\n"
        "  */\n\n";
    comment = comment + "object Data_" + F.getName().str() + "_FlowParam{\n";
    param_name = "Data_" + F.getName().str() + "_FlowParam";

    // Print the first initial part of the object
    printCode(comment);

    // Iterate over branches to pick basicblock branch targets
    typedef map<llvm::BasicBlock *, vector<llvm::Instruction *>>
        BasickBlockBranch;
    BasickBlockBranch bb_branch;
    std::for_each(instruction_branch.begin(), instruction_branch.end(),
                  [this, &bb_branch](Instruction *ins) {
                      auto branch_ins = dyn_cast<llvm::BranchInst>(ins);
                      for (uint32_t i = 0; i < branch_ins->getNumSuccessors();
                           i++) {
                          bb_branch[branch_ins->getSuccessor(i)].push_back(ins);
                      }
                  });

    // XXX NOTE:
    // Entry basicblock is special case since it should be activated all the
    // time
    auto &entry_bb = F.getEntryBlock();
    string command =
        "  val {{bb_name}}_pred = Map(\n"
        "    \"active\" -> 0\n"
        "  )\n\n";
    ins_template.set("bb_name", basic_block_info[&entry_bb].name);

    // Printing entry basic block mapping
    printCode(ins_template.render(command));

    string final_command;
    for (auto bb_to_branch : bb_branch) {
        final_command = "";
        command = "  val {{bb_name}}_pred = Map(\n";
        ins_template.set("bb_name", basic_block_info[bb_to_branch.first].name);
        final_command.append(ins_template.render(command));
        uint32_t c = 0;
        for (auto ins_bb : bb_to_branch.second) {
            llvm::CallSite CS(ins_bb);
            if (CS) continue;

            command = "    \"{{ins_name}}\" -> {{index}},\n";
            ins_template.set("ins_name", instruction_info[ins_bb].name);
            ins_template.set("index", static_cast<int>(c++));
            final_command.append(ins_template.render(command));
        }
        final_command = final_command.substr(0, final_command.length() - 2);
        final_command.append("\n  )\n\n");
        printCode(final_command);
    }

    for (auto branch_to_bb : instruction_branch) {
        llvm::CallSite CS(branch_to_bb);
        if (CS) continue;

        // Connecting branch instructions to their basic block
        final_command.clear();
        command = "  val {{ins_name}}_brn_bb = Map(\n";
        ins_template.set("ins_name", instruction_info[branch_to_bb].name);
        final_command.append(ins_template.render(command));

        auto branch_ins = dyn_cast<llvm::BranchInst>(branch_to_bb);
        for (uint32_t i = 0; i < branch_ins->getNumSuccessors(); i++) {
            command = "    \"{{bb_name}}\" -> {{index}},\n";
            ins_template.set(
                "bb_name", basic_block_info[branch_ins->getSuccessor(i)].name);
            ins_template.set("index", static_cast<int>(i));
            final_command.append(ins_template.render(command));
        }
        final_command = final_command.substr(0, final_command.length() - 2);
        final_command.append("\n  )\n\n");
        printCode(final_command);
    }
#ifdef TAPIR
    // Iterate over branches to pick basicblock branch targets
    typedef map<llvm::BasicBlock *, vector<llvm::Instruction *>>
        BasickBlockDetach;
    BasickBlockBranch bb_detach;
    std::for_each(instruction_detach.begin(), instruction_detach.end(),
                  [this, &bb_detach](Instruction *ins) {
                      auto detach_ins = dyn_cast<llvm::DetachInst>(ins);
                      for (uint32_t i = 0; i < detach_ins->getNumSuccessors();
                           i++) {
                          bb_detach[detach_ins->getSuccessor(i)].push_back(ins);
                      }
                  });
    for (auto bb_to_detach : bb_detach) {
        final_command = "";
        command = "  val {{bb_name}}_pred = Map(\n";
        ins_template.set("bb_name", basic_block_info[bb_to_detach.first].name);
        final_command.append(ins_template.render(command));
        uint32_t c = 0;
        for (auto ins_bb : bb_to_detach.second) {
            command = "    \"{{ins_name}}\" -> {{index}},\n";
            ins_template.set("ins_name", instruction_info[ins_bb].name);
            ins_template.set("index", static_cast<int>(c++));
            final_command.append(ins_template.render(command));
        }
        final_command = final_command.substr(0, final_command.length() - 2);
        final_command.append("\n  )\n\n");
        printCode(final_command);
    }

    for (auto detach_to_bb : instruction_detach) {
        // Connecting branch instructions to their basic block
        final_command.clear();
        command = "  val {{ins_name}}_brn_bb = Map(\n";
        ins_template.set("ins_name", instruction_info[detach_to_bb].name);
        final_command.append(ins_template.render(command));

        auto detach_ins = dyn_cast<llvm::DetachInst>(detach_to_bb);
        for (uint32_t i = 0; i < detach_ins->getNumSuccessors(); i++) {
            command = "    \"{{bb_name}}\" -> {{index}},\n";
            ins_template.set(
                "bb_name", basic_block_info[detach_ins->getSuccessor(i)].name);
            ins_template.set("index", static_cast<int>(i));
            final_command.append(ins_template.render(command));
        }
        final_command = final_command.substr(0, final_command.length() - 2);
        final_command.append("\n  )\n\n");
        printCode(final_command);
    }
#endif
    for (auto &bb : F) {
        final_command.clear();
        command = "  val {{bb_name}}_activate = Map(\n";
        ins_template.set("bb_name", basic_block_info[&bb].name);
        final_command.append(ins_template.render(command));

        uint32_t c = 0;
        for (auto &ins : bb) {
            // llvm::CallSite CS(&ins);
            // if (CS) continue;

            command = "    \"{{ins_name}}\" -> {{index}},\n";
            ins_template.set("ins_name", instruction_info[&ins].name);
            ins_template.set("index", static_cast<int>(c++));
            final_command.append(ins_template.render(command));
        }
        final_command = final_command.substr(0, final_command.length() - 2);
        final_command.append("\n  )\n\n");
        printCode(final_command);
    }

    for (auto &ins : instruction_phi) {
        // Connecting branch instructions to their basic block
        final_command.clear();
        command = "  val {{ins_name}}_phi_in = Map(\n";
        ins_template.set("ins_name", instruction_info[ins].name);
        final_command.append(ins_template.render(command));

        // auto phi_ins = dyn_cast<llvm::PHINode>(ins);

        for (uint32_t i = 0; i < ins->getNumOperands(); i++) {
            auto operand = ins->getOperand(i);
            auto const_op = dyn_cast<llvm::ConstantInt>(ins->getOperand(i));

            if (dyn_cast<llvm::Argument>(operand)) {
                command = "    \"{{ins_name}}\" -> {{index}},\n";

                ptrdiff_t pos = distance(
                    function_argument.begin(),
                    find(function_argument.begin(), function_argument.end(),
                         dyn_cast<llvm::Argument>(operand)));

                string arg_name = "field" + to_string(static_cast<int>(pos));
                ins_template.set("ins_name", arg_name);
                ins_template.set("index", static_cast<int>(i));
            }

            else if (const_op) {
                command = "    \"{{const_name}}\" -> {{index}},\n";
                ins_template.set("const_name", "const_" + to_string(i));
            } else {
                command = "    \"{{ins_name}}\" -> {{index}},\n";
                ins_template.set("ins_name",
                                 instruction_info[dyn_cast<llvm::Instruction>(
                                                      ins->getOperand(i))]
                                     .name);
            }

            ins_template.set("index", static_cast<int>(i));
            final_command.append(ins_template.render(command));
        }
        final_command = final_command.substr(0, final_command.length() - 2);
        final_command.append("\n  )\n\n");
        printCode(final_command);
    }

    for (auto &bb : F) {
        for (auto &ins : bb) {
            auto br_ins = dyn_cast<llvm::BranchInst>(&ins);
            auto call_ins = dyn_cast<llvm::CallInst>(&ins);
            bool emptyMap = true;
            if (br_ins && br_ins->getNumOperands() == 1)
                continue;
            else {
                final_command.clear();

                // Printing each instruction
                string init_test = "  //";
                raw_string_ostream out(init_test);
                out << ins;

                printCode(out.str());

                command = "  val {{ins_name}}_in = Map(\n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                final_command.append(ins_template.render(command));

                for (uint32_t c = 0; c < ins.getNumOperands(); c++) {
                    if (br_ins && (br_ins->getNumOperands() == 1 || c >= 1))
                        continue;

                    if (dyn_cast<llvm::ConstantInt>(ins.getOperand(c)))
                        continue;

                    // else if (dyn_cast<llvm::BranchInst>(ins.getOperand(c)))
                    // continue;
                    else if (dyn_cast<llvm::Argument>(ins.getOperand(c))) {
                        emptyMap = false;
                        command = "    \"{{ins_name}}\" -> {{index}},\n";

                        ptrdiff_t pos = distance(
                            function_argument.begin(),
                            find(function_argument.begin(),
                                 function_argument.end(),
                                 dyn_cast<llvm::Argument>(ins.getOperand(c))));

                        string arg_name =
                            "field" + to_string(static_cast<int>(pos));
                        ins_template.set("ins_name", arg_name);
                        ins_template.set(
                            "index", static_cast<int>(
                                         argument_use[dyn_cast<llvm::Argument>(
                                             ins.getOperand(c))]++));
                        final_command.append(ins_template.render(command));
                    } else {
                        emptyMap = false;
                        command = "    \"{{ins_name}}\" -> {{index}},\n";
                        ins_template.set(
                            "ins_name",
                            instruction_info[dyn_cast<llvm::Instruction>(
                                                 ins.getOperand(c))]
                                .name);
                        ins_template.set(
                            "index",
                            static_cast<int>(
                                instruction_use[dyn_cast<llvm::Instruction>(
                                    ins.getOperand(c))]++));
                        // ins_template.set("index", static_cast<int>(c));
                        final_command.append(ins_template.render(command));
                    }
                }
                // Remove last comma
                if (!emptyMap)
                    final_command =
                        final_command.substr(0, final_command.length() - 2);
                final_command.append("\n  )\n\n");
                printCode(final_command);
            }
        }
    }

    printCode("}\n\n");
}

void DataflowGeneratorPass::PrintDatFlowAbstractIO(llvm::Function &F) {
    LuaTemplater ins_template;
    string final_command;
    string command =
        "abstract class {{module_name}}DFIO"
        "(implicit val p: Parameters) extends Module with CoreParams {\n"
        "  val io = IO(new Bundle {\n";
    ins_template.set("module_name", F.getName().str());
    final_command.append(ins_template.render(command));

    // Print input call parameters
    command = "    val in = Flipped(Decoupled(new Call(List(";
    final_command.append(ins_template.render(command));
    uint32_t c = 0;
    for (auto &ag : F.args()) {
        command = "32,";
        ins_template.set("index", static_cast<int>(c++));
        final_command.append(ins_template.render(command));
    }
    final_command.pop_back();
    command = "))))\n";
    final_command.append(ins_template.render(command));

    // Print sub-function call interface
    c = 0;
    for (auto fc : instruction_call) {
        // Call arguments to subroutine
        string name = instruction_info[fc].name;
        command = "    val {{call}}_out = Decoupled(new Call(List(";
        ins_template.set("call", instruction_info[fc].name);
        final_command.append(ins_template.render(command));
        for (auto &ag : fc->getFunction()->args()) {
            command = "32,";
            final_command.append(command);
        }
        final_command.pop_back();
        command = ")))\n";
        final_command.append(ins_template.render(command));
        // Return values from sub-routine.
        // Only supports a single 32 bit data bundle for now
        command =
            "    val {{call}}_in = Flipped(Decoupled(new Call(List(32))))\n";
        ins_template.set("call", instruction_info[fc].name);
        final_command.append(ins_template.render(command));
    }

    // Print global memory interface
    c = 0;
    for (auto &gl : F.getParent()->getGlobalList()) {
        for (User *U : gl.users()) {
            if (Instruction *Inst = dyn_cast<Instruction>(U)) {
                if (Inst->getFunction() == &F) {
                    command =
                        "    val glob_{{index}} = Flipped(Decoupled(new "
                        "DataBundle))\n";
                    ins_template.set("index", static_cast<int>(c++));
                    final_command.append(ins_template.render(command));
                    break;
                }
            }
        }
    }

    // Print cache memory interface
    final_command.append(
        "    val CacheResp = Flipped(Valid(new CacheRespT))\n"
        "    val CacheReq = Decoupled(new CacheReq)\n");

    // Print output (return) parameters
    //    if (!F.getReturnType()->isVoidTy()) {
    final_command.append("    val out = Decoupled(new Call(List(32)))\n");
    //    }

    final_command.append(
        "  })\n"
        "}\n\n");

    printCode(ins_template.render(final_command));

    final_command =
        "class {{module_name}}DF(implicit p: Parameters)"
        " extends {{module_name}}DFIO()(p) {\n";
    ins_template.set("module_name", F.getName().str());

    printHeader("Printing Module Definition");
    printCode(ins_template.render(final_command));
}

/**
 * Helper funciton for printing BasicBlocks
 */
void DataflowGeneratorPass::HelperPrintBBInit(Function &F) {
    string comment = "  //Initializing BasicBlocks: \n";
    printCode(comment);

    for (auto &BB : F) {
        // if (this->loop_header_bb.count(&BB))
        if (this->loop_end_bb.count(&BB))
            // If the basicblock is loop header
            PrintBasicBlockInit(BB, *loop_end_bb[&BB]);
        else
            PrintBasicBlockInit(BB);
    }
    printCode("\n\n");
}

/**
 * Printing Instruction nodes definition
 * The function iterate over all the instruction within the target function
 * and then print out each of them sepratly
 * XXX: If any new insturction node has been defined here we need to add a new
 * print function call that supports the new type
 */
void DataflowGeneratorPass::HelperPrintInstInit(Function &F) {
    string comment = "  //Initializing Instructions: \n";
    printCode(comment);

    auto loop_header_bb = getBBHeader(*LI);

    for (auto &BB : F) {
        // The comment serpate out each basicblock's set of instructions
        comment = "  // [BasicBlock]  " + BB.getName().str() + ":";
        printCode(comment);
        for (auto &INS : BB) {
            PrintInstInit(INS);
        }

        // Check whether the BasicBlock is loop header
        // If it's loop header basicblock, we need to print out an additional
        // node
        // which forward BasicBlock enable signal for latches this additional
        // node
        // is expand node

        // print exapand node
        // if (loop_header_bb.count(&BB)) {
        // string ex_define =
        //"  val {{bb_name}}_expand = "
        //"Module(new ExpandNode(NumOuts={{num_out}}, ID=0)(new "
        //"ControlBundle))\n";
        // LuaTemplater ex_template;

        // auto &loop = loop_header_bb[&BB];
        // ex_template.set("bb_name", basic_block_info[&BB].name);
        // ex_template.set(
        //"num_out",
        // static_cast<int>(this->loop_liveins[loop].size() + 1));
        // printCode(ex_template.render(ex_define));
        //}
        // printCode("\n");
    }
    printCode("\n");
}

/**
 * Printing Binary operations
 */
void DataflowGeneratorPass::PrintBinaryComparisionIns(Instruction &Ins) {
    // Increase the counter
    count_binary++;

    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    // TODO find out sing vs unsigned
    string ins_define =
        "  val {{ins_name}} = "
        "Module (new {{ins_type}}"
        "(NumOuts = {{num_out}}, ID = {{ins_id}}, "
        "opCode = \"{{op_code}}\")"
        "(sign={{sign_flag}}))";
    LuaTemplater ins_template;

    // Get Instruction Type
    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("ins_type",
                     InstructionInfo::instruction_name_type[ins_type]);

    // If instruction has 0 output
    if (countInsUse(Ins) == 0) assert(!"Instructions can not have 0 output!");
    // ins_template.set("num_out", static_cast<int>(1));
    else
        ins_template.set("num_out", static_cast<int>(countInsUse(Ins)));

    // Getting op code
    string cmp_ins_op = "";
    bool sign = false;

    auto is_sign = [](const bool sign) -> string {
        if (sign)
            return "true";
        else
            return "false";
    };

    if (ins_type == TICmpInst) {
        auto tmp_ins_pred = dyn_cast<llvm::ICmpInst>(&Ins)->getPredicate();

        switch (tmp_ins_pred) {
            case CmpInst::Predicate::ICMP_EQ:
                cmp_ins_op = "EQ";
                break;
            case CmpInst::Predicate::ICMP_NE:
                cmp_ins_op = "NE";
                break;
            case CmpInst::Predicate::ICMP_SGE:
                cmp_ins_op = "UGE";
                break;
            case CmpInst::Predicate::ICMP_SGT:
                cmp_ins_op = "UGT";
                break;
            case CmpInst::Predicate::ICMP_SLE:
                cmp_ins_op = "ULE";
                break;
            case CmpInst::Predicate::ICMP_SLT:
                cmp_ins_op = "ULT";
                break;
            case CmpInst::Predicate::ICMP_UGE:
                cmp_ins_op = "UGE";
                break;
            case CmpInst::Predicate::ICMP_UGT:
                cmp_ins_op = "UGT";
                break;
            case CmpInst::Predicate::ICMP_ULE:
                cmp_ins_op = "ULE";
                break;
            case CmpInst::Predicate::ICMP_ULT:
                cmp_ins_op = "ULT";
                break;
            default:
                assert(!"Unkonw OPERAND type");
                break;
        }

        ins_template.set("op_code", cmp_ins_op);
        ins_template.set("sign_flag", is_sign(sign));
    } else {
        ins_template.set("op_code", Ins.getOpcodeName());
        ins_template.set("sign_flag", is_sign(sign));
    }

    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}

void DataflowGeneratorPass::PrintBranchIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    string ins_define =
        "  val {{ins_name}} = "
        "Module (new {{ins_type}}"
        "(ID = {{ins_id}}))";
    LuaTemplater ins_template;

    // Get Instruction Type

    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("ins_type",
                     InstructionInfo::instruction_name_type[ins_type]);
    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}

void DataflowGeneratorPass::PrintPHIIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    string ins_define =
        "  val {{ins_name}} = "
        "Module (new {{ins_type}}"
        "(NumInputs = {{phi_in}}, NumOuts = {{phi_out}}"
        ", ID = {{ins_id}}))";
    LuaTemplater ins_template;

    // Get Instruction Type

    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("ins_type",
                     InstructionInfo::instruction_name_type[ins_type]);
    ins_template.set(
        "phi_in", static_cast<int>(
                      dyn_cast<llvm::PHINode>(&Ins)->getNumIncomingValues()));
    ins_template.set("phi_out", static_cast<int>(countInsUse(Ins)));
    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}

void DataflowGeneratorPass::PrintGepIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    // Getting gep pass information
    auto &gep_pass_ctx = getAnalysis<helpers::GEPAddrCalculation>();

    LuaTemplater ins_template;
    string ins_define;
    if (Ins.getNumOperands() == 2) {
        ins_define =
            "  val {{ins_name}} = "
            "Module (new GepOneNode"
            "(NumOuts = {{ins_out}}, "
            "ID = {{ins_id}})"
            "(numByte1 = {{num_byte}})"
            ")";
        ins_template.set("ins_name", instruction_info[&Ins].name);
        ins_template.set("ins_out", static_cast<int>(countInsUse(Ins)));
        ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));

        auto gep_value = gep_pass_ctx.SingleGepIns.find(&Ins);

        if (gep_value != gep_pass_ctx.SingleGepIns.end())
            ins_template.set(
                "num_byte",
                static_cast<int>(gep_pass_ctx.SingleGepIns[&Ins].numByte));
        else
            // TODO Fix the GEP computation in non cases
            ins_template.set("num_byte", 1);

    } else if (Ins.getNumOperands() == 3) {
        ins_define =
            "  val {{ins_name}} = "
            "Module (new GepTwoNode"
            "(NumOuts = {{ins_out}}, "
            "ID = {{ins_id}})"
            "(numByte1 = {{num_byte1}}, "
            "numByte2 = {{num_byte2}}))";
        ins_template.set("ins_name", instruction_info[&Ins].name);
        ins_template.set("ins_out", static_cast<int>(countInsUse(Ins)));
        ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));

        auto gep_value = gep_pass_ctx.TwoGepIns.find(&Ins);

        if (gep_value != gep_pass_ctx.TwoGepIns.end()) {
            ins_template.set(
                "num_byte1",
                static_cast<int>(gep_pass_ctx.TwoGepIns[&Ins].numByte1));

            ins_template.set(
                "num_byte2",
                static_cast<int>(gep_pass_ctx.TwoGepIns[&Ins].numByte2));

        } else {
            ins_template.set("num_byte1", 1);

            ins_template.set("num_byte2", 1);
        }

        ins_template.set(
            "num_byte1",
            static_cast<int>(gep_pass_ctx.TwoGepIns[&Ins].numByte1));
        ins_template.set(
            "num_byte2",
            static_cast<int>(gep_pass_ctx.TwoGepIns[&Ins].numByte2));

    } else {
        DEBUG(Ins.print(errs(), true));

        // Printing each instruction
        string init_test = "\n  //";
        raw_string_ostream out(init_test);
        out << Ins;
        printCode(out.str() + "\n");
        assert(!"The GEP instruction has more than two inputs");
    }

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}

void DataflowGeneratorPass::PrintLoadIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    // Getting AA pass information
    auto &aa_pass_ctx = getAnalysis<amem::AliasMem>();

    uint32_t index = 0;
    for (auto in : instruction_load) {
        if (in == &Ins) break;
        index++;
    }

    LuaTemplater ins_template;
    string ins_define =
        "  val {{ins_name}} = "
        "Module(new UnTypLoad(NumPredOps={{num_pred}}, "
        "NumSuccOps={{num_succ}}, "
        "NumOuts={{num_out}},ID={{ins_id}},RouteID={{route_id}}))";

    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("num_pred", static_cast<int>(mem_pred[&Ins].size()));
    ins_template.set("num_succ", static_cast<int>(mem_succ[&Ins].size()));
    ins_template.set("ins_out", static_cast<int>(countInsUse(Ins)));
    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));
    ins_template.set("num_out", static_cast<int>(countInsUse(Ins)));
    ins_template.set("route_id", static_cast<int>(index));

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}

void DataflowGeneratorPass::PrintStoreIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    // Getting AA pass information
    auto &aa_pass_ctx = getAnalysis<amem::AliasMem>();

    uint32_t index = 0;
    for (auto in : instruction_store) {
        if (in == &Ins) break;
        index++;
    }

    LuaTemplater ins_template;
    string ins_define =
        "  val {{ins_name}} = "
        "Module(new UnTypStore(NumPredOps={{num_pred}}, "
        "NumSuccOps={{num_succ}}, "
        "NumOuts={{num_out}},ID={{ins_id}},RouteID={{route_id}}))";

    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("num_pred", static_cast<int>(mem_pred[&Ins].size()));
    ins_template.set("num_succ", static_cast<int>(mem_succ[&Ins].size()));
    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));

    if (countInsUse(Ins) == 0)
        ins_template.set("num_out", static_cast<int>(1));
    else
        ins_template.set("num_out", static_cast<int>(countInsUse(Ins)));

    ins_template.set("route_id", static_cast<int>(index));

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}

void DataflowGeneratorPass::PrintSextIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    auto ins_cast = dyn_cast<llvm::CastInst>(&Ins);
    auto DL = Ins.getModule()->getDataLayout();

    LuaTemplater ins_template;
    string ins_define =
        "  val {{ins_name}} = "
        "Module(new SextNode(SrcW = {{src_width}}, DesW = {{dest_width}}, "
        "NumOuts={{num_out}})(p))";

    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set(
        "src_width",
        static_cast<int>(DL.getTypeAllocSize(ins_cast->getSrcTy()) * 8));
    ins_template.set(
        "dest_width",
        static_cast<int>(DL.getTypeAllocSize(ins_cast->getDestTy()) * 8));

    ins_template.set("num_out", static_cast<int>(Ins.getNumUses()));

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}

void DataflowGeneratorPass::PrintZextIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    auto ins_cast = dyn_cast<llvm::CastInst>(&Ins);
    auto DL = Ins.getModule()->getDataLayout();

    LuaTemplater ins_template;
    string ins_define =
        "  val {{ins_name}} = "
        "Module(new ZextNode(SrcW = {{src_width}}, DesW = {{dest_width}}, "
        "NumOuts={{num_out}}))";

    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set(
        "src_width",
        static_cast<int>(DL.getTypeAllocSize(ins_cast->getSrcTy()) * 8));
    ins_template.set(
        "dest_width",
        static_cast<int>(DL.getTypeAllocSize(ins_cast->getDestTy()) * 8));

    ins_template.set("num_out", static_cast<int>(Ins.getNumUses()));

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}
void DataflowGeneratorPass::PrintBitCastIns(Instruction &Ins) {
    // This stub is required because Tapir inserts spurious bitcast
    // instructions.
    errs() << "BitCast found!\n";
    LuaTemplater ins_template;
    string ins_define =
        "  val {{ins_name}} = "
        "Module(new BitCastNode(NumOuts={{num_out}}, ID={{ins_id}}))";

    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));
    ins_template.set("num_out", static_cast<int>(Ins.getNumUses()));

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}

void DataflowGeneratorPass::PrintRetIns(Instruction &Ins) {
    // Get instruction type
    auto DL = Ins.getModule()->getDataLayout();

    LuaTemplater ins_template;
    string ins_define =
        "  val {{ins_name}} = "
        "Module(new RetNode(NumPredIn=1, retTypes=List(32), ID={{ins_id}}))";

    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}

void DataflowGeneratorPass::PrintAllocaIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    auto ins_cast = dyn_cast<llvm::CastInst>(&Ins);
    auto DL = Ins.getModule()->getDataLayout();

    uint32_t index = 0;
    for (auto in : instruction_alloca) {
        if (in == &Ins) break;
        index++;
    }

    LuaTemplater ins_template;
    string ins_define =
        "  val {{ins_name}} = "
        "Module(new AllocaNode(NumOuts={{num_out}}, RouteID={{num_rout}}, "
        "ID={{ins_id}}))";

    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));
    ins_template.set("num_rout", static_cast<int>(index));
    ins_template.set("num_out", static_cast<int>(Ins.getNumUses()));

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}

void DataflowGeneratorPass::PrintCallIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    LuaTemplater ins_template;
    string final_command;
    string command =
        "  val {{ins_name}} = Module(new CallNode(ID={{ins_id}},argTypes=List(";
    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));
    auto call_ins = dyn_cast<llvm::CallInst>(&Ins);
    final_command.append(ins_template.render(command));
    for (int arg = 0; arg < call_ins->getNumArgOperands(); arg++) {
        command = "32,";
        final_command.append(ins_template.render(command));
    }
    final_command.pop_back();
    command = "),retTypes=List(32)))";
    final_command.append(ins_template.render(command));

    string result = ins_template.render(final_command);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}

#ifdef TAPIR
void DataflowGeneratorPass::PrintDetachIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    LuaTemplater ins_template;
    string ins_define =
        "  val {{ins_name}} = "
        "Module(new Detach(ID = {{ins_id}}))";
    //    "Module(new Detach(ID = {{ins_id}}, ReqBundle = {{req_bundle}}, "
    //            "RespBundle = {{resp_bundle}})(p))";

    // TODO - req_bundle and resp_bundle should be set properly
    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));
    //    ins_template.set("req_bundle", "UInt(32.W)");
    //    ins_template.set("resp_bundle", "UInt(32.W)");

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}

void DataflowGeneratorPass::PrintReattachIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    LuaTemplater ins_template;
    string ins_define =
        "  val {{ins_name}} = "
        "Module(new Reattach(NumPredIn={{ctl_in}}, ID={{ins_id}}))";

    // TODO - interface numbers should be set properly
    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("ctl_in", static_cast<int>(1));
    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}

void DataflowGeneratorPass::PrintSyncIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    LuaTemplater ins_template;
    string ins_define =
        "  val {{ins_name}} = "
        "Module(new Sync(ID = {{ins_id}}, NumOuts = 1, NumInc = 1, NumDec = "
        "1))";

    // TODO - NumInc, NumDec, NumOuts should be set properly
    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));

    string result = ins_template.render(ins_define);

    // Printing each instruction
    string init_test = "\n  //";
    raw_string_ostream out(init_test);
    out << Ins;
    printCode(out.str() + "\n" + result + "\n");
}
#endif

void DataflowGeneratorPass::PrintInstInit(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    // TODO make it switch
    // Binary or Comparision operators
    if (ins_type == TBinaryOperator || ins_type == TICmpInst)
        PrintBinaryComparisionIns(Ins);
    else if (ins_type == TCBranchInst || ins_type == TUBranchInst) {
        PrintBranchIns(Ins);
    } else if (ins_type == TPHINode) {
        PrintPHIIns(Ins);
    } else if (ins_type == TGEP) {
        PrintGepIns(Ins);
    } else if (ins_type == TLoad) {
        PrintLoadIns(Ins);
    } else if (ins_type == TStore) {
        PrintStoreIns(Ins);
    } else if (ins_type == TSEXT) {
        PrintSextIns(Ins);
    } else if (ins_type == TZEXT) {
        PrintZextIns(Ins);
    } else if (ins_type == TAlloca) {
        PrintAllocaIns(Ins);
#ifdef TAPIR
    } else if (ins_type == TDetach) {
        PrintDetachIns(Ins);
    } else if (ins_type == TReattach) {
        PrintReattachIns(Ins);
    } else if (ins_type == TSync) {
        PrintSyncIns(Ins);
#endif
    } else if (ins_type == TReturnInst) {
        PrintRetIns(Ins);
    } else if (ins_type == TCallInst) {
        PrintCallIns(Ins);
    } else if (ins_type == TBitCast) {
        PrintBitCastIns(Ins);
    } else {
        assert(!"Undefined Instruction");
    }
}

/**
 * Priniting Basic Block definition for each basic block
 */
void DataflowGeneratorPass::PrintBasicBlockInit(BasicBlock &BB) {
    // auto &ins_cnt_pass_ctx = getAnalysis<helpers::InstCounter>();

    uint32_t phi_c = CountPhiNode(BB);
    string bb_define;
    string result;
    if (phi_c == 0) {
        LuaTemplater bb_template;
        bb_define =
            "  val {{bb_name}} = "
            "Module(new BasicBlockNoMaskNode"
            "(NumInputs = {{num_target}}, NumOuts = {{num_ins}}, "
            "BID = {{bb_id}}))";

        bb_template.set("bb_name", basic_block_info[&BB].name);

        if (countPred(BB) == 0)
            bb_template.set("num_target", static_cast<int>(1));
        else
            bb_template.set("num_target", static_cast<int>(countPred(BB)));

        bb_template.set("num_ins", static_cast<int>(BB.size()));
        // static_cast<int>(ins_cnt_pass_ctx.BasicBlockCnt[&BB]));
        // bb_template.set("num_ins",
        // static_cast<int>(BB.getInstList().size()));
        bb_template.set("bb_id", static_cast<int>(basic_block_info[&BB].id));

        result = bb_template.render(bb_define);

    } else if (loop_header_bb.count(&BB)) {
        bb_define =
            "  val {{bb_name}} = "
            "Module(new BasicBlockLoopHeadNode"
            "(NumInputs = {{num_target}}, NumOuts = {{num_ins}}, NumPhi = "
            "{{phi_num}}, "
            "BID = {{bb_id}}))";
        LuaTemplater bb_template;

        bb_template.set("bb_name", basic_block_info[&BB].name);
        bb_template.set("num_target", static_cast<int>(countPred(BB)));
        bb_template.set("num_ins", static_cast<int>(BB.getInstList().size()));
        bb_template.set("phi_num", static_cast<int>(phi_c));
        bb_template.set("bb_id", static_cast<int>(basic_block_info[&BB].id));

        result = bb_template.render(bb_define);

    } else {
        bb_define =
            "  val {{bb_name}} = "
            "Module(new BasicBlockNode"
            "(NumInputs = {{num_target}}, NumOuts = {{num_ins}}, NumPhi = "
            "{{phi_num}}, "
            "BID = {{bb_id}}))";
        LuaTemplater bb_template;

        bb_template.set("bb_name", basic_block_info[&BB].name);
        bb_template.set("num_target", static_cast<int>(countPred(BB)));
        bb_template.set("num_ins", static_cast<int>(BB.getInstList().size()));
        bb_template.set("phi_num", static_cast<int>(phi_c));
        bb_template.set("bb_id", static_cast<int>(basic_block_info[&BB].id));

        result = bb_template.render(bb_define);
    }

    result = result + "\n";
    printCode(result);
}

/**
 * Priniting Basic Blcok definition for each basic block
 */
void DataflowGeneratorPass::PrintBasicBlockInit(BasicBlock &BB, Loop &L) {
    uint32_t phi_c = CountPhiNode(BB);
    string bb_define;
    string result;
    if (phi_c == 0) {
        LuaTemplater bb_template;
        bb_define =
            "  val {{bb_name}} = "
            "Module(new BasicBlockNoMaskNode"
            "(NumInputs = {{num_target}}, NumOuts = {{num_ins}}, "
            "BID = {{bb_id}}))";

        bb_template.set("bb_name", basic_block_info[&BB].name);

        if (countPred(BB) == 0)
            bb_template.set("num_target", static_cast<int>(1));
        else
            bb_template.set("num_target", static_cast<int>(countPred(BB)));

        bb_template.set("num_ins",
                        static_cast<int>(BB.getInstList().size() +
                                         this->loop_liveouts[&L].size() +
                                         this->loop_liveins[&L].size()));
        bb_template.set("bb_id", static_cast<int>(basic_block_info[&BB].id));

        result = bb_template.render(bb_define);

    } else {
        bb_define =
            "  val {{bb_name}} = "
            "Module(new BasicBlockNode"
            "(NumInputs = {{num_target}}, NumOuts = {{num_ins}}, NumPhi = "
            "{{phi_num}}, "
            "BID = {{bb_id}}))";
        LuaTemplater bb_template;

        bb_template.set("bb_name", basic_block_info[&BB].name);
        bb_template.set("num_target", static_cast<int>(countPred(BB)));

        // NOTE: plus 1 because we would have one extra expand node for the last
        // phi node
        bb_template.set("num_ins",
                        static_cast<int>(BB.getInstList().size() +
                                         this->loop_liveins[&L].size() +
                                         this->loop_liveouts[&L].size()));
        bb_template.set("phi_num", static_cast<int>(phi_c));
        bb_template.set("bb_id", static_cast<int>(basic_block_info[&BB].id));

        result = bb_template.render(bb_define);
    }

    result = result + "\n";
    printCode(result);
}

/**
 * Printing instantiating the Registerfile
 */
void DataflowGeneratorPass::PrintRegisterFile() {
    string command =
        "\tval RegisterFile = Module(new "
        "TypeStackFile(ID=0,Size=32,NReads={{load_num}},NWrites={{store_num}"
        "})"
        "\n"
        "\t\t            (WControl=new "
        "WriteMemoryController(NumOps={{store_num}},BaseSize=2,NumEntries=2))\n"
        "\t\t            (RControl=new "
        "ReadMemoryController(NumOps={{load_num}},BaseSize=2,NumEntries=2)))\n";

    LuaTemplater stack_template;
    if (instruction_load.size() == 0)
        stack_template.set("load_num", static_cast<int>(2));
    else
        stack_template.set("load_num",
                           static_cast<int>(instruction_load.size()));

    if (instruction_store.size() == 0)
        stack_template.set("store_num", static_cast<int>(2));
    else
        stack_template.set("store_num",
                           static_cast<int>(instruction_store.size()));

    string result = stack_template.render(command);
    printCode(result);
}

/**
 * Instantiating the StackPointer
 */
void DataflowGeneratorPass::PrintStackPointer() {
    string command =
        "\tval StackPointer = Module(new "
        "Stack(NumOps = {{num_ops}}))\n";

    LuaTemplater stack_template;
    if (instruction_alloca.size() == 0)
        stack_template.set("num_ops", static_cast<int>(1));
    else
        stack_template.set("num_ops",
                           static_cast<int>(instruction_alloca.size()));

    string result = stack_template.render(command);
    printCode(result);
}

void DataflowGeneratorPass::PrintCacheMem() {
    string command =
        "\tval CacheMem = Module(new "
        "UnifiedController(ID=0,Size=32,NReads={{load_num}},NWrites={{store_"
        "num}})"
        "\n"
        "\t\t            (WControl=new "
        "WriteMemoryController(NumOps={{store_num}},BaseSize=2,NumEntries=2))\n"
        "\t\t            (RControl=new "
        "ReadMemoryController(NumOps={{load_num}},BaseSize=2,NumEntries=2))\n"
        "\t\t            (RWArbiter=new ReadWriteArbiter()))\n\n"
        "  io.CacheReq <> CacheMem.io.CacheReq\n"
        "  CacheMem.io.CacheResp <> io.CacheResp\n";

    LuaTemplater stack_template;

    if (instruction_load.size() == 0)
        stack_template.set("load_num", static_cast<int>(2));
    else
        stack_template.set("load_num",
                           static_cast<int>(instruction_load.size()));

    if (instruction_store.size() == 0)
        stack_template.set("store_num", static_cast<int>(2));
    else
        stack_template.set("store_num",
                           static_cast<int>(instruction_store.size()));

    string result = stack_template.render(command);
    printCode(result);
}

void DataflowGeneratorPass::PrintInputSplitter(llvm::Function &F) {
    string final_command;
    string command = "  val InputSplitter = Module(new SplitCall(List(";
    final_command.append(command);

    // Print input call parameters
    uint32_t c = 0;
    for (auto &ag : F.getArgumentList()) {
        command = "32,";
        final_command.append(command);
    }
    final_command.pop_back();
    command = ")))\n";
    final_command.append(command);
    command = "  InputSplitter.io.In <> io.in\n";
    final_command.append(command);
    printCode(final_command);
}

void DataflowGeneratorPass::PrintParamObject() {
    string param =
        "  /**\n"
        "    * Instantiating parameters\n"
        "    */\n"
        "  val param = " +
        param_name + "\n";
    printCode(param);
}

void DataflowGeneratorPass::HelperPrintBasicBlockPredicate() {
    string comment =
        ""
        "  /**\n"
        "     * Connecting basic blocks to predicate instructions\n"
        "     */\n\n";
    printCode(comment);

    LuaTemplater bb_template;

    string ground_entry =
        "  {{bb_name}}.io.predicateIn <> InputSplitter.io.Out.enable\n";
    auto find_bb = basic_block_info.find(entry_bb);
    if (find_bb == basic_block_info.end())
        assert(!"COULDNT'T FIND THE BASICBLOCK INFORMATION");

    bb_template.set("bb_name", basic_block_info[this->entry_bb].name);
    string tmp_result = bb_template.render(ground_entry);

    printCode(tmp_result);

    comment =
        "  /**\n"
        "    * Connecting basic blocks to predicate instructions\n"
        "    */\n";
    printCode(comment);

    // Iterate over branch instructions and connect them their related
    // BasicBlock
    if (instruction_branch.size() == 0)
        printCode("\n  // There is no branch instruction\n\n");
    else {
        std::for_each(instruction_branch.begin(), instruction_branch.end(),
                      [this](Instruction *ins) {
                          DataflowGeneratorPass::PrintBranchBasicBlockCon(*ins);
                      });
    }
#ifdef TAPIR
    // Iterate over detach instruction and connect them their BasicBlock
    if (instruction_detach.size() == 0)
        printCode("\n  // There is no detach instruction\n\n");
    else {
        std::for_each(instruction_detach.begin(), instruction_detach.end(),
                      [this](Instruction *ins) {
                          DataflowGeneratorPass::PrintDetachBasicBlockCon(*ins);
                      });
    }
#endif
}

/**
 * Connecting connections between Branches and target BasicBlocks
 * @param ins input branch instruction
 */
void DataflowGeneratorPass::PrintBranchBasicBlockCon(Instruction &ins) {
    auto loop_header_bb = getBBHeader(*LI);

    auto branch_ins = dyn_cast<llvm::BranchInst>(&ins);

    LuaTemplater ins_template;

    for (uint32_t i = 0; i < branch_ins->getNumSuccessors(); i++) {
        // if (loop_header_bb.count(ins.getParent())) {
        // auto I = ins.getParent()->end();
        // if (I == ins.getParent()->begin()) assert(!"WRONG!");
        //// Check wether it's the last instruction
        // if (&ins == &*--I) {
        // if (i == 1) {
        // string comment =
        //"  //Connecting {{ins_name}} to {{basic_block}}";
        // string command =
        //"  "
        //"{{basic_block}}.io.predicateIn(param.{{basic_block}}_"
        //"pred(\"{{"
        //"ins_"
        //"name}}\"))"
        //" <> "
        //"{{ins_name}}.io.Out(param.{{ins_name}}_brn_bb(\"{{"
        //"basic_block}"
        //"}\")"
        //")\n\n";

        // ins_template.set("ins_name", instruction_info[&ins].name);
        // ins_template.set(
        //"basic_block",
        // basic_block_info[branch_ins->getSuccessor(i)].name);
        // ins_template.set(
        //"parent_bb",
        // basic_block_info[branch_ins->getParent()].name);

        // string result = ins_template.render(comment);
        // printCode(result);

        // result = ins_template.render(command);
        // printCode(result);
        // continue;
        //}
        //}
        //}

        auto tmp_bb = branch_ins->getSuccessor(i);
        auto phi_c = CountPhiNode(*tmp_bb);

        string comment = "  //Connecting {{ins_name}} to {{basic_block}}";
        string command = "";
        if (phi_c == 0) {
            command =
                "  "
                "{{basic_block}}.io.predicateIn"
                " <> "
                "{{ins_name}}.io.Out(param.{{ins_name}}_brn_bb(\"{{basic_block}"
                "}\")"
                ")\n\n";

        } else {
            command =
                "  "
                "{{basic_block}}.io.predicateIn(param.{{basic_block}}_pred(\"{{"
                "ins_"
                "name}}\"))"
                " <> "
                "{{ins_name}}.io.Out(param.{{ins_name}}_brn_bb(\"{{basic_block}"
                "}\")"
                ")\n\n";
        }

        ins_template.set("ins_name", instruction_info[&ins].name);
        ins_template.set("basic_block",
                         basic_block_info[branch_ins->getSuccessor(i)].name);

        string result = ins_template.render(comment);
        printCode(result);

        result = ins_template.render(command);
        printCode(result);
    }
}

#ifdef TAPIR
/**
 * Connecting connections between Branches and target BasicBlocks
 * @param ins input branch instruction
 */
void DataflowGeneratorPass::PrintDetachBasicBlockCon(Instruction &ins) {
    auto detach_ins = dyn_cast<llvm::DetachInst>(&ins);

    LuaTemplater ins_template;

    for (uint32_t i = 0; i < detach_ins->getNumSuccessors(); i++) {
        auto tmp_bb = detach_ins->getSuccessor(i);
        auto phi_c = CountPhiNode(*tmp_bb);
        string comment = "  //Connecting {{ins_name}} to {{basic_block}}";
        string command = "";
        if (phi_c == 0) {
            command =
                "  "
                "{{basic_block}}.io.predicateIn"
                " <> "
                "{{ins_name}}.io.Out(param.{{ins_name}}_brn_bb(\"{{basic_block}"
                "}\")"
                ")\n\n";

        } else {
            command =
                "  "
                "{{basic_block}}.io.predicateIn(param.{{basic_block}}_pred(\"{{"
                "ins_"
                "name}}\"))"
                " <> "
                "{{ins_name}}.io.Out(param.{{ins_name}}_brn_bb(\"{{basic_block}"
                "}\")"
                ")\n\n";
        }
        ins_template.set("ins_name", instruction_info[&ins].name);
        ins_template.set("basic_block",
                         basic_block_info[detach_ins->getSuccessor(i)].name);

        string result = ins_template.render(comment);
        printCode(result);

        result = ins_template.render(command);
        printCode(result);
    }
}
#endif
void DataflowGeneratorPass::HelperPrintBasicBlockPhi() {
    string comment =
        "  /**\n"
        "    * Connecting PHI Masks\n"
        "    */\n"
        "  //Connect PHI node\n";
    printCode(comment);

    std::for_each(
        instruction_phi.begin(), instruction_phi.end(),
        [this](Instruction *ins) { DataflowGeneratorPass::PrintPHICon(*ins); });

    comment =
        "  /**\n"
        "    * Connecting PHI Masks\n"
        "    */\n"
        "  //Connect PHI node\n";

    printCode(comment);

    if (instruction_phi.size() == 0) {
        comment = comment + "  // There is no PHI node";
        printCode(comment);
    }

    else {
        map<BasicBlock *, uint32_t> bb_index;
        std::for_each(instruction_phi.begin(), instruction_phi.end(),
                      [this, &bb_index](Instruction *ins) {
                          DataflowGeneratorPass::PrintPHIMask(*ins, bb_index);
                      });
    }
}

/**
 * Connecting phi node connections
 * @param ins Phi instruction
 */
void DataflowGeneratorPass::PrintPHICon(llvm::Instruction &ins) {
    auto phi_ins = dyn_cast<llvm::PHINode>(&ins);

    LuaTemplater ins_template;

    for (uint32_t c = 0; c < phi_ins->getNumOperands(); c++) {
        // Getting target
        auto operand = phi_ins->getOperand(c);
        auto ins_target = dyn_cast<llvm::Instruction>(phi_ins->getOperand(c));
        auto operand_const = dyn_cast<llvm::ConstantInt>(ins.getOperand(c));

        // Check if the input is function argument
        auto tmp_fun_arg = dyn_cast<llvm::Argument>(operand);
        auto tmp_find_arg = find(function_argument.begin(),
                                 function_argument.end(), tmp_fun_arg);

        Loop *target_loop = nullptr;

        /**
         * Check if the edge in LoopEdges
         * This edge count is the loop header
         */
        auto loop_edge = std::find_if(
            this->LoopEdges.begin(), this->LoopEdges.end(),
            [&operand, &ins](const pair<Value *, Value *> &edge) {
                return ((edge.second == &ins) && (edge.first == operand));
            });

        /**
         * If we find the edge in the container it means that
         * we have to connect the instruction to a latch
         * We still need to figure out:
         *  1) The instruction belongs to which loop
         *  2) The edge is live-in or live-out
         *
         *  We have splited the edges and connect the source to the register
         * file
         *  and register file to the destination
         *
         *  Now we have to replace the SRC connection with appropriate loop
         *  header
         *  We need to find two things:
         *      1) Loop
         *      2) index of the loop header
         */

        // Make a priority_queue of for loops
        //  so we start searching loops from the most inner one
        auto cmp = [](Loop *left, Loop *right) {
            return left->getSubLoops().size() > right->getSubLoops().size();
        };
        std::priority_queue<Loop *, vector<Loop *>, decltype(cmp)> order_loops(
            cmp);
        for (auto &L : getLoops(*LI)) {
            order_loops.push(L);
        }

        if (loop_edge != this->LoopEdges.end()) {
            while (!order_loops.empty()) {
                auto L = order_loops.top();
                auto Loc = L->getStartLoc();
                string Filename = string();

                if (Loc)
                    Filename = getBaseName(Loc->getFilename().str());
                else
                    Filename = "tmp_file_name";

                if (L->contains(&ins)) {
                    target_loop = L;
                    break;
                }

                order_loops.pop();
            }
        }

        if (target_loop != nullptr) {
            auto Loc = target_loop->getStartLoc();

            auto op_ins = ins.getOperand(c);
            auto op_arg = dyn_cast<llvm::Argument>(op_ins);
            string comment = "  // Wiring Live in to PHI node\n";

            string command =
                "  "
                "{{phi_name}}.io.InData(param.{{phi_name}}_phi_in(\"{{operand_"
                "name}}\")) <> "
                "{{loop_name}}_liveIN_{{loop_index}}.io.Out"
                "(param.{{phi_name}}_in(\"{{operand_name}}\"))\n";
            ins_template.set("phi_name", instruction_info[&ins].name);
            ins_template.set("index", static_cast<int>(c));

            if (isa<Argument>(op_ins))
                ins_template.set("operand_name", argument_info[op_arg].name);

            else if (isa<Instruction>(op_ins))
                ins_template.set(
                    "operand_name",
                    instruction_info[dyn_cast<llvm::Instruction>(op_ins)].name);

            ins_template.set(
                "loop_name",
                "loop_L_" + std::to_string(loop_index[target_loop]));
            ins_template.set("loop_index",
                             static_cast<int>(this->ins_loop_header_idx[&ins]));

            printCode(comment);
            printCode(ins_template.render(command));

        } else if (tmp_find_arg != function_argument.end()) {
            auto op_ins = ins.getOperand(c);
            auto op_arg = dyn_cast<llvm::Argument>(op_ins);

            string command =
                "  {{phi_name}}.io.InData(param.{{phi_name}}_phi_in"
                "(\"{{ins_name}}\")) <> "
                "InputSplitter.io.Out.data(\"{{operand_name}}\")\n";

            ins_template.set("phi_name", instruction_info[&ins].name);
            ins_template.set("ins_name", argument_info[op_arg].name);
            ins_template.set("operand_name", argument_info[op_arg].name);

            printCode(ins_template.render(command));

        } else if (ins_target) {
            string command;
            command =
                "  {{phi_name}}.io.InData(param.{{phi_name}}_phi_in"
                "(\"{{ins_name}}\")) <> {{ins_name}}.io.Out"
                "(param.{{phi_name}}_in(\"{{ins_name}}\"))\n";
            ins_template.set("phi_name", instruction_info[&ins].name);
            ins_template.set("ins_name", instruction_info[ins_target].name);

            string result = ins_template.render(command);
            printCode(result);

        } else if (operand_const) {
            string comment = "  // Wiring constant\n";
            string command = "";
            command =
                "  "
                "{{phi_name}}.io.InData(param.{{phi_name}}_phi_in(\"{{const_"
                "name}}\")).bits.data := "
                "{{value}}.U\n"
                "  "
                "{{phi_name}}.io.InData(param.{{phi_name}}_phi_in(\"{{const_"
                "name}}\")).bits.predicate "
                ":= true.B\n"
                "  "
                "{{phi_name}}.io.InData(param.{{phi_name}}_phi_in(\"{{const_"
                "name}}\")).valid := "
                "true.B\n";

            ins_template.set("phi_name", instruction_info[&ins].name);
            ins_template.set("const_name", "const_" + to_string(c));
            ins_template.set("value",
                             static_cast<int>(operand_const->getSExtValue()));

            string result = ins_template.render(command);
            printCode(result);

            // ins.dump();
            // assert(!"Cannot support constant for the PHIs for now");
        }
    }
}

void DataflowGeneratorPass::PrintPHIMask(
    llvm::Instruction &ins, map<BasicBlock *, uint32_t> &bb_index) {
    // uint32_t index) {
    auto phi_ins = dyn_cast<llvm::PHINode>(&ins);
    LuaTemplater ins_template;

    string command =
        "  {{phi_name}}.io.Mask <> {{ins_name}}.io.MaskBB({{bb_index}})\n";
    ins_template.set("phi_name", instruction_info[&ins].name);
    ins_template.set("ins_name", basic_block_info[ins.getParent()].name);
    ins_template.set("bb_index", static_cast<int>(bb_index[ins.getParent()]++));

    string result = ins_template.render(command);
    printCode(result);
}

/**
 * Connecting Insturctions in the dataflow order
 * We iterate over each instrauction's operand
 * and then find the previous node and then we connect them
 * together.
 */
void DataflowGeneratorPass::NewPrintDataFlow(llvm::Instruction &ins) {
    /**
     * Each connection has two sides, left and right connections
     * Left connection should be on of the predefined insturctions type
     * within our generator
     * For the right side we can have five different options:
     *  1) Loop latches
     *  2) Const value
     *  3) Global value
     *  4) Function argument
     *  5) Instruction
     *
     *Base of these choices we generate the right value of the connection
     */

    enum RightSide {
        ConLoop = 0,
        ConConstInt,
        ConConstFP,
        ConGlobal,
        ConFunctionArg,
        ConInstruction,
        ConNull
    };

    auto find_right_value_type = [&ins](uint32_t c,
                                        llvm::Loop *loop) -> RightSide {
        if (loop)
            return RightSide::ConLoop;
        else if (dyn_cast<llvm::Instruction>(ins.getOperand(c)))
            return RightSide::ConInstruction;
        else if (dyn_cast<llvm::ConstantInt>(ins.getOperand(c)))
            return RightSide::ConConstInt;
        else if (dyn_cast<llvm::ConstantFP>(ins.getOperand(c)))
            return RightSide::ConConstFP;
        else if (dyn_cast<llvm::ConstantPointerNull>(ins.getOperand(c)))
            return RightSide::ConNull;
        else if (dyn_cast<llvm::GlobalValue>(ins.getOperand(c)))
            return RightSide::ConGlobal;
        else if (dyn_cast<llvm::Argument>(ins.getOperand(c)))
            return RightSide::ConFunctionArg;
        else {
            DEBUG(ins.print(errs(), true));
            DEBUG(ins.getOperand(c)->print(errs(), true));
            assert(!"Unrecognized operand type");
        }
    };

    auto operand_name = [this, &ins](uint32_t c) -> string {
        if (auto op = dyn_cast<llvm::Instruction>(ins.getOperand(c)))
            return this->instruction_info[op].name;
        else if (auto op = dyn_cast<llvm::ConstantInt>(ins.getOperand(c)))
            return std::to_string(static_cast<int>(op->getSExtValue()));
        else if (auto op = dyn_cast<llvm::ConstantFP>(ins.getOperand(c)))
            assert(!"We dont't support floating point right now!");
        // return std::to_string(static_cast<int>(op->getSplatValue()));
        else if (auto op =
                     dyn_cast<llvm::ConstantPointerNull>(ins.getOperand(c)))
            return std::to_string(0);
        else if (auto op = dyn_cast<llvm::GlobalValue>(ins.getOperand(c)))
            return this->global_info[op].name;
        else if (auto op = dyn_cast<llvm::Argument>(ins.getOperand(c)))
            return this->argument_info[op].name;
        else
            assert(!"Unrecognized operand type");
    };

    // For each operand we have to figure out if the connection is
    // comming from a live-in value or not, and for this purpose we
    // use following function:
    auto loop_detector = [&ins](auto LoopEdges, auto loop_info,
                                auto c) -> Loop * {
        auto operand = ins.getOperand(c);

        Loop *target_loop = nullptr;
        /**
         * Check if the edge in LoopEdges
         * This edge count is the loop header
         */
        auto loop_edge = std::find_if(
            LoopEdges.begin(), LoopEdges.end(),
            [&operand, &ins](const pair<Value *, Value *> &edge) {
                return ((edge.second == &ins) && (edge.first == operand));
            });

        /**
         * If we find the edge in the container it means that
         * we have to connect the instruction to latch
         * We still need to figure out:
         *  1) The instruction belongs to which loop
         *  2) The edge is live-in or live-out
         *
         *  We have split the edge and connect the source to the register
         * file
         *  and register file to the destination
         *
         *  Now we have to replace the SRC connection with appropriate loop
         *  header
         *  We need to find two things:
         *      1) Loop
         *      2) index of the loop header
         */
        // Make a priority_queue for loops
        auto cmp = [](Loop *left, Loop *right) {
            return left->getSubLoops().size() > right->getSubLoops().size();
        };
        std::priority_queue<Loop *, vector<Loop *>, decltype(cmp)> order_loops(
            cmp);
        for (auto &L : getLoops(*loop_info)) {
            order_loops.push(L);
        }

        if (loop_edge != LoopEdges.end()) {
            while (!order_loops.empty()) {
                auto L = order_loops.top();
                auto Loc = L->getStartLoc();
                auto Filename = getBaseName(Loc->getFilename().str());

                if (dyn_cast<Argument>(loop_edge->first)) {
                    target_loop = L;
                    break;
                }

                else if (L->contains(dyn_cast<Instruction>(loop_edge->first))) {
                    target_loop = L;
                    break;
                } else
                    assert(!"Wrong edge detector!");

                order_loops.pop();
            }
        }

        return target_loop;
    };

    /**
     * This function figure the right side string of each assignment
     */
    auto print_right_string = [this, &ins](RightSide right_type,
                                           string right_string, Loop *L,
                                           uint32_t c) -> string {
        LuaTemplater lua_right;
        string command = string();

        switch (right_type) {
            case RightSide::ConLoop:
                command =
                    "{{loop_name}}_liveIN_{{loop_index}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                lua_right.set("ins_name", instruction_info[&ins].name);
                lua_right.set(
                    "loop_name",
                    "loop_L_" + std::to_string(L->getStartLoc().getLine()));
                lua_right.set(
                    "loop_index",
                    static_cast<int>(this->ins_loop_header_idx[&ins]));
                lua_right.set("operand_name", right_string);
                break;

            case RightSide::ConInstruction:
                command =
                    "{{operand_name}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                lua_right.set("ins_name", instruction_info[&ins].name);
                lua_right.set("operand_name", right_string);
                break;
            case RightSide::ConGlobal:
                command = "io.{{operand_name}}\n";
                lua_right.set("operand_name", right_string);
                break;

            case RightSide::ConFunctionArg:
                command = "io.in.data(\"{{operand_name}}\")\n";
                lua_right.set("operand_name", right_string);
                break;
            case RightSide::ConConstInt:
            case RightSide::ConNull:
                command = "{{value}}.U\n";
                lua_right.set("value", right_string);
                break;
            case RightSide::ConConstFP:
                assert(!"We don't support floating point right now!");
                break;

            default:
                assert(!"Uknow type!");
                break;
        }

        return lua_right.render(command);
    };

    // First we iterate over function's operands
    auto ins_type = InstructionTypeNode(ins);
    for (uint32_t c = 0; c < ins.getNumOperands(); ++c) {
        // XXX If the operand is BasicBlock it means
        // the insturtion is branch we already have
        // connected the signal
        if (dyn_cast<llvm::BasicBlock>(ins.getOperand(c))) continue;

        // Initilizing command and comment string std::vector<string> command;
        string command = string();
        string command_left = string();
        string command_right = string();

        auto target_loop = loop_detector(this->LoopEdges, this->LI, c);
        auto right_type = find_right_value_type(
            c, loop_detector(this->LoopEdges, this->LI, c));

        auto ins_type = InstructionTypeNode(ins);
        auto right_string =
            print_right_string(right_type, operand_name(c), target_loop, c);

        // Setting right side of the connection
        switch (ins_type) {
            case InstructionType::TBinaryOperator:
            case InstructionType::TICmpInst:
                if (c == 0)
                    command =
                        "  //Connecting left input of {{ins_name}}\n"
                        "  {{ins_name}}.io.LeftIO <> {{right_side}}";
                else if (c == 1)
                    command =
                        "  //Connecting Right input of {{ins_name}}\n"
                        "  {{ins_name}}.io.RightIO <> {{right_side}}";
                else
                    assert(
                        !"Binary operation can not have more than two inputs");
                break;

            case InstructionType::TCBranchInst:
                if (c == 0)
                    command =
                        "  //Connecting comparision input of {{ins_name}}\n"
                        "  {{ins_name}}.io.CmpIO <> {{right_side}}";
                else
                    assert(
                        !"Conditional branch can not have more than one input");
                break;

            case InstructionType::TGEP:
                if (c == 0)
                    command =
                        "  //Connecting base address of {{ins_name}}\n"
                        "  {{ins_name}}.io.baseAddress <> {{right_side}}";
                else if (c == 1)
                    command =
                        "  //Connecting idx1 input of {{ins_nam}}\n"
                        "  {{ins_name}}.io.idx1 <> {{right_side}}";
                else if (c == 2)
                    command =
                        "  //Connecting idx2 input of {{ins_name}}\n"
                        "  {{ins_name}}.io.idx2 <> {{right_side}}";
                else
                    assert(!"The GEP instruction is not simplified!");
                break;

            default:
                DEBUG(ins.print(errs(), true));
                DEBUG(ins.getOperand(c)->print(errs(), true));
                assert(!"UKNOWN INSTRUCTION TYPE!");
                break;
        }
        ins_template.set("ins_name", instruction_info[&ins].name);
        ins_template.set("right_side", right_string);
        // Setting left side of the connection command_left =
        errs() << ins_template.render(command) << "\n";
    }
}

/**
 * Connecting Insturctions in the dataflow order
 * We iterate over each instrauction's operand
 * and then find the previous node and then we connect them
 * together.
 */
void DataflowGeneratorPass::PrintDataFlow(llvm::Instruction &ins) {
    LuaTemplater ins_template;
    auto ins_type = InstructionTypeNode(ins);

    for (uint32_t c = 0; c < ins.getNumOperands(); c++) {
        /**
         * If the operand is constant we don't need to track the previous
         * instruction
         * we need to just get the actual value and hard code it
         */
        auto operand = ins.getOperand(c);
        auto operand_ins = dyn_cast<llvm::Instruction>(ins.getOperand(c));
        auto operand_const = dyn_cast<llvm::ConstantInt>(ins.getOperand(c));
        auto operand_constFloat = dyn_cast<llvm::ConstantFP>(ins.getOperand(c));
        auto operand_null =
            dyn_cast<llvm::ConstantPointerNull>(ins.getOperand(c));
        auto operand_global = dyn_cast<llvm::GlobalValue>(ins.getOperand(c));

        // Check if the input is function argument
        auto tmp_fun_arg = dyn_cast<llvm::Argument>(operand);
        auto tmp_find_arg = find(function_argument.begin(),
                                 function_argument.end(), tmp_fun_arg);

        string command = string();
        string comment = string();

        Loop *target_loop = nullptr;

        /**
         * Check if the edge in LoopEdges
         * This edge count is the loop header
         */
        auto loop_edge = std::find_if(
            this->LoopEdges.begin(), this->LoopEdges.end(),
            [&operand, &ins](const pair<Value *, Value *> &edge) {
                return ((edge.second == &ins) && (edge.first == operand));
            });

        /**
         * If we find the edge in the container it means that
         * we have to connect the instruction to latch
         * We still need to figure out:
         *  1) The instruction belongs to which loop
         *  2) The edge is live-in or live-out
         *
         *  We have split the edge and connect the source to the register file
         *  and register file to the destination
         *
         *  Now we have to replace the SRC connection with appropriate loop
         *  header
         *  We need to find two things:
         *      1) Loop
         *      2) index of the loop header
         */
        // Make a priority_queue for loops
        auto cmp = [](Loop *left, Loop *right) {
            return left->getSubLoops().size() > right->getSubLoops().size();
        };
        std::priority_queue<Loop *, vector<Loop *>, decltype(cmp)> order_loops(
            cmp);
        for (auto &L : getLoops(*LI)) {
            order_loops.push(L);
        }

        if (loop_edge != this->LoopEdges.end()) {
            while (!order_loops.empty()) {
                auto L = order_loops.top();
                auto Loc = L->getStartLoc();
                string Filename = string();

                if (Loc)
                    Filename = getBaseName(Loc->getFilename().str());
                else
                    Filename = "tmp_file_name";

                if (dyn_cast<Argument>(loop_edge->first)) {
                    target_loop = L;
                    break;
                }

                else if (L->contains(&ins) ||
                         L->contains(dyn_cast<Instruction>(loop_edge->first))) {
                    target_loop = L;
                    break;
                }
                order_loops.pop();
            }
        }

        /**
         * If instruction is Binary or Comparision operator
         */
        if (ins_type == TBinaryOperator || ins_type == TICmpInst) {
            // If the operand comes from a loop
            if (target_loop != nullptr) {
                auto Loc = target_loop->getStartLoc();

                comment = "  // Wiring Binary instruction to the loop header\n";
              if (tmp_find_arg != function_argument.end()) {
                // int idx = 0;
                auto op_ins = ins.getOperand(c);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);
                auto op_name = argument_info[op_arg].name;
                DEBUG(dbgs() << "op_name: " << op_name << "\n");
                if (c == 0)
                  command =
                      "  {{ins_name}}.io.LeftIO <>"
                          "{{loop_name}}_liveIN_{{loop_index}}.io.Out"
                          "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                  //"{{loop_name}}_start.io.outputArg({{loop_index}})\n";

                else
                  command =
                      "  {{ins_name}}.io.RightIO <> "
                          "{{loop_name}}_liveIN_{{loop_index}}.io.Out"
                          "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                //"{{loop_name}}_start.io.outputArg({{loop_index}})\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set(
                    "loop_name",
                    "loop_L_" + std::to_string(loop_index[target_loop]));
                ins_template.set(
                    "loop_index",
                    static_cast<int>(this->ins_loop_header_idx[&ins]));
                ins_template.set("operand_name", op_name);
              } else   {              // First get the instruction
                comment = "  // Wiring instructions\n";
                command = "";
                if (c == 0)
                  command =
                      "  {{ins_name}}.io.LeftIO <> "
                          "{{loop_name}}_liveIN_{{loop_index}}.io.Out"
                          "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                else
                  command =
                      "  {{ins_name}}.io.RightIO <> "
                          "{{loop_name}}_liveIN_{{loop_index}}.io.Out"
                          "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set(
                    "loop_name",
                    "loop_L_" + std::to_string(loop_index[target_loop]));
                ins_template.set(
                    "loop_index",
                    static_cast<int>(this->ins_loop_header_idx[&ins]));
                ins_template.set("operand_name",
                                 instruction_info[dyn_cast<llvm::Instruction>(
                                     ins.getOperand(c))]
                                     .name);
              }

                // If the operand is constant
            } else if (operand_const || operand_constFloat || operand_null) {
                comment = "  // Wiring constant\n";
                command = "";
                if (c == 0) {
                    command =
                        "  {{ins_name}}.io.LeftIO.bits.data := "
                        "{{value}}.U\n"
                        "  {{ins_name}}.io.LeftIO.bits.predicate := "
                        "true.B\n"
                        "  {{ins_name}}.io.LeftIO.valid := true.B\n";

                } else {
                    command =
                        "  {{ins_name}}.io.RightIO.bits.data := "
                        "{{value}}.U\n"
                        "  {{ins_name}}.io.RightIO.bits.predicate := "
                        "true.B\n"
                        "  {{ins_name}}.io.RightIO.valid := true.B\n";
                }
                ins_template.set("ins_name", instruction_info[&ins].name);
                if (operand_const)
                    ins_template.set(
                        "value",
                        static_cast<int>(operand_const->getSExtValue()));
                else if (operand_constFloat || operand_null)
                    ins_template.set("value", 0);

            }
            // Else if the operand is function argument
            else if (tmp_find_arg != function_argument.end()) {
                // First get the instruction
                auto op_ins = ins.getOperand(c);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                comment =
                    "  // Wiring Binary instruction to the function "
                    "argument\n";
                if (c == 0)
                    command =
                        "  {{ins_name}}.io.LeftIO <> "
                        "InputSplitter.io.Out.data(\"{{operand_name}}\")\n";
                else
                    command =
                        "  {{ins_name}}.io.RightIO <> "
                        "InputSplitter.io.Out.data(\"{{operand_name}}\")\n";

                ins_template.set("ins_name", instruction_info[&ins].name);

                if (argument_info.find(op_arg) == argument_info.end()) {
                    assert(!"WRONG");
                }

                ins_template.set("operand_name", argument_info[op_arg].name);

            }
            // Else if the operand comes from another instruction
            else {
                // First get the instruction
                comment = "  // Wiring instructions\n";
                command = "";
                if (c == 0)
                    command =
                        "  {{ins_name}}.io.LeftIO <> "
                        "{{operand_name}}.io.Out"
                        "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                else
                    command =
                        "  {{ins_name}}.io.RightIO <> "
                        "{{operand_name}}.io.Out"
                        "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name",
                                 instruction_info[dyn_cast<llvm::Instruction>(
                                                      ins.getOperand(c))]
                                     .name);
            }

            // Print the code
            printCode(comment + ins_template.render(command));
        }

        /**
         * If the instruction is conditional branch
         */
        else if (ins_type == TCBranchInst) {
            // In conditional branch only the first input is data dependence
            // the rest two inputs are control dependence
            if (c == 0) {
                comment = "  // Wiring Branch instruction\n";
                command =
                    "  {{ins_name}}.io.CmpIO <> {{operand_name}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name",
                                 instruction_info[dyn_cast<llvm::Instruction>(
                                                      ins.getOperand(c))]
                                     .name);

                printCode(comment + ins_template.render(command));
            }
        }

        /**
         * Handeling the GEP instruction
         * All the GEPs are simplified by
         * createSeparateConstOffsetFromGEPPass
         * Therefor, they shouldn't have more than at most three inputs
         */
        else if (ins_type == TGEP) {
            if (target_loop != nullptr) {
                auto Loc = target_loop->getStartLoc();
                auto op_ins = ins.getOperand(c);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);
                auto op_name = argument_info[op_arg].name;

                comment = "  // Wiring GEP instruction to the loop header\n";

                command =
                    //"  {{ins_name}}.io.LeftIO <>"
                    //"{{loop_name}}_liveIN_{{loop_index}}.io.Out(0)\n";
                    "  {{ins_name}}.io.{{ins_input}} <> "
                    "{{loop_name}}_liveIN_{{loop_index}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                //"io.{{operand_name}}\n\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set(
                    "loop_name",
                    "loop_L_" + std::to_string(loop_index[target_loop]));
                ins_template.set(
                    "loop_index",
                    static_cast<int>(this->ins_loop_header_idx[&ins]));
                ins_template.set("operand_name", argument_info[op_arg].name);
                // If the operand is constant
                //} else if (operand_const || operand_constFloat ||
                // operand_null) {

                // TODO
                // Check if the GEP has two inputs or one
                // Check if it's the baseaddress or index
                // If the input is function argument

                // First get the instruction
            } else if (tmp_find_arg != function_argument.end()) {
                auto op_ins = ins.getOperand(0);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                comment =
                    "  // Wiring GEP instruction to the function "
                    "argument\n";

                command =
                    "  {{ins_name}}.io.{{ins_input}} <> "
                    "InputSplitter.io.Out.data(\"{{operand_name}}\")\n\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set(
                    "operand_name",
                    argument_info[dyn_cast<llvm::Argument>(ins.getOperand(c))]
                        .name);
            }
            // If the input is constant
            else if (operand_const) {
                comment = "  // Wiring GEP instruction to the Constant\n";
                command =
                    "  {{ins_name}}.io.{{ins_input}}.valid :=  true.B\n"
                    "  {{ins_name}}.io.{{ins_input}}.bits.predicate :=  "
                    "true.B\n"
                    "  {{ins_name}}.io.{{ins_input}}.bits.data :=  "
                    "{{value}}.U\n\n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set(
                    "value", static_cast<int>(operand_const->getSExtValue()));

            } else if (operand_ins) {
                comment =
                    "  // Wiring GEP instruction to the parent "
                    "instruction\n";
                command =
                    "  {{ins_name}}.io.{{ins_input}} <> "
                    "{{operand_name}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n\n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name",
                                 instruction_info[dyn_cast<llvm::Instruction>(
                                                      ins.getOperand(c))]
                                     .name);
            }

            if (c == 0)
                ins_template.set("ins_input", "baseAddress");
            else if (c == 1)
                ins_template.set("ins_input", "idx1");
            else if (c == 2)
                ins_template.set("ins_input", "idx2");
            else
                assert(!"The GEP instruction is not simplified!");

            printCode(comment + ins_template.render(command));
        }

        /**
         * Connecting LOAD instructions
         */
        else if (ins_type == TLoad) {
            // Input of the load comes from either GEP instructions or
            // function
            // arguments
            auto gep_ins = dyn_cast<llvm::GetElementPtrInst>(ins.getOperand(c));
            auto bit_ins = dyn_cast<llvm::BitCastInst>(ins.getOperand(c));
            // If the input is function argument then it should get connect
            // to
            // Cache system
            if (target_loop != nullptr) {
                // Find loop information
                auto Loc = target_loop->getStartLoc();

                // First get the instruction
                auto op_ins = ins.getOperand(c);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                comment = "  // Wiring Load instruction to the loop latch\n";

                command =
                    "  {{ins_name}}.io.GepAddr <>"
                    " {{loop_name}}_liveIN_{{loop_index}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n"
                    "  {{ins_name}}.io.memResp <> "
                    "CacheMem.io.ReadOut({{ins_index}})\n"
                    "  CacheMem.io.ReadIn({{ins_index}}) <> "
                    "{{ins_name}}.io.memReq\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set(
                    "loop_name",
                    "loop_L_" + std::to_string(loop_index[target_loop]));
                ins_template.set(
                    "loop_index",
                    static_cast<int>(this->ins_loop_header_idx[&ins]));

                if (isa<Argument>(op_ins))
                    ins_template.set("operand_name",
                                     argument_info[op_arg].name);

                else if (isa<Instruction>(op_ins))
                    ins_template.set(
                        "operand_name",
                        instruction_info[dyn_cast<llvm::Instruction>(op_ins)]
                            .name);

            } else if (tmp_find_arg != function_argument.end()) {
                // First get the instruction
                auto op_ins = ins.getOperand(c);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                comment =
                    "  // Wiring Load instruction to the function "
                    "argument\n";

                command =
                    "  {{ins_name}}.io.GepAddr <> "
                    "InputSplitter.io.Out.data(\"{{operand_name}}\")\n"
                    "  {{ins_name}}.io.memResp <> "
                    "CacheMem.io.ReadOut({{ins_index}})\n"
                    "  CacheMem.io.ReadIn({{ins_index}}) <> "
                    "{{ins_name}}.io.memReq\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name", argument_info[op_arg].name);
            }

            else if (gep_ins || bit_ins) {
                // TODO Why the second condition?
                // else if (gep_ins ||
                // ins.getOperand(c)->getType()->isPointerTy()) {
                comment =
                    "  // Wiring Load instruction to the parent "
                    "instruction\n";
                command =
                    "  {{ins_name}}.io.GepAddr <> {{operand_name}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n"
                    "  {{ins_name}}.io.memResp <> "
                    "CacheMem.io.ReadOut({{ins_index}})\n"
                    "  CacheMem.io.ReadIn({{ins_index}}) <> "
                    "{{ins_name}}.io.memReq\n\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name",
                                 instruction_info[dyn_cast<llvm::Instruction>(
                                                      ins.getOperand(c))]
                                     .name);
            } else if (operand_global) {
                // TODO handel global values as well
                comment =
                    "  // Wiring Load instruction to the parent "
                    "instruction\n";
                command =
                    "  {{ins_name}}.io.GepAddr <> io.{{operand_name}}\n"
                    "  {{ins_name}}.io.memResp <> "
                    "CacheMem.io.ReadOut({{ins_index}})\n"
                    "  CacheMem.io.ReadIn({{ins_index}}) <> "
                    "{{ins_name}}.io.memReq\n\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name",
                                 global_info[operand_global].name);

            } else if (operand_ins) {
                comment =
                    "  // Wiring Load instruction to another instruction\n";

                command =
                    // TODO fix the Out(0) index
                    "  {{ins_name}}.io.{{ins_input}} <> "
                    "{{operand_name}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n\n";
                "  {{ins_name}}.io.memResp <> "
                "CacheMem.io.ReadOut({{ins_index}})\n"
                "  CacheMem.io.ReadIn({{ins_index}}) <> "
                "{{ins_name}}.io.memReq\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name",
                                 instruction_info[operand_ins].name);

            } else {
                DEBUG(ins.print(errs(), true));
                assert(!"Wrong load input");
            }

            ptrdiff_t pos = distance(
                instruction_load.begin(),
                find(instruction_load.begin(), instruction_load.end(), &ins));
            ins_template.set("ins_index", static_cast<int>(pos));
            printCode(comment + ins_template.render(command));

            /**
             * Connecting Predecessors
             */

            // Getting AA pass information
            comment = "  //Printing succesor of the current Load instruction\n";

            auto ins_mp = mem_pred.find(&ins);
            for (auto mp : ins_mp->second) {
                command =
                    "{{ins_name}}.io.PredOp(0) <> "
                    "{{pred_ins}}.io.SuccOp(0)\n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("pred_ins", instruction_info[mp].name);
                printCode(ins_template.render(command));
            }
            printCode("\n");
        }

        /**
         * Print Store instructions
         */
        else if (ins_type == TStore) {
            auto operand = ins.getOperand(c);
            auto gep_ins = dyn_cast<llvm::GetElementPtrInst>(ins.getOperand(c));
            auto operand_const = dyn_cast<llvm::ConstantInt>(ins.getOperand(c));

            // If the input is from function argument
            auto tmp_fun_arg = dyn_cast<llvm::Argument>(operand);
            auto tmp_find_arg = find(function_argument.begin(),
                                     function_argument.end(), tmp_fun_arg);

            if (c == 0) {
                // If the input is function argument
                if (tmp_find_arg != function_argument.end()) {
                    // First get the instruction
                    auto op_ins = ins.getOperand(c);
                    auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                    comment =
                        "  // Wiring Store instruction to the function "
                        "argument\n";
                    command =
                        "  {{ins_name}}.io.inData <> "
                        "InputSplitter.io.Out.data(\"{{operand_name}}\")\n";
                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set("operand_name",
                                     argument_info[op_arg].name);
                } else if (operand_global) {
                    // TODO handel global values as well
                    comment = "  // Wiring Store instruction to global value\n";
                    command =
                        "  {{ins_name}}.io.inData <> io.{{operand_name}}\n";

                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set("operand_name",
                                     global_info[operand_global].name);

                } else if (operand_const) {
                    comment = "  // Wiring constant instructions to store\n";
                    command =
                        "  {{ins_name}}.io.inData.bits.data := "
                        "{{value}}.U\n"
                        "  {{ins_name}}.io.inData.bits.predicate := "
                        "true.B\n"
                        "  {{ins_name}}.io.inData.valid := true.B\n";
                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set(
                        "value",
                        static_cast<int>(operand_const->getSExtValue()));

                } else {
                    // If the store input comes from an instruction
                    comment =
                        "  // Wiring Store instruction to the parent "
                        "instruction\n";
                    comment = "";
                    command =
                        "  {{ins_name}}.io.inData <> "
                        "{{operand_name}}.io.Out"
                        "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";

                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set(
                        "operand_name",
                        instruction_info[dyn_cast<llvm::Instruction>(
                                             ins.getOperand(c))]
                            .name);
                }

            } else {
                // If the input is function argument
                if (tmp_find_arg != function_argument.end()) {
                    // First get the instruction
                    auto op_ins = ins.getOperand(c);
                    auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                    comment =
                        "  // Wiring Store instruction to the function "
                        "argument\n";
                    command =
                        "  {{ins_name}}.io.GepAddr <> "
                        "InputSplitter.io.Out.data(\"{{operand_name}}\")\n";

                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set("operand_name",
                                     argument_info[op_arg].name);

                } else if (dyn_cast<llvm::Instruction>(ins.getOperand(c))) {
                    comment =
                        "  // Wiring Store instruction to the parent "
                        "instruction\n";
                    command =
                        "  {{ins_name}}.io.GepAddr <> "
                        "{{operand_name}}.io.Out"
                        "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set(
                        "operand_name",
                        instruction_info[dyn_cast<llvm::Instruction>(
                                             ins.getOperand(c))]
                            .name);
                } else if (operand_global) {
                    comment =
                        "  // Wiring Store instruction to the global "
                        "value\n";
                    command =
                        "  {{ins_name}}.io.GepAddr <> "
                        "io.{{operand_name}}\n";
                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set("operand_name",
                                     global_info[operand_global].name);
                } else {
                    comment =
                        "  // Wiring Store instruction to the parent "
                        "instruction\n";
                    command =
                        "  {{ins_name}}.io.GepAddr.bits.data      := "
                        "0.U\n"
                        "  {{ins_name}}.io.GepAddr.bits.predicate := "
                        "true.B\n"
                        "  {{ins_name}}.io.GepAddr.bits.valid     := "
                        "true.B\n";

                    ins_template.set("ins_name", instruction_info[&ins].name);
                }
                // command.append("Amirali\n");
                command.append(
                    "  {{ins_name}}.io.memResp  <> "
                    "CacheMem.io.WriteOut({{ins_index}})\n"
                    "  CacheMem.io.WriteIn({{ins_index}}) <> "
                    "{{ins_name}}.io.memReq\n"
                    "  {{ins_name}}.io.Out(0).ready := true.B\n");
            }

            ptrdiff_t pos = distance(
                instruction_store.begin(),
                find(instruction_store.begin(), instruction_store.end(), &ins));
            ins_template.set("ins_index", static_cast<int>(pos));
            printCode(comment + ins_template.render(command));

            // Connecting Predecessors

            // Getting AA pass information
            comment = "  //Printing succesor of the current Load instruction\n";

            auto ins_mp = mem_pred.find(&ins);
            if (ins_mp != mem_pred.end()) {
                for (auto mp : ins_mp->second) {
                    command =
                        "{{ins_name}}.io.PredOp(0) <> "
                        "{{pred_ins}}.io.SuccOp(0)\n";
                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set("pred_ins", instruction_info[mp].name);
                    printCode(ins_template.render(command));
                }
                printCode("\n");
            }
        }

        else if (ins_type == TAlloca) {
            /**
             * In Alloca instruction we need to compute size of bytes which
             * we
             * asked
             * The first case is the alloca size static
             * TODO The second case is the alloca size dynamic
             */

            comment = "  // Wiring Alloca instructions with Static inputs\n";
            if (c == 0) {
                // Get the alloca instruction
                auto alloca_ins = dyn_cast<llvm::AllocaInst>(&ins);
                auto alloca_type = alloca_ins->getAllocatedType();

                // Getting datalayout to compute the size of the variables
                auto DL = ins.getModule()->getDataLayout();
                string init_test = "  //";
                raw_string_ostream out(init_test);

                // TODO handle struct type
                if (alloca_type->isArrayTy()) {
                    // Connecting AllocaIO input
                    auto num_byte = DL.getTypeAllocSize(alloca_type);
                    command =
                        "  {{ins_name}}.io.allocaInputIO.bits.size      := "
                        "1.U\n"
                        "  {{ins_name}}.io.allocaInputIO.bits.numByte   := "
                        "{{num_byte}}.U\n"
                        "  {{ins_name}}.io.allocaInputIO.bits.predicate := "
                        "true.B\n"
                        "  {{ins_name}}.io.allocaInputIO.bits.valid     := "
                        "true.B\n"
                        "  {{ins_name}}.io.allocaInputIO.valid          := "
                        "true.B\n\n"
                        "  // Connecting Alloca to Stack\n";

                    // Connectin Alloca to StackPointer
                    //
                    // Getting Alloca index
                    uint32_t index = 0;
                    for (auto in : instruction_alloca) {
                        if (in == &ins) break;
                        index++;
                    }

                    command = command +
                              "  StackPointer.io.InData({{sp_index}}) <> "
                              "{{ins_name}}.io.allocaReqIO\n"
                              "  {{ins_name}}.io.allocaRespIO <> "
                              "StackPointer.io.OutData({{sp_index}})\n";

                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set("num_byte", static_cast<int>(num_byte));
                    ins_template.set("sp_index", static_cast<int>(index));

                } else if (alloca_type->isIntegerTy() || alloca_type->isPointerTy()) {
                    // Connecting AllocaIO input
                    auto num_byte = DL.getTypeAllocSize(alloca_type);
                    command =
                        "  {{ins_name}}.io.allocaInputIO.bits.size      := "
                        "1.U\n"
                        "  {{ins_name}}.io.allocaInputIO.bits.numByte   := "
                        "{{num_byte}}.U\n"
                        "  {{ins_name}}.io.allocaInputIO.bits.predicate := "
                        "true.B\n"
                        "  {{ins_name}}.io.allocaInputIO.bits.valid     := "
                        "true.B\n"
                        "  {{ins_name}}.io.allocaInputIO.valid          := "
                        "true.B\n\n"
                        "  // Connecting Alloca to Stack\n";

                    // Connectin Alloca to StackPointer
                    //
                    // Getting Alloca index
                    uint32_t index = 0;
                    for (auto in : instruction_alloca) {
                        if (in == &ins) break;
                        index++;
                    }

                    command = command +
                              "  StackPointer.io.InData({{sp_index}}) <> "
                              "{{ins_name}}.io.allocaReqIO\n"
                              "  {{ins_name}}.io.allocaRespIO <> "
                              "StackPointer.io.OutData({{sp_index}})\n";

                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set("num_byte", static_cast<int>(num_byte));
                    ins_template.set("sp_index", static_cast<int>(index));

                } else if (alloca_type->isStructTy())
                    assert(!"We don't support alloca for struct for now!");
                else {
                    out << ins;
                    printCode(out.str());
                    assert(!"Unknown Alloca type!");
                }
                printCode(comment + ins_template.render(command) + "\n");

            } else
                assert(!"Alloca can not have more than one operand");
        }

        else if (ins_type == TReturnInst) {
            // Check whether Ret instruction return result
            // dyn_cast<llvm::ConstantPointerNull>(ins.getOperand(c));
            //
            if (target_loop != nullptr) {
                auto Loc = target_loop->getStartLoc();
                if (target_loop->contains(
                        dyn_cast<Instruction>(loop_edge->first))) {
                    // LIVOUT
                    command =
                        "  {{ins_name}}.io.predicateIn(0).bits.control := "
                        "true.B\n"
                        "  {{ins_name}}.io.predicateIn(0).bits.taskID := 0.U\n"
                        "  {{ins_name}}.io.predicateIn(0).valid := true.B\n\n"
                        "  {{loop_name}}_LiveOut_{{loop_index}}.io.InData"
                        " <> "
                        "  {{operand_name}}.io.Out"
                        "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n"
                        "  {{ins_name}}.io.In.data(\"field0\") <> "
                        "{{loop_name}}_LiveOut_{{loop_index}}.io.Out"
                        "(0)\n"
                        "  io.out <> {{ins_name}}.io.Out\n";
                    //"{{operand_name}}.io.Out"
                    //"(param.{{ins_name}}_in(\"{{operand_name}}\"))\n"
                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set(
                        "operand_name",
                        instruction_info[dyn_cast<llvm::Instruction>(
                                             ins.getOperand(c))]
                            .name);
                    ins_template.set(
                        "loop_name",
                        "loop_L_" + std::to_string(loop_index[target_loop]));
                    ins_template.set(
                        "loop_index",
                        static_cast<int>(this->ins_loop_end_idx[&ins]));

                    printCode(comment + ins_template.render(command) + "\n");
                }
            } else if (dyn_cast<llvm::Instruction>(ins.getOperand(c))) {
                // First get the instruction
                comment = "  // Wiring return instruction\n";
                command = "";
                if (c == 0)
                    // Evil hack performed to drive enable to downstream.
                    command =
                        "  {{ins_name}}.io.predicateIn(0).bits.control := "
                        "true.B\n"
                        "  {{ins_name}}.io.predicateIn(0).bits.taskID := 0.U\n"
                        "  {{ins_name}}.io.predicateIn(0).valid := true.B\n"
                        "  {{ins_name}}.io.In.data(\"field0\") <> "
                        "{{operand_name}}.io.Out"
                        "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n"
                        "  io.out <> {{ins_name}}.io.Out\n";
                else
                    assert(
                        !"Return instruction cannot have more than one input");

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name",
                                 instruction_info[dyn_cast<llvm::Instruction>(
                                                      ins.getOperand(c))]
                                     .name);

                printCode(comment + ins_template.render(command) + "\n");
            } else if (operand_const || operand_constFloat || operand_null) {
                comment = "  // Wiring return instruction\n";
                command = "";
                command =
                    "  {{ins_name}}.io.predicateIn(0).bits.control := true.B\n"
                    "  {{ins_name}}.io.predicateIn(0).bits.taskID := 0.U\n"
                    "  {{ins_name}}.io.predicateIn(0).valid := true.B\n"
                    "  {{ins_name}}.io.In.data(\"field0\").bits.data := "
                    "{{value}}.U\n"
                    "  {{ins_name}}.io.In.data(\"field0\").bits.predicate := "
                    "true.B\n"
                    "  {{ins_name}}.io.In.data(\"field0\").valid := true.B\n"
                    "  io.out <> {{ins_name}}.io.Out\n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                if (operand_const)
                    ins_template.set(
                        "value",
                        static_cast<int>(operand_const->getSExtValue()));
                else if (operand_constFloat || operand_null)
                    ins_template.set("value", 0);
                printCode(comment + ins_template.render(command) + "\n");
            } else {
                command = "  io.out <> {{ins_name}}.io.Out\n";

                ins_template.set("ins_name", instruction_info[&ins].name);

                printCode(comment + ins_template.render(command) + "\n");
            }
        }

        else if (ins_type == TPtrToInt) {
            // TODO add tptrtoint
            // First get the instruction
            comment = "  // Wiring instructions\n";
            command = "";
            if (c == 0)
                command =
                    "  {{ins_name}}.io.ptr <> {{operand_name}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
            else
                command = "  {{ins_name}}.io.type <> insType\n";
            ins_template.set("ins_name", instruction_info[&ins].name);
            // ins_template.set(
            //"operand_name",
            // instruction_info[dyn_cast<llvm::Instruction>(ins.getOperand(c))]
            //.name);

            printCode(comment + ins_template.render(command) + "\n");
        } else if (ins_type == TBitCast) {
            errs() << "BitCast\n";

            // If the input is function argument
            if (tmp_find_arg != function_argument.end()) {
                // First get the instruction
                auto op_ins = ins.getOperand(c);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                comment = "";
                if (c == 0)
                    command =
                        "  {{ins_name}}.io.Input <> "
                        "InputSplitter.io.Out.data(\"{{operand_name}}\")\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name", argument_info[op_arg].name);
            } else {
                if (c == 0)
                    command =
                        "  {{ins_name}}.io.Input <> {{operand_name}}.io.Out"
                        "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name",
                                 instruction_info[dyn_cast<llvm::Instruction>(
                                                      ins.getOperand(c))]
                                     .name);
            }

            // printCode(comment + ins_template.render(command) + "\n");

            // printCode(comment + ins_template.render(command) + "\n");
        } else if (ins_type == TTrunc || ins_type == TFPTrunc) {
            assert(!"We don't support TTrunc instructions for now!");
        } else if (ins_type == TFpext) {
            // TODO add tptrtoint
            errs() << "FPext\n";
            if (c == 0)
                command =
                    "  {{ins_name}}.io.Input <> {{operand_name}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
            ins_template.set("ins_name", instruction_info[&ins].name);
            ins_template.set(
                "operand_name",
                instruction_info[dyn_cast<llvm::Instruction>(ins.getOperand(c))]
                    .name);

            printCode(comment + ins_template.render(command) + "\n");
        } else if (ins_type == TSelect) {
            errs() << "Select\n";

            // If the input is function argument
            if (tmp_find_arg != function_argument.end()) {
                // First get the instruction
                auto op_ins = ins.getOperand(c);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                comment = "";
                if (c == 1)
                    command =
                        "  {{ins_name}}.io.Input1 <> "
                        "InputSplitter.io.Out.data(\"{{operand_name}}\")\n";
                else if (c == 2)
                    command =
                        "  {{ins_name}}.io.Input2 <> "
                        "InputSplitter.io.Out.data(\"{{operand_name}}\")\n";

                ins_template.set("operand_name", argument_info[op_arg].name);
            }

            else if (operand_const || operand_constFloat) {
                comment = "  // Wiring constant\n";
                command = "";
                if (c == 1) {
                    command =
                        "  {{ins_name}}.io.Input1.bits.data := "
                        "{{value}}.U\n"
                        "  {{ins_name}}.io.Input1.bits.predicate := "
                        "true.B\n"
                        "  {{ins_name}}.io.Input1.valid := true.B\n";

                } else if (c == 2) {
                    command =
                        "  {{ins_name}}.io.Input2.bits.data := "
                        "{{value}}.U\n"
                        "  {{ins_name}}.io.Input2.bits.predicate := "
                        "true.B\n"
                        "  {{ins_name}}.io.Input2.valid := true.B\n";
                }
                ins_template.set("ins_name", instruction_info[&ins].name);
                if (operand_const)
                    ins_template.set(
                        "value",
                        static_cast<int>(operand_const->getSExtValue()));
                else if (operand_constFloat)
                    ins_template.set("value", 0);
            } else {
                if (c == 1)
                    command =
                        "  {{ins_name}}.io.Input1 <> {{operand_name}}.io.Out"
                        "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                else if (c == 2)
                    command =
                        "  {{ins_name}}.io.Input2 <> {{operand_name}}.io.Out"
                        "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";

                ins_template.set("operand_name",
                                 instruction_info[dyn_cast<llvm::Instruction>(
                                                      ins.getOperand(c))]
                                     .name);
            }

            ins_template.set("ins_name", instruction_info[&ins].name);

            printCode(comment + ins_template.render(command) + "\n");

        } else if (ins_type == TSEXT) {
            // First get the instruction
            auto op_ins = ins.getOperand(0);

            // If the input is from function argument
            auto tmp_fun_arg = dyn_cast<llvm::Argument>(op_ins);
            auto tmp_find_arg = find(function_argument.begin(),
                                     function_argument.end(), tmp_fun_arg);

            // If the input is function argument
            if (tmp_find_arg != function_argument.end()) {
                // First get the instruction
                auto op_ins = ins.getOperand(0);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                comment =
                    "  // Wiring SEXT instruction to the function "
                    "argument\n";
                command =
                    "  {{ins_name}}.io.Input <> "
                    "InputSplitter.io.Out.data(\"{{operand_name}}\")\n";
                // XXX uncomment if needed
                // command =
                //"  {{ins_name}}.io.Input <> {{operand_name}}.io.Out"
                //"(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name", argument_info[op_arg].name);
            }

            else {
                comment = "  // Wiring instructions\n";
                command =
                    "  {{ins_name}}.io.Input <> {{operand_name}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name",
                                 instruction_info[dyn_cast<llvm::Instruction>(
                                                      ins.getOperand(c))]
                                     .name);
            }

            printCode(comment + ins_template.render(command) + "\n");
        } else if (ins_type == TZEXT) {
            // First get the instruction
            auto op_ins = ins.getOperand(0);

            // If the input is from function argument
            auto tmp_fun_arg = dyn_cast<llvm::Argument>(op_ins);
            auto tmp_find_arg = find(function_argument.begin(),
                                     function_argument.end(), tmp_fun_arg);

            // If the input is function argument
            if (tmp_find_arg != function_argument.end()) {
                // First get the instruction
                auto op_ins = ins.getOperand(0);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                comment =
                    "  // Wiring ZEXT instruction to the function "
                    "argument\n";
                command =
                    "  {{ins_name}}.io.Input <> "
                    "InputSplitter.io.Out.data(\"{{operand_name}}\")\n";
                // XXX uncomment if needed
                // command =
                //"  {{ins_name}}.io.Input <> {{operand_name}}.io.Out"
                //"(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name", argument_info[op_arg].name);
            }

            else {
                comment = "  // Wiring instructions\n";
                command =
                    "  {{ins_name}}.io.Input <> {{operand_name}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name",
                                 instruction_info[dyn_cast<llvm::Instruction>(
                                                      ins.getOperand(c))]
                                     .name);
            }

            printCode(comment + ins_template.render(command) + "\n");
        } else if (ins_type == TCallInst) {
            if (c == 0) {
                // Wire up I/O interface
                comment = "  // Wiring Call to I/O\n";
                command =
                    "  io.{{ins_name}}_out <> "
                    "{{ins_name}}.io.callOut\n"
                    "  {{ins_name}}.io.retIn <> "
                    "io.{{ins_name}}_in\n"
                    "  {{ins_name}}.io.Out.enable.ready := true.B // Manual "
                    "fix";

                ins_template.set("ins_name", instruction_info[&ins].name);
                printCode(comment + ins_template.render(command));
                if (target_loop != nullptr) {
                    auto Loc = target_loop->getStartLoc();
                    auto call_ins = dyn_cast<llvm::CallInst>(&ins);
                    for (int arg = 0; arg < call_ins->getNumArgOperands();
                         arg++) {
                        comment =
                            "  // Wiring Call instruction to the loop header\n";
                        // int idx = 0;
                        auto op_ins = call_ins->getArgOperand(arg);
                        auto op_arg = dyn_cast<llvm::Argument>(op_ins);
                        auto op_name = argument_info[op_arg].name;
                        auto tmp_fun_arg = dyn_cast<llvm::Argument>(op_ins);
                        auto tmp_find_arg =
                            find(function_argument.begin(),
                                 function_argument.end(), tmp_fun_arg);
                        DEBUG(dbgs() << "op_name: " << op_name << "\n");
                        command =
                            "  "
                            "{{ins_name}}.io.In.data(\"field{{operand_num}}\") "
                            "<>"
                            "{{loop_name}}_liveIN_{{loop_index}}.io.Out"
                            "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";

                        ins_template.set("ins_name",
                                         instruction_info[&ins].name);
                        ins_template.set(
                            "loop_name",
                            "loop_L_" +
                                std::to_string(loop_index[target_loop]));
                        ins_template.set(
                            "loop_index",
                            static_cast<int>(this->ins_loop_header_idx[&ins]));
                        if (tmp_find_arg != function_argument.end()) {
                            ins_template.set("operand_name",
                                             argument_info[op_arg].name);
                        } else {
                            ins_template.set(
                                "operand_name",
                                instruction_info[dyn_cast<llvm::Instruction>(
                                                     op_ins)]
                                    .name);
                        }
                        ins_template.set("operand_num", std::to_string(arg));
                        printCode(comment + ins_template.render(command));
                    }
                    // If the operand is constant
                } else {
                    // Wire up operands
                    auto call_ins = dyn_cast<llvm::CallInst>(&ins);
                    for (int arg = 0; arg < call_ins->getNumArgOperands();
                         arg++) {
                        // If the input is from function argument
                        auto op_ins = call_ins->getArgOperand(arg);
                        auto tmp_fun_arg = dyn_cast<llvm::Argument>(op_ins);
                        auto tmp_find_arg =
                            find(function_argument.begin(),
                                 function_argument.end(), tmp_fun_arg);
                        // If the input is function argument
                        if (tmp_find_arg != function_argument.end()) {
                            // First get the instruction
                            auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                            comment =
                                "  // Wiring Call to the function argument\n";
                            command =
                                "  "
                                "{{ins_name}}.io.In.data(\"field{{operand_num}}"
                                "\") <> "
                                "InputSplitter.io.Out.data(\"{{operand_name}}"
                                "\")\n";

                            ins_template.set("ins_name",
                                             instruction_info[&ins].name);
                            ins_template.set("operand_name",
                                             argument_info[op_arg].name);
                            ins_template.set("operand_num",
                                             std::to_string(arg));
                        } else {
                            comment = "  // Wiring instructions\n";
                            command =
                                "  "
                                "{{ins_name}}.io.In.data(\"field{{operand_num}}"
                                "\") <> "
                                "{{operand_name}}.io.Out"
                                "(param.{{ins_name}}_in(\"{{operand_name}}\"))"
                                "\n";

                            ins_template.set("ins_name",
                                             instruction_info[&ins].name);
                            ins_template.set(
                                "operand_name",
                                instruction_info[dyn_cast<llvm::Instruction>(
                                                     op_ins)]
                                    .name);
                            ins_template.set("operand_num",
                                             std::to_string(arg));
                        }
                        printCode(comment + ins_template.render(command));
                    }
                }
                printCode("\n");
            }

#ifdef TAPIR
        } else if (ins_type == TDetach || ins_type == TReattach ||
                   ins_type == TSync) {
// TODO add Cilk support
#endif
        } else {
            DEBUG(ins.print(errs(), true));
            assert(!"The instruction is not supported in the dataflow connection phase");
        }
    }
}

void DataflowGeneratorPass::HelperPrintInstructionDF(Function &F) {
    string comment =
        "  /**\n"
        "    * Connecting Dataflow signals\n"
        "    */\n";
    printCode(comment);

    for (auto &BB : F) {
        for (auto &ins : BB) {
            // Get instruction type
            if (InstructionTypeNode(ins) == TUBranchInst)
                continue;
            else if (InstructionTypeNode(ins) == TPHINode)
                // We have already connected the PHI nodes
                continue;
            else if ((InstructionTypeNode(ins) == TReturnInst) &&
                     (ins.getNumOperands() == 0)) {
                // If your function is VOID
                LuaTemplater ins_template;
                string command =
                    "  {{ins_name}}.io.predicateIn(0).bits.control := true.B\n"
                    "  {{ins_name}}.io.predicateIn(0).bits.taskID := 0.U\n"
                    "  {{ins_name}}.io.predicateIn(0).valid := true.B\n"
                    "  {{ins_name}}.io.In.data(\"field0\").bits.data := 1.U\n"
                    "  {{ins_name}}.io.In.data(\"field0\").bits.predicate := "
                    "true.B\n"
                    "  {{ins_name}}.io.In.data(\"field0\").valid := true.B\n"
                    "  io.out <> {{ins_name}}.io.Out\n";
                ins_template.set("ins_name", instruction_info[&ins].name);

                printCode(comment + ins_template.render(command) + "\n");
            } else {
                DataflowGeneratorPass::PrintDataFlow(ins);
                DataflowGeneratorPass::NewPrintDataFlow(ins);
            }
        }
    }
}

/**
 * Printing BasicBlock enable signals
 */
void DataflowGeneratorPass::PrintBasicBlockEnableInstruction(Function &F) {
    auto loop_header_bb = getBBHeader(*LI);

    LuaTemplater ins_template;
    string comment =
        "  /**\n"
        "    * Wiring enable signals to the instructions\n"
        "    */\n";

    printCode(comment);

    string command = "";

    for (auto &BB : F) {
        for (auto &ins : BB) {
            // IF the instruction callsite type we need to forward enable signal
            // as an input to its interface
            // Otherwise, we fire the enable signal the insturction

            if (llvm::CallSite(&ins)) {
                command =
                    "  {{ins_name}}.io.In.enable <> {{bb_name}}.io.Out"
                    "(param.{{bb_name}}_activate(\"{{ins_name}}\"))\n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("bb_name", basic_block_info[&BB].name);
                printCode(ins_template.render(command));

                //} else if (InstructionTypeNode(ins) == TBitCast) {
                // Ignore bitcasts for now
                // continue;
            } else {
                command =
                    "  {{ins_name}}.io.enable <> {{bb_name}}.io.Out"
                    "(param.{{bb_name}}_activate(\"{{ins_name}}\"))\n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("bb_name", basic_block_info[&BB].name);
                printCode(ins_template.render(command));
            }
        }

        // if (loop_header_bb.count(&BB)) {
        if (loop_end_bb.count(&BB)) {
            // string ex_define =
            //"  {{bb_name}}_expand.io.enable <> "
            //"{{bb_name}}.io.Out({{last_index}})\n";
            // LuaTemplater ex_template;

            //// Get Instruction Type
            // ex_template.set("bb_name", basic_block_info[&BB].name);
            // ex_template.set("last_index",
            // static_cast<int>(BB.getInstList().size() +
            // this->loop_liveins[loop].size()));

            // printCode(ex_template.render(ex_define));

            auto &loop = loop_end_bb[&BB];
            // Connecting enable signal for loop headers
            uint32_t ll_index = 0;
            auto Loc = loop->getStartLoc();
            for (auto li : this->loop_liveins[loop]) {
                string ex_define =
                    "  {{loop_name}}_liveIN_{{en_index}}.io.enable <> "
                    "{{bb_name}}.io.Out({{con_index}})";
                LuaTemplater ex_template;

                ex_template.set("loop_name",
                                "loop_L_" + std::to_string(loop_index[loop]));
                ex_template.set("bb_name", basic_block_info[&BB].name);
                ex_template.set(
                    "con_index",
                    static_cast<int>(BB.getInstList().size() + ll_index));
                ex_template.set("en_index", static_cast<int>(ll_index));

                printCode(ex_template.render(ex_define));

                ll_index++;
            }

            printCode("");

            // Connecting enable signal for loop headers
            uint32_t lo_index = 0;
            for (auto li : this->loop_liveouts[loop]) {
                string ex_define =
                    "  {{loop_name}}_LiveOut_{{en_index}}.io.enable <> "
                    "{{bb_name}}.io.Out({{con_index}})";
                LuaTemplater ex_template;

                ex_template.set("loop_name",
                                "loop_L_" + std::to_string(loop_index[loop]));
                ex_template.set("bb_name", basic_block_info[&BB].name);
                ex_template.set("con_index",
                                static_cast<int>(BB.getInstList().size() +
                                                 ll_index + lo_index));
                ex_template.set("en_index", static_cast<int>(lo_index));

                printCode(ex_template.render(ex_define));

                lo_index++;
            }

            printCode("");
        }

        printCode("\n");
    }
}

/**
 * Helper function for printing serial loops
 */
void DataflowGeneratorPass::PrintLoopHeader(Function &F) {
    // Getting loop information
    // auto &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    if (getLoops(*LI).size() == 0)
        printCode("  //Function doesn't have any loop");

    else {
        // Printing header part of each loop
        LuaTemplater ins_template;

        auto cmp = [](Loop *left, Loop *right) {
            return left->getSubLoops().size() < right->getSubLoops().size();
        };
        std::priority_queue<Loop *, vector<Loop *>, decltype(cmp)> order_loops(
            cmp);
        for (auto &L : getLoops(*LI)) {
            order_loops.push(L);
        }

        std::map<Instruction *, Loop *> loop_instruction_map;
        while (!order_loops.empty()) {
            auto loop = order_loops.top();
            for (auto BB : loop->blocks()) {
                for (auto &I : *BB) {
                    loop_instruction_map[&I] = loop;
                }
            }
            order_loops.pop();
        }

        for (auto &L : getLoops(*LI)) {
            auto Loc = L->getStartLoc();
            // assert(Loc && "Loop information are empty!");
            //
            string Filename = string();
            if (Loc)
                Filename = getBaseName(Loc->getFilename().str());
            else
                Filename = "tmp_file_name";

            for (auto B : L->blocks()) {
                for (auto &I : *B) {
                    auto find_loop = loop_instruction_map.find(&I);

                    if (find_loop->second != L) continue;

                    // Detecting Live-ins
                    for (auto OI = I.op_begin(); OI != I.op_end(); OI++) {
                        Value *V = *OI;

                        if (definedInCaller(
                                SetVector<BasicBlock *>(L->blocks().begin(),
                                                        L->blocks().end()),
                                V)) {
                            this->loop_liveins[L].insert(V);

                            this->LoopEdges.insert(std::make_pair(V, &I));

                            if (loop_liveins_count[L].find(V) !=
                                loop_liveins_count[L].end())
                                loop_liveins_count[L][V]++;
                            else
                                loop_liveins_count[L][V] = 1;
                        }

                        // Detecting instructions
                        // if (Instruction *Ins = dyn_cast<Instruction>(V)) {
                        // if (!L->contains(Ins) &&
                        // definedInCaller(tmp_bb, V)) {
                        // this->loop_liveins[L].insert(V);
                        // this->LoopEdges.insert(std::make_pair(V, &I));
                        ////errs() << "Ins: \n";
                        ////Ins->dump();
                        //}
                        //}

                        //// Detecting function arguments
                        // else if (isa<Argument>(V)) {
                        // this->loop_liveins[L].insert(V);
                        // this->LoopEdges.insert(std::make_pair(V, &I));
                        ////errs() << "Arg: \n";
                        ////V->dump();
                        //}
                    }

                    // Detecting live-outs
                    // TODO: Remove live-outs
                    for (auto *U : I.users()) {
                        if (!definedInRegion(
                                SetVector<BasicBlock *>(L->blocks().begin(),
                                                        L->blocks().end()),
                                U)) {
                            this->loop_liveouts[L].insert(U);
                            this->LoopEdges.insert(std::make_pair(&I, U));

                            if (loop_liveouts_count[L].find(U) !=
                                loop_liveouts_count[L].end())
                                loop_liveouts_count[L][U]++;
                            else
                                loop_liveouts_count[L][U] = 1;
                        }
                    }
                }
            }

            // Dumping loop start node
            // TODO Fix number of outputs for loop head
            // TODO Do we need output anymore?
            if (this->loop_liveins[L].size()) {
                uint32_t index = 0;
                for (auto &livein : this->loop_liveins[L]) {
                    string loop_define =
                        "  val {{loop_name}}_liveIN_{{idx}}"
                        " = Module(new LiveInNode(NumOuts = {{num_outputs}}, "
                        "ID = 0))";

                    ins_template.set("loop_name",
                                     "loop_L_" + std::to_string(loop_index[L]));
                    //"loop_L_" + std::to_string(Loc.getLine()));

                    // TODO Change it to an array of outputs
                    ins_template.set("idx", static_cast<int>(index++));

                    // errs() << loop_liveins_count[L][livein] << "\n";

                    ins_template.set(
                        "num_outputs",
                        static_cast<int>(loop_liveins_count[L][livein]));

                    printCode(ins_template.render(loop_define));
                }
            }

            printCode("");

            if (this->loop_liveouts[L].size()) {
                // Dumping Loop end node
                uint32_t index = 0;
                for (auto &liveout : this->loop_liveouts[L]) {
                    string loop_define =
                        "  val {{loop_name}}_LiveOut_{{index}} = "
                        "Module(new LiveOutNode(NumOuts = {{num_outputs}}, "
                        "ID "
                        "= 0))";

                    ins_template.set("loop_name",
                                     "loop_L_" + std::to_string(loop_index[L]));
                    ins_template.set("index", static_cast<int>(index++));
                    ins_template.set(
                        "num_inputs",
                        static_cast<int>(this->loop_liveouts[L].size()));

                    // TODO Change it to an array of outputs
                    ins_template.set(
                        "num_outputs",
                        static_cast<int>(this->loop_liveouts[L].size()));

                    printCode(ins_template.render(loop_define));
                }
            }
            printCode("");
        }
    }
}

/**
 * This function connects each loop's live-ins to the
 * header register file
 */
void DataflowGeneratorPass::PrintLoopRegister(Function &F) {
    // Getting loop information
    //
    if (getLoops(*LI).size() == 0) {
        errs() << "No LOOP!\n";
        printCode("  //Function doesn't have any for loop");
    }

    else {
        // Printing header part of each loop
        LuaTemplater ins_template;

        for (auto &L : getLoops(*LI)) {
            auto Loc = L->getStartLoc();
            // auto Filename = getBaseName(Loc->getFilename().str());

            auto loop_live_in = this->loop_liveins.find(L);
            auto loop_live_out = this->loop_liveouts.find(L);

            // We iterate over live-in of each loop and then find the
            // source and destination of the values
            //
            // The first element is the SRC and it should get connected to
            // the loop header register file and the loop register file
            // should get connected to the DEST instruction
            //
            // NOTE: Second value is always instructions while the first
            // value can be either instruction or function argument
            if (loop_live_in != this->loop_liveins.end()) {
                uint32_t live_index = 0;

                set<Value *> input_set_liveIn;

                for (auto p : loop_live_in->second) {
                    for (auto search_elem : LoopEdges) {
                        if (p == search_elem.first) {
                            auto operand = search_elem.first;

                            if (input_set_liveIn.count(operand) == 0)
                                input_set_liveIn.insert(operand);
                            else
                                continue;

                            auto target = search_elem.second;
                            auto target_ins =
                                dyn_cast<llvm::Instruction>(target);

                            auto target_phi = dyn_cast<llvm::PHINode>(target);

                            // Saving the index of loop header of corresponding
                            // instruction
                            if (target_ins)
                                this->ins_loop_header_idx[target] = live_index;

                            auto operand_ins =
                                dyn_cast<llvm::Instruction>(operand);

                            // Check if the input is function argument
                            auto operand_arg =
                                dyn_cast<llvm::Argument>(operand);

                            // If SRC is function argument
                            // connect SRC to the loop header
                            if (operand_arg) {
                                string comment =
                                    "  // Connecting function argument to "
                                    "the loop header\n";

                                string command =
                                    "  "
                                    "{{loop_name}}_liveIN_{{arg_index}}.io."
                                    "InData"
                                    " <> "
                                    "InputSplitter.io.Out.data(\"{{operand_"
                                    "name}}\")\n";

                                ins_template.set(
                                    "loop_name",
                                    "loop_L_" + std::to_string(loop_index[L]));
                                ins_template.set("arg_index",
                                                 static_cast<int>(live_index));

                                if (argument_info.find(operand_arg) ==
                                    argument_info.end()) {
                                    assert(
                                        !"Funcion argument can't be find!\n");
                                }

                                ins_template.set(
                                    "operand_name",
                                    argument_info[operand_arg].name);

                                // Printing each instruction
                                string init_test = "  //";
                                raw_string_ostream out(init_test);
                                out << *search_elem.first << "\n";

                                // printCode(out.str());
                                printCode(comment + out.str() +
                                          ins_template.render(command));

                            } else if (operand_ins) {
                                // If SRC is an instruction
                                string comment =
                                    "  // Connecting instruction to the "
                                    "loop header\n";

                                string command =
                                    "  "
                                    "{{loop_name}}_liveIN_{{arg_index}}.io."
                                    "InData"
                                    " <> "
                                    "{{instruction_name}}.io.Out"
                                    "(param.{{operand_name}}_in(\"{{"
                                    "instruction_name}}\"))\n";

                                ins_template.set(
                                    "loop_name",
                                    "loop_L_" + std::to_string(loop_index[L]));
                                ins_template.set("arg_index",
                                                 static_cast<int>(live_index));

                                ins_template.set(
                                    "instruction_name",
                                    instruction_info[operand_ins].name);

                                ins_template.set(
                                    "operand_name",
                                    instruction_info[target_ins].name);

                                // Printing each instruction
                                string init_test = "  //";
                                raw_string_ostream out(init_test);
                                out << *search_elem.first << "\n";

                                // printCode(out.str());
                                printCode(comment + out.str() +
                                          ins_template.render(command));
                            }

                            live_index++;
                        }
                    }
                }
            }

            // if (loop_live_out != this->loop_liveouts.end()) {
            // uint32_t live_index = 0;
            // for (auto p : loop_live_out->second) {
            // for (auto search_elem : LoopEdges) {
            // if (p == search_elem.second) {
            // auto target = search_elem.second;
            // auto target_ins =
            // dyn_cast<llvm::Instruction>(target);

            //// Saving the index of loop header of corresponding
            //// instruction
            // if (target_ins)
            // this->ins_loop_end_idx[target] = live_index;
            //// TODO Fix Me: Currently hard wired to io.Out(0).
            //// Shouldn't it have its own output?
            // string live_out_conn =
            //"  "
            //"//{{loop_name}}_LiveOut_{{lo_index}}.io.InData "
            //"<> "
            //"{{ins_name}}.io.Out(0)\n";
            ////"{{ins_name}}.io.Out(param.m_10_in(\"{{ins_"
            ////"name}}\"))\n";
            ////"{{ins_name}}.io.Out(param.m_10_in(\"{{ins_"

            // ins_template.set(
            //"loop_name",
            //"loop_L_" + std::to_string(Loc.getLine()));

            // ins_template.set("lo_index",
            // static_cast<int>(live_index));

            // ins_template.set(
            //"ins_name",
            // instruction_info[dyn_cast<llvm::Instruction>(
            // search_elem.first)]
            //.name);
            // ins_template.set("out_index",
            // static_cast<int>(live_index));

            // printCode(ins_template.render(live_out_conn));

            // live_index++;
            //}
            //}
            //}
            //}
        }
    }
}

/**
 * Generating a template scala file which can be used
 * for writing test cases
 */
void DataflowGeneratorPass::generateTestFunction(llvm::Function &F) {
    generateImportSection(this->outTest);

    // Printing Tests class
    LuaTemplater ins_template;
    string final_command;
    string command =
        "class {{class_name}}CacheWrapper()(implicit p: Parameters) extends "
        "{{module_name}}()(p)\n"
        "  with CacheParams {\n\n"
        "  // Instantiate the AXI Cache\n"
        "  val cache = Module(new Cache)\n"
        "  cache.io.cpu.req <> CacheMem.io.CacheReq\n"
        "  CacheMem.io.CacheResp <> cache.io.cpu.resp\n"
        "  cache.io.cpu.abort := false.B\n"
        "  // Instantiate a memory model with AXI slave interface for cache\n"
        "  val memModel = Module(new NastiMemSlave)\n"
        "  memModel.io.nasti <> cache.io.nasti\n\n}\n\n"
        "class {{class_name}}Test01"
        "(c: {{class_name}}CacheWrapper) extends PeekPokeTester(c) {\n";
    //"(c: {{module_name}}) extends PeekPokeTester(c) {\n";
    ins_template.set("class_name", F.getName().str());
    ins_template.set("module_name", F.getName().str() + "DF");
    final_command.append(ins_template.render(command));

    printCode(ins_template.render(final_command), this->outTest);

    // Printing comments and the moduel information
    command = "\n  /**\n  *  {{module_name}}DF interface:\n  *\n";

    ins_template.set("module_name", F.getName().str());

    final_command = ins_template.render(command);

    uint32_t c = 0;
    string init_command = "  poke(c.io.in.valid, false.B)\n\n";
    "  poke(c.io.in.bits.enable.control, false.B)\n";

    command = "  *    in = Flipped(Decoupled(new Call(List(...))))\n";
    final_command.append(ins_template.render(command));
    for (auto &ag : F.args()) {
        command =
            "  poke(c.io.in.bits.data(\"field{{index}}\").data, 0.U)\n"
            "  poke(c.io.in.bits.data(\"field{{index}}\").predicate, "
            "false.B)\n";
        ins_template.set("index", static_cast<int>(c++));
        init_command.append(ins_template.render(command));
    }

    c = 0;
    for (auto &gl : F.getParent()->getGlobalList()) {
        for (User *U : gl.users()) {
            if (Instruction *Inst = dyn_cast<Instruction>(U)) {
                if (Inst->getFunction() == &F) {
                    command =
                        "  *    glob_{{index}} = Flipped(Decoupled(new "
                        "DataBundle))\n";
                    ins_template.set("index", static_cast<int>(c++));
                    final_command.append(ins_template.render(command));

                    command =
                        "  poke(c.io.glob_{{index}}.bits.data, 0.U)\n"
                        "  poke(c.io.glob_{{index}}.bits.predicate, false.B)\n"
                        "  poke(c.io.glob_{{index}}.valid, false.B)\n\n";

                    init_command.append(ins_template.render(command));
                    break;
                }
            }
        }
    }

    command = "  poke(c.io.out.ready, false.B)\n\n";
    init_command.append(command);

    if (!F.getReturnType()->isVoidTy()) {
        final_command.append("  *    out = Decoupled(new Call(List(32)))\n");
    }

    final_command.append("  */\n\n\n");

    command = "  // Initializing the signals\n\n";
    final_command.append(command);
    final_command.append(init_command);

    command =
        "/**\n   *\n"
        "   * @todo Add your test cases here\n   *\n"
        "   * The test harness API allows 4 interactions with the DUT:\n"
        "   *  1. To set the DUT'S inputs: poke\n"
        "   *  2. To look at the DUT'S outputs: peek\n"
        "   *  3. To test one of the DUT's outputs: expect\n"
        "   *  4. To advance the clock of the DUT: step\n"
        "   *\n"
        "   * Conditions:\n"
        "   *  1. while(peek(c.io.XXX) == UInt(0))\n"
        "   *  2. for(i <- 1 to 10)\n"
        "   *  3. for{ i <- 1 to 10\n"
        "   *          j <- 1 to 10\n"
        "   *        }\n"
        "   *\n"
        "   * Print Statement:\n"
        "   *    println(s\"Waited $count cycles on gcd inputs $i, $j, "
        "giving "
        "up\")\n"
        "   *\n"
        "   */\n\n"
        "  // Example to just increment clock 150 times\n"
        "  step(1)\n"
        "  var time = 1  //Cycle counter\n"
        "  while (time < 150) {\n"
        "   time += 1\n"
        "   step(1)\n"
        "   println(s\"Cycle: $time\")\n"
        "  }\n\n";

    printCode(final_command + command + "}\n", this->outTest);

    // Printing Tester class
    //
    command =
        "class {{class_name}}Tester extends FlatSpec with Matchers {\n"
        "  implicit val p = config.Parameters.root((new "
        "MiniConfig).toInstance)\n"
        "  it should \"Check that {{class_name}} works correctly.\" in {\n"
        "    // iotester flags:\n"
        "    // -ll  = log level <Error|Warn|Info|Debug|Trace>\n"
        "    // -tbn = backend <firrtl|verilator|vcs>\n"
        "    // -td  = target directory\n"
        "    // -tts = seed for RNG\n"
        "    chisel3.iotesters.Driver.execute(\n"
        "     Array(\n"
        "       // \"-ll\", \"Info\",\n"
        "       \"-tbn\", \"verilator\",\n"
        "       \"-td\", \"test_run_dir\",\n"
        "       \"-tts\", \"0001\"),\n"
        "     () => new {{class_name}}CacheWrapper()) {\n"
        "     c => new {{class_name}}Test01(c)\n"
        "    } should be(true)\n"
        "  }\n}\n";

    printCode(ins_template.render(command), this->outTest);
}

void DataflowGeneratorPass::printEndingModule(llvm::Function &F) {
    // Printing Tests class
    LuaTemplater ins_template;
    string command =
        "import java.io.{File, FileWriter}\n"
        "object {{class_name}}Main extends App {\n"
        "  val dir = new File(\"RTL/{{class_name}}\") ; dir.mkdirs\n"
        "  implicit val p = config.Parameters.root((new "
        "MiniConfig).toInstance)\n"
        "  val chirrtl = firrtl.Parser.parse(chisel3.Driver.emit(() => new "
        "{{module_name}}()))\n\n"
        "  val verilogFile = new File(dir, s\"${chirrtl.main}.v\")\n"
        "  val verilogWriter = new FileWriter(verilogFile)\n"
        "  val compileResult = (new "
        "firrtl.VerilogCompiler).compileAndEmit(firrtl.CircuitState(chirrtl, "
        "firrtl.ChirrtlForm))\n"
        "  val compiledStuff = compileResult.getEmittedCircuit\n"
        "  verilogWriter.write(compiledStuff.value)\n"
        "  verilogWriter.close()\n}\n";
    ins_template.set("class_name", F.getName().str());
    ins_template.set("module_name", F.getName().str() + "DF");
    // final_command.append(ins_template.render(command));

    printCode(ins_template.render(command));
    // printCode(ins_template.render(command), this->outTest);
}

/**
 * This function is the main core of the code generator
 * generateFunction function starts dumping scala code for
 * a specific function step by step.
 * At each step specific part of the code will be generated
 * and each function internally call printCode function to
 * dump the e
 * \param  Target function
 *
 */
void DataflowGeneratorPass::generateFunction(llvm::Function &F) {
    // Step 0: Preprocessing step.
    // This step fills the containers
    // Assigning name to each basic block
    NamingBasicBlock(F);
    // Assigning name to each instruction
    NamingInstruction(F);

    // Fill the instruction containers
    FillInstructionContainers(F);

    // Filling function arguments
    FillFunctionArg(F);

    // Filling glboal variables
    FillGlobalVar(*F.getParent());

    // Filling loop header basicblocks
    FillLoopHeader(*LI);

    // Step1: Dump import section of the scala file
    generateImportSection(this->outCode);

    // Step2: Printing helper param object
    //
    // This object contains mapping from each instructions to their
    // consequent
    // instructions and basic blocks
    // Since we have to use indexes to to connect differet modules to getter
    // we use this mapping so that instead of using pure indexes we can use
    // a mapping from the name's to their index
    PrintHelperObject(F);

    // Step3:
    // Printing Datflow abstract IO class
    printHeader("Printing Ports Definition");
    PrintDatFlowAbstractIO(F);

    // Step4:
    // Printing Memory system and Stackfile
    printHeader("Printing Memory System");
    PrintStackPointer();
    PrintRegisterFile();
    PrintCacheMem();
    PrintInputSplitter(F);

    // Step5:
    // Printing Loop headers
    printHeader("Printing Loop Headers");
    PrintLoopHeader(F);

    // Step6:
    // Printing BasicBlock definitions
    printHeader("Printing BasicBlock nodes");
    HelperPrintBBInit(F);

    // Step6:
    // Printing Instruction initialization
    printHeader("Printing Instruction nodes");
    HelperPrintInstInit(F);

    // Step 7:
    // Initilizing the parameters object
    // TODO: Helper object doesn't help us, we should remove it
    printHeader("Initializing Param");
    PrintParamObject();

    // Step 8:
    // Connecting enable signal of BasicBlocks
    printHeader("Connecting Basic Blocks to Predicate Instructions");
    HelperPrintBasicBlockPredicate();

    printHeader("Connecting Basic Blocks to instructions");
    PrintBasicBlockEnableInstruction(F);

    // Connecting Instructions in dataflow order
    printHeader("Connecting LoopHeaders");
    PrintLoopRegister(F);

    // Connecting BasicBlock masks to their Phi nodes
    printHeader("Dumping PHI nodes");
    HelperPrintBasicBlockPhi();

    printHeader("Dumping Dataflow");
    HelperPrintInstructionDF(F);

    // Closing the object
    printCode("}\n");

    printEndingModule(F);
}
