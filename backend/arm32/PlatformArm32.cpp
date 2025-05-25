///
/// @file PlatformArm32.cpp
/// @brief  ARM32平台相关实现
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-11-21
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-21 <td>1.0     <td>zenglj  <td>新做
/// </table>
///
#include "PlatformArm32.h"

#include "IntegerType.h"
#include <cstdint> // For uint32_t
#include "Common.h" 
const std::string PlatformArm32::regName[PlatformArm32::maxRegNum] = {
    "r0",  // 用于传参或返回值等，不需要栈保护
    "r1",  // 用于传参或返回值（64位结果时后32位）等，不需要栈保护
    "r2",  // 用于传参等，不需要栈保护
    "r3",  // 用于传参等，不需要栈保护
    "r4",  // 需要栈保护
    "r5",  // 需要栈保护
    "r6",  // 需要栈保护
    "r7",  // 需要栈保护
    "r8",  // 用于加载操作数1,保存表达式结果
    "r9",  // 用于加载操作数2,写回表达式结果,立即数，标签地址
    "r10", // 用于保存乘法结果，虽然mul
           // r8,r8,r9也能正常执行，但是避免交叉编译提示错误！
    "fp",  // r11,局部变量寻址
    "ip",  // r12，临时寄存器
    "sp",  // r13，堆栈指针寄存器
    "lr", // r14，链接寄存器。LR存储子程序调用的返回地址。当执行BL指令时，PC的当前值会被保存到LR中。
    "pc", // r15，程序计数器。PC 存储着下一条将要执行的指令的地址。在执行分支指令时，PC会更新为新的地址。
};

RegVariable * PlatformArm32::intRegVal[PlatformArm32::maxRegNum] = {
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[0], 0),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[1], 1),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[2], 2),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[3], 3),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[4], 4),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[5], 5),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[6], 6),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[7], 7),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[8], 8),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[9], 9),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[10], 10),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[11], 11),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[12], 12),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[13], 13),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[14], 14),
    new RegVariable(IntegerType::getTypeInt(), PlatformArm32::regName[15], 15),
};

// 实现 getRegNameSafe
std::string PlatformArm32::getRegNameSafe(int reg_id) {
    if (reg_id >= 0 && reg_id < PlatformArm32::maxRegNum) {
        // 检查一下初始化是否正确，防止 regName[reg_id] 是空字符串
        if (PlatformArm32::regName[reg_id].empty()) {
             minic_log(LOG_ERROR, "PlatformArm32::getRegNameSafe: Register name for valid ID %d is unexpectedly empty!", reg_id);
             return "EMPTY_REG_NAME(" + std::to_string(reg_id) + ")"; // 返回一个不同的错误标记
        }
        return PlatformArm32::regName[reg_id];
    }
    // 对于无效ID，记录日志并返回一个易于识别的错误标记
    // 可以考虑根据编译器配置决定是 WARN 还是 ERROR
    minic_log(LOG_WARNING, "PlatformArm32::getRegNameSafe: Attempting to get name for invalid register ID: %d", reg_id);
    return "REG_ID(" + std::to_string(reg_id) + "_ERR)";
}


/// @brief 循环左移两位
/// @param num
void PlatformArm32::roundLeftShiftTwoBit(unsigned int & num)
{
    // 取左移即将溢出的两位
    const unsigned int overFlow = num & 0xc0000000;

    // 将溢出部分追加到尾部
    num = (num << 2) | (overFlow >> 30);
}

/// @brief 判断num是否是常数表达式，8位数字循环右移偶数位得到
/// @param num
/// @return
bool PlatformArm32::__constExpr(int num)
{
    unsigned int new_num = (unsigned int) num;

    for (int i = 0; i < 16; i++) {

        if (new_num <= 0xff) {
            // 有效表达式
            return true;
        }

        // 循环左移2位
        roundLeftShiftTwoBit(new_num);
    }

    return false;
}

/// @brief 同时处理正数和负数
/// @param num
/// @return
bool PlatformArm32::constExpr(int num)
{
    return __constExpr(num) || __constExpr(-num);
}

/// @brief 判定是否是合法的偏移
/// @param num
/// @return
bool PlatformArm32::isDisp(int num)
{
    return num < 4096 && num > -4096;
}

/// @brief 判断是否是合法的寄存器名
/// @param s 寄存器名字
/// @return 是否是
bool PlatformArm32::isReg(std::string name)
{
    return name == "r0" || name == "r1" || name == "r2" || name == "r3" || name == "r4" || name == "r5" ||
           name == "r6" || name == "r7" || name == "r8" || name == "r9" || name == "r10" || name == "fp" ||
           name == "ip" || name == "sp" || name == "lr" || name == "pc";
}

bool PlatformArm32::isValidCmpImmediate(int32_t imm_val) {
    // ARM CMP Rd, #imm. 立即数可以是8位值，经过偶数位的循环右移得到。
    // MOV Rd, #imm 的立即数规则类似。
    uint32_t val = static_cast<uint32_t>(imm_val);

    // 尝试所有偶数位循环右移 (0, 2, ..., 30)
    for (int rot = 0; rot < 32; rot += 2) {
        uint32_t rotated_val = (val >> rot) | (val << (32 - rot));
        // 检查旋转后的值的高24位是否为0 (即是否能用8位表示)
        if ((rotated_val & 0xFFFFFF00) == 0) {
            return true;
        }
    }
    // 对于负数，有时可以表示为正数的逻辑非，然后检查。
    // 例如 CMP r0, #-1  等价于 CMN r0, #1 (CMP r0, NEG #1)
    // ARMv7 Thumb-2 有一些特殊的立即数编码。
    // 对于简单的编译器，上述检查可能足够，或者你可以更严格地只允许0-255。
    // if (imm_val >= 0 && imm_val <= 255) return true; // 一个更简单的（但限制性更强）版本

    return false; // 如果所有旋转都不匹配
}