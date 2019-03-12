#define DEBUG_TYPE "parser_code"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#include <llvm/Transforms/Utils/ValueMapper.h>
#include <queue>
#include <set>
#include <string>

#include "AliasEdgeWriter.h"
#include "Common.h"
#include "Parser.h"

using namespace llvm;
using namespace std;
using namespace common;
using namespace xketch;

char ParserPass::ID = 0;
RegisterPass<ParserPass> X("parser", "parsing function");

extern bool isTargetFunction(const Function& f,
                             const cl::list<std::string>& FunctionList);

extern cl::opt<string> outFile;

static bool DetachBlk(BasicBlock* bbl) {
    return (dyn_cast<DetachInst>(bbl->getTerminator()) != NULL);
}  // DetachBlk
static bool ReattachBlk(BasicBlock* bbl) {
    return (dyn_cast<ReattachInst>(bbl->getTerminator()) != NULL);
}  // ReattachBlk
static bool DetachEdge(BasicBlock* b1, BasicBlock* b2) {
    return (cast<DetachInst>(b1->getTerminator())->getContinue() != b2);
}  // DetachEdge

bool ParserPass::doInitialization(llvm::Module& M) { return false; }

// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool ParserPass::runOnModule(Module& M) {
    for (auto& F : M) {
        if (F.isDeclaration()) continue;

        if (F.getName() == this->FunctionName) {
            stripDebugInfo(F);
            DEBUG(dbgs() << "FUNCTION FOUND\n");

            ParseDetach(F);

            std::error_code errc;
            raw_fd_ostream dot(("dfg." + F.getName() + ".dot").str().c_str(),
                               errc, sys::fs::F_None);

            raw_fd_ostream json(("dfg." + F.getName() + ".json").str().c_str(),
                                errc, sys::fs::F_None);

            ctxGraph->TraverseCtxTree(dot, json);
        }
    }

    return true;
}

/**
 * Set pass dependencies
 */
void ParserPass::getAnalysisUsage(AnalysisUsage& AU) const {
    AU.setPreservesAll();
}

void ParserPass::ParseDetach(Function& F) {
    vector<BasicBlock*> stk;  // stack for DFS

    auto top = &*(F.begin());
    auto n = new Blk(NULL, DetachBlk(top), false, 0, ctxGraph, top);

    stk.push_back(top);
    InsertNode(top, n);
    DecomposeFunc(F, stk);

}  // ParseDetach

void ParserPass::DecomposeFunc(Function& F, vector<BasicBlock*>& stk) {
    // this is essentially a depth-first traversal of the function basic blocks
    // reattaches will be matched to their nearest detach in the stack

    if (stk.empty()) return;
    auto bbl = stk.back();
    auto nd = ctxGraph->FindBbl(bbl);

    for (auto s : successors(bbl)) {
        auto n = ctxGraph->FindBbl(s);

        if (n == NULL) {
            auto parent = ctxGraph->FindBbl(nd->GetParent());

            if (nd->HasDetach() && DetachEdge(bbl, s)) {  // new context spawned
                n = new Blk(bbl, DetachBlk(s), ReattachBlk(s),
                            nd->GetColor() + 1, ctxGraph, s);

            } else if (nd->HasReattach()) {  // successor is one level up
                n = new Blk(parent->GetParent(), DetachBlk(s), ReattachBlk(s),
                            parent->GetColor(), parent->GetCtx(), ctxGraph, s);

            } else {  // same context
                n = new Blk(nd->GetParent(), DetachBlk(s), ReattachBlk(s),
                            nd->GetColor(), nd->GetCtx(), ctxGraph, s);
            }

            InsertNode(s, n);
            InsertEdge(bbl, s);

            stk.push_back(s);
            DecomposeFunc(F, stk);
        }  // prevent cycles in DFS
        else {
            InsertEdge(bbl, s);
        }

    }  // for each successor

    stk.pop_back();

}  // DecomposeFunc

void ParserPass::InsertNode(BasicBlock* bbl, Blk* n) {
    // insert into data structure
    ctxGraph->Insert(bbl, n);
}  // PrintNode

void ParserPass::InsertEdge(BasicBlock* b1, BasicBlock* b2) {
    EdgTy t;

    if (DetachBlk(b1)) {           // detach or continue edge
        if (DetachEdge(b1, b2)) {  // detach edge
            t = DET;
        } else {  // continuation edge
            t = CNT;
        }
    } else if (ReattachBlk(b1)) {  // reattach edge
        t = REA;
    } else {  // regular edge
        t = CTL;
    }

    ctxGraph->InsertEdge(b1, b2, t);
}  // PrintEdge

