///
/// @file User.cpp
/// @brief 使用Value的User，该User也是Value。函数、指令都是User
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

#include "User.h"
#include "Value.h" // For Value::removeUse
#include "Use.h"   // For Use class and delete use_edge
///
/// @brief 构造函数
/// @param _type  类型
///
User::User(Type * _type) : Value(_type)
{}

User::~User() {
    clearOperands(); // 在析构时调用 clearOperands 来释放资源
}
///
/// @brief 更新指定Pos的Value
/// @param pos 位置
/// @param val 操作数
///
void User::setOperand(int32_t pos, Value * val)
{
    if (pos < (int32_t) operands.size()) {
        operands[pos]->setUsee(val);
    }
}

///
/// @brief 增加操作数，或者说本身的值由这些操作数来计算得到
/// @param pos 索引位置
/// @param val 值
///
void User::addOperand(Value * val)
{
    // If not, add the given Value as a new use.
    auto use = new Use(val, this);

    // 增加到操作数中
    operands.push_back(use);

    // 该val被使用
    val->addUse(use);
}

///
/// @brief 清除指定的操作数
/// @param val 操作数
///
void User::removeOperand(Value * val_to_remove) {
    if (!val_to_remove) return;

    // 遍历查找对应的 Use 对象
    for (auto it = operands.begin(); it != operands.end(); ++it) {
        Use* use_edge = *it;
        if (use_edge && use_edge->getUsee() == val_to_remove) {
            operands.erase(it); // 从 User 的 operands 列表移除

            // value_being_used 就是 val_to_remove
            val_to_remove->removeUse(use_edge); // 通知 Value 移除
            delete use_edge; // 删除 Use 对象
            return; // 假设每个 Value 只作为操作数出现一次，找到就返回
        }
    }
}

///
/// @brief 清除指定的操作数
/// @param pos 操作数的索引
///
void User::removeOperand(int pos) {
    if (pos >= 0 && pos < (int32_t)operands.size()) {
        Use * use_edge = operands[pos]; // 获取要删除的 Use

        // 1. 从 User 的 operands 列表中移除 (erase会使后续迭代器/索引失效，所以要小心)
        //    一种安全的方式是先获取，再移除，再处理
        operands.erase(operands.begin() + pos);

        if (use_edge) {
            Value* value_being_used = use_edge->getUsee();
            if (value_being_used) {
                value_being_used->removeUse(use_edge);
            }
            delete use_edge; // 删除 Use 对象
        }
    }
}

///
/// @brief 清除指定的操作数
/// @param pos 操作数的索引
///
void User::removeOperandRaw(Use * use)
{
    auto pIter = std::find(operands.begin(), operands.end(), use);
    if (pIter != operands.end()) {
        operands.erase(pIter);
    }
}

///
/// @brief 清除指定的Use
/// @param use define-use边
///
void User::removeUse(Use * use)
{
    auto pIter = std::find(operands.begin(), operands.end(), use);
    if (pIter != operands.end()) {
        use->remove();
    }
}

///
/// @brief 清除所有的操作数
///
// User.cpp
void User::clearOperands() {
    // 从后向前遍历并删除，这样在删除元素时不会影响后续迭代
    while (!operands.empty()) {
        Use* use_edge = operands.back(); // 获取最后一个 Use 对象
        operands.pop_back();             // 从 User 的 operands 列表中移除指针

        if (use_edge) {
            Value* value_being_used = use_edge->getUsee(); // 获取被该 Use 边指向的 Value
            if (value_being_used) {
                // 通知被使用的 Value 对象，移除这个 Use 引用
                // Value::removeUse 应该只从 Value::uses 向量中移除指针，不 delete Use 对象
                value_being_used->removeUse(use_edge);
            }
            // User 作为 Use 对象的所有者，负责 delete 它
            delete use_edge;
            use_edge = nullptr; // 好习惯
        }
    }
    // operands vector 现在应该是空的
}
///
/// @brief Get the Operands object
/// @return std::vector<Use *>&
///
std::vector<Use *> & User::getOperands()
{
    return operands;
}

///
/// @brief 取得操作数
/// @return std::vector<Value *>
///
std::vector<Value *> User::getOperandsValue() const
{
    std::vector<Value *> operandsVec;
    for (auto & use: operands) {
        operandsVec.emplace_back(use->getUsee());
    }
    return operandsVec;
}

///
/// @brief 获取操作数的个数
/// @return int32_t 个数
///
int32_t User::getOperandsNum() const
{
    return (int32_t) operands.size();
}

///
/// @brief 获取指定的操作数
/// @param pos 位置
/// @return Value* 操作数
///
Value * User::getOperand(int32_t pos) const
{
    if (pos < (int32_t) operands.size()) {
        return operands[pos]->getUsee();
    }

    return nullptr;
}
