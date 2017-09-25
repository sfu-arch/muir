#define DEBUG_TYPE "generator_code"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#include "luacpptemplater/LuaTemplater.h"


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

    //Alloca instruction
    else if(isa<llvm::AllocaInst>(ins))
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
        }
    }

    return false;
}

/**
 * Set pass dependencies
 */
void DataflowGeneratorPass::getAnalysisUsage(AnalysisUsage &AU) const {
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
void DataflowGeneratorPass::generateImportSection() {
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
        "import arbiters._\n"
        "import node._\n\n";

    // Print to the OUTPUT
    printCode(command);
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
    comment = "object Data_" + F.getName().str() + "_FlowParam{\n";
    param_name = "Data_" + F.getName().str() + "_FlowParam";

    //Print the first initial part of the object
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
        outs() << final_command;
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
        outs() << final_command;
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
        outs() << final_command;
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
        outs() << final_command;
    }

    for (auto &bb : F) {
        for (auto &ins : bb) {
            llvm::CallSite CS(&ins);
            if (CS) continue;
            if (dyn_cast<llvm::BranchInst>(&ins))
                continue;
            else {
                final_command.clear();
                command = "  val {{ins_name}}_in = Map( \n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                final_command.append(ins_template.render(command));

                for (uint32_t c = 0; c < ins.getNumOperands(); c++) {
                    if (dyn_cast<llvm::ConstantInt>(ins.getOperand(c)))
                        continue;
                    else if (dyn_cast<llvm::BranchInst>(ins.getOperand(c)))
                        continue;
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
                outs() << final_command;
            }
        }
    }

    outs() << "}\n\n";
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
    generateImportSection();

    // Step2: Printing helper param object
    // This object contains mapping from each instructions to their consequent
    // instructions and basic blocks
    // Since we have to use indexes to to connect differet modules to getter
    // we use this mapping so that instead of using pure indexes we can use
    // a mapping from the name's to their index
    PrintHelperObject(F);
}

// Output for a pure analysis pass should happen in the print method.
// It is called automatically after the analysis pass has finished collecting
// its information.
// void
// DataflowGeneratorPass::print(raw_ostream &out, Module const *m) const {
// out << "Function Counts\n"
//<< "===============\n";
// for (auto &kvPair : counts) {
// auto *function = kvPair.first;
// uint64_t count = kvPair.second;
// out << function->getName() << " : " << count << "\n";
//}
//}
