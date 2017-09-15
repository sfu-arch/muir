
#define DEBUG_TYPE "split-geps"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"

#include <cxxabi.h>

//#include "llvm/Pass.h"
//#include "llvm/IR/BasicBlock.h"
//#include "llvm/IR/Constants.h"
//#include "llvm/IR/Function.h"
//#include "llvm/IR/Instructions.h"
//#include "llvm/Support/Debug.h"

#include "luacpptemplater/LuaTemplater.h"

#include <fstream>

#include "AliasMem.h"
#include "Common.h"
#include "DataFlowGenerator.h"
#include "NodeType.h"

using namespace llvm;
using namespace std;
using namespace common;
using namespace dataflowgen;

extern cl::list<std::string> FunctionList;

extern bool isTargetFunction(const Function &f,
                             const cl::list<std::string> &FunctionList);

namespace dataflowgen {
char DataFlowGenPass::ID = 0;
static RegisterPass<DataFlowGenPass> X(
    "dataflowgen", "Generating chisel dataflow file from bitcode");
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

inline std::string demangle(const char *name) {
    int status = -1;

    std::unique_ptr<char, void (*)(void *)> res{
        abi::__cxa_demangle(name, NULL, NULL, &status), std::free};
    return (status == 0) ? res.get() : std::string(name);
}

/**
 * Printing hear file
 */
inline void printHeader(raw_ostream &out, string header) {
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

    out << "\n\n\n  /* "
           "================================================================== "
           "*\n"
           "   *                   "
        << header << header_final;
    out << tmp_line;
}

InstructionType InstructionTypeNode(Instruction &ins) {
    if (isa<BinaryOperator>(ins))
        return common::TBinaryOperator;
    else if (isa<llvm::ICmpInst>(ins))
        return common::TICmpInst;
    else if (isa<llvm::BranchInst>(ins)) {
        if (ins.getNumOperands() == 3)
            return common::TCBranchInst;
        else if (ins.getNumOperands() == 1)
            return common::TUBranchInst;
    } else if (isa<llvm::PHINode>(ins))
        return common::TPHINode;
    else if (isa<llvm::ReturnInst>(ins))
        return common::TReturnInst;
    else if (isa<llvm::GetElementPtrInst>(ins))
        return common::TGEP;
    else if (isa<llvm::LoadInst>(ins))
        return common::TLoad;
    else if (isa<llvm::StoreInst>(ins))
        return common::TStore;
    else if (isa<llvm::SExtInst>(ins))
        return common::TSEXT;
    else if (isa<llvm::ZExtInst>(ins))
        return common::TZEXT;
    else if (isa<llvm::PtrToIntInst>(ins))
        return common::TPtrToInt;
    else if (isa<llvm::BitCastInst>(ins))
        return common::TBitCast;
    else if (isa<llvm::TruncInst>(ins))
        return common::TTrunc;

    else {
        // TODO if there is any unkonw instruction break
        ins.dump();
        assert(!"ERROR: Uknown node type");
    }
}

bool DataFlowGenPass::doInitialization(llvm::Module &M) {
    count_ins = 0;
    count_binary = 0;
    count_node = 0;
    count_branch = 0;
    //    common::optimizeModule(&M);
    return false;
}

bool DataFlowGenPass::doFinalization(llvm::Module &M) { return false; }

void DataFlowGenPass::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<helpers::InsTest>();
    AU.addRequired<aew::AliasEdgeWriter>();
    AU.setPreservesAll();
}

void DataFlowGenPass::NamingBasicBlock(Function &F) {
    uint32_t id_count = 0;
    uint32_t c = 0;
    this->entry_bb = &F.getEntryBlock();
    for (auto &BB : F) {
        string tmp_name = BB.getName().str();
        if (tmp_name.empty()) tmp_name = "unkonw." + to_string(c++);

        std::replace(tmp_name.begin(), tmp_name.end(), '.', '_');
        BBInfo t_info = {tmp_name, id_count++};
        basic_block_info[&BB] = t_info;
    }
}

