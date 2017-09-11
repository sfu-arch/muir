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
using namespace helpers;

// LabelUID Helper Class
char LabelUID::ID = 0;

extern cl::list<std::string> FunctionList;

extern bool isTargetFunction(const Function &f,
                             const cl::list<std::string> &FunctionList);

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

void LabelUID::visitFunction(Function &F) { visitGeneric<Function>("UID", F); }

void LabelUID::visitInstruction(Instruction &I) {
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

char DFGPrinter::ID = 0;

bool DFGPrinter::doInitialization(Module &M) {
    dot.clear();
    return false;
}

void DFGPrinter::visitFunction(Function &F) {}

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

void DFGPrinter::visitBasicBlock(BasicBlock &BB) {
    // DEBUG(outs() << "ENTER! \n");
    auto checkCall = [](const Instruction &I, string name) -> bool {
        if (isa<CallInst>(&I) && dyn_cast<CallInst>(&I)->getCalledFunction() &&
            dyn_cast<CallInst>(&I)->getCalledFunction()->getName().startswith(
                name))
            return true;
        return false;
    };

    auto &nodes = this->nodes;

    auto insertNode = [&nodes](Value *V, uint64_t counter) {
        nodes[V] = counter;
        if (isa<BasicBlock>(V)) {
            if (auto *N = dyn_cast<BasicBlock>(V)->getTerminator()->getMetadata(
                    "BB_UID")) {
                auto *S = dyn_cast<MDString>(N->getOperand(0));
                auto id = stoi(S->getString().str());
                nodes[V] = id;
                return true;
            } else {
                return false;
            }
        } else if (isa<Instruction>(V)) {
            if (auto *N = dyn_cast<Instruction>(V)->getMetadata("UID")) {
                auto *S = dyn_cast<MDString>(N->getOperand(0));
                auto id = stoi(S->getString().str());
                nodes[V] = id;
                return true;
            } else {
                return false;
            }
        }
        // Removed the unreachable check since Args and Constants will
        // end up here and thats ok.
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

    auto nodeFormat = [&escape_quotes](uint64_t id, string label, string color,
                                       string ir) -> string {
        stringstream sstr;
        auto eir = escape_quotes(ir);
        sstr << id << " [label=\"" << label << "(" << id << ")\", opcode=\""
             << label << "\", color=" << color << ",ir=\"" << eir << "\"];\n";
        return sstr.str();
    };

    if (nodes.count(&BB) == 0) {
        bool success = insertNode(&BB, counter);
        if (!success) return;
        // nodes.insert(make_pair(&BB, counter));
        counter++;
        dot << nodeFormat(nodes[&BB], "BB", "red", BB.getName().str());
    }
    auto BBId = nodes[&BB];

    for (auto &I : BB) {
        if (checkCall(I, "llvm.dbg")) continue;

        std::string ir;
        llvm::raw_string_ostream rso(ir);
        I.print(rso);

        // If this does not exist in the node map
        // then create a new entry for it and save
        // the value of the counter (identifier).
        if (nodes.count(&I) == 0) {
            bool success = insertNode(&I, counter);
            if (!success) continue;
            // nodes.insert(make_pair(&I, counter));
            counter++;
            if (checkCall(I, "__guard_func")) {
                dot << nodeFormat(nodes[&I], "G", "red", rso.str());
            } else {
                dot << nodeFormat(nodes[&I], getOpcodeStr(I.getOpcode()),
                                  "black", rso.str());
            }
        }

        vector<Value *> Operands;
        if (isa<CallInst>(&I) || isa<InvokeInst>(&I)) {
            CallSite CS(&I);
            for (unsigned c = 0; c < CS.getNumArgOperands(); c++)
                Operands.push_back(CS.getArgument(c));
        } else {
            for (unsigned c = 0; c < I.getNumOperands(); c++)
                Operands.push_back(I.getOperand(c));
        }

        for (auto OI : Operands) {
            std::string op;
            llvm::raw_string_ostream rso2(op);
            OI->print(rso2);
            if (nodes.count(OI) == 0) {
                insertNode(OI, counter);
                counter++;
                if (isa<Argument>(OI)) {
                    dot << nodeFormat(nodes[OI], "Arg", "blue", rso2.str());
                    dot << nodes[OI] << "->" << nodes[&I] << " [color=blue];\n";
                } else if (isa<Constant>(OI)) {
                    dot << nodeFormat(nodes[OI], "Const", "green", rso2.str());
                    dot << nodes[OI] << "->" << nodes[&I]
                        << " [color=green];\n";
                } else if (isa<BasicBlock>(OI)) {
                    dot << nodeFormat(nodes[OI], "BB", "green",
                                      BB.getName().str());
                    dot << nodes[&I] << "->" << nodes[OI] << " [color=red];\n";
                } else {
                    // TODO : This will break later when there are PHINodes
                    // for chops.
                    llvm_unreachable("unexpected");
                }
            } else {
                dot << nodes[OI] << "->" << nodes[&I] << ";\n";
            }
        }
        // Every Instruction is control depedent on its BB_START
        dot << BBId << "->" << nodes[&I] << " [style=dotted];\n";
    }
}

void DFGPrinter::visitInstruction(Instruction &I) {}

bool DFGPrinter::doFinalization(Module &M) { return false; }

bool DFGPrinter::runOnFunction(Function &F) {
    ofstream dotfile(("dfg." + F.getName() + ".dot").str().c_str(), ios::out);
    dot << "digraph G {\n";
    visit(F);
    dot << "}\n";
    dotfile << dot.rdbuf();
    dotfile.close();
    return false;
}
