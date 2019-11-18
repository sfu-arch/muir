#ifndef GRAPHGENERATORPASS_H
#define GRAPHGENERATORPASS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#ifdef __APPLE__
#include "json/json.h"
#else
#include "jsoncpp/json/json.h"
#endif

#include "Common.h"
#include "NodeType.h"

#include "AliasEdgeWriter.h"
//#include "LoopClouser.h"

#include "Dandelion/Edge.h"
#include "Dandelion/Graph.h"
#include "Dandelion/Node.h"

#include <map>
#include <queue>
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

struct LoopSummary {
    std::string loop_name;
    // Control information
    llvm::Instruction *enable;
    llvm::Instruction *loop_back;
    std::set<llvm::Instruction *> loop_finish;

    llvm::BasicBlock *header;
    llvm::SmallVector<llvm::BasicBlock *, 8> exit_blocks;

    llvm::SetVector<llvm::Loop *> sub_loops;

    // Data information
    //
    // Live-in
    // Loop -> ins (output)
    llvm::DenseMap<llvm::Value *, llvm::SmallSetVector<llvm::Instruction *, 8>>
        live_in_out_ins;

    // Loop -> Loop (output)
    llvm::DenseMap<llvm::Value *, llvm::SmallSetVector<llvm::Loop *, 8>>
        live_in_out_loop;

    // Loop -> ins (input)
    llvm::SmallSetVector<llvm::Value *, 8> live_in_in_ins;

    // Loop -> Loop (input)
    llvm::DenseMap<llvm::Value *, llvm::SmallSetVector<llvm::Loop *, 8>>
        live_in_in_loop;

    // Data information
    //
    // Live-out
    // Loop -> ins (output)
    llvm::DenseMap<llvm::Value *, llvm::SmallSetVector<llvm::Instruction *, 8>>
        live_out_out_ins;

    // Loop -> Loop (output)
    llvm::DenseMap<llvm::Value *, llvm::SmallSetVector<llvm::Loop *, 8>>
        live_out_out_loop;

    // Loop -> ins (input)
    llvm::SmallSetVector<llvm::Value *, 8> live_out_in_ins;

    // Loop -> Loop (input)
    llvm::DenseMap<llvm::Value *, llvm::SmallSetVector<llvm::Loop *, 8>>
        live_out_in_loop;

    // Data information
    //
    // Carry-depen
    llvm::DenseMap<llvm::Value *, llvm::SmallVector<llvm::Instruction *, 8>>
        carry_dependencies;

    std::string getNmae() { return loop_name; }

    LoopSummary() : loop_name("No_Name"), enable(nullptr), loop_back(nullptr) {}
    LoopSummary(std::string name)
        : loop_name(name), enable(nullptr), loop_back(nullptr) {}
};

