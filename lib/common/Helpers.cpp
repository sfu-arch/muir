#define DEBUG_TYPE "helpers"

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <cmath>
#include <experimental/iterator>
#include <fstream>
#include <iostream>

#include "Common.h"

using namespace llvm;

using helpers::LabelUID;
using helpers::pdgDump;
using helpers::DFGPrinter;
using helpers::GepInformation;
using helpers::InstCounter;
using helpers::CallInstSpliter;

/**
 * Helper classes
 */

namespace helpers {
// LabelUID Helper Class
char LabelUID::ID = 0;

RegisterPass<LabelUID> X("lableUID", "Labeling the instructions with UID");
}

template <typename T>
void LabelUID::visitGeneric(string S, T &IT) {
    if (values.count(&IT) == 0) {
        values.insert(make_pair(&IT, counter));
        counter++;
    }
    auto &Context = IT.getContext();
    MDNode *N =
        MDNode::get(Context, MDString::get(Context, to_string(values[&IT])));
    IT.setMetadata(S, N);
}

void LabelUID::visitFunction(Function &F) {
    if (F.isDeclaration()) return;
    visitGeneric<Function>("UID", F);
}

void LabelUID::visitInstruction(Instruction &I) {
    if (auto call_inst = dyn_cast<CallInst>(&I)) {
        if (call_inst->getCalledFunction()->isDeclaration()) return;
    }
    visitGeneric<Instruction>("UID", I);
}

void LabelUID::visitBasicBlock(BasicBlock &BB) {
    if (values.count(&BB) == 0) {
        values.insert(make_pair(&BB, counter));
        counter++;
    }
    auto &Context = BB.getContext();
    MDNode *N =
        MDNode::get(Context, MDString::get(Context, to_string(values[&BB])));
    BB.getTerminator()->setMetadata("BB_UID", N);
}

bool LabelUID::runOnFunction(Function &F) {
    visit(F);
    return false;
}

namespace helpers {
char pdgDump::ID = 0;
RegisterPass<pdgDump> Y("pdgDump", "Dumping PDG");
}

bool pdgDump::runOnFunction(Function &F) {
    errs() << "digraph " + F.getName() + "{\n";
    errs() << "\n";
    for (auto block = F.getBasicBlockList().begin();
         block != F.getBasicBlockList().end(); block++) {
        for (auto inst = block->begin(); inst != block->end(); inst++) {
            for (Use &U : inst->operands()) {
                Value *v = U.get();
                if (dyn_cast<Instruction>(v)) {
                    errs() << "\"" << *dyn_cast<Instruction>(v) << "\""
                           << " -> "
                           << "\"" << *inst << "\""
                           << ";\n";
                }
                if (v->getName() != "") {
                    errs() << "\"" << v->getName() << "\""
                           << " -> "
                           << "\"" << *inst << "\""
                           << ";\n";
                    errs() << "\"" << v->getName() << "\""
                           << " [ color = red ]\n";
                }
            }
        }
    }
    errs() << "\n}\n";
    return false;
}

