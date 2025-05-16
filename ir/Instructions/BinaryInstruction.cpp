///
/// @file BinaryInstruction.cpp
/// @brief 二元操作指令
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-09-29
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// </table>
///
#include "BinaryInstruction.h"

/// @brief 构造函数
/// @param _op 操作符
/// @param _result 结果操作数
/// @param _srcVal1 源操作数1
/// @param _srcVal2 源操作数2
BinaryInstruction::BinaryInstruction(Function * _func,
                                     IRInstOperator _op,
                                     Value * _srcVal1,
                                     Value * _srcVal2,
                                     Type * _type)
    : Instruction(_func, _op, _type)
{
    addOperand(_srcVal1);
    addOperand(_srcVal2);
}

/// @brief 转换成字符串
/// @param str 转换后的字符串
// 在 BinaryInstruction.cpp 中
#include "BinaryInstruction.h" // 确保包含头文件
#include "Value.h"             // 为了 Value::getIRName() 或 Value::getName()

std::string BinaryInstruction::toString() const {
    std::string result_str;
    std::string op_str;

    // 假设 getOperand(0) 和 getOperand(1) 返回 Value*
    // 并且它们是 const 方法
    Value *src1 = getOperand(0);
    Value *src2 = getOperand(1);

    // 假设 'op' 是 BinaryInstruction 的成员变量 (IRInstOperator 类型)
    // 或者通过 getOpcode() 获取
    // IRInstOperator current_op = getOpcode();

    // 假设 getIRName() 返回指令结果的IR名称 (例如 %t0)
    // 并且它是 Instruction 的 const 方法

	if (!src1 || !src2) {
        return "; <Error: BinaryInstruction has null operands>";
    }

    switch (op) { // 或者 switch(current_op)
        case IRInstOperator::IRINST_OP_ADD_I: op_str = "add"; break;
        case IRInstOperator::IRINST_OP_SUB_I: op_str = "sub"; break;
        case IRInstOperator::IRINST_OP_MUL_I: op_str = "mul"; break;
        case IRInstOperator::IRINST_OP_DIV_I: op_str = "div"; break;
        case IRInstOperator::IRINST_OP_MOD_I: op_str = "mod"; break;
        default:
            // 如果 Instruction::toString() 是纯虚的或不适用，提供一个默认错误消息
            return "; <Error: Unknown binary operator in BinaryInstruction::toString()>";
    }

    // DragonIR 格式: 变量 = 算术运算符 变量, 变量
    result_str = this->getIRName() + " = " + op_str + " " +
                 src1->getIRName() + ", " + src2->getIRName();

    return result_str;
}