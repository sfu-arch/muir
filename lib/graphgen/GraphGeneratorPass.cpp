#define DEBUG_TYPE "lx"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include "GraphGeneratorPass.h"

#include "Dandelion/Node.h"

using namespace llvm;
using namespace std;
using namespace graphgen;
using namespace dandelion;
// using graphgen::GraphGeneratorPass;
//
using InstructionList = std::list<InstructionNode>;
using ArgumentList = std::list<ArgumentNode>;
using BasicBlockList = std::list<SuperNode>;

extern cl::opt<string> XKETCHName;

// namespace graphgen {

char GraphGeneratorPass::ID = 0;

RegisterPass<GraphGeneratorPass> X("graphgen", "Generating xketch graph");

//}

bool GraphGeneratorPass::doInitialization(Module &M) {
    // TODO: Add code here if it's needed before running loop extraction
    return false;
}

bool GraphGeneratorPass::doFinalization(Module &M) {
    // TODO: Add code here to do post loop extraction
    return false;
}

bool GraphGeneratorPass::runOnModule(Module &M) {
    for (auto &F : M) {
        std::vector<SuperNode *> Graph;
        InstructionList tmp_ll;
        if (F.isDeclaration() || F.getName() != XKETCHName)
            continue;
        else {
            for (auto &BB : F) {
                // Filling instruction container for each BasicBlock
                // XXX The follwoing code doesn't work because of explicit
                // conversions
                // SmallVector<Instruction *, 16> tmp(BB.begin(), BB.end());

                SmallVector<Instruction *, 16> tmp;
                for (auto &ins : BB) {
                    tmp.push_back(&ins);
                    tmp_ll.push_back(InstructionNode(&ins));
                }
                SuperNode tmp_bb(tmp, &BB);
            }
        }

            // tmp_ll.push_back(InstructionNode(*it_ins));
    }

    // Graph tmp_graph(std::list<InstructionNode>())

    return false;
}