void DataFlowGenPass::NamingInstruction(llvm::Function &F) {
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

void DataFlowGenPass::PrintBasicBlockInit(BasicBlock &BB) {
    uint32_t phi_c = CountPhiNode(BB);
    if (phi_c == 0) {
        string bb_define =
            "  val {{bb_name}} = "
            "Module(new BasicBlockNoMaskNode"
            "(NumInputs = {{num_target}}, NumOuts = {{num_ins}}, "
            "BID = {{bb_id}})(p))";
        LuaTemplater bb_template;

        bb_template.set("bb_name", basic_block_info[&BB].name);

        if (countPred(BB) == 0)
            bb_template.set("num_target", static_cast<int>(1));
        else
            bb_template.set("num_target", static_cast<int>(countPred(BB)));

        bb_template.set("num_ins", static_cast<int>(BB.getInstList().size()));
        bb_template.set("bb_id", static_cast<int>(basic_block_info[&BB].id));

        string result = bb_template.render(bb_define);

        outs() << result << "\n";
    } else {
        string bb_define =
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

        string result = bb_template.render(bb_define);

        outs() << result << "\n";
    }
}

void DataFlowGenPass::PrintBinaryComparisionIns(Instruction &Ins) {
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
            default:
                Ins.dump();
                assert(!"Unkonw CMP operand");
                break;
        }

        ins_template.set("op_code", cmp_ins_op);
    } else
        ins_template.set("op_code", Ins.getOpcodeName());

    ins_template.set("ins_id", static_cast<int>(instruction_info[&Ins].id));

    string result = ins_template.render(ins_define);

    // TODO Change outs to file
    outs() << "\n  //";
    Ins.print(outs());
    outs() << "\n" << result << "\n";
}

void DataFlowGenPass::PrintBranchIns(Instruction &Ins) {
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

    // TODO change the outs to file
    outs() << "\n  //";
    Ins.print(outs());
    outs() << "\n" << result << "\n";
}

void DataFlowGenPass::PrintPHIIns(Instruction &Ins) {
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

    // TODO change the outs to file
    outs() << "\n  //";
    Ins.print(outs());
    outs() << "\n" << result << "\n";
}

void DataFlowGenPass::PrintGepIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    // Getting gep pass information
    auto &gep_pass_ctx = getAnalysis<helpers::InsTest>();

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
        assert(!"The GEP instruction has more than two inputs");
    }

    string result = ins_template.render(ins_define);

    // TODO change the outs to file
    outs() << "\n  //";
    Ins.print(outs());
    outs() << "\n" << result << "\n";
}

void DataFlowGenPass::PrintLoadIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    // Getting AA pass information
    auto &aa_pass_ctx = getAnalysis<aew::AliasEdgeWriter>();

    uint32_t index = 0;
    for (auto in : instruction_load) {
        if (in == &Ins) break;
        index++;
    }

    // TODO change the outs to file

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

    // TODO change the outs to file
    outs() << "\n  //";
    Ins.print(outs());
    outs() << "\n" << result << "\n";
}

void DataFlowGenPass::PrintStoreIns(Instruction &Ins) {
    // Get instruction type
    auto ins_type = InstructionTypeNode(Ins);

    // Getting AA pass information
    auto &aa_pass_ctx = getAnalysis<aew::AliasEdgeWriter>();

    uint32_t index = 0;
    for (auto in : instruction_store) {
        if (in == &Ins) break;
        index++;
    }

    // TODO change the outs to file

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

    // TODO change the outs to file
    outs() << "\n  //";
    Ins.print(outs());
    outs() << "\n" << result << "\n";
}

void DataFlowGenPass::PrintSextIns(Instruction &Ins) {
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

    // TODO change the outs to file
    outs() << "\n  //";
    Ins.print(outs());
    outs() << "\n" << result << "\n";
}

void DataFlowGenPass::PrintZextIns(Instruction &Ins) {
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

    // TODO change the outs to file
    outs() << "\n  //";
    Ins.print(outs());
    outs() << "\n" << result << "\n";
}

void DataFlowGenPass::PrintInstInit(Instruction &Ins) {
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

        outs() << "\n  //";
        Ins.print(outs());
        outs() << "\n" << result << "\n";
    }
}

/**
 * Connecting connections between Branches and target BasicBlocks
 * @param ins input branch instruction
 */
