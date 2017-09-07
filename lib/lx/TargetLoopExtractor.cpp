#define DEBUG_TYPE "lx"

#include "llvm/ADT/SmallString.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include "TargetLoopExtractor.h"

using namespace llvm;
using namespace std;
using namespace lx;

extern cl::opt<string> XKETCHName;

bool TargetLoopExtractor::doInitialization(Module &M) {
    //TODO: Add code here if it's needed before running loop extraction
    return false;
}

bool TargetLoopExtractor::doFinalization(Module &M) {
    //TODO: Add code here to do post loop extraction
    return false;
}

//! Adapted from lib/Transform/IPO/LoopExtractor.cpp
bool TargetLoopExtractor::extractLoop(Loop *L, LoopInfo &LI, DominatorTree &DT,
                                      string Name) {
    bool ShouldExtractLoop = true;

    if (!L->isLoopSimplifyForm()) {
        errs() << "[llvm-lx] Loop not in Simplify Form\n";
        ShouldExtractLoop = false;
    }

    // Split all the exit basic blocks to ensure that in case there is a
    // exit basic block which has work done in the block before return, then
    // we should split the block and leave the extra instructions outside the
    // extracted function.

    SmallVector<BasicBlock *, 8> ExitBlocks;
    L->getExitBlocks(ExitBlocks);

    for (unsigned i = 0, e = ExitBlocks.size(); i != e; ++i) {
        SplitBlock(ExitBlocks[i], &*ExitBlocks[i]->getFirstInsertionPt(), &DT,
                   &LI);
    }

    if (ShouldExtractLoop) {
        // We must omit landing pads. Landing pads must accompany the invoke
        // instruction. But this would result in a loop in the extracted
        // function. An infinite cycle occurs when it tries to extract that loop
        // as
        // well.
        SmallVector<BasicBlock *, 8> ExitBlocks;
        L->getExitBlocks(ExitBlocks);
        for (unsigned i = 0, e = ExitBlocks.size(); i != e; ++i) {
            if (ExitBlocks[i]->isLandingPad()) {
                ShouldExtractLoop = false;
                errs() << "[llvm-lx] Omit Landing Pad Instructions\n";
                break;
            }
        }
    }

    if (ShouldExtractLoop) {
        CodeExtractor Extractor(DT, *L);
        Function *F = Extractor.extractCodeRegion();
        if (F) {
            F->setName(Name);
            F->addFnAttr(Attribute::NoInline);
            stripDebugInfo(*F);
            ExtractedLoopFunctions.insert(F);
            return true;
        }
    }

    return false;
}

static SetVector<Loop *> getLoops(LoopInfo &LI) {
    SetVector<Loop *> Loops;

    for (auto &L : LI) {
        for (auto &SL : L->getSubLoops()) {
            Loops.insert(SL);
        }
        Loops.insert(L);
    }

    return Loops;
}

static string getBaseName(string Path) {
    auto Idx = Path.find_last_of('/');
    return Idx == string::npos ? Path : Path.substr(Idx + 1);
}

static string getLXName(string File, size_t Line) {
    return string("_lx_") + File + string("_L") + to_string(Line);
}

bool TargetLoopExtractor::runOnModule(Module &M) {
    for (auto &F : M) {
        if (F.isDeclaration() || ExtractedLoopFunctions.count(&F)) continue;

        bool hasLoop = true;

        //Iterating over function's loops and start from
        //most inner loop to make it as a function call
        //and re-run the extraction, untill there is no other loops
        do {
            auto &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
            auto &DT = getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();

            if(getLoops(LI).size() == 0)
                hasLoop = false;

            for (auto &L : getLoops(LI)) {
                auto Loc = L->getStartLoc();
                auto Filename = getBaseName(Loc->getFilename().str());
                auto Line = Loc.getLine();

                if (L->getSubLoops().size() == 0) {
                    if (!extractLoop(L, LI, DT, getLXName(Filename, Line))) {
                        errs() << "[llvm-lx] Unable to extract loop at "
                               << Filename << ":" << Line << "\n";
                    }
                }
            }

        } while (hasLoop);
    }

    return false;
}

char TargetLoopExtractor::ID = 0;
