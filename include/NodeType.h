#ifndef NODETYPE_H
#define NODETYPE_H

#include <map>
#include <string>

namespace common {

enum InstructionType {
    TBinaryOperator,
    TICmpInst,
    TBranchInst,
    TUBranchInst,
    TCBranchInst,
    TPHINode,
    TAlloca,
    TGEP,
    TLoad,
    TStore,
    TSEXT,
    TZEXT,
    TPtrToInt,
    TBitCast,
    TTrunc,
#ifdef TAPIR
    TDetach,
    TReattach,
    TSync,
#endif
    TReturnInst,
    TNULL
};

struct InstructionInfo {
    typedef std::map<InstructionType, std::string> InstructionName;
    static InstructionName instruction_name_type;
};

}
#endif
