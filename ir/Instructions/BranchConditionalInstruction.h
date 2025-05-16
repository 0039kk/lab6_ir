// ir/Instructions/BranchConditionalInstruction.h
#pragma once

#include "Instruction.h"
#include "Value.h" // 条件变量是 Value*
// #include "LabelInstruction.h" // 标签是 LabelInstruction*

// 前向声明
class LabelInstruction; // 目标标签
class Function;

/// @brief 条件跳转指令 (bc)
/// @details 例如: bc %cond, label .TrueLabel, label .FalseLabel
class BranchConditionalInstruction : public Instruction {
private:
    Value *condition_reg_;       // 条件寄存器/临时变量 (i1类型)
    LabelInstruction *true_target_;  // 条件为真时的跳转目标标签
    LabelInstruction *false_target_; // 条件为假时的跳转目标标签

public:
    /// @brief 构造函数
    /// @param cond 条件Value (必须是i1类型)
    /// @param true_label 真出口标签
    /// @param false_label 假出口标签
    /// @param parent_func 父函数
    BranchConditionalInstruction(Value *cond, LabelInstruction *true_label, LabelInstruction *false_label, Function *parent_func);

    // --- Getter 方法 ---
	[[nodiscard]]Value *getCondition() const { return condition_reg_; }
	[[nodiscard]]LabelInstruction *getTrueTarget() const { return true_target_; }
	[[nodiscard]]LabelInstruction *getFalseTarget() const { return false_target_; }

    /// @brief 打印IR指令
	[[nodiscard]] std::string toString() const override;
    
    /// @brief 条件跳转指令是终结符指令
	[[nodiscard]] bool isTerminator() const override { return true; }
};