void DataFlowGenPass::PrintBranchBasicBlockCon(Instruction &ins) {
    auto branch_ins = dyn_cast<llvm::BranchInst>(&ins);

    LuaTemplater ins_template;

    for (uint32_t i = 0; i < branch_ins->getNumSuccessors(); i++) {
        string comment = "  //Connecting {{ins_name}} to {{basic_block}}\n";
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
        outs() << result;

        result = ins_template.render(command);
        outs() << result;

        // outs() << basic_block_info[branch_ins->getSuccessor(i)].name << "\n";
    }
}

/**
 * Connecting phi node connections
 * @param ins Phi instruction
 */
void DataFlowGenPass::PrintPHICon(llvm::Instruction &ins) {
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
            outs() << result;

        } else {
            string command =
                "  //@todo {{phi_name}}.io.InData(param.{{phi_name}}_phi_in"
                "(\"{{ins_name}}\")) <> {{ins_name}}.io.Out(0)\n";
            ins_template.set("phi_name", instruction_info[&ins].name);
            // ins_template.set("ins_name", instruction_info[ins_target].name);

            string result = ins_template.render(command);
            outs() << result;

            // ins.dump();
            // assert(!"Cannot support constant for the PHIs for now");
        }
    }
}

void DataFlowGenPass::PrintPHIMask(llvm::Instruction &ins) {
    auto phi_ins = dyn_cast<llvm::PHINode>(&ins);
    LuaTemplater ins_template;

    string command = "  {{phi_name}}.io.Mask <> {{ins_name}}.io.MaskBB(0)\n";
    ins_template.set("phi_name", instruction_info[&ins].name);
    ins_template.set("ins_name", basic_block_info[ins.getParent()].name);

    string result = ins_template.render(command);
    outs() << result;
}

/**
 * Connecting BasicBlock enable signals to their instructions
 * @param BB    Input BasicBlock
 */

void DataFlowGenPass::PrintBasicBlockEnableInstruction(llvm::BasicBlock &BB) {
    LuaTemplater ins_template;

    for (auto &ins : BB) {
        llvm::CallSite CS(&ins);
        if (CS) continue;
        string command = "";
        if (isa<llvm::ReturnInst>(ins))
            command =
                "  //{{ins_name}}.io.enable <> {{bb_name}}.io.Out"
                "(param.{{bb_name}}_activate(\"{{ins_name}}\"))\n";
        else
            command =
                "  {{ins_name}}.io.enable <> {{bb_name}}.io.Out"
                "(param.{{bb_name}}_activate(\"{{ins_name}}\"))\n";
        ins_template.set("ins_name", instruction_info[&ins].name);
        ins_template.set("bb_name", basic_block_info[&BB].name);
        outs() << ins_template.render(command);
    }

    outs() << "\n";
}

/**
 * Connecting Insturctions in the dataflow order
 */