Context* Graph::Insert(BasicBlock* bbl, Blk* n) {
    // first figure out which context it should be inserted into
    // contexts are mapped by parent detach block

    if (n->GetCtx() == NULL) {
        Context* newCtx = new Context(bbl, this, ctxs.size());
        if (n->GetParent())
            ctxs.insert(pair<BasicBlock*, Context*>(bbl, newCtx));
        else
            ctxs.insert(pair<BasicBlock*, Context*>(NULL, newCtx));
        n->SetCtx(newCtx);
        blks.insert(pair<BasicBlock*, Blk*>(bbl, n));
        return newCtx;
    } else {  // found
        blks.insert(pair<BasicBlock*, Blk*>(bbl, n));
        n->GetCtx()->Insert(bbl);
        return n->GetCtx();
    }

}  // Graph::Insert

void Graph::InsertEdge(llvm::BasicBlock* b1, llvm::BasicBlock* b2, EdgTy t) {
    auto blk1 = FindBbl(b1);
    auto blk2 = FindBbl(b2);
    assert(blk1 && blk2 && "basic block not discovered yet\n");

    auto edg = new Edge(b1, b2, t, this);

    blk1->AddOutgoing(edg);
    blk2->AddIncoming(edg);

    if (t == DET) {  // detach edge, different contexts
        blk1->GetCtx()->AddOutDet(edg);
        blk2->GetCtx()->SetInDet(edg);

    } else if (t == REA) {  // reattach edge, different contexts
        blk1->GetCtx()->AddOutRe(edg);
        blk2->GetCtx()->AddInRe(edg);
    }
}  // Graph::InsertEdge

CtxMap::iterator Graph::FindCtx(BasicBlock* bbl) {
    return ctxs.find(bbl);
}  // FindCtx

Blk* Graph::FindBbl(llvm::BasicBlock* bbl) {
    BlkMap::iterator iter = blks.find(bbl);
    if (iter == blks.end()) return NULL;
    return iter->second;
}  // FindBbl

void Graph::EraseBbl(llvm::BasicBlock* bbl) {
    auto blk = FindBbl(bbl);
    assert(blk && "block not found\n");

    blk->RemoveIncoming();

    blk->RemoveOutgoing();

}  // Graph::EraseBbl

void Graph::Print(raw_fd_ostream& dot) {
    dot << "digraph G {\nordering=out\ncompound=true\n";

    // print each context
    for (auto& C : ctxs) {
        C.second->Print(dot);

    }  // for each ctx

    // print each context
    for (auto& C : ctxs) {
        C.second->PrintExt(dot);

    }  // for each ctx

    dot << "}\n";
}  // Graph::Print

Context::Context(BasicBlock* b, Graph* g, int i) {
    fn = g;
    blks.insert(b);
    inDet = NULL;
    StaticFunc = NULL;
    id = i;

}  // Context::Context

void Context::Insert(BasicBlock* b) { blks.insert(b); }  // Context::insert

void Context::Print(raw_fd_ostream& dot) {
    dot << "subgraph \"clustercontext";
    dot << (*(blks.begin()))->getName().str()
        << "\" {\n"  // context named after entry block
        << "style=filled\n"
        << "color=lightgrey\n";

    for (auto& blk : blks) {
        dot << "subgraph \"cluster" << blk->getName().str() << "\" {\n"
            << "style=filled\n"
            << "color=white\n";

        // print instructions
        for (auto& ins : *blk) {
            dot << "\"" << fn->FindNode(&ins)->GetName()
                << "\" [shape=box, style=filled, label=\"";
            dot << ins << "\", fillcolor=";
            switch (fn->FindBbl(blk)->GetColor() % 4) {
                case 0:
                    dot << "pink";
                    break;
                case 1:
                    dot << "green";
                    break;
                case 2:
                    dot << "yellow";
                    break;
                case 3:
                    dot << "lightblue";
                    break;
                default:
                    break;
            };

            dot << "]\n";
        }

        fn->FindBbl(blk)->Print(dot);

        dot << "}\n";

    }  // for each block

    for (auto& blk : blks) {
        fn->FindBbl(blk)->PrintExt(dot);
    }

    dot << "}\n";

}  // Context::Print

void Context::PrintExt(raw_fd_ostream& dot) {
    for (auto& edg : outDet) {
        edg->Print(dot);
    }

    for (auto& edg : inRe) {
        edg->Print(dot);
    }

}  // Context::PrintExt

