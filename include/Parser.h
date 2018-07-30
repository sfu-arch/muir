

#ifndef PARSER_H
#define PARSER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/MapVector.h"

#include "Common.h"
#include "NodeType.h"


#include <map>
#include <string>
#include <vector>
#include <set>

namespace xketch {


    class Context;
    class Graph;
    class Node;
    class Blk;

    typedef enum {DAT, CTR, CMP, DEP_INVALID} DepTy;
    class Dep { //dependence edge
        llvm::Instruction* src;
        llvm::Instruction* dst;
        llvm::Value* val;
        DepTy ty;
        Graph* fn;
    public:
        Dep(llvm::Instruction* s, llvm::Instruction* d, llvm::Value* v, DepTy t, Graph* g):
                src(s), dst(d), val(v), ty(t), fn(g) {}
        void Print (llvm::raw_fd_ostream&);

        void serialize(llvm::raw_fd_ostream&, bool);
    };

    class Node { //instruction node
        std::string name;
        llvm::SetVector <Dep*> incoming;
        llvm::SetVector <Dep*> outgoing;
        Blk* blk;

    public:
        Node (std::string s, Blk* b): name(s), blk(b) {}
        void AddIncoming (Dep* d) { incoming.insert(d); }
        void AddOutgoing (Dep* d) { outgoing.insert(d); }
        bool IsTop () { return (incoming.empty()); }
        bool IsBot () { return (outgoing.empty()); }
        std::string GetName () { return name; }
        void Print (llvm::raw_fd_ostream&);

        void serialize(llvm::raw_fd_ostream&);
    };



    typedef enum {DET, REA, CNT, CTL, INVALID} EdgTy;

    class Edge {
        BasicBlock* src;
        BasicBlock* dst;
        EdgTy ty;
        Graph* fn;

    public:
        Edge (BasicBlock* b1, BasicBlock* b2, EdgTy t, Graph* f)
                : src(b1), dst(b2), ty(t), fn(f) {}
        bool External () {
            return ((ty == DET) || (ty == REA));
        }

        EdgTy GetType() { return ty; }

        llvm::BasicBlock* GetSrc () {
            return src;
        }

        llvm::BasicBlock* GetDst () {
            return dst;
        }

        void SetDst (llvm::BasicBlock* b) {
            dst = b;
        }

        void Print (llvm::raw_fd_ostream&);

        // each class requires a public serialize function
        void serialize(llvm::raw_fd_ostream&, bool);

    };//Edge


    class Blk {
        Graph* fn;
        Context* ctx;
        BasicBlock *parent; //parent spawning detach context
        bool hasDetach;
        bool hasReattach;
        int co; //color
        llvm::SetVector <Edge*> incoming;
        llvm::SetVector <Edge*> outgoing;

        llvm::SetVector <llvm::Instruction*> ops;
        llvm::SetVector <llvm::Instruction*> tops;
        llvm::Instruction* bottom;
        void MakeDAG ();
        void DAGDataDeps (llvm::Instruction*);


    public:
        Blk (llvm::BasicBlock* par, bool det, bool re, int c, Graph* g, llvm::BasicBlock* bbl)
                : parent(par), hasDetach(det), hasReattach(re), co(c), ctx(NULL), fn(g) {
            for (auto& I:*bbl) ops.insert(&I);
            MakeDAG ();
        }
        Blk (llvm::BasicBlock* par, bool det, bool re, int c, Context* cx, Graph* g, llvm::BasicBlock* bbl)
                : parent(par), hasDetach(det), hasReattach(re), co(c), ctx(cx), fn(g) {
            for (auto& I:*bbl) ops.insert(&I);
            MakeDAG ();
        }
        llvm::BasicBlock* GetParent () { return parent; }
        void SetCtx (Context* c) { ctx = c; }
        Context* GetCtx () { return ctx; }
        bool HasDetach () { return hasDetach; }
        bool HasReattach () { return hasReattach; }
        int GetColor () { return co; }
        void AddIncoming (Edge* edg) { incoming.insert (edg); }
        void AddOutgoing (Edge* edg) { outgoing.insert (edg); }
        void Print (llvm::raw_fd_ostream&);
        void PrintExt (llvm::raw_fd_ostream&);
        Graph* GetFn () { return fn; }
        void RemoveIncoming();
        void RemoveOutgoing();
        // each class requires a public serialize function
        void serialize(llvm::raw_fd_ostream&);


    };