void DataFlowGenPass::PrintDataFlow(llvm::Instruction &ins) {
    LuaTemplater ins_template;
    auto ins_type = InstructionTypeNode(ins);

    for (uint32_t c = 0; c < ins.getNumOperands(); c++) {
        // Handling constant operands
        auto operand = ins.getOperand(c);
        auto operand_ins = dyn_cast<llvm::Instruction>(ins.getOperand(c));
        auto operand_const = dyn_cast<llvm::ConstantInt>(ins.getOperand(c));
        string command = string();
        string comment = string();

        if (operand_const &&
            (ins_type == TBinaryOperator || ins_type == TICmpInst)) {
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
            ins_template.set("value",
                             static_cast<int>(operand_const->getSExtValue()));

            outs() << comment << ins_template.render(command) << "\n";
        } else if (ins_type == TBinaryOperator || ins_type == TICmpInst) {
            // Check if the input is function argument
            auto tmp_fun_arg = dyn_cast<llvm::Argument>(operand);
            auto tmp_find_arg = find(function_argument.begin(),
                                     function_argument.end(), tmp_fun_arg);

            // Check if the GEP has two inputs or one
            // Check if it's the baseaddress or index
            // If the input is function argument
            if (tmp_find_arg != function_argument.end()) {
                // First get the instruction
                auto op_ins = ins.getOperand(c);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                comment =
                    "  // Wiring Binary instruction to the function "
                    "argument\n";
                if (c == 0)
                    command =
                        "  {{ins_name}}.io.LeftIO <> io.{{operand_name}}\n\n";
                else
                    command =
                        "  {{ins_name}}.io.RightIO <> io.{{operand_name}}\n\n";

                // XXX uncomment if needed
                // command =
                //"  {{ins_name}}.io.Input <> {{operand_name}}.io.Out"
                //"(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";

                ins_template.set("ins_name", instruction_info[&ins].name);

                if (argument_info.find(op_arg) == argument_info.end()) {
                    // errs() << *op_arg << "\n";
                    assert(!"WRONG");
                }

                ins_template.set("operand_name", argument_info[op_arg].name);
            }

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
            outs() << comment << ins_template.render(command) << "\n";
        } else if (ins_type == TCBranchInst && ins.getNumOperands() == 1) {
            comment = " // Wiring Branch instruction\n";
            command =
                "{{ins_name}}.io.CmpIO <> {{operand_name}}.io.Out"
                "(param.{{ins_name}}_brn_cmp(\"{{operand_name}}\"))\n";
            ins_template.set("ins_name", instruction_info[&ins].name);
            ins_template.set(
                "operand_name",
                instruction_info[dyn_cast<llvm::Instruction>(ins.getOperand(c))]
                    .name);

            outs() << comment << ins_template.render(command);
        } else if (ins_type == TCBranchInst)
            continue;
        else if (ins_type == TGEP) {
            auto tmp_fun_arg = dyn_cast<llvm::Argument>(operand);
            auto tmp_find_arg = find(function_argument.begin(),
                                     function_argument.end(), tmp_fun_arg);

            // Check if the GEP has two inputs or one
            // Check if it's the baseaddress or index
            // If the input is function argument
            if (tmp_find_arg != function_argument.end()) {
                // First get the instruction
                auto op_ins = ins.getOperand(0);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                comment =
                    "  // Wiring GEP instruction to the function "
                    "argument\n";
                if (c == 0) {
                    command =
                        "  {{ins_name}}.io.baseAddress <> "
                        "io.{{operand_name}}\n\n";
                } else {
                    command =
                        "  {{ins_name}}.io.{{ins_input}} <> "
                        "io.{{operand_name}}\n\n";
                    // command =
                    //"  {{ins_name}}.io.{{ins_input}} <> "
                    //"{{operand_name}}.io.Out"
                    //"(param.{{ins_name}}_in(\"{{operand_name}}\"))\n\n";
                }
                // XXX uncomment if needed
                // command =
                //"  {{ins_name}}.io.Input <> {{operand_name}}.io.Out"
                //"(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set(
                    "operand_name",
                    argument_info[dyn_cast<llvm::Argument>(ins.getOperand(c))]
                        .name);
                // ins_template.set("operand_name", argument_info[op_arg].name);
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

            outs() << comment << ins_template.render(command);

        } else if (ins_type == TLoad) {
            auto operand = ins.getOperand(c);

            // Input of the load alwasy comes from GEP instruction
            auto gep_ins = dyn_cast<llvm::GetElementPtrInst>(ins.getOperand(c));

            // If the input is from function argument
            auto tmp_fun_arg = dyn_cast<llvm::Argument>(operand);
            auto tmp_find_arg = find(function_argument.begin(),
                                     function_argument.end(), tmp_fun_arg);

            // If the input is function argument
            if (tmp_find_arg != function_argument.end()) {
                // First get the instruction
                auto op_ins = ins.getOperand(0);
                auto op_arg = dyn_cast<llvm::Argument>(op_ins);

                comment =
                    "  // Wiring Load instruction to the function "
                    "argument\n";

                command =
                    "  {{ins_name}}.io.GepAddr <> io.{{operand_name}}\n"
                    "  {{ins_name}}.io.memResp <> "
                    "StackFile.io.ReadOut({{ins_index}})\n"
                    "  StackFile.io.ReadIn({{ins_index}}) <> "
                    "{{ins_name}}.io.memReq\n\n";

                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("operand_name", argument_info[op_arg].name);
            }

            else if (gep_ins || ins.getOperand(c)->getType()->isPointerTy()) {
                comment =
                    "  // Wiring Load instruction to the parent instruction\n";
                command =
                    "  {{ins_name}}.io.GepAddr <> {{operand_name}}.io.Out"
                    "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n"
                    "  {{ins_name}}.io.memResp  <> "
                    "StackFile.io.ReadOut({{ins_index}})\n"
                    "  StackFile.io.ReadIn({{ins_index}}) <> "
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
            outs() << comment << ins_template.render(command);

            // Connecting Predecessors

            // Getting AA pass information
            comment = "  //Printing succesor of the current Load instruction\n";

            auto ins_mp = mem_pred.find(&ins);
            for (auto mp : ins_mp->second) {
                command =
                    "{{ins_name}}.io.PredOp(0) <> "
                    "{{pred_ins}}.io.SuccOp(0)\n";
                ins_template.set("ins_name", instruction_info[&ins].name);
                ins_template.set("pred_ins", instruction_info[mp].name);
                outs() << ins_template.render(command);
            }
            outs() << "\n";

        } else if (ins_type == TStore) {
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
                    //"StackFile.io.WriteOut({{ins_index}})\n"
                    //"  StackFile.io.WriteIn({{ins_index}}) <> "
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
            }
            else{

                // If the input is function argument
                if (tmp_find_arg != function_argument.end()) {
                    // First get the instruction
                    auto op_ins = ins.getOperand(c);
                    auto op_arg = dyn_cast<llvm::Argument>(op_ins);
   
                    comment = "";
                    //comment =
                        //"  // Wiring Store instruction to the function "
                        //"argument\n";
                    command = "  {{ins_name}}.io.GepAddr <> io.{{operand_name}}\n";
                    command =
                        "  {{ins_name}}.io.inData <> io.{{operand_name}}\n";
                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set("operand_name", argument_info[op_arg].name);
                } else {
                    // If the store input comes from an instruction
                    //comment =
                        //"  // Wiring Store instruction to the parent instruction\n";
                    comment = "";
                    command =
                        "  {{ins_name}}.io.inData <> {{operand_name}}.io.Out"
                        "(param.{{ins_name}}_in(\"{{operand_name}}\"))\n";
    
                    ins_template.set("ins_name", instruction_info[&ins].name);
                    ins_template.set("operand_name",
                                     instruction_info[dyn_cast<llvm::Instruction>(
                                                          ins.getOperand(c))]
                                         .name);
                }

                //command.append("Amirali\n");
                command.append("  {{ins_name}}.io.memResp  <> "
                    "StackFile.io.WriteOut({{ins_index}})\n"
                    "  StackFile.io.WriteIn({{ins_index}}) <> "
                    "{{ins_name}}.io.memReq\n"
                    "  {{ins_name}}.io.Out(0).ready := true.B\n\n");
            }

            ptrdiff_t pos = distance(
                instruction_store.begin(),
                find(instruction_store.begin(), instruction_store.end(), &ins));
            ins_template.set("ins_index", static_cast<int>(pos));
            outs() << comment << ins_template.render(command);

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
                    outs() << ins_template.render(command);
                }
                outs() << "\n";
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

            outs() << comment << ins_template.render(command) << "\n";

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

            outs() << comment << ins_template.render(command) << "\n";

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

            outs() << comment << ins_template.render(command) << "\n";

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

            outs() << comment << ins_template.render(command) << "\n";

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

            outs() << comment << ins_template.render(command) << "\n";

        } else {
            ins.dump();
            assert(!"The instruction is not supported in the dataflow connection phase");
        }
    }
}

