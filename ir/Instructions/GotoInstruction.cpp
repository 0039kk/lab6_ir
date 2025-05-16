///
/// @file GotoInstruction.cpp
/// @brief 无条件跳转指令即goto指令
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

#include "GotoInstruction.h"
#include "Value.h" 
///
/// @brief 无条件跳转指令的构造函数
/// @param target 跳转目标
///
GotoInstruction::GotoInstruction(Function * _func, LabelInstruction * _target_label) // 参数应为 LabelInstruction*
    : Instruction(_func, IRInstOperator::IRINST_OP_GOTO, VoidType::getType()) {
    this->target = _target_label; // 赋值给成员变量 this->target
}

/// @brief 转换成IR指令文本
std::string GotoInstruction::toString() const {
    std::string result_str;
    if (this->target) { // this->target 是 LabelInstruction*
        result_str = "br label " + this->target->getIRName(); // <--- 使用 getIRName()
    } else {
        result_str = "; <Error: GotoInstruction has null target>";
    }
    return result_str;
}
///
/// @brief 获取目标Label指令
/// @return LabelInstruction* label指令
///
LabelInstruction * GotoInstruction::getTarget() const
{
    return target;
}