bool Context::InCtx(BasicBlock* bb) {
    return (blks.count(bb) != 0);

}  // Context::InCtx

void Blk::Print(raw_fd_ostream& dot) {
    /*

    dot << "]\n";*/

    // print intra-block edges
    for (auto& op : ops) {
        fn->FindNode(op)->Print(dot);
    }
}  // Blk::Print

void Blk::PrintExt(llvm::raw_fd_ostream& dot) {
    for (auto& edg : incoming) {
        if (!edg->External()) edg->Print(dot);
    }

}  // Blk::PrintExt

void Edge::Print(raw_fd_ostream& dot) {
    if (External() && (ty == DET)) {
        dot << "\"" << fn->FindNode(&(GetSrc()->back()))->GetName() << "\"->"
            << "\"" << fn->FindNode(&(GetDst()->front()))->GetName() << "\""
            << "[color=red, penwidth=5, fontsize=25, "
            << "ltail=\""
            << "cluster" << GetSrc()->getName().str() << "\","
            << "lhead=\""
            << "cluster" << GetDst()->getName().str() << "\","
            << "label=\"";

        for (auto& liv : fn->FindBbl(GetDst())->GetCtx()->GetLiveIns()) {
            dot << "," << liv->getName();
        }  // for each live in

        for (auto& glv : fn->FindBbl(GetDst())->GetCtx()->GetGlobIns()) {
            dot << "," << glv->getName();
        }  // for each global in

        dot << "\"]\n";
    } else if (External()) {
        dot << "\"" << fn->FindNode(&(GetSrc()->back()))->GetName() << "\"->"
            << "\"" << fn->FindNode(&(GetDst()->front()))->GetName() << "\""
            << "[color=red, penwidth=5, "
            << "ltail=\""
            << "cluster" << GetSrc()->getName().str() << "\","
            << "lhead=\""
            << "cluster" << GetDst()->getName().str() << "\","
            << "]\n";

    } else
        dot << "\"" << fn->FindNode(&(GetSrc()->back()))->GetName() << "\"->"
            << "\"" << fn->FindNode(&(GetDst()->front()))->GetName() << "\""
            << "[ltail=\""
            << "cluster" << GetSrc()->getName().str() << "\","
            << "lhead=\""
            << "cluster" << GetDst()->getName().str() << "\","
            << "]\n";
}  // Edge::Print

void Graph::TraverseCtxTree(raw_fd_ostream& dot, raw_fd_ostream& json) {
    // this is a bottom-up traversal of the context tree
    // since the contexts were discovered in a top-down manner
    // we will traverse the contexts vector in reverse order

    for (auto iter = ctxs.rbegin(); iter != ctxs.rend() - 1;
         ++iter) {  // no need to fn-ize top context

        iter->second->GatherInputs();
    }

    Print(dot);
    dot.close();

    serialize(json);
    json.close();

    for (auto iter = ctxs.rbegin(); iter != ctxs.rend() - 1;
         ++iter) {  // no need to fn-ize top context

        iter->second->FuncWrap();
        // break;
    }

}  // Graph::TraverseCtxTree

void Context::GatherInputs() {
    // first gather the values produced by this context
    for (auto& blk : blks) {
        for (auto& I : *blk) {
            auto val = dyn_cast<Value>(&I);
            if (val) defs.insert(val);

        }  // for each instruction
    }      // for each block

    // Live In Loop
    for (auto& blk : blks) {
        for (auto& I : *blk) {
            LiveInHelper(&I);

        }  // for each instruction
    }      // for each block

    // now process the child contexts
    // they already know their live-ins because these are discovered bottom-up
    // case 1: parent produces a value that is live-in to child, will not be
    // propagated up/no change
    // case 2: child and parent have a common live-in that is coming from
    // ancestor, nothing changes
    // case 3: child has a live-in that is not produced by parent and is not in
    // inputs set, add to live-in/global set of parent
    // for each child
    for (auto& edg : outDet) {
        auto dst = edg->GetDst();
        auto child = fn->FindBbl(dst)->GetCtx();

        auto li = child->GetLiveIns();
        for (auto& liv : li) {
            if ((defs.count(liv) == 0) && (liveIns.count(liv) == 0)) {
                liveIns.insert(liv);
            }  // case 3
        }      // for each live-in of child context

        auto gl = child->GetGlobIns();
        for (auto& glv : gl) {
            if (globIns.count(glv) == 0) {
                globIns.insert(glv);
            }  // case 3
        }      // for each live-in of child context
    }          // for each child context

}  // Context::GatherInputs

