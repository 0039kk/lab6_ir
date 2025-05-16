///
/// @file ArgInstruction.cpp
/// @brief 函数调用前的实参指令
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
#include <string>
#include "ArgInstruction.h"
#include "Function.h"
#include "VoidType.h"

/// @brief 函数实参指令
/// @param target 跳转目标
ArgInstruction::ArgInstruction(Function * _func, Value * src)
    : Instruction(_func, IRInstOperator::IRINST_OP_ARG, VoidType::getType())
{
    this->addOperand(src);
}

/// @brief 转换成字符串
std::string ArgInstruction::toString() const { // <--- 签名修改
    std::string result_str_build;
    Value *src = getOperand(0); // 确保 getOperand 是 const

    if (!src) {
        result_str_build = "; <Error: ArgInstruction has null operand>";
        return result_str_build;
    }

    // DragonIR 中没有 'arg' 指令。输出为注释。
    result_str_build = "; arg " + src->getIRName(); // 确保 getIRName 是 const

    int32_t regId_val;
    int64_t offset_val;
    // 确保 getRegId 和 getMemoryAddr 是 const
    if (src->getRegId() != -1) {
        result_str_build += " ; (reg: " + std::to_string(src->getRegId()) + ")";
    } else if (src->getMemoryAddr(&regId_val, &offset_val)) {
        result_str_build += " ; (mem: " + std::to_string(regId_val) + "[" + std::to_string(offset_val) + "])";
    }
    // 移除 func->realArgCountInc();
    return result_str_build;
}