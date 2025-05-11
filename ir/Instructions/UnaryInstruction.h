#pragma once
#include "Instruction.h"

class UnaryInstruction : public Instruction {
public:
    UnaryInstruction(Function *func, 
                    IRInstOperator op, 
                    Value *srcVal, 
                    Type *type);
    void toString(std::string &str) override;
};