void Context::LiveInHelper(llvm::Value* Val) {
    if (auto Ins = dyn_cast<Instruction>(Val)) {  // value is instruction
        for (auto OI = Ins->op_begin(), EI = Ins->op_end(); OI != EI;
             OI++) {  // for each operand
            if (auto OIns =
                    dyn_cast<Instruction>(OI)) {  // if operand is instruction

                bool is_live_in = true;
                auto _intrinsic_op = CallSite(OIns);
                if (_intrinsic_op.getInstruction()) {
                    auto _intrinsic_call = dyn_cast<Function>(
                        _intrinsic_op.getCalledValue()->stripPointerCasts());

                    if (_intrinsic_call) {
                        if (_intrinsic_call->isDeclaration()) {
                            is_live_in = false;
                        }
                    }
                }

                if ((defs.count(OIns) == 0) && (is_live_in)) {
                    liveIns.insert(OIns);

                }  // operand was not produced by context

            } else
                LiveInHelper(*OI);  // operand is not an instruction
        }                           // for each operand
    } else if (auto CE = dyn_cast<ConstantExpr>(
                   Val)) {  // value is constant expression
        for (auto OI = CE->op_begin(), EI = CE->op_end(); OI != EI;
             OI++) {  // for each operand
            assert(!isa<Instruction>(OI) &&
                   "Don't expect operand of ConstExpr to be an Instruction");

            LiveInHelper(*OI);
        }  // for each operand of const expr
    } else if (auto Arg = dyn_cast<Argument>(Val)) {  // value is an arg
        liveIns.insert(Val);

    } else if (auto GV =
                   dyn_cast<GlobalVariable>(Val)) {  // value is a global var
        globIns.insert(GV);

    }  // value is global var

}  // Context::LiveInHelper

void Context::FuncWrap() {
    // create func prototype and add to module
    MakeProto();

    // copy original code to func
    CopyFunc();

    // replace original code with a call to the func
    InsertCall();

}  // Context::FuncWrap

void Context::MakeProto() {
    auto Mod = blks.back()->getParent()->getParent();

    auto DataLayoutStr = Mod->getDataLayout();
    auto TargetTripleStr = Mod->getTargetTriple();

    // Bool return type for extracted function
    auto VoidTy = Type::getVoidTy(Mod->getContext());
    auto Int1Ty = IntegerType::getInt1Ty(Mod->getContext());

    std::vector<Type*> ParamTy;
    // Add the types of the input values
    // to the function's argument list
    for (auto Val : liveIns) ParamTy.push_back(Val->getType());

    for (auto& G : globIns) {
        ParamTy.push_back(G->getType());
    }

    FunctionType* StFuncType = FunctionType::get(VoidTy, ParamTy, false);

    // Create the trace function
    std::string name = (*(blks.begin()))->getName().str();
    std::replace(name.begin(), name.end(), '.', '_');

    auto p = Mod->getName().find_last_of('.');

    StaticFunc = Function::Create(
        StFuncType, GlobalValue::ExternalLinkage,
        Mod->getName().substr(0, p) + "_detach" + to_string(id), Mod);

}  // Context::MakeProto