namespace helpers {

string getOpcodeStr(unsigned int N) {
    switch (N) {
#define HANDLE_INST(N, OPCODE, CLASS) \
    case N:                           \
        return string(#OPCODE);

#include "llvm/IR/Instruction.def"

        default:
            llvm_unreachable("Unknown Instruction");
    }
}

char DFGPrinter::ID = 0;
}

bool DFGPrinter::doInitialization(Module &M) {
    dot.clear();
    return false;
}

void DFGPrinter::visitFunction(Function &F) {
    auto &nodes = this->nodes;

    auto checkCall = [](const Instruction &I, string name) -> bool {
        if (isa<CallInst>(&I) && dyn_cast<CallInst>(&I)->getCalledFunction() &&
            dyn_cast<CallInst>(&I)->getCalledFunction()->getName().startswith(
                name))
            return true;
        return false;
    };

    auto escape_quotes = [](const string &before) -> string {
        string after;
        after.reserve(before.length() + 4);

        for (string::size_type i = 0; i < before.length(); ++i) {
            switch (before[i]) {
                case '"':
                case '\\':
                    after += '\\';
                // Fall through.
                default:
                    after += before[i];
            }
        }
        return after;
    };

    auto subGraphFormat = [&escape_quotes](
        uint64_t id, string label, string color, string name) -> string {
        stringstream sstr;
        std::replace(name.begin(), name.end(), '.', '_');
        auto eir = escape_quotes(name);
        sstr << "    subgraph cluster" << label << id << " {\n"
                                                         "        label=\""
             << name << "\"\n";
        return sstr.str();
    };

    auto nodeFormat = [&escape_quotes](uint64_t id, string label, string color,
                                       string ir) -> string {
        stringstream sstr;
        auto eir = escape_quotes(ir);
        sstr << "        m_" << id << "[label=\"" << label << "\", opcode=\""
             << label << "\", color=" << color << ",ir=\"" << eir << "\"];\n";
        return sstr.str();
    };

    auto branchFormat = [&escape_quotes](uint64_t id, string label,
                                         uint64_t num_op, string color,
                                         string ir) -> string {
        stringstream sstr;
        string branch_label;

        auto eir = escape_quotes(ir);

        if (num_op == 1)
            branch_label = "\"{" + label + " \\l|{<s0>T}} \"";
        else
            branch_label = "\"{" + label + " \\l|{<s0>T|<s1>F}} \"";

        sstr << "        m_" << id
             << "[shape=\"record\", label=" << branch_label << ", opcode=\""
             << label << "\", color=" << color << ",ir=\"" << eir << "\"];\n";
        return sstr.str();
    };

    for (auto &ag : F.args()) {
        nodes[&ag] = counter_arg;

        dot << "    arg_" << counter_arg << "[label=\"ARG(" << counter_arg
            << ")\", color=green]\n";
        counter_arg++;
    }
    for (auto &BB : F) {
        nodes[&BB] = counter_bb++;
        dot << subGraphFormat(nodes[&BB], "BB", "red", BB.getName().str());

        for (auto &I : BB) {
            nodes[&I] = counter_ins++;
            std::string ir;
            llvm::raw_string_ostream rso(ir);
            I.print(rso);

            // If this does not exist in the node map
            // then create a new entry for it and save
            // the value of the counter (identifier).
            auto scalaLabel =
                cast<MDString>(I.getMetadata("ScalaLabel")->getOperand(0))
                    ->getString();

            if (isa<BranchInst>(&I)) {
                counter_ins++;
                dot << branchFormat(nodes[&I], scalaLabel.str(),
                                    I.getNumOperands(), "black", rso.str());
#ifdef TAPIR
            } else if (llvm::isa<llvm::DetachInst>(I) ||
                       llvm::isa<llvm::SyncInst>(I)) {
                counter_ins++;
                dot << branchFormat(nodes[&I], scalaLabel.str(),
                                    I.getNumOperands(), "black", rso.str());
#endif
            } else {
                counter_ins++;
                dot << nodeFormat(nodes[&I], scalaLabel.str(), "black",
                                  rso.str());
            }
        }

        dot << "    }\n";
    }
}

void DFGPrinter::visitBasicBlock(BasicBlock &BB) {}

void DFGPrinter::visitInstruction(Instruction &I) {
    // Filling operands container
    vector<Value *> Operands;
    if (isa<CallInst>(&I) || isa<InvokeInst>(&I)) {
        CallSite CS(&I);
        for (unsigned c = 0; c < CS.getNumArgOperands(); c++)
            Operands.push_back(CS.getArgument(c));
    } else {
        for (unsigned c = 0; c < I.getNumOperands(); c++)
            Operands.push_back(I.getOperand(c));
    }

    auto &nodes = this->nodes;

    auto escape_quotes = [](const string &before) -> string {
        string after;
        after.reserve(before.length() + 4);

        for (string::size_type i = 0; i < before.length(); ++i) {
            switch (before[i]) {
                case '"':
                case '\\':
                    after += '\\';
                // Fall through.
                default:
                    after += before[i];
            }
        }
        return after;
    };

    auto controlEdge = [&escape_quotes](uint64_t src_id, uint64_t dst_id,
                                        uint64_t port, string bb_lable,
                                        uint64_t bb_id, string color) {

        stringstream sstr;
        sstr << "    m_" << src_id << ":s" << port << " -> m_" << dst_id
             << "[color=red, style=dotted, lhead=\"cluster" << bb_lable << bb_id
             << "\"]\n";

        return sstr.str();
    };

    // auto BB = I.getParent();

    // Check if the instruction is branch
    // the label needs to have port then
    if (llvm::isa<llvm::BranchInst>(I)) {
        uint64_t br_counter = 0;
        for (auto OI : Operands) {
            std::string op;
            llvm::raw_string_ostream rso2(op);
            OI->print(rso2);

            if (isa<BasicBlock>(OI)) {
                auto t_bb = dyn_cast<BasicBlock>(OI);

                auto target_instruction = t_bb->getFirstNonPHI();

                dot << controlEdge(nodes[&I], nodes[target_instruction],
                                   br_counter, "BB", nodes[t_bb], "black");
                br_counter++;
            } else {
                dot << "    "
                    << "m_" << nodes[OI] << "->"
                    << "m_" << nodes[&I] << "[constraint=false];\n";
            }
        }
    }
#ifdef TAPIR
    else if (llvm::isa<llvm::DetachInst>(I) ||
             llvm::isa<llvm::ReattachInst>(I) || llvm::isa<llvm::SyncInst>(I)) {
        uint64_t br_counter = 0;
        for (auto OI : Operands) {
            std::string op;
            llvm::raw_string_ostream rso2(op);
            OI->print(rso2);

            if (isa<BasicBlock>(OI)) {
                auto t_bb = dyn_cast<BasicBlock>(OI);

                auto target_instruction = t_bb->getFirstNonPHI();

                dot << controlEdge(nodes[&I], nodes[target_instruction],
                                   br_counter, "BB", nodes[t_bb], "black");
                br_counter++;
            } else {
                dot << "    "
                    << "m_" << nodes[OI] << "->"
                    << "m_" << nodes[&I] << "[constraint=false];\n";
            }
        }
    }
#endif
    else {
        auto BB = I.getParent();

        for (auto OI : Operands) {
            std::string op;
            llvm::raw_string_ostream rso2(op);
            OI->print(rso2);

            if (isa<Argument>(OI)) {
                dot << "    "
                    << "arg_" << nodes[OI] << " ->"
                    << " m_" << nodes[&I]
                    << " [color=blue, constraint=false];\n";
            } else if (isa<Constant>(OI)) {
                if (auto cnt = dyn_cast<llvm::ConstantInt>(OI)) {
                    auto cnt_value = cnt->getSExtValue();
                    if (nodes.count(OI) == 0) {
                        nodes[OI] = cnt->getSExtValue();
                        dot << "    cnst_" << cnt->getSExtValue() << "[label=\""
                            << cnt->getSExtValue()
                            << "\", color=blue, constraint=false]\n";
                    }

                    dot << "    cnst_" << nodes[OI] << "->"
                        << "m_" << nodes[&I]
                        << " [color=green, constraint=false];\n";
                }
            } else if (isa<Instruction>(OI)) {
                dot << "    "
                    << "m_" << nodes[OI] << "->"
                    << "m_" << nodes[&I] << "[constraint=false];\n";
            } else {
                // TODO : This will break later when there are PHINodes
                // for chops.
                llvm_unreachable("unexpected");
            }
        }
    }
}

bool DFGPrinter::doFinalization(Module &M) { return false; }

bool DFGPrinter::runOnFunction(Function &F) {
    ofstream dotfile(("dfg." + F.getName() + ".dot").str().c_str(), ios::out);
    dot << "digraph G {\n";
    dot << "    compound=true;\n"
           "    labelloc=\"t\"\n";
    dot << "    label=\"" << F.getName().str() << "\"\n";
    dot << "    ranksep=1\n\n";
    visit(F);
    dot << "}\n";
    dotfile << dot.rdbuf();
    dotfile.close();
    return false;
}

// GepInformation Helper class
namespace helpers {

char GepInformation::ID = 0;
}

void GepInformation::visitGetElementPtrInst(llvm::GetElementPtrInst &I) {
    // Getting datalayout
    auto DL = I.getModule()->getDataLayout();
    uint32_t numByte = 0;
    uint64_t start_align = 0;
    uint64_t end_align = 0;
    std::vector<uint32_t> tmp_align;

    auto src_type = I.getSourceElementType();

    if (src_type->isStructTy()) {
        tmp_align.push_back(0);
        auto src_struct_type = dyn_cast<llvm::StructType>(src_type);
        for (auto _element : src_struct_type->elements()) {
            tmp_align.push_back(tmp_align.back() +
                                DL.getTypeAllocSize(_element));
        }

    } else if (src_type->isArrayTy()) {
        auto src_array_type = dyn_cast<llvm::ArrayType>(src_type);
        tmp_align.push_back(DL.getTypeAllocSize(src_array_type->getArrayElementType()));
        //tmp_align.push_back(DL.getTypeAllocSize(src_type));

        DEBUG(errs() << src_array_type->getArrayNumElements() << "\n");
        DEBUG(errs() << DL.getTypeAllocSize(src_array_type) << "\n");
        DEBUG(
            errs() << "Num byte: "
                   << DL.getTypeAllocSize(src_array_type->getArrayElementType())
                   << "\n");
    } else if (src_type->isIntegerTy() || src_type->isDoubleTy() || src_type->isFloatTy()) {
        tmp_align.push_back(DL.getTypeAllocSize(src_type));

    } else {
        // Dumping the instruction
        DEBUG(errs() << PURPLE("[DEBUG] "));
        DEBUG(I.print(errs(), true));
        DEBUG(errs() << "\n");
        DEBUG(src_type->print(errs(), true));
        assert(!"GepInformation pass doesn't support this type of input");
    }

    this->GepAddress.insert(
        std::pair<llvm::Instruction *, std::vector<uint32_t>>(&I, tmp_align));

    DEBUG(std::copy(tmp_align.begin(), tmp_align.end(),
                    std::experimental::make_ostream_joiner(std::cout, ", ")));
    DEBUG(std::cout << "\n");
    DEBUG(errs() << DL.getTypeAllocSize(src_type) << "\n");
}

// void GepInformation::visitGetElementPtrInst(llvm::GetElementPtrInst &I) {
// assert(I.getNumOperands() <= 3 &&
//"Gep with more than 2 operand is not supported");

//// Getting datalayout
// auto DL = I.getModule()->getDataLayout();
// uint32_t numByte = 0;
// uint64_t start_align = 0;
// uint64_t end_align = 0;

// auto src_type = I.getSourceElementType();
// if (src_type->isStructTy()) {
// auto src_struct_type = dyn_cast<llvm::StructType>(src_type);
// std::vector<uint32_t> tmp_align = {0};
// for (auto _element : src_struct_type->elements()) {
// tmp_align.push_back(tmp_align.back() +
// DL.getTypeAllocSize(_element));
//}

//// this->GepStruct[&I] = tmp_align;
// this->GepStruct.insert(
// std::pair<llvm::Instruction *, std::vector<uint32_t>>(&I,
// tmp_align));

// DEBUG(
// std::copy(tmp_align.begin(), tmp_align.end(),
// std::experimental::make_ostream_joiner(std::cout, ", ")));
// DEBUG(std::cout << "\n");
// DEBUG(errs() << DL.getTypeAllocSize(src_type) << "\n");

//} else if (src_type->isArrayTy()) {
// auto src_array_type = dyn_cast<llvm::ArrayType>(src_type);
// this->GepArray.insert(
// std::pair<llvm::Instruction *, common::GepArrayInfo>(
//&I,
// common::GepArrayInfo(
// DL.getTypeAllocSize(src_array_type->getArrayElementType()),
// src_array_type->getNumElements())));

// DEBUG(errs() << src_array_type->getArrayNumElements() << "\n");
// DEBUG(errs() << DL.getTypeAllocSize(src_array_type) << "\n");
// DEBUG(
// errs() << "Num byte: "
//<< DL.getTypeAllocSize(src_array_type->getArrayElementType())
//<< "\n");
//} else if (src_type->isIntegerTy()) {
// auto src_integer_type = dyn_cast<llvm::IntegerType>(src_type);
// this->GepArray.insert(
// std::pair<llvm::Instruction *, common::GepArrayInfo>(
//&I, common::GepArrayInfo(DL.getTypeAllocSize(src_integer_type),
// 1)));

//} else if (src_type->isDoubleTy()) {
// auto src_double_type = dyn_cast<llvm::Type>(src_type);
// this->GepArray.insert(
// std::pair<llvm::Instruction *, common::GepArrayInfo>(
//&I,
// common::GepArrayInfo(DL.getTypeAllocSize(src_double_type), 1)));

//} else {
//// Dumping the instruction
// DEBUG(errs() << PURPLE("[DEBUG] "));
// DEBUG(I.print(errs(), true));
// DEBUG(errs() << "\n");
// DEBUG(src_type->print(errs(), true));
// assert(!"GepInformation pass doesn't support this type of input");
//}
//}

bool GepInformation::runOnModule(Module &M) {
    for (auto &ff : M) {
        if (ff.getName() == this->function_name) visit(&ff);
    }
    return false;
}

//namespace helpers {

//char GEPAddrCalculation::ID = 0;
//}

//void GEPAddrCalculation::visitSExtInst(Instruction &I) {
    //// Getting datalayout
    //auto DL = I.getModule()->getDataLayout();