void DataFlowGenPass::PrintBasicBlockPorts(llvm::Function &F) {
    LuaTemplater ins_template;

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

    // Entry basicblock is special case since it should be activated all the
    // time
    auto &entry_bb = F.getEntryBlock();
    string command =
        "  val {{bb_name}}_pred = Map(\n"
        "    \"active\" -> 0\n"
        "  )\n";
    ins_template.set("bb_name", basic_block_info[&entry_bb].name);
    outs() << ins_template.render(command) << "\n";

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

        //        final_command.clear();
        //        command = "  val {{bb_name}}_activate = Map(\n";
        //        ins_template.set("bb_name", basic_block_info[&bb].name);
        //        final_command.append(ins_template.render(command));
        //
        //        uint32_t c = 0;
        //        for (auto &ins : bb) {
        //            command = "    \"{{ins_name}}\" -> {{index}},\n";
        //            ins_template.set("ins_name", instruction_info[&ins].name);
        //            ins_template.set("index", static_cast<int>(c++));
        //            final_command.append(ins_template.render(command));
        //
        //        }
        //        final_command = final_command.substr(0, final_command.length()
        //        - 2);
        //        final_command.append("\n  )\n\n");
        //        outs() << final_command;
    }

    outs() << "}\n\n";
}

void DataFlowGenPass::FillFunctionArg(llvm::Function &F) {
    uint32_t c = 0;
    for (auto &f_arg : F.getArgumentList()) {
        function_argument.push_back(&f_arg);

        // ptrdiff_t pos =
        // distance(function_argument.begin(),
        // find(function_argument.begin(), function_argument.end(),
        // dyn_cast<llvm::Argument>(ins.getOperand(c))));
        ArgInfo temp_arg = {"data_" + to_string(c), static_cast<uint32_t>(c)};
        // argument_info[dyn_cast<llvm::Argument>(ins.getOperand(c))] =
        // temp_arg;
        argument_info[&f_arg] = temp_arg;
        c++;
    }
}