    typedef llvm::MapVector <llvm::BasicBlock*, Blk*> BlkMap; //bbl -> info
    typedef llvm::SetVector<llvm::BasicBlock*> BblSet;
    typedef llvm::MapVector<llvm::Value*, llvm::Value*> ValueToValueMapTy;
    typedef llvm::MapVector<llvm::Instruction*, Node*> NodeMap;




    class Context {
        Graph* fn; // function
        int id;

        BblSet blks;
        int color;

        llvm::Function* StaticFunc;
        ValueToValueMapTy VMap;
        ValueToValueMapTy GlobalPointer;

        Edge* inDet; //incoming detach
        llvm::SetVector<Edge*> outDet; //outgoing detaches
        llvm::SetVector<Edge*> inRe; //incoming reattaches
        llvm::SetVector<Edge*> outRe; //outgoing reattaches

        llvm::SetVector<llvm::Value*> liveIns; //live-ins, sent as values
        llvm::SetVector<llvm::Value*> globIns; //globals used as input, will be sent as pointer
        llvm::SetVector<llvm::Value*> defs;

        void LiveInHelper (llvm::Value*);
        void MakeProto ();
        void CopyFunc ();
        llvm::Value* MapFunc(llvm::Value*);
        void InsertCall();


    public:
        Context (llvm::BasicBlock*, Graph*, int);
        void Insert (llvm::BasicBlock*);
        void Print (llvm::raw_fd_ostream&);
        void PrintExt (llvm::raw_fd_ostream&);
        int GetColor () {return color;}
        bool InCtx (llvm::BasicBlock*);


        void SetInDet (Edge* edg) { inDet = edg; }
        void AddOutDet (Edge* edg) { outDet.insert(edg); }
        void AddInRe (Edge* edg) { inRe.insert(edg); }
        void AddOutRe (Edge* edg) { outRe.insert (edg); }
        void RemoveInRe (Edge* edg) { inRe.remove(edg); }
        void RemoveOutRe (Edge* edg) { outRe.remove(edg); }


        void GatherInputs ();
        llvm::SetVector<llvm::Value*>& GetLiveIns () { return liveIns; }
        llvm::SetVector<llvm::Value*>& GetGlobIns () { return globIns; }

        void FuncWrap ();

        // each class requires a public serialize function
        void serialize(llvm::raw_fd_ostream&);



    };




    typedef llvm::MapVector <BasicBlock*, Context*> CtxMap; //parent -> context



    class Graph {

        CtxMap ctxs;
        BlkMap blks;
        NodeMap nodes;



        CtxMap::iterator FindCtx (llvm::BasicBlock*);


    public:
        Graph () {}
        Context* Insert (llvm::BasicBlock*, Blk*);
        void InsertEdge (llvm::BasicBlock*, llvm::BasicBlock*, EdgTy);
        void Print (llvm::raw_fd_ostream&);
        Blk* FindBbl(llvm::BasicBlock*);
        void EraseBbl(llvm::BasicBlock*);
        void TraverseCtxTree (raw_fd_ostream&, raw_fd_ostream&);

        // each class requires a public serialize function
        void serialize(llvm::raw_fd_ostream&);

        void CreateNode(llvm::Instruction*, std::string, Blk*);
        Node* FindNode (llvm::Instruction*);


    };

    class ParserPass : public llvm::ModulePass {

#ifdef TAPIR

#endif


        llvm::StringRef FunctionName;

        Graph* ctxGraph;

        virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

        virtual bool doInitialization(llvm::Module &M) override;

    public:
        static char ID;


        ParserPass()
                : llvm::ModulePass(ID) { ctxGraph = new Graph(); }


        ParserPass(llvm::StringRef name)
                : llvm::ModulePass(ID), FunctionName(name) { ctxGraph = new Graph; }

        //raw_fd_ostream dot;


        virtual bool runOnModule(llvm::Module &m) override;




        void ParseDetach(llvm::Function &F);

        void DecomposeFunc(llvm::Function &, std::vector<BasicBlock *> &);

        /*bool DetachBlk(llvm::BasicBlock *);

        bool ReattachBlk(llvm::BasicBlock *);

        bool DetachEdge(llvm::BasicBlock *, llvm::BasicBlock *);*/

        void InsertNode(llvm::BasicBlock*, Blk *);

        void InsertEdge(llvm::BasicBlock*, llvm::BasicBlock*);


#ifdef TAPIR

#endif

    };







}//namespace xketch

#endif
