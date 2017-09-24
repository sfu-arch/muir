

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

#include "DataflowGenerator.h"

using namespace llvm;
using namespace std;
using codegen::DataflowGenerator;

namespace codegen {

char DataflowGenerator::ID = 0;

RegisterPass<DataflowGenerator> X("codegen", "Generating chisel code");
}

// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool DataflowGenerator::runOnModule(Module &m) {
    std::error_code errc; 
    raw_fd_ostream out("amirali.test", errc, sys::fs::F_None);
    out << "Amirali";
    for (auto &f : m) {
        stripDebugInfo(f);
        for (auto &bb : f) {
            for (auto &i : bb) {
                i.dump();
            }
        }
    }

    return false;
}

/**
 * Printing the input code
 */
void DataflowGenerator::printCode(string code) {
    this->outCode << code << "\n";
}

// Output for a pure analysis pass should happen in the print method.
// It is called automatically after the analysis pass has finished collecting
// its information.
// void
// DataflowGenerator::print(raw_ostream &out, Module const *m) const {
// out << "Function Counts\n"
//<< "===============\n";
// for (auto &kvPair : counts) {
// auto *function = kvPair.first;
// uint64_t count = kvPair.second;
// out << function->getName() << " : " << count << "\n";
//}
//}