    //auto op = dyn_cast<llvm::CastInst>(&I);
    //DEBUG(dbgs() << DL.getTypeAllocSize(op->getSrcTy()) * 8 << "\n");
    //DEBUG(dbgs() << DL.getTypeAllocSize(op->getDestTy()) * 8 << "\n");
//}

//void GEPAddrCalculation::visitGetElementPtrInst(llvm::GetElementPtrInst &I) {
    //assert(I.getNumOperands() <= 3 &&
           //"Gep with more than 2 operand is not supported");

    //// Dumping the instruction
    //DEBUG(I.print(errs(), true));

    //// Getting datalayout
    //auto DL = I.getModule()->getDataLayout();
    //uint32_t numByte = 0;
    //uint64_t start_align = 0;
    //uint64_t end_align = 0;

    //auto src_type = I.getSourceElementType();
    //auto tar_type = I.getResultElementType();
    //DEBUG(src_type->print(errs(), true));
    //if (src_type->isStructTy()) {
        //auto src_struct_type = dyn_cast<llvm::StructType>(src_type);
        //for (auto _element : src_struct_type->elements()) {
            //errs() << "Num byte: " << DL.getTypeAllocSize(_element) << "\n";
        //}
    //} else if (src_type->isArrayTy()) {
        //auto src_array_type = dyn_cast<llvm::ArrayType>(src_type);
        //errs() << src_array_type->getArrayNumElements() << "\n";
        //errs() << "Num byte: "
               //<< DL.getTypeAllocSize(src_array_type->getArrayElementType())
               //<< "\n";
    //}
//}

//bool GEPAddrCalculation::runOnModule(Module &M) {
    //for (auto &ff : M) {
        //if (ff.getName() == this->function_name) visit(&ff);
    //}
    //return false;
//}

namespace helpers {

char InstCounter::ID = 0;
// static RegisterPass<pdgDump> X(
//"InstructionCounter", "Counting number of instructions at each BasicBlock");
}

bool InstCounter::doInitialization(Module &M) { return false; }

bool InstCounter::doFinalization(Module &M) { return false; }

bool InstCounter::runOnModule(Module &M) {
    for (auto &ff : M) {
        if (ff.getName() == this->function_name) {
            for (auto &bb : ff) {
                for (auto &Ins : bb) {
                    CallSite CS(&Ins);
                    if (CS.isCall()) continue;
                    this->BasicBlockCnt[&bb]++;
                }
            }
        }
    }
    return false;
}

/**
 * Function lists
 */
void helpers::FunctionUIDLabel(Function &FF) {
    legacy::FunctionPassManager FPM(FF.getParent());
    FPM.add(new helpers::LabelUID());
    FPM.doInitialization();
    FPM.run(FF);
    FPM.doFinalization();
}

void helpers::printDFG(Function &F) {
    legacy::FunctionPassManager FPM(F.getParent());
    FPM.add(new helpers::DFGPrinter());
    FPM.doInitialization();
    FPM.run(F);
    FPM.doFinalization();
}

void helpers::UIDLabel(Function &F) {
    legacy::FunctionPassManager FPM(F.getParent());
    FPM.add(new helpers::LabelUID());
    FPM.doInitialization();
    FPM.run(F);
    FPM.doFinalization();
}

void helpers::PDGPrinter(Function &F) {
    legacy::FunctionPassManager FPM(F.getParent());
    FPM.add(new pdgDump());
    FPM.doInitialization();
    FPM.run(F);
    FPM.doFinalization();
}

bool helpers::helperReplace(std::string &str, const std::string &from,
                            const std::string &to) {
    assert(!from.compare(0, 1, "$") && "Replace string should start with $!");
    bool _ret = false;
    while (true) {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos) break;
        str.replace(start_pos, from.length(), to);
        _ret = true;
    }
    return _ret;
}

