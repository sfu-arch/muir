//===- GEPSplitter.cpp - Split complex GEPs into simple ones --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This function breaks GEPs with more than 2 non-zero operands into smaller
// GEPs each with no more than 2 non-zero operands. This exposes redundancy
// between GEPs with common initial operand sequences.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "split-geps"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
//#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Scalar.h"

#include "GEPSplitter.h"
//#include "AllocaCount.h"
//#include "Common.h"

using namespace llvm;
// using namespace allocacount;
using gepsplitter::GEPSplitter;

namespace gepsplitter {
    char GEPSplitter::ID = 0;
    static RegisterPass<GEPSplitter> X("split-geps",
                                       "split complex GEPs into simple GEPs");
}


bool GEPSplitter::runOnFunction(Function &F) {

    bool Changed = false;

    // Visit each GEP instruction.
    for (Function::iterator I = F.begin(), E = F.end(); I != E; ++I)
        for (BasicBlock::iterator II = I->begin(), IE = I->end(); II != IE;)
            if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(II++)) {
                uint32_t NumOps = GEP->getNumOperands();
                // Ignore GEPs which are already simple.
                if (NumOps <= 2) continue;
                bool FirstIndexIsZero =
                        isa<ConstantInt>(GEP->getOperand(1)) &&
                        cast<ConstantInt>(GEP->getOperand(1))->isZero();

                if (NumOps == 3 && FirstIndexIsZero) continue;

                // The first index is special and gets expanded with a 2-operand
                // GEP
                // (unless it's zero, in which case we can skip this).
                Value *NewGEP =
                        FirstIndexIsZero
                        ? GEP->getOperand(0)
                        : GetElementPtrInst::Create(
                                GEP->getSourceElementType(), GEP->getOperand(0),
                                GEP->getOperand(1), "tmp", GEP);

                // All remaining indices get expanded with a 3-operand GEP with
                // zero
                // as the second operand.
                //
                std::vector<Value *> Idxs(2);
                Idxs[0] = ConstantInt::get(Type::getInt64Ty(F.getContext()), 0);
                for (unsigned i = 2; i != NumOps; ++i) {
                    Idxs[1] = GEP->getOperand(i);

                    NewGEP = GetElementPtrInst::Create(
                            nullptr, NewGEP, llvm::ArrayRef<llvm::Value *>(Idxs),
                            "tmp", GEP);
                }
                GEP->replaceAllUsesWith(NewGEP);
                GEP->eraseFromParent();
                Changed = true;
            }

    return Changed;
}

void GEPSplitter::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
}