void Context::CopyFunc() {
    Function* F = (*(blks.begin()))->getParent();
    Module* Mod = F->getParent();

    // Add Globals
    for (auto Val : globIns) {
        auto OldGV = dyn_cast<GlobalVariable>(Val);
        assert(OldGV && "Could not reconvert to Global Variable");
        GlobalVariable* GV = new GlobalVariable(
            *StaticFunc->getParent(), OldGV->getType()->getElementType(),
            OldGV->isConstant(), GlobalValue::ExternalLinkage,
            (Constant*)nullptr, OldGV->getName(), (GlobalVariable*)nullptr,
            OldGV->getThreadLocalMode(), OldGV->getType()->getAddressSpace());
        GV->copyAttributesFrom(OldGV);
        assert(VMap.count(OldGV) == 0 && "Need new values");
        VMap[OldGV] = GV;

        // Just set the linkage for the original global variable in
        // case the it was private or something.
        OldGV->setLinkage(GlobalValue::ExternalLinkage);
    }

    for (auto IT = blks.begin(), IE = blks.end(); IT != IE; ++IT) {
        auto C = *IT;
        auto* NewBB = BasicBlock::Create(
            (*(blks.begin()))->getParent()->getParent()->getContext(),
            StringRef("my_") + C->getName(), StaticFunc, nullptr);

        // the following assert is not needed anymore because
        // there maybe multiple copies of the same basic block
        assert(VMap.count(C) == 0 && "Need new values");
        VMap[C] = NewBB;

        for (auto& I : *C) {
            if (dyn_cast<ReattachInst>(&I)) {
                auto NewI = ReturnInst::Create(
                    (*(blks.begin()))->getParent()->getParent()->getContext(),
                    NewBB);

                // the following assert is not needed anymore
                assert(VMap.count(&I) == 0 && "Need new values");
                VMap[&I] = NewI;
            } else {
                auto NewI = I.clone();
                NewBB->getInstList().push_back(NewI);

                // the following assert is not needed anymore
                assert(VMap.count(&I) == 0 && "Need new values");
                VMap[&I] = NewI;
            }

        }  // for each instruction
    }      // for each block

    // now for each child context, clone their blocks, but the reattaches are
    // left unchanged
    for (auto& edg : outDet) {
        auto child = fn->FindBbl(edg->GetDst())->GetCtx();

        for (auto IT = child->blks.begin(), IE = child->blks.end(); IT != IE;
             ++IT) {
            auto C = *IT;
            auto* NewBB = BasicBlock::Create(Mod->getContext(),
                                             StringRef("my_") + C->getName(),
                                             StaticFunc, nullptr);

            // the following assert is not needed anymore because
            // there maybe multiple copies of the same basic block
            assert(VMap.count(C) == 0 && "Need new values");
            VMap[C] = NewBB;

            for (auto& I : *C) {
                auto NewI = I.clone();
                NewBB->getInstList().push_back(NewI);

                // the following assert is not needed anymore
                assert(VMap.count(&I) == 0 && "Need new values");
                VMap[&I] = NewI;

            }  // for each instruction
        }      // for each block
    }          // for each child

    // Assign names, if you don't have a name,
    // a name will be assigned to you.
    Function::arg_iterator AI = StaticFunc->arg_begin();
    uint32_t VReg = 0;
    for (auto Val : liveIns) {
        auto Name = Val->getName();
        if (Name.empty())
            AI->setName(StringRef(string("vr.") + to_string(VReg++)));
        else
            AI->setName(Name + string(".in"));
        VMap[Val] = &*AI;
        ++AI;
    }

    for (auto G : globIns) {
        AI->setName(G->getName() + ".in");
        Value* RewriteVal = &*AI++;
        GlobalPointer[G] = RewriteVal;
    }

    function<void(Value*, Value*)> rewriteHelper;
    rewriteHelper = [this, &rewriteHelper](Value* I, Value* P) {
        if (auto* CE = dyn_cast<ConstantExpr>(I)) {
            for (auto OI = CE->op_begin(), OE = CE->op_end(); OI != OE; OI++) {
                rewriteHelper(*OI, CE);
            }
        } else if (auto* GV = dyn_cast<GlobalVariable>(I)) {
            dyn_cast<User>(P)->replaceUsesOfWith(GV, GlobalPointer[GV]);
        }
    };

    for (auto& BB : *StaticFunc) {
        for (auto& I : BB) {
            for (auto OI = I.op_begin(), OE = I.op_end(); OI != OE; OI++) {
                rewriteHelper(*OI, &I);
            }
        }
    }

    // the original code maps uses of live-ins, globals, and other values to
    // arguments, and, new values respectively.
    // however, for a sequence, different uses of the same value could be
    // referring to different copies of it. therefore,
    // instead of updating uses of a value, we have search for the definition
    // nearest to the use and see what this definition
    // has been mapped to. we use the value map stack for this.

    for (auto& IT : *StaticFunc) {
        // walk sequence in reverse topological order
        // for each basic block, find the block it has been mapped to

        for (auto& I : IT) {
            if (auto Phi = dyn_cast<PHINode>(&I)) {
                auto NV = Phi->getNumIncomingValues();
                for (unsigned i = 0; i < NV; i++) {
                    auto* Blk = Phi->getIncomingBlock(i);
                    Phi->setIncomingBlock(i,
                                          dyn_cast<BasicBlock>(MapFunc(Blk)));
                    auto* Val = Phi->getIncomingValue(i);
                    Phi->setIncomingValue(i, MapFunc(Val));
                }

            } else {  // non-phi ins

                for (auto OI = I.op_begin(), OE = I.op_end(); OI != OE; OI++) {
                    auto oldVal = dyn_cast<Value>(OI);
                    (dyn_cast<User>(&I))
                        ->replaceUsesOfWith(oldVal, MapFunc(oldVal));
                }  // for each operand
            }      // non-phi ins
        }          // for each ins
    }              // for each bbl

    return;

}  // Context::CopyFunc

