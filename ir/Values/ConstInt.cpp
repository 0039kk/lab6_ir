// ir/Values/ConstInt.cpp
#include "ConstInt.h"
#include "Module.h" // 如果静态 get 方法需要 Module
#include <stdexcept> // 用于错误处理

ConstInt::ConstInt(Type* type, int32_t val)
    : Constant(type), intVal(val) { // 调用 Constant 基类的构造函数
    if (!type || !type->isIntegerType()) { // 确保传入的是整数类型
        throw std::invalid_argument("ConstInt 必须使用整数类型初始化。");
    }
    // 设置 Value 基类的名字为其字符串表示
    // 假设 User/Value 有 setName(const std::string&)
    this->setIRName(std::to_string(val)); 
}

std::string ConstInt::getName() const {
    // 如果 Value 基类有 name_ 成员并已通过 setName 设置，则可以直接返回
    // return Value::getName(); 
    // 或者如果 ConstInt 就是用其值作为名字
    return std::to_string(intVal);
}

// 静态 get 方法的实现
ConstInt* ConstInt::get(Type* type, int32_t value, Module& module) {
    // 这个方法会调用 Module 中的缓存机制来获取或创建 ConstInt
    return static_cast<ConstInt*>(module.getOrCreateIntegerConstant(type, value));
}