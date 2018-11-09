#define DEBUG_TYPE "aew"
#include "llvm/IR/DebugInfo.h"
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
#include "llvm/Support/Debug.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include "AliasEdgeWriter.h"
#include "Common.h"

using namespace llvm;
using namespace std;
using namespace aew;

extern cl::opt<string> target_fn;

void AliasEdgeWriter::writeEdges(CallInst *CI, Function *OF) {
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

    // errs() << "MemOps\n";
    // for(auto &M : MemOps) {
    //     errs() << *M << "\n";
    // }

    // Setup Arg-Param Map for use with IPAA
    // What if I set this up for all functions and their callsites
    ValueToValueMapTy ArgParamMap;
    uint32_t Idx = 0;
    for (auto &A : OF->args()) {
        ArgParamMap[&A] = CI->getArgOperand(Idx++);
    }

    auto &AA = getAnalysis<AAResultsWrapperPass>(*OF).getAAResults();

    auto getUID = [](Instruction *I) -> uint32_t {
        auto *N = I->getMetadata("UID");
        auto *S = dyn_cast<MDString>(N->getOperand(0));
        return stoi(S->getString().str());
    };

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
            // Since we only mapped the target callsite
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
                    Data["num-must-alias"]++;
                    AliasEdges.push_back({getUID(*MB), getUID(*NB)});
                    NaiveAliasEdges.push_back({getUID(*MB), getUID(*NB)});
                    MustAliasEdges.push_back({getUID(*MB), getUID(*NB)});

                    AliasEdgesMap[*MB].push_back(*NB);
                    NaiveAliasEdgesMap[*MB].push_back(*NB);
                    MustAliasEdgesMap[*MB].push_back(*NB);
                    break;
                case AliasResult::PartialAlias:
                    Data["num-partial-alias"]++;
                    MayAliasEdges.push_back({getUID(*MB), getUID(*NB)});
                    AliasEdges.push_back({getUID(*MB), getUID(*NB)});
                    NaiveAliasEdges.push_back({getUID(*MB), getUID(*NB)});

                    MayAliasEdgesMap[*MB].push_back(*NB);
                    AliasEdgesMap[*MB].push_back(*NB);
                    NaiveAliasEdgesMap[*MB].push_back(*NB);

                    break;
                case AliasResult::MayAlias: {
                    Data["num-may-alias-naive"]++;
                    auto *P = getPtrWrapper(*MB);
                    auto *Q = getPtrWrapper(*NB);
                    NaiveAliasEdges.push_back({getUID(*MB), getUID(*NB)});
                    NaiveAliasEdgesMap[*MB].push_back(*NB);
                    if (P && Q && P != Q) {
                        if (!isa<AllocaInst>(P) && !isa<GlobalValue>(P)) {
                            DEBUG(errs() << "checkP: " << *P << "\n");
                            DEBUG(errs() << "checkQ: " << *Q << "\n");
                        }
                        Data["num-ipaa-no-alias"]++;
                    } else {
                        Data["num-ipaa-may-alias"]++;
                        AliasEdges.push_back({getUID(*MB), getUID(*NB)});
                        MayAliasEdges.push_back({getUID(*MB), getUID(*NB)});

                        AliasEdgesMap[*MB].push_back(*NB);
                        MayAliasEdgesMap[*MB].push_back(*NB);
                    }
                    break;
                }
            }
        }
    }

    if (print) {
        ofstream MustEdgeFile((OF->getName() + ".must.txt").str(), ios::out);
        for (auto P : MustAliasEdges) {
            MustEdgeFile << P.first << " " << P.second << "\n";
        }
        MustEdgeFile.close();

        ofstream EdgeFile((OF->getName() + ".aa.txt").str(), ios::out);
        for (auto P : AliasEdges) {
            EdgeFile << P.first << " " << P.second << "\n";
        }
        EdgeFile.close();

        ofstream NaiveEdgeFile((OF->getName() + ".naiveaa.txt").str(),
                               ios::out);
        for (auto P : NaiveAliasEdges) {
            NaiveEdgeFile << P.first << " " << P.second << "\n";
        }
        NaiveEdgeFile.close();

        ofstream MayEdgeFile((OF->getName() + ".may.txt").str(), ios::out);
        for (auto P : MayAliasEdges) {
            MayEdgeFile << P.first << " " << P.second << "\n";
        }
        MayEdgeFile.close();
    }
}

bool AliasEdgeWriter::runOnModule(Module &M) {
    DenseMap<StringRef, SmallVector<CallInst *, 1>> Map;

    for (auto &F : M) {
        if (F.isDeclaration()) continue;

        // We need to label the instructions first
        if (F.getName() == target_fn.getValue()) {
            helpers::FunctionUIDLabel(F);
        }
        for (auto &BB : F) {
            for (auto &I : BB) {
                if (auto *CI = dyn_cast<CallInst>(&I)) {
                    if (auto *F = CI->getCalledFunction()) {
                        if (F->getName() == target_fn.getValue()) {
                            if (Map.count(F->getName()) == 0) {
                                Map.insert({F->getName(),
                                            SmallVector<CallInst *, 1>()});
                            }
                            Map[F->getName()].push_back(CI);
                        }
                    }
                }
            }
        }
    }

    assert(Map.size() == 1 && "Only one extracted function at the moment");

    for (auto &KV : Map) {
        assert(KV.second.size() == 1 && "Only one call site at the moment");
        for (auto &CI : KV.second) {
            auto *OF = CI->getCalledFunction();
            writeEdges(CI, OF);
        }
    }

    return false;
}

bool AliasEdgeWriter::doInitialization(Module &M) {
    Data.clear();
    return false;
}

bool AliasEdgeWriter::doFinalization(Module &M) {
    ofstream Outfile("aew.stats.txt", ios::out);
    for (auto KV : Data) {
        Outfile << KV.first << " " << KV.second << "\n";
    }
    return false;
}

char AliasEdgeWriter::ID = 0;
RegisterPass<AliasEdgeWriter> X("aew", "Print memory aliases");
