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
// ir/Instructions/FuncCallInstruction.cpp
#include "Instructions/FuncCallInstruction.h" 
#include "Function.h"      
#include "Common.h" 
#include "Type.h"          
#include "Types/VoidType.h"  
#include "Value.h"         

// 构造函数定义与 .h 中的声明匹配
FuncCallInstruction::FuncCallInstruction(
    Function* parentFuncScope,             // 这个对应 Instruction 构造函数的 _func
    const std::string& func_name_to_call,
    const std::vector<Value*>& args,
    Type* result_type_if_any,            // 这个对应 Instruction 构造函数的 _type
    Function* target_func_object 
) 
// --- 修正对 Instruction 基类构造函数的调用 ---
: Instruction(
      parentFuncScope,                                                 // 1. Function* _func
      IRInstOperator::IRINST_OP_FUNC_CALL,                             // 2. IRInstOperator op
      (result_type_if_any ? result_type_if_any : VoidType::getType())  // 3. Type* _type
  ),
// --- 结束修正 ---
    calledFunctionName_(func_name_to_call),
    calledFunction_(target_func_object) // 假设 .h 中成员名为 calledFunction_
{
    for (Value* arg_val : args) {
        if (arg_val) {
            addOperand(arg_val); 
        } else {
            minic_log(LOG_WARNING, "FuncCallInstruction CTOR for call to '%s': Null argument value passed.", 
                      calledFunctionName_.c_str());
        }
    }

    minic_log(LOG_DEBUG, "FuncCallInstruction CREATED: Call to FuncName='%s'. TargetFuncPtr=%p. ResultType='%s'. NumArgs=%d. ParentFunc='%s'. ThisPtr=%p",
              calledFunctionName_.c_str(),
              (void*)calledFunction_, 
              (this->getType() ? this->getType()->toString().c_str() : "null_type"),
              getOperandsNum(), 
              parentFuncScope ? parentFuncScope->getName().c_str() : "null_parent",
              (void*)this);
    
    if (calledFunctionName_.empty()) {
        minic_log(LOG_ERROR, "FuncCallInstruction CTOR: Created a call instruction with an EMPTY function name! Instruction IRName: %s",
                  this->getIRName().c_str()); 
    }
}


// getName() 定义与 .h 中的声明匹配
[[nodiscard]] std::string FuncCallInstruction::getName() const {
    minic_log(LOG_DEBUG, "FuncCallInstruction (Ptr: %p, IRName: %s)::getName() returning stored name '%s'",
              (void*)this, this->getIRName().c_str(), calledFunctionName_.c_str());
              
    // 优先返回存储的字符串名称，因为它可能来自外部函数调用，此时 calledFunction_ 为 nullptr
    if (!calledFunctionName_.empty()) {
        return calledFunctionName_;
    }
    // 如果存储的名称为空，但有关联的 Function 对象，则尝试从对象获取
    if (calledFunction_ && !calledFunction_->getName().empty()) {
        minic_log(LOG_WARNING, "FuncCallInstruction (Ptr: %p, IRName: %s)::getName(): calledFunctionName_ is empty, using name from targetFunction_ object: '%s'",
                  (void*)this, this->getIRName().c_str(), calledFunction_->getName().c_str());
        return calledFunction_->getName();
    }
    minic_log(LOG_ERROR, "FuncCallInstruction (Ptr: %p, IRName: %s)::getName(): Both calledFunctionName_ and targetFunction_ name are empty!",
              (void*)this, this->getIRName().c_str());
    return "<UNKNOWN_OR_EMPTY_FUNCTION_NAME>";
}

std::string FuncCallInstruction::toString() const {
    std::string result_str_build;
    std::string func_to_print = getName(); // 使用修改后的 getName()

    if (func_to_print.empty() || func_to_print == "<UNKNOWN_OR_EMPTY_FUNCTION_NAME>") {
        // 如果 getName() 仍然返回空或错误标记，toString 中也反映出来
        minic_log(LOG_ERROR, "FuncCallInstruction::toString() (for IRName: %s): getName() returned empty or error string! Using placeholder.", this->getIRName().c_str());
        func_to_print = "<ERROR_EMPTY_FUNC_NAME_IN_TOSTRING>";
    }

    Type* instruction_result_type = this->getType(); 

    if (instruction_result_type && !instruction_result_type->isVoidType()) {
        result_str_build = this->getIRName() + " = call " + 
                           instruction_result_type->toString() + 
                           " @" + func_to_print;
    } else {
        result_str_build = "call void @" + func_to_print;
    }

    result_str_build += "(";
    int32_t num_actual_params = getOperandsNum(); 
    for (int32_t k = 0; k < num_actual_params; ++k) {
        Value* operand = getOperand(k); 
        if (operand && operand->getType()) { 
            result_str_build += operand->getType()->toString() + " " + operand->getIRName();
        } else {
            std::string operand_ir_name = operand ? operand->getIRName() : "null_operand";
            std::string operand_type_str = (operand && operand->getType()) ? operand->getType()->toString() : "null_type";
            minic_log(LOG_ERROR, "FuncCallInstruction::toString(): For call to @%s, param index %d (Operand IRName: %s, Type: %s) is invalid.",
                      func_to_print.c_str(), k, operand_ir_name.c_str(), operand_type_str.c_str());
            result_str_build += "<error_param>";
        }
        if (k < (num_actual_params - 1)) {
            result_str_build += ", ";
        }
    }
    result_str_build += ")";
    return result_str_build;
}