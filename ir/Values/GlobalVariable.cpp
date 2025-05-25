// ir/Values/GlobalVariable.cpp
#include "GlobalVariable.h" // 确保包含了头文件
// ... 其他 include ...

// ... 构造函数和其他方法 ...

void GlobalVariable::setInitializer(Constant *initVal)
{
    this->initializer = initVal;
}