Value* Context::MapFunc(Value* oldVal) {
    if (auto CE = dyn_cast<ConstantExpr>(oldVal)) {
        // map globals and constant expression operands to nearest definitions
        // generate a new mapping for this constant expression
        auto updatedCE = CE;
        int nOperands = CE->getNumOperands();

        for (int i = 0; i < nOperands; i++) {
            auto opnd = updatedCE->getOperand(i);
            if (auto GV = dyn_cast<GlobalVariable>(opnd)) {
                auto mappedGV = VMap[GV];
                assert(mappedGV && "global variable not mapped\n");
                auto newCE =
                    cast<ConstantExpr>(updatedCE->getWithOperandReplaced(
                        i, cast<Constant>(mappedGV)));
                updatedCE = newCE;
            } else if (auto ACE = dyn_cast<ConstantExpr>(opnd)) {
                auto mappedCE = VMap[ACE];
                assert(mappedCE && "constant expression not mapped\n");
                auto newCE =
                    cast<ConstantExpr>(updatedCE->getWithOperandReplaced(
                        i, cast<Constant>(mappedCE)));
                updatedCE = newCE;
            }
        }  // each operand of constant expression

        VMap[CE] = updatedCE;
        return updatedCE;

    }  // constant expression
    else if (!dyn_cast<Constant>(oldVal)) {
        if (VMap.count(oldVal) > 0)
            return VMap[oldVal];
        else
            return oldVal;  // this is a duplicate, and a new value

    } else
        return oldVal;

}  // Context::MapFunc

void Context::InsertCall() {
    auto F = (*(blks.begin()))->getParent();
    auto Mod = F->getParent();
    auto& cx = Mod->getContext();

    // first create a basic block with a call to staticfunc
    auto funcCall = BasicBlock::Create(
        cx, "offload." + ((*(blks.begin()))->getName().str()), F);
    vector<Value*> Params;
    for (auto& V : liveIns) Params.push_back(V);

    for (auto& G : globIns) {
        Params.push_back(G);
    }

    auto _sync_region =
        cast<DetachInst>(inDet->GetSrc()->getTerminator())->getSyncRegion();

    auto* CI = CallInst::Create(StaticFunc, Params, "", funcCall);

    auto RI = ReattachInst::Create(
        outRe.back()->GetSrc()->getTerminator()->getSuccessor(0), _sync_region);

     RI->insertAfter(CI);

    // point parent's detach edge to caller block
    (cast<DetachInst>(inDet->GetSrc()->getTerminator()))
        ->setSuccessor(0, funcCall);

    auto parBlk = fn->FindBbl(blks.back());
    auto parBbl = parBlk->GetParent();
    auto parCtx = parBlk->GetCtx();

    for (auto& blk : blks) {
        blk->dropAllReferences();
    }

    for (auto& edg : outDet) {
        edg->GetDst()->dropAllReferences();
    }

    for (auto& blk : blks) {
        fn->EraseBbl(blk);
        blk->eraseFromParent();
    }
    blks.clear();

    for (auto& edg : outDet) {
        auto blk = edg->GetDst();
        fn->EraseBbl(blk);
        blk->eraseFromParent();
    }

    blks.insert(funcCall);
    fn->Insert(funcCall,
               new Blk(parBbl, false, true, color, this, fn, funcCall));

    // erase outgoing detach edges and incoming reattach edges
    outDet.clear();
    inRe.clear();

    // reset incoming detach edge and outgoing reattach edge
    outRe.clear();

    this->inDet->SetDst(funcCall);
    fn->InsertEdge(funcCall, funcCall->getSingleSuccessor(), REA);

}  // Context::InsertCall

void Blk::RemoveIncoming() {
    for (auto& iter : incoming) {
        auto src = fn->FindBbl(iter->GetSrc());

        if (iter->GetType() == REA) src->GetCtx()->RemoveOutRe(iter);

        src->outgoing.remove(iter);
    }
}
void Blk::RemoveOutgoing() {
    for (auto& iter : outgoing) {
        auto dst = fn->FindBbl(iter->GetDst());

        if (iter->GetType() == REA) dst->GetCtx()->RemoveInRe(iter);

        dst->incoming.remove(iter);
    }
}

