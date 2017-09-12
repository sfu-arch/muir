#define DEBUG_TYPE "generator_amem"

#include <queue>
#include <typeinfo>
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include "AliasMem.h"

using namespace llvm;
using namespace amem;
using namespace std;

extern cl::opt<bool> aaTrace;

void AliasMem::findEdges(CallInst *CI, Function *OF) {
    // Get all the things we need to check
    // aliasing for
    SetVector<Instruction *> MemOps;
    ReversePostOrderTraversal<Function *> RPOT(OF);
    for (auto BB = RPOT.begin(); BB != RPOT.end(); ++BB) {
        for (auto &I : **BB) {
            if (isa<LoadInst>(&I) || isa<StoreInst>(&I)) {
                if (auto *SI = dyn_cast<StoreInst>(&I)) {
                    if (SI->getMetadata("LO") != nullptr) {
                        continue;
                    }
                }
                // Unlabeled shit will be undo log
                if (I.getMetadata("UID")) MemOps.insert(&I);
            }
        }
    }


    // Setup Arg-Param Map for use with IPAA
    // What if I set this up for all functions and their callsites
    // outs() << MemOps.size() << "\n";
    ValueToValueMapTy ArgParamMap;
    uint32_t Idx = 0;
    for (auto &A : OF->args()) {
        ArgParamMap[&A] = CI->getArgOperand(Idx++);
    }

    SmallVector<pair<uint32_t, uint32_t>, 16> t_NaiveAliasEdges;
    SmallVector<pair<uint32_t, uint32_t>, 16> t_AliasEdges;
    SmallVector<pair<uint32_t, uint32_t>, 16> t_MayAliasEdges;
    SmallVector<pair<uint32_t, uint32_t>, 16> t_MustAliasEdges;
    auto &AA = getAnalysis<AAResultsWrapperPass>(*OF).getAAResults();

    auto getUID = [](Instruction *I) -> uint32_t {
        auto *N = I->getMetadata("UID");
        auto *S = dyn_cast<MDString>(N->getOperand(0));
        return stoi(S->getString().str());
    };

    // Class template std::function is a general-purpose polymorphic function
    // wrapper.
    function<Value *(Value *)> getPtr;

    auto getPtrWrapper = [&getPtr](Value *V) -> Value * {
        if (auto *LI = dyn_cast<LoadInst>(V)) {
            auto *Ptr = LI->getPointerOperand();
            return getPtr(Ptr);
        } else if (auto *SI = dyn_cast<StoreInst>(V)) {
            auto *Ptr = SI->getPointerOperand();
            return getPtr(Ptr);
        }
        llvm_unreachable("Nope");
        return nullptr;
    };

    getPtr = [&getPtr, &ArgParamMap](Value *V) -> Value * {
        if (auto *GEP = dyn_cast<GEPOperator>(V)) {
            auto *Ptr = GEP->getPointerOperand();
            return getPtr(Ptr);
        } else if (auto *A = dyn_cast<Argument>(V)) {
            // Since we only mapped the offload callsite
            // and its args this ensures that we bound the
            // recursion depth with respect to the callgraph
            if (ArgParamMap.count(A))
                return getPtr(ArgParamMap[V]);
            else
                return nullptr;
        } else if (isa<LoadInst>(V)) {
            return nullptr;
        } else if (auto *BC = dyn_cast<BitCastInst>(V)) {
            auto *Ptr = BC->getOperand(0);
            return getPtr(Ptr);
        }

        return V;
    };

    Data["num-aa-pairs"] = 0;
    Data["num-no-alias"] = 0;
    Data["num-must-alias"] = 0;
    Data["num-partial-alias"] = 0;
    Data["num-may-alias-naive"] = 0;
    Data["num-ld-ld-pairs"] = 0;
    Data["num-ipaa-no-alias"] = 0;
    Data["num-ipaa-may-alias"] = 0;

    for (auto MB = MemOps.begin(), ME = MemOps.end(); MB != ME; MB++) {
        for (auto NB = next(MB); NB != ME; NB++) {
            Data["num-aa-pairs"]++;

            if (isa<LoadInst>(*MB) && isa<LoadInst>(*NB)) {
                Data["num-ld-ld-pairs"]++;
                continue;
            }

            switch (
                AA.alias(MemoryLocation::get(*MB), MemoryLocation::get(*NB))) {
                case AliasResult::NoAlias:
                    Data["num-no-alias"]++;
                    break;
                case AliasResult::MustAlias:
                    must_edge.push_back({*MB, *NB});
                    Data["num-must-alias"]++;
                    t_AliasEdges.push_back({getUID(*MB), getUID(*NB)});
                    t_NaiveAliasEdges.push_back({getUID(*MB), getUID(*NB)});
                    t_MustAliasEdges.push_back({getUID(*MB), getUID(*NB)});
                    break;
                case AliasResult::PartialAlias:
                    must_edge.push_back({*MB, *NB});
                    Data["num-partial-alias"]++;
                    t_MayAliasEdges.push_back({getUID(*MB), getUID(*NB)});
                    t_AliasEdges.push_back({getUID(*MB), getUID(*NB)});
                    t_NaiveAliasEdges.push_back({getUID(*MB), getUID(*NB)});
                    break;
                case AliasResult::MayAlias: {
                    must_edge.push_back({*MB, *NB});
                    Data["num-may-alias-naive"]++;
                    auto *P = getPtrWrapper(*MB);
                    auto *Q = getPtrWrapper(*NB);
                    t_NaiveAliasEdges.push_back({getUID(*MB), getUID(*NB)});
                    if (P && Q && P != Q) {
                        if (!isa<AllocaInst>(P) && !isa<GlobalValue>(P)) {
                            errs() << "checkP: " << *P << "\n";
                            errs() << "checkQ: " << *Q << "\n";
                        }
                        Data["num-ipaa-no-alias"]++;
                    } else {
                        Data["num-ipaa-may-alias"]++;
                        t_AliasEdges.push_back({getUID(*MB), getUID(*NB)});
                        t_MayAliasEdges.push_back({getUID(*MB), getUID(*NB)});
                    }
                    break;
                }
            }
        }
    }

    //Saving analysis information
    AliasContainer map_entry(OF, CI);

    //Write down alis analysis information to file
    MustAliasEdges[map_entry] = t_MustAliasEdges;

    if(aaTrace){
        //Dumping alias information to file
        ofstream MustEdgeFile((OF->getName() + ".must.txt").str(), ios::out);
        for (auto P : t_MustAliasEdges) {
            MustEdgeFile << P.first << " " << P.second << "\n";
        }
        MustEdgeFile.close();

        ofstream EdgeFile((OF->getName() + ".aa.txt").str(), ios::out);
        for (auto P : t_AliasEdges) {
            EdgeFile << P.first << " " << P.second << "\n";
        }
        EdgeFile.close();

        ofstream NaiveEdgeFile((OF->getName() + ".naiveaa.txt").str(), ios::out);
        for (auto P : t_NaiveAliasEdges) {
            NaiveEdgeFile << P.first << " " << P.second << "\n";
        }
        NaiveEdgeFile.close();

        ofstream MayEdgeFile((OF->getName() + ".may.txt").str(), ios::out);
        for (auto P : t_MayAliasEdges) {
            MayEdgeFile << P.first << " " << P.second << "\n";
        }
        MayEdgeFile.close();
    }
}