class GraphGeneratorPass : public llvm::ModulePass,
                           public llvm::InstVisitor<GraphGeneratorPass> {
    friend class InstVisitor<GraphGeneratorPass>;

   public:
    std::unique_ptr<Graph> dependency_graph;

    llvm::DenseMap<llvm::Loop *, LoopSummary> loop_sum;
    llvm::DenseMap<llvm::Instruction *,
                   llvm::SmallVector<llvm::BasicBlock *, 8>>
        blacklist_control_edge;

    llvm::DenseMap<llvm::Value *, llvm::SmallVector<llvm::Instruction *, 8>>
        blacklist_loop_live_in_data_edge;

    llvm::DenseMap<llvm::Value *, llvm::SmallVector<llvm::Instruction *, 8>>
        blacklist_loop_live_out_data_edge;

    llvm::DenseMap<llvm::Value *, llvm::SmallVector<llvm::Instruction *, 8>>
        blacklist_carry_dependency_data_edge;

    std::map<std::pair<llvm::Value *, llvm::Value *>, ArgumentNode *>
        loop_edge_map;

    std::map<llvm::Value *, llvm::SmallSetVector<llvm::Loop *, 8>>
        live_in_ins_loop_edge;

    std::map<llvm::Value *,
             std::set<std::pair<llvm::Loop *, llvm::Loop *>>>
        live_in_loop_loop_edge;

    llvm::DenseMap<
        llvm::Loop *,
        std::set<std::pair<llvm::Value *, llvm::Value *>>>
        live_in_loop_ins_edge;

    std::map<llvm::Value *, std::set<llvm::Loop *>>
        live_out_ins_loop_edge;

    std::map<llvm::Value *,
             std::set<std::pair<llvm::Loop *, llvm::Loop *>>>
        live_out_loop_loop_edge;

    llvm::DenseMap<
        llvm::Loop *,
        std::set<std::pair<llvm::Value *, llvm::Value *>>>
        live_out_loop_ins_edge;

    std::map<llvm::Value *, std::vector<std::pair<llvm::Loop *, llvm::Loop *>>>
        loop_loop_edge_lin_map;
    std::map<llvm::Value *, std::vector<std::pair<llvm::Loop *, llvm::Loop *>>>
        loop_loop_edge_lout_map;

    std::map<llvm::Value *, Loop *> live_in_outer_edge;
    std::map<llvm::Value *, Loop *> live_out_outer_edge;

    std::map<llvm::Value *, Node *> map_value_node;
    std::map<llvm::Loop *, LoopNode *> loop_value_node;

    uint32_t LID;

   private:
    // Loop Info
    llvm::LoopInfo *LI;

    // Default value is standard out
    llvm::raw_ostream &code_out;

    // NOTE: Uncomment if there is any dependent analysis
    // virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

    virtual bool doInitialization(llvm::Module &M) override;
    virtual bool doFinalization(llvm::Module &M) override;

    void init(llvm::Function &);

    void visitFunction(llvm::Function &);
    void visitBasicBlock(llvm::BasicBlock &);
    void visitBitCastInst(llvm::BitCastInst &);
    void visitInstruction(llvm::Instruction &);
    void visitBinaryOperator(llvm::BinaryOperator &);
    void visitICmpInst(llvm::ICmpInst &);
    void visitBranchInst(llvm::BranchInst &);
    void visitPHINode(llvm::PHINode &);
    void visitSelectInst(llvm::SelectInst &);
    void visitAllocaInst(llvm::AllocaInst &);
    void visitGetElementPtrInst(llvm::GetElementPtrInst &);
    void visitLoadInst(llvm::LoadInst &);
    void visitStoreInst(llvm::StoreInst &);
    void visitReturnInst(llvm::ReturnInst &);
    void visitCallInst(llvm::CallInst &);

    void visitSExtInst(llvm::SExtInst &);
    void visitZExtInst(llvm::ZExtInst &);
    void visitTruncInst(llvm::TruncInst&);
    void visitSIToFPInst(llvm::SIToFPInst&);

    void visitFAdd(llvm::BinaryOperator &);
    void visitFSub(llvm::BinaryOperator &);
    void visitFMul(llvm::BinaryOperator &);
    void visitFDiv(llvm::BinaryOperator &);
    void visitFCmp(llvm::FCmpInst &);

    void fillBasicBlockDependencies(llvm::Function &);

    [
        [deprecated("This function doesn't support nested for loops. Instead "
                    "use updateLoopDependencies function")]] void
    fillLoopDependencies(llvm::LoopInfo &);

    void updateLoopDependencies(llvm::LoopInfo &loop_info);

    // void makeLoopNodes(llvm::LoopInfo &loop_info);
    void buildLoopNodes(llvm::Function &, llvm::LoopInfo &loop_info);
    void connectLoopEdge();
    void findControlPorts(llvm::Function &);
    void findDataPorts(llvm::Function &);
    void connectOutToReturn(llvm::Function &);
    void connectParalleNodes(llvm::Function &);
    void connectingCalldependencies(llvm::Function &);
    void connectingAliasEdges(llvm::Function &);

    void buildingGraph();

   public:
    static char ID;

    GraphGeneratorPass()
        : llvm::ModulePass(ID),
          dependency_graph(std::make_unique<Graph>(NodeInfo(0, "dummy"))),
          LID(0),
          code_out(llvm::outs()) {}
    GraphGeneratorPass(NodeInfo _n_info)
        : llvm::ModulePass(ID),
          dependency_graph(std::make_unique<Graph>(_n_info)),
          LID(0),
          code_out(llvm::outs()) {}

    GraphGeneratorPass(NodeInfo _n_info, llvm::raw_ostream &out)
        : llvm::ModulePass(ID),
          dependency_graph(std::make_unique<Graph>(_n_info, out)),
          LID(0),
          code_out(out) {}

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        // AU.addRequired<llvm::AAResultsWrapperPass>();
        // AU.addRequired<aew::AliasEdgeWriter>();
        AU.addRequired<llvm::LoopInfoWrapperPass>();
        AU.addRequired<helpers::GepInformation>();
        // AU.addRequired<loopclouser::LoopClouser>();
        AU.setPreservesAll();
    }

    // virtual bool runOnFunction(llvm::Function &) override;
    virtual bool runOnModule(llvm::Module &m) override;
    LoopSummary summarizeLoop(llvm::Loop *, llvm::LoopInfo &);
};
}  // namespace graphgen

#endif