void static PrintDebugValue(Value*& val) { val->dump(); }

void static PrintDebugInstr(Instruction*& val) { val->dump(); }

void static PrintDebugGlobal(GlobalVariable*& val) { val->dump(); }

void static PrintDebugBbl(BasicBlock*& val) { val->dump(); }

void static PrintDebugFunc(Function*& val) { val->dump(); }

void static PrintDebugPhi(PHINode*& val) { val->dump(); }

static void Pair(raw_fd_ostream& json, string key, string val, bool first) {
    if (!first) json << ",";
    json << "\"" << key << "\":\"" << val << "\"\n";

}  // Field

static void ListVal(raw_fd_ostream& json, bool first) {
    if (!first) json << ",";
    json << "[\n";

}  // ListField

static void ListKey(raw_fd_ostream& json, string key, bool first) {
    if (!first) json << ",";
    json << "\"" << key << "\":\n";
    ListVal(json, true);
}

static void ListEnd(raw_fd_ostream& json) { json << "]\n"; }  // ListEnd

static void StructVal(raw_fd_ostream& json, bool first) {
    if (!first) json << ",";
    json << "{\n";

}  // StructVal

static void StructEnd(raw_fd_ostream& json) { json << "}\n"; }  // StructEnd

static void Key(raw_fd_ostream& json, string key, bool first) {
    if (!first) json << ",";
    json << "\"" << key << "\":\n";
}

static void Val(raw_fd_ostream& json, string val, bool first) {
    if (!first) json << ",";
    json << "\"" << val << "\""
         << "\n";
}

void Graph::serialize(raw_fd_ostream& json) {
    json << "{\n";
    ListKey(json, "contexts", true);

    for (auto ctx = ctxs.begin(); ctx != ctxs.end(); ++ctx) {
        StructVal(json, (ctx == (ctxs.begin())));

        if ((*ctx).first)
            Pair(json, "parent", (*ctx).first->getName(), true);
        else
            Pair(json, "parent", "", true);

        (*ctx).second->serialize(json);

        StructEnd(json);
    }

    ListEnd(json);
    json << "}\n";

}  // Graph::serialize

void Context::serialize(raw_fd_ostream& json) {
    Pair(json, "entry", (*(blks.begin()))->getName().str(), false);

    ListKey(json, "blks", false);
    for (auto& blk : blks) {
        StructVal(json, (blk == *(blks.begin())));

        Pair(json, "name", blk->getName(), true);

        fn->FindBbl(blk)->serialize(json);

        StructEnd(json);
    }
    ListEnd(json);

    if (StaticFunc)
        Pair(json, "StaticFunc", StaticFunc->getName().str(), false);

    Key(json, "inDet", false);
    if (inDet)
        inDet->serialize(json, true);
    else
        Val(json, "null", true);

    ListKey(json, "outDet", false);
    for (auto& edg : outDet) {
        edg->serialize(json, (edg == *(outDet.begin())));
    }
    ListEnd(json);

    ListKey(json, "inRe", false);
    for (auto& edg : inRe) {
        edg->serialize(json, (edg == *(inRe.begin())));
    }
    ListEnd(json);

    ListKey(json, "outRe", false);
    for (auto& edg : outRe) {
        edg->serialize(json, (edg == *(outRe.begin())));
    }
    ListEnd(json);

    ListKey(json, "liveIns", false);
    for (auto& li : liveIns) {
        Val(json, li->getName().str(), (li == *(liveIns.begin())));
    }
    ListEnd(json);

    ListKey(json, "globIns", false);
    for (auto& gi : globIns) {
        Val(json, gi->getName().str(), (gi == *(globIns.begin())));
    }
    ListEnd(json);

}  // Context::serialize

void Blk::serialize(raw_fd_ostream& json) {
    ListKey(json, "incoming", false);
    for (auto& edg : incoming) {
        edg->serialize(json, (edg == *(incoming.begin())));
    }
    ListEnd(json);

    ListKey(json, "outgoing", false);
    for (auto& edg : outgoing) {
        edg->serialize(json, (edg == *(outgoing.begin())));
    }
    ListEnd(json);

    // print nodes
    ListKey(json, "instructions", false);
    for (auto& op : ops) {
        StructVal(json, (op == *(ops.begin())));
        Pair(json, "name", fn->FindNode(op)->GetName(), true);
        json << ",\"IR\":\"" << *op << "\"\n";
        fn->FindNode(op)->serialize(json);
        StructEnd(json);
    }
    ListEnd(json);

}  // Blk::serialize

