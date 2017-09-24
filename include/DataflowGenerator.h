

#ifndef STATICCALLCOUNTER_H
#define STATICCALLCOUNTER_H


#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <string>


namespace codegen{


struct DataflowGenerator: public llvm::ModulePass {

  static char ID;

  //Default value is standard out
  llvm::raw_ostream &outCode;

  DataflowGenerator()
    : llvm::ModulePass(ID),outCode(llvm::outs())
      { }

  DataflowGenerator(llvm::raw_ostream &out)
    : llvm::ModulePass(ID),
    outCode(out)
      { }

  virtual bool runOnModule(llvm::Module &m) override;

  void printCode(std::string code);

  void setOutput(llvm::raw_ostream &out);

  /**
   * Print method gets called right after the pass finishes
   */
  //virtual void print(llvm::raw_ostream &out,
                     //llvm::Module const *m) const override;

};


}


#endif

