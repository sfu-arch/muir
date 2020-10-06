

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Analysis/CFG.h"

#include "DebugInfo.h"

#include <features.h>
#include <map>


using namespace llvm;
using debuginfo::DebugInfo;


namespace debuginfo {

char DebugInfo::ID = 0;

}


uint32_t getUID(Instruction &I){
    auto *N = I.getMetadata("UID");
    if(N == nullptr)
        return 0;
    auto *S = dyn_cast<MDString>(N->getOperand(0));
    return stoi(S->getString().str());
}

// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool
DebugInfo::runOnModule(Module& m) {
  for (auto& f : m) {
    for (auto &bb : f) {
        //for( auto it = llvm::pred_begin(&bb), et = llvm::pred_end(&bb); it != et; it++){
            //BasicBlock *test = *it;
            //outs() << "BasicBlock: \n";
            //test->print(outs());
        //}
      for (auto& i : bb) {
          //for(auto &op: i.operands()){
              //op->print(outs());
          //}
          //for(auto &use: i.uses()){
              //use->print(outs());
          //}
         
          auto inst_uid = getUID(i);
          if(inst_uid == this->node_id){
            outs() << "\n Instruction (" << this->node_id << "): \n";
            i.print(outs());
            inst_bb.insert({&i, &bb});
          }
      }
    }
  }

  return false;
}