void Edge::serialize(raw_fd_ostream& json, bool first) {
    StructVal(json, first);

    Pair(json, "src", src->getName().str(), true);
    Pair(json, "dst", dst->getName().str(), false);

    switch (ty) {
        case REA:
            Pair(json, "ty", "reattach", false);
            break;
        case DET:
            Pair(json, "ty", "detach", false);
            break;
        case CNT:
            Pair(json, "ty", "continue", false);
            break;
        default:
            Pair(json, "ty", "control", false);
            break;
    }

    StructEnd(json);

}  // Edge::serialize

void Node::serialize(raw_fd_ostream& json) {
    ListKey(json, "incoming", false);
    for (auto& edg : incoming) {
        edg->serialize(json, (edg == *(incoming.begin())));
    }
    ListEnd(json);

    ListKey(json, "outgoing", false);
    for (auto& edg : outgoing) {
        edg->serialize(json, (edg == *(outgoing.begin())));
    }
    ListEnd(json);

}  // Node::serialize

void Dep::serialize(raw_fd_ostream& json, bool first) {
    StructVal(json, first);

    Pair(json, "src", fn->FindNode(src)->GetName(), true);
    Pair(json, "dst", fn->FindNode(dst)->GetName(), false);
    Pair(json, "val", val->getName(), false);

    switch (ty) {
        case DAT:
            Pair(json, "ty", "data", false);
            break;
        case CMP:
            Pair(json, "ty", "completion", false);
            break;
        default:
            break;
    }

    StructEnd(json);

}  // Dep::serialize

void Graph::CreateNode(llvm::Instruction* op, string s, Blk* b) {
    auto n = new Node(s, b);
    nodes.insert(pair<Instruction*, Node*>(op, n));
}  // Graph::CreateNode

Node* Graph::FindNode(llvm::Instruction* op) {
    auto iter = nodes.find(op);
    if (iter == nodes.end()) return NULL;
    return iter->second;
}  // Graph::FindNode

void Blk::MakeDAG() {
    bottom = ops.back();

    // create nodes
    auto ctr = 0;
    for (auto& op : ops) {
        fn->CreateNode(
            op, op->getParent()->getName().str() + "_" + to_string(ctr), this);
        ctr++;
    }

    // iterate over ops and build the dependences
    for (auto& op : ops) {
        // skip over phis because their deps will not be inside this DAG
        if (auto Phi = dyn_cast<PHINode>(op)) {
            continue;
        }

        // build data deps
        DAGDataDeps(op);

    }  // for each op

    // write completion deps
    for (auto& op : ops) {
        if (op == bottom) continue;

        auto n1 = fn->FindNode(op);
        if (!n1->IsBot()) continue;

        auto n2 = fn->FindNode(bottom);
        assert(n2 && "terminator not registered\n");

        Dep* d = new Dep(
            op, bottom,
            ConstantInt::getTrue(
                op->getParent()->getParent()->getParent()->getContext()),
            CMP, fn);
        n1->AddOutgoing(d);
        n2->AddIncoming(d);
    }

    // determine the top layer
    for (auto& op : ops) {
        if (fn->FindNode(op)->IsTop()) tops.insert(op);
    }

}  // Blk::MakeDAG

void Blk::DAGDataDeps(Instruction* op) {
    // for each operand, check if the producer is in the same block
    for (auto OI = op->op_begin(), OE = op->op_end(); OI != OE; OI++) {
        if (auto I = dyn_cast<Instruction>(OI)) {
            if (ops.count(I) != 0) {
                auto src = fn->FindNode(I);
                auto dst = fn->FindNode(op);
                assert(src && dst && "instructions not registered \n");

                Dep* d = new Dep(I, op, dyn_cast<Value>(OI), DAT, fn);
                src->AddOutgoing(d);
                dst->AddIncoming(d);
            }
        }  // operand is an instruction
    }      // for each operand

}  // Blk::DAGDataDeps

void Node::Print(llvm::raw_fd_ostream& dot) {
    for (auto& dep : incoming) {
        dep->Print(dot);
    }
}  // Node::Print

void Dep::Print(llvm::raw_fd_ostream& dot) {
    dot << "\"" << fn->FindNode(src)->GetName() << "\"->"
        << "\"" << fn->FindNode(dst)->GetName() << "\""
        << "[label=\"" << val->getName() << "\"]"
        << "\n";

}  // Dep::Print
