#define DEBUG_TYPE "data_common"

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
#include <fstream>

#include "Common.h"

using namespace llvm;

using helpers::LabelUID;
using helpers::pdgDump;
using helpers::DFGPrinter;
using helpers::GEPAddrCalculation;

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
        sstr << "        m_" << id << "[label=\"" << label << "(" << id
             << ")\", opcode=\"" << label << "\", color=" << color << ",ir=\""
             << eir << "\"];\n";
        return sstr.str();
    };

    auto branchFormat = [&escape_quotes](uint64_t id, string label,
                                         uint64_t num_op, string color,
                                         string ir) -> string {
        stringstream sstr;
        string branch_label;

        auto eir = escape_quotes(ir);

        if (num_op == 1)
            branch_label =
                "\"{" + label + "(" + std::to_string(id) + ") \\l|{<s0>T}} \"";
        else
            branch_label = "\"{" + label + "(" + std::to_string(id) +
                           ") \\l|{<s0>T|<s1>F}} \"";

        sstr << "        m_" << id
             << "[shape=\"record\", label=" << branch_label << ", opcode=\""
             << label << "\", color=" << color << ",ir=\"" << eir << "\"];\n";
        return sstr.str();
    };

    for (auto &ag : F.getArgumentList()) {
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
            if (isa<BranchInst>(&I)) {
                counter_ins++;
                dot << branchFormat(nodes[&I], getOpcodeStr(I.getOpcode()),
                                    I.getNumOperands(), "black", rso.str());
            } else {
                counter_ins++;
                dot << nodeFormat(nodes[&I], getOpcodeStr(I.getOpcode()),
                                  "black", rso.str());
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

    // Check if the insturction is branch
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
            }
            else{
                dot << "    "
                    << "m_" << nodes[OI] << "->"
                    << "m_" << nodes[&I] << ";\n";
            }
        }
    }
    //else if(isa<llvm::PHINode>(I)){
        //uint32_t op_counter = 0;
        //for(auto OI : Operands){
           //if (isa<Instruction>(OI)){
                //dot << "    "
                    //<< "m_" << nodes[OI] << "->"
                    //<< "m_" << nodes[&I] << ";\n";
            //}
        //}
    //}
    else {
        auto BB = I.getParent();

        for (auto OI : Operands) {
            std::string op;
            llvm::raw_string_ostream rso2(op);
            OI->print(rso2);

            if (isa<Argument>(OI)) {
                dot << "    "
                    << "arg_" << nodes[OI] << " ->"
                    << " m_" << nodes[&I] << " [color=blue];\n";
            } else if (isa<Constant>(OI)) {
                auto cnt = dyn_cast<llvm::ConstantInt>(OI);
                auto cnt_value = cnt->getSExtValue();

                if (nodes.count(OI) == 0) {
                    nodes[OI] = cnt->getSExtValue();
                    dot << "    cnst_" << cnt->getSExtValue() << "[label=\""
                        << cnt->getSExtValue() << "\", color=blue]\n";
                }

                dot << "    cnst_" << nodes[OI] << "->"
                    << "m_" << nodes[&I] << " [color=green];\n";
            } else if (isa<Instruction>(OI)){
                dot << "    "
                    << "m_" << nodes[OI] << "->"
                    << "m_" << nodes[&I] << ";\n";
            }
            else {
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
    dot << "    label=\"" << F.getName().str() << "\"\n\n";
    visit(F);
    dot << "}\n";
    dotfile << dot.rdbuf();
    dotfile.close();
    return false;
}

// GEPAddrCalculation Helper class
namespace helpers {

char GEPAddrCalculation::ID = 0;
}

void GEPAddrCalculation::visitSExtInst(Instruction &I) {
    // Getting datalayout
    auto DL = I.getModule()->getDataLayout();

    auto op = dyn_cast<llvm::CastInst>(&I);
    errs() << DL.getTypeAllocSize(op->getSrcTy()) * 8 << "\n";
    errs() << DL.getTypeAllocSize(op->getDestTy()) * 8 << "\n";
}

void GEPAddrCalculation::visitGetElementPtrInst(Instruction &I) {
    assert(I.getNumOperands() <= 3 &&
           "Gep with more than 2 operand is not supported");

    // Dumping the instruction
    DEBUG(I.dump());

    // Getting datalayout
    auto DL = I.getModule()->getDataLayout();
    uint32_t numByte = 0;
    uint64_t start_align = 0;
    uint64_t end_align = 0;
    llvm::Type *op;

    for (uint32_t c = 0; c < I.getNumOperands(); c++) {
        start_align = end_align;

        // First operand is the pointer the variable
        if (c == 0) {
            op = I.getOperand(c)->getType()->getPointerElementType();
            // outs() << *I.getOperand(c)->getType()->getPointerElementType()
            //<< "\n";

            // Index zero is pointer type
            // It can be either struct or constant
            if (op->isStructTy()) {
                auto struct_op = dyn_cast<llvm::StructType>(op);
                numByte = DL.getTypeAllocSize(struct_op);
                // outs() << "Size: " << numByte << "\n";
            } else if (op->isArrayTy()) {
                auto array_op = dyn_cast<llvm::ArrayType>(op);
                numByte = DL.getTypeAllocSize(array_op);
                // outs() << "Size: " << numByte << "\n";

            } else if (op->isFloatTy()) {
                numByte = DL.getTypeAllocSize(op);
                // outs() << "Size: " << numByte << "\n";
            } else if (op->isIntegerTy()) {
                numByte = DL.getTypeAllocSize(op);
                // outs() << "Size: " << numByte << "\n";
            } else if (op->isPointerTy()) {
                // TODO Fix the pointer computation
                numByte = DL.getTypeAllocSize(op);
                // assert(!"DETECT");
            }
        } else {
            auto op_type = I.getOperand(c)->getType();

            auto value = dyn_cast<llvm::ConstantInt>(I.getOperand(c));
            if (op_type->isIntegerTy() && value) {
                // outs() << "Value: " << value->getSExtValue() << "\n";

                if (c == 2) {
                    if (op->isStructTy()) {
                        auto struct_op = dyn_cast<llvm::StructType>(op);
                        for (uint32_t i = 0; i <= value->getSExtValue(); i++) {
                            uint64_t size = 0;

                            auto operand = struct_op->getStructElementType(i);

                            if (operand->isArrayTy()) {
                                auto op_array =
                                    dyn_cast<llvm::ArrayType>(operand);
                                auto array_size = DL.getTypeAllocSize(operand);

                                size = DL.getTypeAllocSize(operand);

                                auto op_element =
                                    op_array->getArrayElementType();
                                while (op_element->isArrayTy()) {
                                    op_element =
                                        op_element->getArrayElementType();
                                }

                                auto elem_size =
                                    DL.getTypeAllocSize(op_element);
                                /**
                                 * If the struct's element is an array we need
                                 * to:
                                 * 1. Align the begining element size
                                 * 2. Compute the array's size for end of the
                                 * alignment
                                 */
                                if (i == 0)
                                    end_align += array_size - 1;
                                else {
                                    start_align =
                                        (int)ceil((float)(end_align + 1) /
                                                  (float)elem_size) *
                                        elem_size;
                                    end_align = start_align + array_size - 1;
                                }

                            } else if (operand->isStructTy()) {
                                /**
                                 * If the struct's element is struct:
                                 * 1. Align the begining with itself
                                 * 2. Compute the size
                                 */
                                size = DL.getTypeAllocSize(operand);

                            } else if (operand->isIntegerTy()) {
                                /**
                                 * If the struct's element is scala we only need
                                 * to:
                                 * 1. Align the begining with itself
                                 * 2. Compute the size
                                 */
                                size = DL.getTypeAllocSize(operand);
                                if (i == 0)
                                    end_align += size - 1;
                                else {
                                    start_align =
                                        (int)ceil((float)(end_align + 1) /
                                                  (float)size) *
                                        size;
                                    end_align = start_align + size - 1;
                                }

                            } else if (operand->isFloatTy()) {
                                /**
                                 * If the struct's element is scala we only need
                                 * to:
                                 * 1. Align the begining with itself
                                 * 2. Compute the size
                                 */
                                size = DL.getTypeAllocSize(operand);
                                if (i == 0)
                                    end_align += size - 1;
                                else {
                                    start_align =
                                        (int)ceil((float)(end_align + 1) /
                                                  (float)size) *
                                        size;
                                    end_align = start_align + size - 1;
                                }
                            } else if (operand->isPointerTy()) {
                                /**
                                 * If the struct's element is scala we only need
                                 * to:
                                 * 1. Align the begining with itself
                                 * 2. Compute the size
                                 */
                                size = DL.getTypeAllocSize(operand);
                                if (i == 0)
                                    end_align += size - 1;
                                else {
                                    start_align =
                                        (int)ceil((float)(end_align + 1) /
                                                  (float)size) *
                                        size;
                                    end_align = start_align + size - 1;
                                }
                            } else {
                                outs() << *operand << "\n";
                                I.dump();
                                assert(!"Not supported type!\n");
                            }
                        }
                        // outs() << "Alignment start: " << start_align << "\n";
                        // outs() << "Alignment end  : " << end_align << "\n";
                    } else if (op->isArrayTy()) {
                        auto array_op = dyn_cast<llvm::ArrayType>(op);
                        auto array_size = DL.getTypeAllocSize(array_op);
                        auto array_elem_size =
                            DL.getTypeAllocSize(array_op->getElementType());

                        start_align = array_elem_size * value->getSExtValue();
                        // outs() << "Alignment: " << start_align << "\n";
                    }
                }
            }
        }
    }

    // Filling the containers
    if (I.getNumOperands() == 2) {
        auto value_int = dyn_cast<llvm::ConstantInt>(I.getOperand(1));
        auto value_fp = dyn_cast<llvm::ConstantFP>(I.getOperand(1));
        if (value_int) {
            common::GepOne tmp_gep = {value_int->getSExtValue(), numByte};
            SingleGepIns[&I] = tmp_gep;
        }
    } else if (I.getNumOperands() == 3) {
        auto value1 = dyn_cast<llvm::ConstantInt>(I.getOperand(1));
        auto value2 = dyn_cast<llvm::ConstantInt>(I.getOperand(2));
        if (value1 && value2) {
            common::GepTwo tmp_gep = {value1->getSExtValue(), numByte,
                                      value2->getSExtValue(),
                                      static_cast<int64_t>(start_align)};
            TwoGepIns[&I] = tmp_gep;
        }
    }
}

bool GEPAddrCalculation::runOnModule(Module &M) {
    for (auto &ff : M) {
        if (ff.getName() == this->function_name) visit(&ff);
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
