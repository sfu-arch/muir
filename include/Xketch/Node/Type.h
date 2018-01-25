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
        GlobalTyID = 0,
        ConstTyID,
        BasicBlockTyID,
        UMBasicBlockTyID,
        MBasicBlockTyID,
        InstructionTyID
    };

    //Returning type information
    TypeID getTypeID() const { return ID; }

   private:
    TypeID ID : 1;

   protected:
    explicit Type(TypeID tid) : ID(tid) {}
    ~Type() = default;

};

// template <typename NodeTy> class SymbolTableListTraits;

}  // End dandelion namespace

#endif  // XKETCH_TYPE_H
