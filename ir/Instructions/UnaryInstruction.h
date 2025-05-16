#pragma once
#include "Instruction.h"

class UnaryInstruction : public Instruction {
public:
    UnaryInstruction(Function *func, 
                    IRInstOperator op, 
                    Value *srcVal, 
                    Type *type);
    std::string toString() const override;
};