// ir/Type.cpp
#include "Type.h"
// 如果在这里的实现中需要日志，才包含 Common.h
// #include "common/Common.h" 

// 为虚析构函数提供一个（空的）定义
Type::~Type() {
    // 空实现即可，目的是确保 vtable 和 typeinfo 被发射到这个编译单元
}

// 为虚函数 getSize() 提供一个默认实现
// 即使子类会重写它，基类也应该有一个定义
int32_t Type::getSize() const {
    // minic_log(LOG_DEBUG, "Base Type::getSize() called for type ID %d, returning -1.", ID);
    return -1; // 默认大小，或表示未知/不适用
}

// isInt1Byte() 和 isInt32Type() 的默认实现在头文件中是 return false; 这通常没问题。
// 但如果想更集中地管理 Type 类的定义，也可以移到这里：
// bool Type::isInt1Byte() const {
//     return false;
// }
// bool Type::isInt32Type() const {
//     return false;
// }

// toString() 是纯虚函数，不需要在 Type.cpp 中定义，除非你想提供一个
// “如果子类没有实现就调用我”的备用方案，但这违背了纯虚函数的目的。