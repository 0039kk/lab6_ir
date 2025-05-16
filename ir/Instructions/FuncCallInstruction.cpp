///
/// @file FuncCallInstruction.cpp
/// @brief 函数调用指令
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
#include "FuncCallInstruction.h"
#include "Function.h"
#include "Common.h"
#include "Type.h"
#include "VoidType.h"

/// @brief 含有参数的函数调用
/// @param srcVal 函数的实参Value
/// @param result 保存返回值的Value
FuncCallInstruction::FuncCallInstruction(Function* current_func_scope,Function* target_func,std::vector<Value*>& args,Type* result_type_if_any)
	: Instruction(current_func_scope,
	IRInstOperator::IRINST_OP_FUNC_CALL,
	result_type_if_any ? result_type_if_any : VoidType::getType()), // <--- 修改这里
	calledFunction(target_func) {
	for (Value* arg_val : args) {
	if (arg_val) {
	addOperand(arg_val);
	} else {
	minic_log(LOG_WARNING, "Null argument value passed to FuncCallInstruction for function @%s", target_func ? target_func->getName().c_str() : "<unknown_func>");
	}
	}
}

/// @brief 转换成字符串显示
/// @param str 转换后的字符串
// FuncCallInstruction.h
// 确保声明是：
// class FuncCallInstruction : public Instruction {
// private:
//     Function* called_function_ = nullptr; // 被调用的函数
//     // 参数通过 User 基类的 operands 存储
// public:
//     // 构造函数示例，接收当前函数作用域、目标函数、参数列表、调用结果的类型
//     FuncCallInstruction(Function* current_func_scope, Function* target_func,
//                         const std::vector<Value*>& args, Type* result_type_if_any);
//
//     [[nodiscard]] std::string toString() const override;
//     [[nodiscard]] Function* getCalledFunction() const { return called_function_; } // Getter
// };
// toString() 实现
std::string FuncCallInstruction::toString() const {
    std::string result_str_build;

    if (!calledFunction) {
        minic_log(LOG_ERROR, "FuncCallInstruction::toString(): calledFunction is null.");
        return "; <Error: FuncCallInstruction has no target function specified>";
    }

    // 1. 构建函数调用头部 (call <return_type> @func_name 或 <result_var> = call <return_type> @func_name)
    // this->getType() 是这条 call 指令的结果类型 (例如 i32, void)
    // this->getIRName() 是存储结果的临时变量名 (例如 %t0), 如果调用有返回值
    Type* instruction_result_type = this->getType(); // 类型由构造函数设置

    if (instruction_result_type && !instruction_result_type->isVoidType()) {
        // 有返回值
        // calledFunction->getReturnType() 应该是和 instruction_result_type 一致的
        result_str_build = this->getIRName() + " = call " + calledFunction->getReturnType()->toString() +
                           " @" + calledFunction->getName(); // DragonIR 函数名以 @ 开头
    } else {
        // 无返回值
        result_str_build = "call void @" + calledFunction->getName(); // DragonIR 函数名以 @ 开头
    }

    // 2. 构建参数列表
    result_str_build += "(";
    int32_t num_actual_params = getOperandsNum(); // 参数从 User 基类的操作数列表获取

    for (int32_t k = 0; k < num_actual_params; ++k) {
        Value* operand = getOperand(k); // 确保 getOperand, getType, getIRName 都是 const
        if (operand && operand->getType()) {
            result_str_build += operand->getType()->toString() + " " + operand->getIRName();
        } else {
            minic_log(LOG_ERROR, "FuncCallInstruction::toString(): Operand or its type is null for call to @%s, param index %d.",
                      calledFunction->getName().c_str(), k);
            result_str_build += "<error_param>";
        }

        if (k < (num_actual_params - 1)) {
            result_str_build += ", ";
        }
    }
    result_str_build += ")";

    // --------------------------------------------------------------------
    // 之前讨论的 func->realArgCountReset() 和 func->getRealArgcount()
    // 不应出现在这里。参数计数和重置应由 IRGenerator 管理。
    // --------------------------------------------------------------------

    return result_str_build;
}

///
/// @brief 获取被调用函数的名字
/// @return std::string 被调用函数名字
///
