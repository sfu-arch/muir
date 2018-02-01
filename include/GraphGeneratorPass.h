

#ifndef STATICCALLCOUNTER_H
#define STATICCALLCOUNTER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Common.h"
#include "NodeType.h"

#include "Dandelion/Node.h"

#include <map>
#include <set>
#include <string>
#include <vector>

using namespace dandelion;

using InstructionList = std::list<InstructionNode>;
using ArgumentList = std::list<ArgumentNode>;
using BasicBlockList = std::list<SuperNode>;

namespace graphgen {

class GraphGeneratorPass : public llvm::ModulePass,
                           public llvm::InstVisitor<GraphGeneratorPass> {
    friend class InstVisitor<GraphGeneratorPass>;

    // Maintaining supernode list
    BasicBlockList super_node_list;
    InstructionList instruction_list;

    // Default value is standard out
    llvm::raw_ostream &code_out;

    // NOTE: Uncomment if there is any dependent analysis
    // virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

    virtual bool doInitialization(llvm::Module &M) override;
    virtual bool doFinalization(llvm::Module &M) override;

    void visitFunction(llvm::Function &F);
    void visitBasicBlock(llvm::BasicBlock &BB);
    void visitInstruction(llvm::Instruction &I);

   public:
    static char ID;

    GraphGeneratorPass() : llvm::ModulePass(ID), code_out(llvm::outs()) {}

    virtual bool runOnModule(llvm::Module &m) override;
};
}

#endif
