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

/**
 * TODO: Right now we don't run loop simplify pass, because of cross checking
 * with our old output, but next step is to run simplify pass
 * because of the gauranties that the pass gives us
 */
LoopSummary LoopClouser::summarizeLoop(Loop *L, LoopInfo &LI) {
    LoopSummary summary;

    // Check if loop is in a simplify form
    if (!L->isLoopSimplifyForm()) {
        errs() << "[llvm-clouser] Loop not in Simplify Form\n";
    }

    // Start filling loop summary
    // 1)Loop enable signal and backward edge these two are always uniqu,
    summary.header = L->getHeader();
    for (auto _bb_it : llvm::predecessors(L->getHeader())) {
        // Forward edge
        if (!L->contains(_bb_it))summary.enable = _bb_it->getTerminator();
        // Backward edge
        else
            summary.loop_back = _bb_it->getTerminator();

        blacklist_control_edge[_bb_it->getTerminator()] = L->getHeader();
    }

    // 2)Looping exit branches
    L->getExitBlocks(summary.exit_blocks);
    for (auto _bb_it : summary.exit_blocks) {
        for (auto _bb_ex_it : llvm::predecessors(_bb_it)) {
            if (L->contains(_bb_ex_it)){
                summary.loop_finish.insert(_bb_ex_it->getTerminator());
                blacklist_control_edge[_bb_ex_it->getTerminator()] = _bb_it;
            }
        }
    }

    return summary;
}

bool LoopClouser::runOnModule(Module &M) {
    for (auto &F : M) {
        if (F.isDeclaration()) continue;



        // Iterating over function's loops and start from
        // most inner loop to make it as a function call
        // and re-run the extraction, untill there is no other loops
        auto &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
        auto loops = getLoops(LI);

        bool hasLoop = (loops.size() > 0);
        do {
            if (loops.size() == loop_sum.size()) break;

            for (auto &L : loops) {
                if (L->getSubLoops().size() == 0 || loop_sum.count(L) == 0) {
                    this->loop_sum.insert(std::make_pair(L, summarizeLoop(L, LI)));
                    outs() << "FIND LOOP\n";
                }
            }

        } while (hasLoop);
    }

    return false;
}

