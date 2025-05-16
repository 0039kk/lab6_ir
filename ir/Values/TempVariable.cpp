// ir/Values/TempVariable.cpp
#include "TempVariable.h"
TempVariable::TempVariable(Type* type, const std::string& ir_name_to_set) // 参数名改为 ir_name_to_set 更清晰
    : Value(type) { // <--- 只传递 type 给 Value 基类构造函数
    this->setIRName(ir_name_to_set); // 在构造函数体内部设置 IRName
    // 如果也需要设置普通 name，并且 Value 有 setName()
    // this->setName(ir_name_to_set);
}