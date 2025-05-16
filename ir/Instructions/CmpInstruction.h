// ir/Instructions/CmpInstruction.h
#pragma once

#include "Instruction.h" // 继承自 Instruction
#include "Value.h"       // 操作数是 Value*
#include <string>

// 前向声明，避免循环依赖 (如果需要)
class Function; 
class Type;

/// @brief 关系比较指令 (cmp)
/// @details 例如: %t1 = cmp lt %op1, %op2
class CmpInstruction : public Instruction {
public:
    /// @brief Cmp指令的比较操作符枚举
    enum CmpOp {
        EQ, // ==
        NE, // !=
        GT, // >
        GE, // >=
        LT, // <
        LE  // <=
    };

private:
    Value *dest_reg_;      // 目标寄存器/临时变量 (存储比较结果, i1类型)
    CmpOp cmp_operator_;   // 比较操作符
    Value *operand1_;      // 左操作数
    Value *operand2_;      // 右操作数

public:
    /// @brief 构造函数
    /// @param dest 存储比较结果的Value (必须是i1类型)
    /// @param op 比较操作符
    /// @param op1 左操作数
    /// @param op2 右操作数
    /// @param parent_func 父函数
    CmpInstruction(Value *dest, CmpOp op, Value *op1, Value *op2, Function *parent_func);

    // --- Getter 方法 ---
    [[nodiscard]] Value* getDest() const { return dest_reg_; }
	[[nodiscard]]CmpOp getOperator() const { return cmp_operator_; }
	[[nodiscard]]Value *getOperand1() const { return operand1_; }
	[[nodiscard]]Value *getOperand2() const { return operand2_; }

    /// @brief 将CmpOp枚举转换为字符串表示 (用于IR打印)
    static std::string CmpOpToString(CmpOp op);

    /// @brief 将字符串表示的比较操作符转换为CmpOp枚举
    static CmpOp StringToCmpOp(const std::string& op_str);


    /// @brief 获取指令的操作码的字符串表示 (可选, 如果 Instruction 基类需要)
    // std::string getOpcodeName() const override { return "cmp"; } // 如果有这样的虚函数

    /// @brief 打印IR指令 (重写Instruction的虚函数)
	[[nodiscard]] std::string toString() const override;
};