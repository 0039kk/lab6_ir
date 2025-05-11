#include "UnaryInstruction.h"

UnaryInstruction::UnaryInstruction(Function *func, 
                                  IRInstOperator op, 
                                  Value *srcVal, 
                                  Type *type)
    : Instruction(func, op, type) 
{
    addOperand(srcVal);  // 只添加一个操作数
}

void UnaryInstruction::toString(std::string &str) {
    Value *src = getOperand(0);
    switch(op) {
        case IRInstOperator::IRINST_OP_NEG_I:
            str = getIRName() + " = neg " + src->getIRName();
            break;
        default:
            Instruction::toString(str);
    }
}