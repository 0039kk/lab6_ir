///
/// @file MoveInstruction.cpp
/// @brief Move指令，也就是DragonIR的Asssign指令
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

#include "VoidType.h"

#include "MoveInstruction.h"
#include "Value.h"
///
/// @brief 构造函数
/// @param _func 所属的函数
/// @param result 结构操作数
/// @param srcVal1 源操作数
///
MoveInstruction::MoveInstruction(Function * _func, Value * _result, Value * _srcVal1)
    : Instruction(_func, IRInstOperator::IRINST_OP_ASSIGN, VoidType::getType())
{
    addOperand(_result);
    addOperand(_srcVal1);
}

/// @brief 转换成字符串显示
/// @param str 转换后的字符串
std::string MoveInstruction::toString() const { // 返回 std::string，无参数
    std::string result_str; // 用于构建结果字符串

    // 假设 getOperand(0) 返回目标 Value*
    // 假设 getOperand(1) 返回源 Value*
    // 并且这些 Value* 存储在 Instruction/User 的操作数列表中
    Value *dstVal = getOperand(0);
    Value *srcVal = getOperand(1);

    if (dstVal && srcVal) {
        // 假设 Value::getName() 返回该值的IR字符串表示 (例如 "%l0", "%t1", "@gvar")
        // 并且它是 const 方法
        result_str = dstVal->getIRName() + " = " + srcVal->getIRName();
    } else {
        result_str = "; <Error: MoveInstruction has invalid operands>"; // 更明确的错误信息
    }
    return result_str; // 返回构建好的字符串
}