bool helpers::helperReplace(std::string &str, const std::string &from,
                            std::vector<const std::string> &to,
                            const std::string &split) {
    assert(!from.compare(0, 1, "$") && "Replace string should start with $!");
    bool _ret = false;
    return _ret;
}

bool helpers::helperReplace(std::string &str, const std::string &from,
                            std::vector<uint32_t> to,
                            const std::string &split) {
    assert(!from.compare(0, 1, "$") && "Replace string should start with $!");
    bool _ret = false;
    std::stringstream test;
    std::copy(to.begin(), to.end(),
              std::experimental::make_ostream_joiner(test, split));

    while (true) {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos) break;
        str.replace(start_pos, from.length(), test.str());
        _ret = true;
    }
    return _ret;
}

bool helpers::helperReplace(std::string &str, const std::string &from,
                            std::list<std::pair<uint32_t, uint32_t>> &to,
                            const std::string &split) {
    assert(!from.compare(0, 1, "$") && "Replace string should start with $!");
    bool _ret = false;
    std::stringstream test;
    for (auto &node : to) {
        test << "(" << std::to_string(node.first) << ", "
             << std::to_string(node.second) << ") , ";
    }
    // Remove last three additional characters
    string _replace_string = test.str().substr(0, test.str().size() - 3);

    // replace all the existing substring
    while (true) {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos) break;
        str.replace(start_pos, from.length(), _replace_string);
        _ret = true;
    }
    return _ret;
}

