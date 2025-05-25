// ir/Instructions/FuncCallInstruction.h
#pragma once
#include "Instruction.h" 
#include <string>
#include <vector>

class Value;    
class Type;     
class Function; 

class FuncCallInstruction : public Instruction { 
private:
    std::string calledFunctionName_; 
    Function*   calledFunction_;     // 使用下划线

public:
    FuncCallInstruction(
        Function* parentFuncScope,              
        const std::string& func_name_to_call,   
        const std::vector<Value*>& args,        
        Type* result_type_if_any,               
        Function* target_func_object = nullptr  
    );

    // 假设 Instruction 或 Value 有虚函数 getName() const
    [[nodiscard]] std::string getName() const override; 
    // 如果基类没有 getName()，或者你想提供一个不同的方法：
    // [[nodiscard]] std::string getCalledFunctionName() const; 

    [[nodiscard]] Function* getTargetFunction() const { return calledFunction_; }

    [[nodiscard]] std::string toString() const override; 
};