void DataFlowGenPass::FillInstructionContainers(llvm::Function &F) {
    NamingBasicBlock(F);
    NamingInstruction(F);

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

void DataFlowGenPass::PrintDatFlowAbstractIO(llvm::Function &F) {
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

    final_command.append(
        "class {{module_name}}DF(implicit p: Parameters)"
        " extends {{module_name}}DFIO()(p) {\n");
    ins_template.set("module_name", F.getName().str());

    outs() << ins_template.render(final_command);
}

void DataFlowGenPass::PrintImport() {
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

    outs() << command;
}

void DataFlowGenPass::PrintStackFile() {
    string command =
        "\tval StackFile = Module(new "
        "TypeStackFile(ID=0,Size=32,NReads={{load_num}},NWrites={{store_num}})"
        "\n"
        "\t\t            (WControl=new "
        "WriteMemoryController(NumOps={{store_num}},BaseSize=2,NumEntries=2))\n"
        "\t\t            (RControl=new "
        "ReadMemoryController(NumOps={{load_num}},BaseSize=2,NumEntries=2)))";

    LuaTemplater stack_template;
    stack_template.set("load_num", static_cast<int>(instruction_load.size()));
    stack_template.set("store_num", static_cast<int>(instruction_store.size()));

    outs() << stack_template.render(command);
}

void DataFlowGenPass::AACompute() {
    auto &aa_ctx = getAnalysis<aew::AliasEdgeWriter>();

    // outs() << "\nAA Res: \n";
    // outs() << "Size: " << aa_ctx.must_edge.size() << "\n";

    auto edge = aa_ctx.must_edge;
    for (auto it = edge.begin(); it != edge.end(); it++) {
        mem_pred[it->first].push_back(it->second);
        mem_succ[it->second].push_back(it->first);
    }
}

void DataFlowGenPass::Process(llvm::Function &F) {
    //@TODO Add loop info pass

    map<llvm::BasicBlock *, SmallVector<llvm::Instruction *, 16>> BlockInsMap;

    // Read the template file std::ifstream template_file("Dataflow.scala");
    // std::stringstream buffer;
    //    buffer << template_file.rdbuf();

    //    outs() << buffer.str() << "\n";

    // AAPrint();

    // Dump import
    PrintImport();

    // Fill the instruction containers
    FillInstructionContainers(F);

    // Filling function arguments
    FillFunctionArg(F);

    // Printing param object
    string comment =
        "/**\n"
        "  * This Object should be initialize at the first step\n"
        "  * It contains all the transformation from indecies to their "
        "module's name\n"
        "  */\n\n";
    outs() << comment;
    comment = "object Data_" + F.getName().str() + "_FlowParam{\n";
    param_name = "Data_" + F.getName().str() + "_FlowParam";
    outs() << comment;
    PrintBasicBlockPorts(F);

    // Printing Datflow abstract IO class
    printHeader(outs(), "Printing Ports Definition");
    PrintDatFlowAbstractIO(F);
    printHeader(outs(), "Printing Module Definition");

    // Printing BasicBlock initialization
    // Step 1:
    printHeader(outs(), "Printing BasicBlocks");
    comment = "  //Initializing BasicBlocks: \n";
    outs() << comment;
    for (auto &BB : F) {
        // Initial step is to naming BasicBlocks and the Instructions
        PrintBasicBlockInit(BB);
    }
    outs() << "\n\n";

    // Print StackFile
    printHeader(outs(), "Printing StackFile");
    PrintStackFile();

    // Printing Instruction initialization
    // Step 2:
    printHeader(outs(), "Printing Insturctions");
    comment = "  //Initializing Instructions: \n";
    outs() << comment;
    for (auto &BB : F) {
        outs() << "  // " << BB.getName().str() << ": \n";
        for (auto &INS : BB) {
            llvm::CallSite CS(&INS);
            if (CS) continue;
            PrintInstInit(INS);
        }
        outs() << "\n";
    }
    outs() << "\n\n";

    // Connecting enable signal of BasicBlocks
    // Step 3:
    string param =
        "  /**\n"
        "    * Instantiating parameters\n"
        "    */\n"
        "  val param = " +
        param_name + "\n\n";

    printHeader(outs(), "Initializing Param");
    outs() << param;

    printHeader(outs(), "Connecting BasicBlocks to Predicate Instructions");
    comment =
        ""
        "  /**\n"
        "     * Connecting basic blocks to predicate instructions\n"
        "     */\n\n"
        "  //We always ground entry BasicBlock\n";
    outs() << comment;

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

    outs() << tmp_result;
    // outs() << ground_entry;

    comment =
        "  /**\n"
        "    * Connecting basic blocks to predicate instructions\n"
        "    */\n";
    outs() << comment;

    // Iterate over branch instruction and connect them their BasicBlock
    // Step 4:
    if (instruction_branch.size() == 0)
        outs() << "\n  // There is no branch insruction\n\n";
    else {
        std::for_each(instruction_branch.begin(), instruction_branch.end(),
                      [this](Instruction *ins) {
                          DataFlowGenPass::PrintBranchBasicBlockCon(*ins);
                      });
    }

    // Connecting enable signals from Basicblocks to the instructions
    // Step 5:
    printHeader(outs(), "Connecting BasicBlocks to instructions");
    comment =
        "  /**\n"
        "    * Wireing enable signals to the instructions\n"
        "    */\n"
        "  //Wiring enable signals\n";

    outs() << comment;

    for (auto &BB : F) PrintBasicBlockEnableInstruction(BB);

    // Connecting PHI nodes
    // Step 6:
    printHeader(outs(), "Dumping PHI nodes");
    comment =
        "  /**\n"
        "    * Connecting PHI nodes\n"
        "    */\n"
        "  //Connect PHI node\n";
    outs() << comment;
    if (instruction_phi.size() == 0) outs() << "  //There is no PHI node\n";
    std::for_each(
        instruction_phi.begin(), instruction_phi.end(),
        [this](Instruction *ins) { DataFlowGenPass::PrintPHICon(*ins); });

    // Connecting BasicBlock masks to their Phi nodes
    // Step 7:
    comment =
        "  /**\n"
        "    * Connecting PHI Masks\n"
        "    */\n"
        "  //Connect PHI node\n";
    std::for_each(
        instruction_phi.begin(), instruction_phi.end(),
        [this](Instruction *ins) { DataFlowGenPass::PrintPHIMask(*ins); });

    outs() << "\n";

    // Connecting Instructions in dataflow order
    // Step 8:
    printHeader(outs(), "Dumping Dataflow");
    comment =
        "  /**\n"
        "    * Connecting Dataflow signals\n"
        "    */\n";
    outs() << comment;
    for (auto &BB : F) {
        for (auto &ins : BB) {
            CallSite CS(&ins);
            if (CS.isCall()) continue;

            // Get instruction type
            if (InstructionTypeNode(ins) == TUBranchInst)
                continue;
            else if (InstructionTypeNode(ins) == TPHINode)
                // We have already connected the PHI nodes
                continue;
            else if (InstructionTypeNode(ins) == TReturnInst)
                // We have to support return in another fashion
                // TODO connect return instruction
                continue;
            else
                DataFlowGenPass::PrintDataFlow(ins);
        }
    }

    outs() << "}\n";
}

bool DataFlowGenPass::runOnModule(Module &module) {
    // Filltering functions
    for (auto &F : module) {
        if (isTargetFunction(F, FunctionList)) Process(F);
        // else
        // outs() << demangle(F.getName().str().c_str()) << "\n";
    }
    return false;
}
