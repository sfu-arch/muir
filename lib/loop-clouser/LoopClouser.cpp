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

#include "LoopClouser.h"

using namespace llvm;
using namespace std;
using namespace loopclouser;

extern cl::opt<string> target_fn;

namespace loopclouser {

char LoopClouser::ID = 0;
RegisterPass<LoopClouser> LL("lc", "Extracting loop clouser information");
}

bool LoopClouser::doInitialization(Module &M) {
    // TODO: Add code here if it's needed before running loop extraction
    return false;
}

bool LoopClouser::doFinalization(Module &M) {
    // TODO: Add code here to do post loop extraction
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

bool LoopClouser::runOnModule(Module &M) {
    for (auto &F : M) {
        if (F.isDeclaration()) continue;

        bool hasLoop = true;

        // Iterating over function's loops and start from
        // most inner loop to make it as a function call
        // and re-run the extraction, untill there is no other loops
        //do {
            //auto &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
            //auto &DT = getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();

            //if (getLoops(LI).size() == 0) hasLoop = false;

            // for (auto &L : getLoops(LI)) {
            // auto Loc = L->getStartLoc();
            // auto Filename = getBaseName(Loc->getFilename().str());
            // auto Line = Loc.getLine();
            //}

        //} while (hasLoop);
    }

    return false;
}

