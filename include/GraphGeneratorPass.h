#ifndef GRAPHGENERATORPASS_H
#define GRAPHGENERATORPASS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "json/json.h"

#include "Common.h"
#include "NodeType.h"

#include "Dandelion/Edge.h"
#include "Dandelion/Graph.h"
#include "Dandelion/Node.h"

#include <map>
#include <set>
#include <string>
#include <vector>

using namespace dandelion;

using InstructionList = std::list<InstructionNode>;
using ArgumentList = std::list<ArgumentNode>;
using BasicBlockList = std::list<SuperNode>;
using GlobalValueList = std::list<GlobalValueNode>;
using ConstIntList = std::list<ConstIntNode>;
using EdgeList = std::list<Edge>;

namespace graphgen {

class GraphGeneratorPass : public llvm::ModulePass,
                           public llvm::InstVisitor<GraphGeneratorPass> {
    friend class InstVisitor<GraphGeneratorPass>;

   public:
    std::unique_ptr<Graph> dependency_graph;

   private:
    std::map<llvm::Value *, Node *> map_value_node;

    // Loop Info
    llvm::LoopInfo *LI;

    // Default value is standard out
    llvm::raw_ostream &code_out;

    // NOTE: Uncomment if there is any dependent analysis
    //virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

    virtual bool doInitialization(llvm::Module &M) override;
    virtual bool doFinalization(llvm::Module &M) override;

    void init(llvm::Function &);

    void visitFunction(llvm::Function &);
    void visitBasicBlock(llvm::BasicBlock &);
    void visitInstruction(llvm::Instruction &);
    void visitBinaryOperator(llvm::BinaryOperator &);
    void visitICmpInst(llvm::ICmpInst &);
    void visitBranchInst(llvm::BranchInst &);
    void visitPHINode(llvm::PHINode &);
    void visitAllocaInst(llvm::AllocaInst &);
    void visitGetElementPtrInst(llvm::GetElementPtrInst &);
    void visitLoadInst(llvm::LoadInst &);
    void visitStoreInst(llvm::StoreInst &);
    void visitReturnInst(llvm::ReturnInst &);
    void visitCallInst(llvm::CallInst &);

    void fillBasicBlockDependencies(llvm::Function &);
    void fillLoopDependencies(llvm::LoopInfo &);
    void findDataPort(llvm::Function &);
    void connectOutToReturn(llvm::Function &);
    void connectParalleNodes(llvm::Function &);

    void buildingGraph();

   public:
    static char ID;

    GraphGeneratorPass()
        : llvm::ModulePass(ID),
          dependency_graph(std::make_unique<Graph>(NodeInfo(0, "dummy"))),
          code_out(llvm::outs()) {}
    GraphGeneratorPass(NodeInfo _n_info)
        : llvm::ModulePass(ID),
          dependency_graph(std::make_unique<Graph>(_n_info)),
          code_out(llvm::outs()) {}

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
    //virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
        //AU.addRequired<LoopInfoWrapperPass>();
        //AU.addRequired<helpers::GEPAddrCalculation>();
        //AU.setPreservesAll();
    //}

    // virtual bool runOnFunction(llvm::Function &) override;
    virtual bool runOnModule(llvm::Module &m) override;
};
}  // namespace graphgen

#endif
