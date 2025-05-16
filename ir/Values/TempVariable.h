// ir/Values/TempVariable.h
#pragma once
#include "Value.h" // 假设 Value 是所有值的基类
#include <string>

class TempVariable : public Value {
public:
    TempVariable(Type* type, const std::string& name);
    //std::string toString() const override; // 或与 Instruction::toString 匹配
    // 如果 Value 有 getName(), 也 override 它
    // std::string getName() const override;
};
