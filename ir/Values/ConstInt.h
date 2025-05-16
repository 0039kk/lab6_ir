///
/// @file ConstInt.h
/// @brief int类型的常量
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

#pragma once

#include "Constant.h"
#include "IRConstant.h"
#include "IntegerType.h"

class Module;
///
/// @brief 整型常量类
///
class ConstInt : public Constant {

	public:
    // 修改构造函数以接受 Type* 和 int32_t 值
    ConstInt(Type* type, int32_t val);

    // getIRName() 应该与 Value 基类中的 getName() 匹配并 override
    // 如果 Value 中是 getName(), 这里也应该是 getName()

    [[nodiscard]] std::string getName() const override; // 或者 getIRName() 如果基类是这个

    [[nodiscard]] int32_t getVal() const { // 标记为 const
        return intVal;
    }

    // setLoadRegId 和 getLoadRegId 看起来是从 User/Value 继承的，保持不变
    int32_t getLoadRegId() override {
        return this->loadRegNo;
    }
    void setLoadRegId(int32_t regId) override {
        this->loadRegNo = regId;
    }

    // 静态方法用于获取/创建特定类型的整数常量，并由 Module 管理缓存 (推荐)
    static ConstInt* get(Type* type, int32_t value, Module& module);

private:
    ///
    /// @brief 整数值
    ///
    int32_t intVal;

    ///
    /// @brief 变量加载到寄存器中时对应的寄存器编号
    ///
    int32_t loadRegNo = -1;
};