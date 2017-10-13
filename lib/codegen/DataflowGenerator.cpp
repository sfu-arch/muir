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
}

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
    uint32_t count = 0;
    for (auto ins_it = Ins.use_begin(), e_it = Ins.use_end(); ins_it != e_it;
         ins_it++)
        count++;
    return count;
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

    // Default case
    // TODO: Other type of instructions are note supported for now!
    ins.dump();

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
            CallSite CS(&Ins);
            // TODO add custom module
            if (CS.isCall()) continue;

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
            else if (ins_type == TAlloca)
                instruction_alloca.push_back(&Ins);
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
    for (auto &f_arg : F.getArgumentList()) {
        function_argument.push_back(&f_arg);
        ArgInfo temp_arg = {"data_" + to_string(c), static_cast<uint32_t>(c)};
        argument_info[&f_arg] = temp_arg;
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
        string tmp_name = BB.getName().str();

        // If the basic block doesn't have name we add "unkonw" prefix
        if (tmp_name.empty()) tmp_name = "unkonw." + to_string(c++);

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
            // TODO Add support for function calls
            llvm::CallSite CS(&INS);
            if (CS) continue;
            instruction_info[&INS] = {"m_" + to_string(this->count_ins++),
                                      counter++};
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
        "import chisel3.iotesters.{ChiselFlatSpec, Driver, "
        "OrderedDecoupledHWIOTester, PeekPokeTester}\n"
        "import org.scalatest.{FlatSpec, Matchers}\n"
        "import muxes._\n"
        "import config._\n"
        "import control.{BasicBlockNoMaskNode, BasicBlockNode}\n"
        "import util._\n"
        "import interfaces._\n"
        "import regfile._\n"
        "import memory._\n"
        "import stack._\n"
        "import arbiters._\n"
        "import loop._\n"
        "import node._\n\n";

    // Print to the OUTPUT
    printCode(command, out);
}

/**
 * This function dumps helper object which maps all
 * the instrucitons, basic block and arguments to their indexes
 */
