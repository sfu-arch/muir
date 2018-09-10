
#ifndef GEPSPLITTER_H
#define GEPSPLITTER_H


#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "Common.h"


namespace gepsplitter {


    class GEPSplitter : public llvm::FunctionPass {

        virtual bool runOnFunction(llvm::Function &F);
        virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

    public:
        explicit GEPSplitter() : FunctionPass(ID) {}
        static char ID; // Pass identification, replacement for typeid

    };
}

#endif
