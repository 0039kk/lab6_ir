#include "UnaryInstruction.h"

UnaryInstruction::UnaryInstruction(Function *func, 
                                  IRInstOperator op, 
                                  Value *srcVal, 
                                  Type *type)
    : Instruction(func, op, type) 
{
    addOperand(srcVal);  // 只添加一个操作数
}

std::string UnaryInstruction::toString() const { // 返回 std::string，无参数
    std::string result_str; // 用于构建结果字符串
    Value *src = getOperand(0); // 假设 getOperand(0) 返回 Value* 并且是 const 的

    // 假设 'op' 是 UnaryInstruction 的成员变量，代表操作码
    // 或者通过 getOpcode() 获取
    // IRInstOperator current_op = getOpcode(); // 如果 op 不是直接成员

    switch(op) { // 或者 switch(current_op)
        case IRInstOperator::IRINST_OP_NEG_I:
            // 假设 getIRName() 是 Instruction 或 Value 的方法，返回指令/值的IR表示
            // 并且 src->getIRName() 也是 const 的
            result_str = getIRName() + " = neg " + src->getIRName();
            break;
        default:
            result_str = "; Unhandled UnaryInstruction operator in toString()"; // 或者抛出异常
            break;
    }
    return result_str; // 返回构建好的字符串
}