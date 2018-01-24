//===----------------------------------------------------------------------===//
//
// This file declares the Node class.
//
//===----------------------------------------------------------------------===//

#ifndef XKETCH_TYPE_H
#define XKETCH_TYPE_H

namespace dandelion {

class Type {
   public:
    enum TypeID {
        FunctionTyID = 0,
        GlobalTyID,
        ConstTyID,
        BasicBlockTyID,
        UMBasicBlockTyID,
        MBasicBlockTyID
    };

   private:
    TypeID : 0;

   protected:
};

// template <typename NodeTy> class SymbolTableListTraits;

}  // End dandelion namespace

#endif  // XKETCH_TYPE_H
