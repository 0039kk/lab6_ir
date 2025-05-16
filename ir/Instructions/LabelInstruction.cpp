///
/// @file LabelInstruction.cpp
/// @brief Label指令
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
#include "Function.h" 
#include "LabelInstruction.h"
#include "Value.h" // For setName/setIRName
///
/// @brief 构造函数
/// @param _func 所属函数
///


LabelInstruction::LabelInstruction(Function* _func, const std::string& name_for_label)
    : Instruction(_func, IRInstOperator::IRINST_OP_LABEL, VoidType::getType()) {
    this->setIRName(name_for_label); // <--- 核心: 创建时就设置最终的IRName
    this->setName(name_for_label);   // (可选) 也可以设置原始 name
    // 移除 this->name_ = name; 如果 name_ 是独立成员
}

std::string LabelInstruction::toString() const {
    return this->getIRName() + ":"; // 使用 getIRName()
}