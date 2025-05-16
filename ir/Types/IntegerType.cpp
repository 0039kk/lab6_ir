///
/// @file IntegerType.cpp
/// @brief 整型类型类，可描述1位的bool类型或32位的int类型
///
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

// IntegerType.cpp
#include "IntegerType.h"
#include <map>
#include <string> // std::to_string 需要

// 静态成员定义
IntegerType * IntegerType::oneInstanceBool = nullptr; // 初始化为 nullptr
IntegerType * IntegerType::oneInstanceInt = nullptr;  // 初始化为 nullptr

// 静态缓存定义
static std::map<int, IntegerType*> integer_type_cache_for_get;

// 构造函数实现
IntegerType::IntegerType(int bitWidthValue) // 使用与 .h 中声明一致的参数名
    : Type(Type::TypeID::IntegerTyID), bit_width_(bitWidthValue) { // 正确初始化 bit_width_
    // 不需要其他操作
}

// get(int bitWidth) 实现 (保持不变，它是正确的)
IntegerType* IntegerType::get(int bitWidth) {
    auto it = integer_type_cache_for_get.find(bitWidth);
    if (it != integer_type_cache_for_get.end()) {
        return it->second;
    }
    IntegerType* new_type = new IntegerType(bitWidth);
    integer_type_cache_for_get[bitWidth] = new_type;
    return new_type;
}

// getTypeBool() 实现 (保持不变，它是正确的)
IntegerType * IntegerType::getTypeBool() {
    if (!oneInstanceBool) {
        oneInstanceBool = new IntegerType(1); // 内部调用构造函数，设置 bit_width_ = 1
    }
    return oneInstanceBool;
}

// getTypeInt() 实现 (保持不变，它是正确的)
IntegerType * IntegerType::getTypeInt() {
    if (!oneInstanceInt) {
        oneInstanceInt = new IntegerType(32); // 内部调用构造函数，设置 bit_width_ = 32
    }
    return oneInstanceInt;
}