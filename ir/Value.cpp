///
/// @file Value.cpp
/// @brief 值操作类型，所有的变量、函数、常量都是Value
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

#include <algorithm>

#include "Value.h"
#include "Use.h"

/// @brief 构造函数
/// @param _type
Value::Value(Type * _type) : type(_type)
{
    // 不需要增加代码
}

/// @brief 析构函数
Value::~Value()
{
    // 如有资源清理，请这里追加代码
}

/// @brief 获取名字
/// @return 变量名
std::string Value::getName() const
{
    return name;
}

///
/// @brief 设置名字
/// @param _name 名字
///
void Value::setName(std::string _name)
{
    this->name = _name;
}

/// @brief 获取名字
/// @return 变量名
std::string Value::getIRName() const
{
    if (IRName.empty()) { // 如果 IRName 还没有被显式设置（通过 setIRName）
        // 对于未命名的Value (比如指令结果在被 renameIR 处理前)，
        // 或者某些确实没有源码名的Value，生成一个唯一的临时占位符。
        // 你的实现是 "UNNAMED_VALUE(...)" 或 "name_NO_IRNAME"，
        // 这在 renameIR 之前是可接受的。
        // renameIR 之后，所有应该有规范名称的 Value 都应该有非空的 IRName。
        if (!name.empty()) { // 仅当 name 非空时才使用它作为后缀的基础
            return name + "_IR_UNSET"; // 或者其他明确表示“尚未最终命名”的后缀
        }
        // 对于完全无名的（如大多数指令结果在最终命名之前）
        return "TEMP_VAL_ADDR_" + std::to_string(reinterpret_cast<uintptr_t>(this));
    }
    return IRName;
}

///
/// @brief 设置名字
/// @param _name 名字
///
void Value::setIRName(std::string _name)
{
    this->IRName = _name;
}

/// @brief 获取类型
/// @return 变量名
Type * Value::getType() const
{
    return this->type;
}

///
/// @brief 增加一条边，增加Value被使用次数
/// @param use
///
void Value::addUse(Use * use)
{
    uses.push_back(use);
}

///
/// @brief 消除一条边，减少Value被使用次数
/// @param use
///
void Value::removeUse(Use * use)
{
    auto pIter = std::find(uses.begin(), uses.end(), use);
    if (pIter != uses.end()) {
        uses.erase(pIter);
    }
}

///
/// @brief 取得变量所在的作用域层级
/// @return int32_t 层级
///
int32_t Value::getScopeLevel() const
{
    return -1;
}

/*
/// @brief 获得分配的寄存器编号或ID
/// @return int32_t 寄存器编号 -1代表无效的寄存器编号
///

///
/// @brief @brief 如是内存变量型Value，则获取基址寄存器和偏移
/// @param regId 寄存器编号
/// @param offset 相对偏移
/// @return true 是内存型变量
/// @return false 不是内存型变量
///*/
bool Value::getMemoryAddr(int32_t * regId, int64_t * offset)
{
    (void) regId;
    (void) offset;
    return false;
}