bool helpers::helperReplace(std::string &str, const std::string &from,
                            const uint32_t to) {
    assert(!from.compare(0, 1, "$") && "Replace string should start with $!");
    bool _ret = false;
    while (true) {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos) break;
        str.replace(start_pos, from.length(), std::to_string(to));
        _ret = true;
    }
    return _ret;
}

bool helpers::helperReplace(std::string &str, const std::string &from,
                            const int to) {
    assert(!from.compare(0, 1, "$") && "Replace string should start with $!");
    bool _ret = false;
    while (true) {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos) break;
        str.replace(start_pos, from.length(), std::to_string(to));
        _ret = true;
    }
    return _ret;
}

namespace helpers {
// LabelUID Helper Class
char CallInstSpliter::ID = 0;

RegisterPass<CallInstSpliter> W(
    "CallInstSpliter", "Spliting basic blocks after each call function");
}

bool CallInstSpliter::doInitialization(Module &M) { return false; }

bool CallInstSpliter::doFinalization(Module &M) { return false; }

bool CallInstSpliter::runOnModule(Module &M) {
    for (auto &ff : M) {
        if (ff.getName() == this->function_name) visit(&ff);
        for (auto &_call : this->call_container) {
            auto _bb = _call->getParent();
            if (_call != &_bb->back() &&
                !(isa<llvm::BranchInst>(_call->getNextNode()) ||
                  isa<llvm::ReattachInst>(_call->getNextNode()))) {
                _bb->splitBasicBlock(_call->getNextNode(), "bb_contine");
            }
        }
    }
    return true;
}

void CallInstSpliter::visitCallInst(CallInst &I) {
    if (I.getCalledFunction()->isDeclaration()) return;
    this->call_container.push_back(&I);
}