/**
 * We start from top level function and start traversing the nested functions
 * The target function should have been called in the program
 * Here we traverse the target function and run the analysis an all
 * the child functions
 */
bool AliasMem::runOnModule(Module &M) {
    DenseMap<StringRef, SmallVector<CallInst *, 1>> Map;
    std::queue<CallInst *> call_site_depth;

    for (auto &F : M) {
        if (F.isDeclaration()) continue;
        for (auto &BB : F) {
            for (auto &I : BB) {
                if (auto *CI = dyn_cast<CallInst>(&I)) {
                    if (auto *F = CI->getCalledFunction()) {
                        if (F->getName() == this->FunctionName) {
                            if (Map.count(F->getName()) == 0) {
                                Map.insert({F->getName(),
                                            SmallVector<CallInst *, 1>()});
                            }
                            Map[F->getName()].push_back(CI);
                            call_site_depth.push(CI);
                        }
                    }
                }
            }
        }
    }

    assert(Map.size() == 1 && "Only one extracted function at the moment");

    /**
     * We relax this limitation and process all the callsites
     */
    while(!call_site_depth.empty()) {
        auto temp_call = call_site_depth.front();
        auto called_function = temp_call->getCalledFunction();
        for (auto &BB : *called_function) {
            for (auto &I : BB) {
                if (auto *CI = dyn_cast<CallInst>(&I)) {
                    if (auto *F = CI->getCalledFunction()) {
                        if (F->isDeclaration())
                            continue;
                        else {
                            if (Map.count(F->getName()) == 0) {
                                Map.insert({F->getName(),
                                            SmallVector<CallInst *, 1>()});
                            }
                            Map[F->getName()].push_back(CI);
                            call_site_depth.push(CI);
                        }
                    }
                }
            }
        }
        call_site_depth.pop();
    }


    for (auto &KV : Map) {
        assert(KV.second.size() == 1 && "Only one call site at the moment");
        for (auto &CI : KV.second) {
            auto *OF = CI->getCalledFunction();
            findEdges(CI, OF);
        }
    }

    return false;
}

bool AliasMem::doInitialization(Module &M) {
    Data.clear();
    return false;
}

bool AliasMem::doFinalization(Module &M) {
    ofstream Outfile("amem.stats.txt", ios::out);
    for (auto KV : Data) {
        Outfile << KV.first << " " << KV.second << "\n";
    }
    return false;
}

char AliasMem::ID = 0;

static RegisterPass<AliasMem> X("amem", "Alias edge writer pass");