void DataflowGeneratorPass::PrintHelperObject(llvm::Function &F) {
    LuaTemplater ins_template;

    string comment =
        "/**\n"
        "  * This Object should be initialize at the first step\n"
        "  * It contains all the transformation from indecies to their "
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

    // Printing entry basic block maping
    printCode(ins_template.render(command));

    string final_command;
    for (auto bb_to_branch : bb_branch) {
        final_command = "";
        command = "  val {{bb_name}}_pred = Map(\n";
        ins_template.set("bb_name", basic_block_info[bb_to_branch.first].name);
        final_command.append(ins_template.render(command));
        uint32_t c = 0;
        for (auto ins_bb : bb_to_branch.second) {
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

    for (auto &bb : F) {
        final_command.clear();
        command = "  val {{bb_name}}_activate = Map(\n";
        ins_template.set("bb_name", basic_block_info[&bb].name);
        final_command.append(ins_template.render(command));

        uint32_t c = 0;
        for (auto &ins : bb) {
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

        auto phi_ins = dyn_cast<llvm::PHINode>(ins);
        for (uint32_t i = 0; i < phi_ins->getNumOperands(); i++) {
            if (!phi_ins) {
                command = "    \"{{const_name}}\" -> {{index}},\n";
                ins_template.set("ins_name", "const_" + to_string(i));
            } else {
                command = "    \"{{ins_name}}\" -> {{index}},\n";
                ins_template.set("ins_name",
                                 instruction_info[dyn_cast<llvm::Instruction>(
                                                      phi_ins->getOperand(i))]
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
            llvm::CallSite CS(&ins);
            auto br_ins = dyn_cast<llvm::BranchInst>(&ins);

            if (CS)
                continue;
            else if (br_ins && br_ins->getNumOperands() == 1)
                continue;
            else {
                final_command.clear();

                // Printing each instruction
                string init_test = "  //";
                raw_string_ostream out(init_test);
                out << ins;

                printCode(out.str());
                command = "  val {{ins_name}}_in = Map( \n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                final_command.append(ins_template.render(command));

                for (uint32_t c = 0; c < ins.getNumOperands(); c++) {
                    if (dyn_cast<llvm::ConstantInt>(ins.getOperand(c)))
                        continue;

                    else if (br_ins) {
                        if (br_ins->getNumOperands() == 1 || c >= 1) continue;
                    }
                    // else if (dyn_cast<llvm::BranchInst>(ins.getOperand(c)))
                    // continue;
                    else if (dyn_cast<llvm::Argument>(ins.getOperand(c))) {
                        command = "    \"{{ins_name}}\" -> {{index}},\n";

                        ptrdiff_t pos = distance(
                            function_argument.begin(),
                            find(function_argument.begin(),
                                 function_argument.end(),
                                 dyn_cast<llvm::Argument>(ins.getOperand(c))));

                        // ArgInfo temp_arg = {"data_" + to_string(pos),
                        // static_cast<uint32_t>(pos)};
                        // argument_info[dyn_cast<llvm::Argument>(
                        // ins.getOperand(c))] = temp_arg;

                        string arg_name =
                            "data_" + to_string(static_cast<int>(pos));
                        ins_template.set("ins_name", arg_name);
                        ins_template.set("index", static_cast<int>(c));
                        final_command.append(ins_template.render(command));
                    } else {
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

    // for (uint32_t i = 0; i < F.getNumOperands(); i++) {
    // for (uint32_t i = 0; i < ; i++) {
    uint32_t c = 0;
    for (auto &ag : F.getArgumentList()) {
        command =
            "    val data_{{index}} = Flipped(Decoupled(new DataBundle))\n";
        ins_template.set("index", static_cast<int>(c++));
        final_command.append(ins_template.render(command));
    }

    final_command.append(
        "    val pred = Decoupled(new Bool())\n"
        "    val start = Input(new Bool())\n");

    if (!F.getReturnType()->isVoidTy()) {
        final_command.append("    val result = Decoupled(new DataBundle)\n");
    }

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

    // outs() << ins_template.render(final_command);
}

/**
 * Helper funciton for printing BasicBlocks
 */
void DataflowGeneratorPass::HelperPrintBBInit(Function &F) {
    string comment = "  //Initializing BasicBlocks: \n";
    printCode(comment);
    for (auto &BB : F) {
        // Initial step is to naming BasicBlocks and the Instructions
        PrintBasicBlockInit(BB);
    }
    printCode("\n\n");
}

/**
 * Helper funciton for printing BasicBlocks
 */
void DataflowGeneratorPass::HelperPrintInistInit(Function &F) {
    string comment = "  //Initializing Instructions: \n";
    printCode(comment);

    for (auto &BB : F) {
        comment = "  // [BasicBlock]" + BB.getName().str() + ":";
        printCode(comment);
        for (auto &INS : BB) {
            // Check wether the instruction is function call
            // TODO: Implement function call
            // TODO: Inline the function call
            llvm::CallSite CS(&INS);
            if (CS) {
                // Printing each instruction
                string init_test = "\n  //";
                raw_string_ostream out(init_test);
                out << INS;
                printCode("\n  // Function Call: " +
                          CS->getFunction()->getName().str() + out.str() +
                          "\n");
                continue;
            }

            PrintInstInit(INS);
        }
        printCode("\n");
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
        "(sign=false)(p))";
    LuaTemplater ins_template;

    // Get Instruction Type
    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("ins_type",
                     InstructionInfo::instruction_name_type[ins_type]);

    // TODO fix instructions with 0 output
    if (countInsUse(Ins) == 0)
        ins_template.set("num_out", static_cast<int>(1));
    else
        ins_template.set("num_out", static_cast<int>(countInsUse(Ins)));

    // Getting op code
    string cmp_ins_op = "";
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
                cmp_ins_op = "SGE";
                break;
            case CmpInst::Predicate::ICMP_SGT:
                cmp_ins_op = "SGT";
                break;
            case CmpInst::Predicate::ICMP_SLE:
                cmp_ins_op = "SLE";
                break;
            case CmpInst::Predicate::ICMP_SLT:
                cmp_ins_op = "SLT";
                break;
            case CmpInst::Predicate::ICMP_UGE:
                cmp_ins_op = "UGE";
                break;
            case CmpInst::Predicate::ICMP_ULE:
                cmp_ins_op = "ULE";
                break;
            case CmpInst::Predicate::ICMP_ULT:
                cmp_ins_op = "ULT";
                break;
            default:
                // Ins.dump();
                // Printing each instruction
                string init_test = "\n  //";
                raw_string_ostream out(init_test);
                out << Ins;
                printCode(out.str() + "\n");
                assert(!"Unkonw CMP operand");
                break;
        }

        ins_template.set("op_code", cmp_ins_op);
    } else
        ins_template.set("op_code", Ins.getOpcodeName());

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
        "(ID = {{ins_id}})(p))";
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
        ", ID = {{ins_id}})(p))";
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
            "(p))";
        ins_template.set("ins_name", instruction_info[&Ins].name);
        ins_template.set("ins_out", static_cast<int>(countInsUse(Ins)));
        ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));
        ins_template.set(
            "num_byte",
            static_cast<int>(gep_pass_ctx.SingleGepIns[&Ins].numByte));

    } else if (Ins.getNumOperands() == 3) {
        ins_define =
            "  val {{ins_name}} = "
            "Module (new GepTwoNode"
            "(NumOuts = {{ins_out}}, "
            "ID = {{ins_id}})"
            "(numByte1 = {{num_byte1}}, "
            "numByte2 = {{num_byte2}})"
            "(p))";
        ins_template.set("ins_name", instruction_info[&Ins].name);
        ins_template.set("ins_out", static_cast<int>(countInsUse(Ins)));
        ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));
        ins_template.set(
            "num_byte1",
            static_cast<int>(gep_pass_ctx.TwoGepIns[&Ins].numByte1));
        ins_template.set(
            "num_byte2",
            static_cast<int>(gep_pass_ctx.TwoGepIns[&Ins].numByte2));

    } else {
        Ins.dump();

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

void DataflowGeneratorPass::PrintRetIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    auto ins_cast = dyn_cast<llvm::CastInst>(&Ins);
    auto DL = Ins.getModule()->getDataLayout();

    LuaTemplater ins_template;
    string ins_define =
        "  val {{ins_name}} = "
        "Module(new RetNode(NumOuts={{num_out}}, ID={{ins_id}}))";

    ins_template.set("ins_name", instruction_info[&Ins].name);
    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));
    if (Ins.getNumUses() == 0)
        ins_template.set("num_out", static_cast<int>(Ins.getNumUses() + 1));

    if (Ins.getNumUses() == 0)
        // In cases which there is no consumer for an instruction we
        // hardwire the number of output to 1
        ins_template.set("num_out", static_cast<int>(1));
    else
        ins_template.set("num_out", static_cast<int>(Ins.getNumUses()));

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

void DataflowGeneratorPass::PrintInstInit(Instruction &Ins) {
    // Check if the instruction type is call site return
    CallSite CS(&Ins);
    if (CS.isCall()) return;
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    // TODO make it switch
    //
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
    } else if (ins_type == TReturnInst) {
        PrintRetIns(Ins);
    } else {
        string ins_define =
            "  //val {{ins_name}} = "
            "Module (new {{ins_type}}"
            "(ID = {{ins_id}})(p))";
        LuaTemplater ins_template;

        // Get Instruction Type

        ins_template.set("ins_name", instruction_info[&Ins].name);
        ins_template.set("ins_type",
                         InstructionInfo::instruction_name_type[ins_type]);
        ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));

        string result = ins_template.render(ins_define);

        // outs() << "\n  //";
        // Ins.print(outs());
        // outs() << "\n" << result << "\n";

        // Printing each instruction
        string init_test = "\n  //";
        raw_string_ostream out(init_test);
        out << Ins;
        printCode(out.str() + "\n" + result + "\n");
    }
}

/**
 * Priniting Basic Blcok definition for each basic block
 */
void DataflowGeneratorPass::PrintBasicBlockInit(BasicBlock &BB) {
    uint32_t phi_c = CountPhiNode(BB);
    string bb_define;
    string result;
    if (phi_c == 0) {
        LuaTemplater bb_template;
        bb_define =
            "  val {{bb_name}} = "
            "Module(new BasicBlockNoMaskNode"
            "(NumInputs = {{num_target}}, NumOuts = {{num_ins}}, "
            "BID = {{bb_id}})(p))";

        bb_template.set("bb_name", basic_block_info[&BB].name);

        if (countPred(BB) == 0)
            bb_template.set("num_target", static_cast<int>(1));
        else
            bb_template.set("num_target", static_cast<int>(countPred(BB)));

        bb_template.set("num_ins", static_cast<int>(BB.getInstList().size()));
        bb_template.set("bb_id", static_cast<int>(basic_block_info[&BB].id));

        result = bb_template.render(bb_define);

    } else {
        bb_define =
            "  val {{bb_name}} = "
            "Module(new BasicBlockNode"
            "(NumInputs = {{num_target}}, NumOuts = {{num_ins}}, NumPhi = "
            "{{phi_num}}, "
            "BID = {{bb_id}})(p))";
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
        "\t\t            (RWArbiter=new ReadWriteArbiter()))";

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
        "     */\n\n"
        "  //We always ground entry BasicBlock\n";
    printCode(comment);

    LuaTemplater bb_template;

    string ground_entry =
        "  {{bb_name}}.io.predicateIn(param.{{bb_name}}_pred(\"active\")).bits "
        " := "
        "ControlBundle.Activate\n"
        "  "
        "{{bb_name}}.io.predicateIn(param.{{bb_name}}_pred(\"active\")).valid "
        ":= "
        "true.B\n\n";
    auto find_bb = basic_block_info.find(entry_bb);
    if (find_bb == basic_block_info.end()) assert(!"NOT FOUND");

    bb_template.set("bb_name", basic_block_info[this->entry_bb].name);
    string tmp_result = bb_template.render(ground_entry);

    printCode(tmp_result);

    comment =
        "  /**\n"
        "    * Connecting basic blocks to predicate instructions\n"
        "    */\n";
    printCode(comment);

    // Iterate over branch instruction and connect them their BasicBlock
    if (instruction_branch.size() == 0)
        printCode("\n  // There is no branch insruction\n\n");
    else {
        std::for_each(instruction_branch.begin(), instruction_branch.end(),
                      [this](Instruction *ins) {
                          DataflowGeneratorPass::PrintBranchBasicBlockCon(*ins);
                      });
    }
}

/**
 * Connecting connections between Branches and target BasicBlocks
 * @param ins input branch instruction
 */
void DataflowGeneratorPass::PrintBranchBasicBlockCon(Instruction &ins) {
    auto branch_ins = dyn_cast<llvm::BranchInst>(&ins);

    LuaTemplater ins_template;

    for (uint32_t i = 0; i < branch_ins->getNumSuccessors(); i++) {
        string comment = "  //Connecting {{ins_name}} to {{basic_block}}";
        string command =
            "  "
            "{{basic_block}}.io.predicateIn(param.{{basic_block}}_pred(\"{{ins_"
            "name}}\"))"
            " <> "
            "{{ins_name}}.io.Out(param.{{ins_name}}_brn_bb(\"{{basic_block}}\")"
            ")\n\n";

        ins_template.set("ins_name", instruction_info[&ins].name);
        ins_template.set("basic_block",
                         basic_block_info[branch_ins->getSuccessor(i)].name);

        string result = ins_template.render(comment);
        printCode(result);

        result = ins_template.render(command);
        printCode(result);
    }
}

void DataflowGeneratorPass::HelperPrintBasicBlockPhi() {
    string comment =
        "  /**\n"
        "    * Connecting PHI Masks\n"
        "    */\n"
        "  //Connect PHI node\n";
    printCode(comment);

    if (instruction_phi.size() == 0) outs() << "  //There is no PHI node\n";
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
        std::for_each(instruction_phi.begin(), instruction_phi.end(),
                      [this](Instruction *ins) {
                          DataflowGeneratorPass::PrintPHIMask(*ins);
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

    // TODO check if the operand is constant, then hand right the signals VERY
    // IMPORTANT
    //
    for (uint32_t c = 0; c < phi_ins->getNumOperands(); c++) {
        // Getting target
        auto ins_target = dyn_cast<llvm::Instruction>(phi_ins->getOperand(c));
        if (ins_target) {
            string command =
                "  {{phi_name}}.io.InData(param.{{phi_name}}_phi_in"
                "(\"{{ins_name}}\")) <> {{ins_name}}.io.Out(0)\n";
            ins_template.set("phi_name", instruction_info[&ins].name);
            ins_template.set("ins_name", instruction_info[ins_target].name);

            string result = ins_template.render(command);
            printCode(result);

        } else {
            string command =
                "  //@todo {{phi_name}}.io.InData(param.{{phi_name}}_phi_in"
                "(\"{{ins_name}}\")) <> {{ins_name}}.io.Out(0)\n";
            ins_template.set("phi_name", instruction_info[&ins].name);
            // ins_template.set("ins_name", instruction_info[ins_target].name);

            string result = ins_template.render(command);
            printCode(result);

            // ins.dump();
            // assert(!"Cannot support constant for the PHIs for now");
        }
    }
}

void DataflowGeneratorPass::PrintPHIMask(llvm::Instruction &ins) {
    auto phi_ins = dyn_cast<llvm::PHINode>(&ins);
    LuaTemplater ins_template;

    string command = "  {{phi_name}}.io.Mask <> {{ins_name}}.io.MaskBB(0)\n";
    ins_template.set("phi_name", instruction_info[&ins].name);
    ins_template.set("ins_name", basic_block_info[ins.getParent()].name);

    string result = ins_template.render(command);
    printCode(result);
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

        // Check if the input is function argument
        auto tmp_fun_arg = dyn_cast<llvm::Argument>(operand);
        auto tmp_find_arg = find(function_argument.begin(),
                                 function_argument.end(), tmp_fun_arg);

        string command = string();
        string comment = string();

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
         */
        if (loop_edge != this->LoopEdges.end()) {
            continue;
        }

        /**
         * If instruction is Binary or Comparision operator
         */
        else if (ins_type == TBinaryOperator || ins_type == TICmpInst) {
            // If the operand is constant
            if (operand_const) {
                comment = "  // Wiring constant\n";
                command = "";
                if (c == 0) {
                    command =
                        "  {{ins_name}}.io.LeftIO.bits.data := {{value}}.U\n"
                        "  {{ins_name}}.io.LeftIO.bits.predicate := true.B\n"
                        "  {{ins_name}}.io.LeftIO.valid := true.B\n";

                } else {
                    command =
                        "  {{ins_name}}.io.RightIO.bits.data := {{value}}.U\n"
                        "  {{ins_name}}.io.RightIO.bits.predicate := true.B\n"
                        "  {{ins_name}}.io.RightIO.valid := true.B\n";
                }
                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set(
                    "value", static_cast<int>(operand_const->getSExtValue()));

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
                        "  {{ins_name}}.io.LeftIO <> io.{{operand_name}}\n";
                else
                    command =
                        "  {{ins_name}}.io.RightIO <> io.{{operand_name}}\n";

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
                        "  {{ins_name}}.io.LeftIO <> {{operand_name}}.io.Out"
                        "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
                else
                    command =
                        "  {{ins_name}}.io.RightIO <> {{operand_name}}.io.Out"
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
            // TODO
            // Check if the GEP has two inputs or one
            // Check if it's the baseaddress or index
            // If the input is function argument

            // First get the instruction
            if (tmp_find_arg != function_argument.end()) {
                auto op_ins = ins.getOperand(0);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                comment =
                    "  // Wiring GEP instruction to the function "
                    "argument\n";

                command =
                    "  {{ins_name}}.io.{{ins_input}} <> "
                    "io.{{operand_name}}\n\n";

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
                    "  // Wiring GEP instruction to the parent instruction\n";
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
            // Input of the load comes from either GEP instructions or function
            // arguments
            auto gep_ins = dyn_cast<llvm::GetElementPtrInst>(ins.getOperand(c));

            // If the input is function argument then it should get connect to
            // Cache system
            if (tmp_find_arg != function_argument.end()) {
                // First get the instruction
                auto op_ins = ins.getOperand(c);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                comment =
                    "  // Wiring Load instruction to the function "
                    "argument\n";

                command =
                    "  {{ins_name}}.io.GepAddr <> io.{{operand_name}}\n"
                    "  {{ins_name}}.io.memResp <> "
                    "CacheMem.io.ReadOut({{ins_index}})\n"
                    "  RegisterFile.io.ReadIn({{ins_index}}) <> "
                    "{{ins_name}}.io.memReq\n\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name", argument_info[op_arg].name);
            }

            else if (gep_ins) {
                // TODO Why the second condition?
                // else if (gep_ins ||
                // ins.getOperand(c)->getType()->isPointerTy()) {
                comment =
                    "  // Wiring Load instruction to the parent instruction\n";
                command =
                    "  {{ins_name}}.io.GepAddr <> {{operand_name}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n"
                    "  {{ins_name}}.io.memResp <> "
                    "RegisterFile.io.ReadOut({{ins_index}})\n"
                    "  RegisterFile.io.ReadIn({{ins_index}}) <> "
                    "{{ins_name}}.io.memReq\n\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name",
                                 instruction_info[dyn_cast<llvm::Instruction>(
                                                      ins.getOperand(c))]
                                     .name);
            } else {
                ins.dump();
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
            auto gep_ins = dyn_cast<llvm::GetElementPtrInst>(ins.getOperand(c));
            auto operand = ins.getOperand(c);

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
                        "  {{ins_name}}.io.GepAddr <> io.{{operand_name}}\n";
                    //"  {{ins_name}}.io.memResp  <> "
                    //"RegisterFile.io.WriteOut({{ins_index}})\n"
                    //"  RegisterFile.io.WriteIn({{ins_index}}) <> "
                    //"{{ins_name}}.io.memReq\n\n";

                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set("operand_name",
                                     argument_info[op_arg].name);

                } else {
                    comment =
                        "  // Wiring Store instruction to the parent "
                        "instruction\n";
                    command =
                        "  {{ins_name}}.io.GepAddr <> {{operand_name}}.io.Out"
                        "(param.{{ins_name}}_in(\"{{operand_name}}\"))";
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

                    comment = "";
                    // comment =
                    //"  // Wiring Store instruction to the function "
                    //"argument\n";
                    command =
                        "  {{ins_name}}.io.GepAddr <> io.{{operand_name}}\n";
                    command =
                        "  {{ins_name}}.io.inData <> io.{{operand_name}}\n";
                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set("operand_name",
                                     argument_info[op_arg].name);
                } else {
                    // If the store input comes from an instruction
                    // comment =
                    //"  // Wiring Store instruction to the parent
                    // instruction\n";
                    comment = "";
                    command =
                        "  {{ins_name}}.io.inData <> {{operand_name}}.io.Out"
                        "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";

                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set(
                        "operand_name",
                        instruction_info[dyn_cast<llvm::Instruction>(
                                             ins.getOperand(c))]
                            .name);
                }

                // command.append("Amirali\n");
                command.append(
                    "  {{ins_name}}.io.memResp  <> "
                    "RegisterFile.io.WriteOut({{ins_index}})\n"
                    "  RegisterFile.io.WriteIn({{ins_index}}) <> "
                    "{{ins_name}}.io.memReq\n"
                    "  {{ins_name}}.io.Out(0).ready := true.B\n\n");
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
             * In Alloca instruction we need to compute size of bytes which we
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
                else
                    assert(!"Unknown alloca type!");

                printCode(comment + ins_template.render(command) + "\n");

            } else
                assert(!"Alloca can not have more than one operand");
        }

        else if (ins_type == TReturnInst) {
            // First get the instruction
            comment = "  // Wiring return instructions\n";
            command = "";
            if (c == 0)
                command =
                    "  {{ins_name}}.io.InputIO <> {{operand_name}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n"
                    "  io.result <> {{ins_name}}.io.Out(0)\n";
            else
                assert(!"Return instruction cannot have more than one input");

            ins_template.set("ins_name", instruction_info[&ins].name);
            ins_template.set(
                "operand_name",
                instruction_info[dyn_cast<llvm::Instruction>(ins.getOperand(c))]
                    .name);

            printCode(comment + ins_template.render(command) + "\n");
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

        } else if (ins_type == TTrunc) {
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
                command = "  {{ins_name}}.io.Input <> io.{{operand_name}}\n";
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
                command = "  {{ins_name}}.io.Input <> io.{{operand_name}}\n";
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

        } else {
            ins.dump();
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
            // TODO supporting for function calls
            CallSite CS(&ins);
            if (CS.isCall()) continue;

            // Get instruction type
            if (InstructionTypeNode(ins) == TUBranchInst)
                continue;
            else if (InstructionTypeNode(ins) == TPHINode)
                // We have already connected the PHI nodes
                continue;
            // else if (InstructionTypeNode(ins) == TReturnInst)
            // TODO connect return instruction
            // We have to support return in another fashion
            // continue;
            else
                DataflowGeneratorPass::PrintDataFlow(ins);
        }
    }
}

/**
 * Printing BasicBlock enable signals
 */
void DataflowGeneratorPass::PrintBasicBlockEnableInstruction(Function &F) {
    LuaTemplater ins_template;
    string comment =
        "  /**\n"
        "    * Wireing enable signals to the instructions\n"
        "    */\n"
        "  //Wiring enable signals\n";

    printCode(comment);

    for (auto &BB : F) {
        for (auto &ins : BB) {
            llvm::CallSite CS(&ins);
            if (CS) continue;
            string command = "";
            command =
                "  {{ins_name}}.io.enable <> {{bb_name}}.io.Out"
                "(param.{{bb_name}}_activate(\"{{ins_name}}\"))\n";
            ins_template.set("ins_name", instruction_info[&ins].name);
            ins_template.set("bb_name", basic_block_info[&BB].name);
            printCode(ins_template.render(command));
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

        for (auto &L : getLoops(*LI)) {
            auto Loc = L->getStartLoc();
            auto Filename = getBaseName(Loc->getFilename().str());

            /**
             * Detecting Live-in and Live-out of each for loop
             */
            std::set<Value *> liveIns;
            std::set<Value *> liveOuts;

            llvm::SetVector<llvm::BasicBlock *> tmp_bb(L->blocks().begin(),
                                                       L->blocks().end());
            for (auto B : L->blocks()) {
                for (auto &I : *B) {
                    // Detecting Live-ins
                    for (auto OI = I.op_begin(); OI != I.op_end(); OI++) {
                        Value *V = *OI;

                        // Detecting instructions
                        if (Instruction *Ins = dyn_cast<Instruction>(V)) {
                            if (!L->contains(Ins) &&
                                definedInCaller(tmp_bb, V)) {
                                this->loop_liveins[L].insert(V);
                                this->LoopEdges.insert(std::make_pair(V, &I));
                            }
                        }
                        // Detecting function arguments
                        else if (isa<Argument>(V)) {
                            this->loop_liveins[L].insert(V);
                            this->LoopEdges.insert(std::make_pair(V, &I));
                        }
                    }

                    for (auto *U : I.users()) {
                        if (!definedInRegion(tmp_bb, U)) {
                            this->loop_liveouts[L].insert(U);
                            this->LoopEdges.insert(std::make_pair(&I, U));
                        }
                    }
                }
            }

            // this->loop_liveins[L] = liveIns;
            //this->loop_liveouts[L] = liveOuts;

            // TODO Fix number of inputs and outputs for loop head
            // TODO Do we need output anymore?
            string loop_define =
                "  val {{ins_name}} = "
                "Module(new LoopHeader(NumInputs = {{num_inputs}}, NumOuts = "
                "{{num_outputs}}, ID "
                "= 0)(p))\n";

            ins_template.set("ins_name",
                             "loop_L_" + std::to_string(Loc.getLine()));
            ins_template.set("num_inputs", static_cast<int>(liveIns.size()));
            ins_template.set("num_outputs", static_cast<int>(liveOuts.size()));

            printCode(ins_template.render(loop_define));
        }
    }
}

/**
 * This function connects each loop's live-ins to the
 * header register file
 */
void DataflowGeneratorPass::PrintLoopRegister(Function &F) {
    // Getting loop information
    if (getLoops(*LI).size() == 0)
        printCode("  //Function doesn't have any for loop");

    else {
        // Printing header part of each loop
        LuaTemplater ins_template;

        for (auto &L : getLoops(*LI)) {
            auto Loc = L->getStartLoc();
            auto Filename = getBaseName(Loc->getFilename().str());

            auto loop_live_in  = this->loop_liveins.find(L);
            auto loop_live_out = this->loop_liveouts.find(L);

            if (loop_live_in != this->loop_liveins.end()) {
                for (auto p : loop_live_in->second) {
                    for(auto search_elem : LoopEdges){
                        if(p == search_elem.first)
                            //TODO connect the edge
                            errs() << "EDGE here\n";
                    }
                }
            }


            if (loop_live_out != this->loop_liveouts.end()) {
                for (auto p : loop_live_out->second) {
                    for(auto search_elem : LoopEdges){
                        if(p == search_elem.second)
                            //TODO connect the edge
                            errs() << "EDGE here\n";
                    }
                }
            }

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
        "class {{class_name}}Tests"
        "(c: {{module_name}}) extends PeekPokeTester(c) {\n";
    ins_template.set("class_name", F.getName().str());
    ins_template.set("module_name", F.getName().str() + "DF");
    final_command.append(ins_template.render(command));

    printCode(ins_template.render(final_command), this->outTest);

    // Printing comments and the moduel information
    command = "\n  /**\n  *  {{module_name}}DF interface:\n  *\n";

    ins_template.set("module_name", F.getName().str());

    final_command = ins_template.render(command);

    uint32_t c = 0;
    string init_command = "";

    for (auto &ag : F.getArgumentList()) {
        command =
            "  *    data_{{index}} = Flipped(Decoupled(new DataBundle))\n";
        ins_template.set("index", static_cast<int>(c++));
        final_command.append(ins_template.render(command));

        command =
            "  poke(c.io.data_{{index}}.bits.data, 0.U)\n"
            "  poke(c.io.data_{{index}}.bits.predicate, false.B)\n"
            "  poke(c.io.data_{{index}}.bits.valid, false.B)\n"
            "  poke(c.io.data_{{index}}.valid, false.B)\n\n";

        init_command.append(ins_template.render(command));
    }

    command = "  poke(c.io.result.ready, false.B)\n\n";
    init_command.append(command);

    final_command.append(
        "   *    val pred = Decoupled(new Bool())\n"
        "   *    val start = Input(new Bool())\n");

    if (!F.getReturnType()->isVoidTy()) {
        final_command.append(
            "   *    val result = Decoupled(new DataBundle)\n");
    }

    final_command.append("   */\n\n\n");

    command = "  // Initializing the signals\n\n";
    final_command.append(command);
    final_command.append(init_command);

    command =
        "  step(1)\n"
        "  var time = 1  //Cycle counter\n  /**\n   *\n"
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
        "   *    println(s\"Waited $count cycles on gcd inputs $i, $j, giving "
        "up\")\n"
        "   *\n"
        "   */\n\n";

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
        "     () => new {{class_name}}DF()) {\n"
        "     c => new {{class_name}}Tests(c)\n"
        "    } should be(true)\n"
        "  }\n}\n";

    printCode(ins_template.render(command), this->outTest);
}

/**
 * This function is the main core of the code generator
 * generateFunction function starts dumping scala code for
 * a specific function step by step.
 * At each step specific part of the code will be generated
 * and each function internally call printCode function to
 * dump the code
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

    // Step5:
    // Printing BasicBlock definitions
    printHeader("Printing BasicBlocks");
    HelperPrintBBInit(F);

    // Step6:
    // Printing Loop headers
    printHeader("Printing Loop Headers");
    PrintLoopHeader(F);

    // Step6:
    // Printing Instruction initialization
    printHeader("Printing Insturctions");
    HelperPrintInistInit(F);

    // Step 7:
    // Initilizing the parameters object
    printHeader("Initializing Param");
    PrintParamObject();

    // Step 8:
    // Connecting enable signal of BasicBlocks
    printHeader("Connecting BasicBlocks to Predicate Instructions");
    HelperPrintBasicBlockPredicate();

    printHeader("Connecting BasicBlocks to instructions");
    PrintBasicBlockEnableInstruction(F);

    // Connecting BasicBlock masks to their Phi nodes
    printHeader("Dumping PHI nodes");
    HelperPrintBasicBlockPhi();

    // Step 9:
    // Connecting Instructions in dataflow order
    printHeader("Connecting LoopHeaders");
    PrintLoopRegister(F);
    // TODO Connect the loop headers
    printHeader("Dumping Dataflow");
    HelperPrintInstructionDF(F);

    // Closing the object
    printCode("}\n");
}
