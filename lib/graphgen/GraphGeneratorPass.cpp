#define DEBUG_TYPE "lx"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include <iostream>

#include "Dandelion/Node.h"
#include "GraphGeneratorPass.h"

using namespace llvm;
using namespace std;
using namespace graphgen;
using namespace dandelion;

using InstructionList = std::list<InstructionNode>;
using ArgumentList = std::list<ArgumentNode>;
using BasicBlockList = std::list<SuperNode>;

extern cl::opt<string> XKETCHName;

namespace graphgen {

char GraphGeneratorPass::ID = 0;

RegisterPass<GraphGeneratorPass> X("graphgen", "Generating xketch graph");
}

bool GraphGeneratorPass::doInitialization(Module &M) {
    // TODO: Add code here if it's needed before pas
    return false;
}

bool GraphGeneratorPass::doFinalization(Module &M) {
    cout << "Number of instruction nodes: " << instruction_list.size() << endl;
    // TODO: Add code here to do post pass
    return false;
}

void GraphGeneratorPass::visitBasicBlock(BasicBlock &BB) {
    // TODO find all the basicblock dependencies
    // TODO SmallVector should be type of Node
    SmallVector<Instruction *, 16> _ins_vector;
    for (auto &ins : BB) {
        _ins_vector.push_back(&ins);
    }
    SuperNode new_super_node(_ins_vector, &BB);

    this->super_node_list.push_back(new_super_node);
}


void GraphGeneratorPass::visitInstruction(Instruction &Ins) {
    Ins.dump();
    assert(!"Instruction is not supported");
}

void GraphGeneratorPass::visitBinaryOperator(llvm::BinaryOperator &I) {
    this->instruction_list.push_back(
        BinaryOperatorNode(&I, BinaryInstructionTy));
}

void GraphGeneratorPass::visitICmpInst(llvm::ICmpInst &I) {
    this->instruction_list.push_back(IcmpNode(&I, IcmpInstructionTy));
}

void GraphGeneratorPass::visitBranchInst(llvm::BranchInst &I) {
    this->instruction_list.push_back(BranchNode(&I, BranchInstructionTy));
}

void GraphGeneratorPass::visitPHINode(llvm::PHINode &I) {
    this->instruction_list.push_back(PHIGNode(&I, PhiInstructionTy));
}

void GraphGeneratorPass::visitAllocaInst(llvm::AllocaInst &I) {
    this->instruction_list.push_back(AllocaNode(&I, AllocaInstructionTy));
}

void GraphGeneratorPass::visitGetElementPtrInst(llvm::GetElementPtrInst &I) {
    this->instruction_list.push_back(GEPNode(&I, GetElementPtrInstTy));
}

void GraphGeneratorPass::visitLoadInst(llvm::LoadInst &I) {
    this->instruction_list.push_back(LoadNode(&I, LoadInstructionTy));
}

void GraphGeneratorPass::visitStoreInst(llvm::StoreInst &I) {
    this->instruction_list.push_back(StoreNode(&I, StoreInstructionTy));
}

void GraphGeneratorPass::visitReturnInst(llvm::ReturnInst &I) {
    this->instruction_list.push_back(ReturnNode(&I, ReturnInstrunctionTy));
}

void GraphGeneratorPass::visitCallInst(llvm::CallInst &I) {
    this->instruction_list.push_back(CallNode(&I, CallInstructionTy));
}


void GraphGeneratorPass::visitFunction(Function &F) {
    // TODO
    // Here we make a graph
    // Graph gg()
    //
    // Filling function argument nodes
    for (auto &f_arg : F.getArgumentList()) 
        this->argument_list.push_back(ArgumentNode(&f_arg));

    // Filling global variables
    for (auto &g_var : F.getParent()->getGlobalList())
        this->glob_list.push_back(GlobalValueNode(&g_var));


}

bool GraphGeneratorPass::runOnFunction(Function &F) {
    visit(F);

    // std::vector<SuperNode *> Graph;
    // InstructionList tmp_ll;
    // for (auto &BB : F) {
    //// Filling instruction container for each BasicBlock
    //// XXX The follwoing code doesn't work because of explicit
    //// conversions
    //// SmallVector<Instruction *, 16> tmp(BB.begin(), BB.end());

    // SmallVector<Instruction *, 16> tmp;
    // for (auto &ins : BB) {
    // tmp.push_back(&ins);
    // tmp_ll.push_back(InstructionNode(&ins));
    //}
    // SuperNode tmp_bb(tmp, &BB);
    //}

    return false;
}
