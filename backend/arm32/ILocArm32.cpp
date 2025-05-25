///
/// @file ILocArm32.cpp
/// @brief 指令序列管理的实现，ILOC的全称为Intermediate Language for Optimizing Compilers
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
#include <cstdio>
#include <string>
#include <typeinfo>

#include "ILocArm32.h"
#include "Common.h"
#include "Function.h"
#include "PlatformArm32.h"
#include "Module.h"
#include "ConstInt.h"   // 因为用到了 ConstInt
#include "GlobalVariable.h" // 因为用到了 GlobalVariable
#include "LocalVariable.h"  // 因为用到了 LocalVariable
#include "TempVariable.h"   // <--- 新增这个
#include "Instruction.h"       // 因为用到了 Instruction

ArmInst::ArmInst(std::string _opcode,
                 std::string _result,
                 std::string _arg1,
                 std::string _arg2,
                 std::string _cond,
                 std::string _addition)
    : opcode(_opcode), cond(_cond), result(_result), arg1(_arg1), arg2(_arg2), addition(_addition), dead(false)
{}

/*
    指令内容替换
*/
void ArmInst::replace(std::string _opcode,
                      std::string _result,
                      std::string _arg1,
                      std::string _arg2,
                      std::string _cond,
                      std::string _addition)
{
    opcode = _opcode;
    result = _result;
    arg1 = _arg1;
    arg2 = _arg2;
    cond = _cond;
    addition = _addition;

#if 0
    // 空操作，则设置为dead
    if (op == "") {
        dead = true;
    }
#endif
}

/*
    设置为无效指令
*/
void ArmInst::setDead()
{
    dead = true;
}

/*
    输出函数
*/
std::string ArmInst::outPut()
{
    // 无用代码，什么都不输出
    if (dead) {
        return "";
    }

    // 占位指令,可能需要输出一个空操作，看是否支持 FIXME
    if (opcode.empty()) {
        return "";
    }

    std::string ret = opcode;

    if (!cond.empty()) {
        ret += cond;
    }

    // 结果输出
    if (!result.empty()) {
        if (result == ":") {
            ret += result;
        } else {
            ret += " " + result;
        }
    }

    // 第一元参数输出
    if (!arg1.empty()) {
        ret += "," + arg1;
    }

    // 第二元参数输出
    if (!arg2.empty()) {
        ret += "," + arg2;
    }

    // 其他附加信息输出
    if (!addition.empty()) {
        ret += "," + addition;
    }

    return ret;
}

#define emit(...) code.push_back(new ArmInst(__VA_ARGS__))

/// @brief 构造函数
/// @param _module 符号表
ILocArm32::ILocArm32(Module * _module)
{
    this->module = _module;
}

/// @brief 析构函数
ILocArm32::~ILocArm32()
{
    std::list<ArmInst *>::iterator pIter;

    for (pIter = code.begin(); pIter != code.end(); ++pIter) {
        delete (*pIter);
    }
}

/// @brief 删除无用的Label指令
// backend/arm32/ILocArm32.cpp

void ILocArm32::deleteUnusedLabel() {
    if (code.empty()) {
        return;
    }

    minic_log(LOG_DEBUG, "ILocArm32::deleteUnusedLabel: Starting. %zu referenced labels found.", referencedLabelNames.size());
    // for(const auto& ref_label : referencedLabelNames) { // 可选：打印所有引用的标签
    //     minic_log(LOG_DEBUG, "ILocArm32: Referenced Label: '%s'", ref_label.c_str());
    // }

    // 遍历所有指令，找到标签定义并检查是否被引用
    for (ArmInst * inst : code) {
        if (inst->dead) {
            continue;
        }

        // 假设：标签定义的 ArmInst 中，opcode 存储标签名 (如 ".L4")，result 存储 ":"
        if (!inst->opcode.empty() && inst->opcode[0] == '.' && inst->result == ":") {
            const std::string& definedLabelName = inst->opcode;
            // 检查这个定义的标签是否在被引用的集合中
            if (referencedLabelNames.find(definedLabelName) == referencedLabelNames.end()) {
                // 未找到，说明这个标签没有被任何跳转指令引用
                minic_log(LOG_INFO, "ILocArm32::deleteUnusedLabel: Deleting unused label: '%s'", definedLabelName.c_str());
                inst->setDead(); // 标记为 dead
            } else {
                minic_log(LOG_DEBUG, "ILocArm32::deleteUnusedLabel: Keeping used label: '%s'", definedLabelName.c_str());
            }
        }
    }
     minic_log(LOG_DEBUG, "ILocArm32::deleteUnusedLabel: Finished.");
}

/// @brief 输出汇编
/// @param file 输出的文件指针
/// @param outputEmpty 是否输出空语句
void ILocArm32::outPut(FILE * file, bool outputEmpty)
{
    for (auto arm: code) {

        std::string s = arm->outPut();

        if (arm->result == ":") {
            // Label指令，不需要Tab输出
            fprintf(file, "%s\n", s.c_str());
            continue;
        }

        if (!s.empty()) {
            fprintf(file, "\t%s\n", s.c_str());
        } else if ((outputEmpty)) {
            fprintf(file, "\n");
        }
    }
}

/// @brief 获取当前的代码序列
/// @return 代码序列
std::list<ArmInst *> & ILocArm32::getCode()
{
    return code;
}

/**
 * 数字变字符串，若flag为真，则变为立即数寻址（加#）
 */
std::string ILocArm32::toStr(int num, bool flag)
{
    std::string ret;

    if (flag) {
        ret = "#";
    }

    ret += std::to_string(num);

    return ret;
}

/*
    产生标签
*/
void ILocArm32::label(std::string name)
{
    // .L1:
    emit(name, ":");
}

/// @brief 0个源操作数指令
/// @param op 操作码
/// @param rs 操作数
void ILocArm32::inst(std::string op, std::string rs)
{
    emit(op, rs);
}

/// @brief 一个操作数指令
/// @param op 操作码
/// @param rs 操作数
/// @param arg1 源操作数
void ILocArm32::inst(std::string op, std::string rs, std::string arg1)
{
    emit(op, rs, arg1);
}

/// @brief 一个操作数指令
/// @param op 操作码
/// @param rs 操作数
/// @param arg1 源操作数
/// @param arg2 源操作数
void ILocArm32::inst(std::string op, std::string rs, std::string arg1, std::string arg2)
{
    emit(op, rs, arg1, arg2);
}

///
/// @brief 注释指令，不包含分号
///
void ILocArm32::comment(std::string str)
{
    emit("@", str);
}

/*
    加载立即数 ldr r0,=#100
*/
void ILocArm32::load_imm(int rs_reg_no, int constant)
{
    // movw:把 16 位立即数放到寄存器的低16位，高16位清0
    // movt:把 16 位立即数放到寄存器的高16位，低 16位不影响
    if (0 == ((constant >> 16) & 0xFFFF)) {
        // 如果高16位本来就为0，直接movw
        emit("movw", PlatformArm32::getRegNameSafe(rs_reg_no), "#:lower16:" + std::to_string(constant));
    } else {
        // 如果高16位不为0，先movw，然后movt
        emit("movw", PlatformArm32::getRegNameSafe(rs_reg_no), "#:lower16:" + std::to_string(constant));
        emit("movt", PlatformArm32::getRegNameSafe(rs_reg_no), "#:upper16:" + std::to_string(constant));
    }
}

/// @brief 加载符号值 ldr r0,=g ldr r0,=.L1
/// @param rs_reg_no 结果寄存器编号
/// @param name 符号名
void ILocArm32::load_symbol(int rs_reg_no, std::string name)
{
    // movw r10, #:lower16:a
    // movt r10, #:upper16:a
    emit("movw", PlatformArm32::getRegNameSafe(rs_reg_no), "#:lower16:" + name);
	emit("movt", PlatformArm32::getRegNameSafe(rs_reg_no), "#:upper16:" + name);
}

/// @brief 基址寻址 ldr r0,[fp,#100]
/// @param rsReg 结果寄存器
/// @param base_reg_no 基址寄存器
/// @param offset 偏移
void ILocArm32::load_base(int rs_reg_no, int base_reg_no, int offset)
{
    std::string rsReg = PlatformArm32::getRegNameSafe(rs_reg_no);
	std::string base = PlatformArm32::getRegNameSafe(base_reg_no);

    if (PlatformArm32::isDisp(offset)) {
        // 有效的偏移常量
        if (offset) {
            // [fp,#-16] [fp]
            base += "," + toStr(offset);
        }
    } else {

        // ldr r8,=-4096
        load_imm(rs_reg_no, offset);

        // fp,r8
        base += "," + rsReg;
    }

    // 内存寻址
    base = "[" + base + "]";

    // ldr r8,[fp,#-16]
    // ldr r8,[fp,r8]
    emit("ldr", rsReg, base);
}

/// @brief 基址寻址 str r0,[fp,#100]
/// @param srcReg 源寄存器
/// @param base_reg_no 基址寄存器
/// @param disp 偏移
/// @param tmp_reg_no 可能需要临时寄存器编号
void ILocArm32::store_base(int src_reg_no, int base_reg_no, int disp, int tmp_reg_no)
{
    std::string base = PlatformArm32::getRegNameSafe(base_reg_no);
    if (PlatformArm32::isDisp(disp)) {
        // 有效的偏移常量

        // 若disp为0，则直接采用基址，否则采用基址+偏移
        // [fp,#-16] [fp]
        if (disp) {
            base += "," + toStr(disp);
        }
    } else {
        // 先把立即数赋值给指定的寄存器tmpReg，然后采用基址+寄存器的方式进行

        // ldr r9,=-4096
        load_imm(tmp_reg_no, disp);

        // fp,r9
        base += "," + PlatformArm32::getRegNameSafe(tmp_reg_no);
    }

    // 内存间接寻址
    base = "[" + base + "]";

    // str r8,[fp,#-16]
    // str r8,[fp,r9]
    emit("str", PlatformArm32::getRegNameSafe(src_reg_no), base);
}

/// @brief 寄存器Mov操作
/// @param rs_reg_no 结果寄存器
/// @param src_reg_no 源寄存器
void ILocArm32::mov_reg(int rs_reg_no, int src_reg_no)
{
    emit("mov", PlatformArm32::getRegNameSafe(rs_reg_no), PlatformArm32::getRegNameSafe(src_reg_no));
}

/// @brief 加载变量到寄存器，保证将变量放到reg中
/// @param rs_reg_no 结果寄存器
/// @param src_var 源操作数
// backend/arm32/ILocArm32.cpp

void ILocArm32::load_var(int rs_reg_no, Value * src_var)
{
    if (src_var == nullptr) {
        minic_log(LOG_ERROR, "ILocLoadVar: src_var is nullptr! Cannot load into reg %s.", 
                  PlatformArm32::getRegNameSafe(rs_reg_no).c_str());
        // 可能需要 emit 一个错误标记或者直接返回
        emit("@ ERROR: load_var called with null src_var for reg " + PlatformArm32::getRegNameSafe(rs_reg_no));
        return;
    }

    if (Instanceof(constVal, ConstInt *, src_var)) {
        // 整型常量
        minic_log(LOG_DEBUG, "ILocLoadVar: Loading ConstInt %s (value %d) into reg %s",
                  src_var->getIRName().c_str(), constVal->getVal(), PlatformArm32::getRegNameSafe(rs_reg_no).c_str());
        load_imm(rs_reg_no, constVal->getVal());
    } else if (src_var->getRegId() != -1 && src_var->getRegId() < PlatformArm32::maxUsableRegNum ) { // 检查 regId 的有效性
        // 源操作数为寄存器变量
        int32_t src_regId = src_var->getRegId();
        minic_log(LOG_DEBUG, "ILocLoadVar: Src %s is already in reg %s. Target reg is %s.",
                  src_var->getIRName().c_str(), PlatformArm32::getRegNameSafe(src_regId).c_str(), PlatformArm32::getRegNameSafe(rs_reg_no).c_str());
        if (src_regId != rs_reg_no) {
            emit("mov", PlatformArm32::getRegNameSafe(rs_reg_no), PlatformArm32::getRegNameSafe(src_regId));
        }
    } else if (Instanceof(globalVar, GlobalVariable *, src_var)) {
        // 全局变量
        minic_log(LOG_DEBUG, "ILocLoadVar: Loading GlobalVariable %s into reg %s",
                  globalVar->getName().c_str(), PlatformArm32::getRegNameSafe(rs_reg_no).c_str());
        load_symbol(rs_reg_no, globalVar->getName());
        std::string reg_str_for_ldr = PlatformArm32::getRegNameSafe(rs_reg_no);
        emit("ldr", reg_str_for_ldr, "[" + reg_str_for_ldr + "]");
    } else { 
        // 尝试从栈加载
        // 通用日志，打印基本信息
        minic_log(LOG_DEBUG, "ILocLoadVar: Attempting stack load for Value (IRName: '%s', OrigName: '%s', DynType: %s, Ptr: %p, current regId: %d) into reg %s",
                  src_var->getIRName().c_str(),
                  src_var->getName().c_str(), // 原始名称
                  typeid(*src_var).name(),    // 动态类型
                  (void*)src_var,             // 指针地址
                  src_var->getRegId(),        // 当前（通用）regId
                  PlatformArm32::getRegNameSafe(rs_reg_no).c_str());

        // ---- A2: 针对 LocalVariable 的详细日志 ----
        if (LocalVariable* lv = dynamic_cast<LocalVariable*>(src_var)) {
            minic_log(LOG_DEBUG, "ILocLoadVar: src_var IS LocalVariable. Ptr: %p, IRName: '%s'. Internal state BEFORE getMemoryAddr: baseRegNo=%d, offset=%d.",
                      (void*)lv, 
                      lv->getIRName().c_str(), 
                      lv->getBaseRegNoForDebug(), // 调用调试 getter
                      lv->getOffsetForDebug());   // 调用调试 getter
        } 
        // ---- 结束 A2 LocalVariable 日志 ----
        // (可以为 TempVariable 和 Instruction 添加类似的 "IS XxxVariable" 日志，但不打印 baseRegNo/offset，因为它们没有)
        else if (dynamic_cast<TempVariable*>(src_var)) {
            minic_log(LOG_DEBUG, "ILocLoadVar: src_var IS TempVariable. Ptr: %p, IRName: '%s'.", (void*)src_var, src_var->getIRName().c_str());
        } else if (dynamic_cast<Instruction*>(src_var) && src_var != nullptr) { // 确保不是 Value 本身，而是 Instruction
            minic_log(LOG_DEBUG, "ILocLoadVar: src_var IS Instruction result. Ptr: %p, IRName: '%s'.", (void*)src_var, src_var->getIRName().c_str());
        }


        int32_t var_baseRegId = -1;
        int64_t var_offset = 0; 
    
        bool has_mem_addr = src_var->getMemoryAddr(&var_baseRegId, &var_offset); 

        // 在调用 getMemoryAddr 之后，再次检查 LocalVariable 的内部状态 (如果它是 LocalVariable)
        if (LocalVariable* lv_after = dynamic_cast<LocalVariable*>(src_var)) {
            minic_log(LOG_DEBUG, "ILocLoadVar: LocalVariable (Ptr: %p, IRName: '%s') AFTER getMemoryAddr. Return: %s. Effective baseRegId: %d, offset: %lld. Internal base: %d, internal offset: %d.",
                      (void*)lv_after, lv_after->getIRName().c_str(),
                      has_mem_addr ? "true" : "false", var_baseRegId, (long long)var_offset,
                      lv_after->getBaseRegNoForDebug(), lv_after->getOffsetForDebug());
        } else {
            minic_log(LOG_DEBUG, "ILocLoadVar: Value (IRName: '%s', Ptr: %p, DynType: %s) AFTER getMemoryAddr. Return: %s. Effective baseRegId: %d, offset: %lld.",
                      src_var->getIRName().c_str(), (void*)src_var, typeid(*src_var).name(),
                      has_mem_addr ? "true" : "false", var_baseRegId, (long long)var_offset);
        }


        if (has_mem_addr && var_baseRegId != -1 && var_baseRegId < PlatformArm32::maxRegNum) {
            minic_log(LOG_DEBUG, "ILocLoadVar: Proceeding to load_base for %s from [%s, #%lld] into %s",
                src_var->getIRName().c_str(), PlatformArm32::getRegNameSafe(var_baseRegId).c_str(), (long long)var_offset, PlatformArm32::getRegNameSafe(rs_reg_no).c_str());
            load_base(rs_reg_no, var_baseRegId, var_offset);
        } else {
             minic_log(LOG_ERROR, "ILocLoadVar: FINAL DECISION - Value %s (DynType: %s, Ptr: %p) -> getMemoryAddr FAILED or returned invalid baseId. Actual has_mem_addr: %s, actual var_baseRegId: %d. Cannot LDR into %s.",
                       src_var->getIRName().c_str(), typeid(*src_var).name(), (void*)src_var,
                       has_mem_addr ? "true":"false", var_baseRegId, PlatformArm32::getRegNameSafe(rs_reg_no).c_str());
             emit("ldr", PlatformArm32::getRegNameSafe(rs_reg_no), "[NO_VALID_MEM_ADDR_FOR_" + src_var->getIRName() + "]");
        }
    }
}
/// @brief 加载变量地址到寄存器
/// @param rs_reg_no
/// @param var
void ILocArm32::lea_var(int rs_reg_no, Value * var)
{
    // 被加载的变量肯定不是常量！
    // 被加载的变量肯定不是寄存器变量！

    // 目前只考虑局部变量

    // 栈帧偏移
    int32_t var_baseRegId = -1;
    int64_t var_offset = -1;

    bool result = var->getMemoryAddr(&var_baseRegId, &var_offset);
    if (!result) {
        minic_log(LOG_ERROR, "BUG");
    }

    // lea r8, [fp,#-16]
    leaStack(rs_reg_no, var_baseRegId, var_offset);
}

/// @brief 保存寄存器到变量，保证将计算结果（r8）保存到变量
/// @param src_reg_no 源寄存器
/// @param dest_var  变量
/// @param tmp_reg_no 第三方寄存器
// backend/arm32/ILocArm32.cpp

/// @brief 保存寄存器到变量，保证将计算结果（例如 r8）保存到变量
/// @param src_reg_no 源寄存器
/// @param dest_var  目标变量
/// @param tmp_reg_no 用于加载全局变量地址等的临时寄存器 (例如 r10)
// backend/arm32/ILocArm32.cpp

void ILocArm32::store_var(int src_reg_no, Value * dest_var, int tmp_reg_no)
{
    // 检查 dest_var 是否为空
    if (dest_var == nullptr) {
        minic_log(LOG_ERROR, "ILocStoreVar: dest_var is nullptr! Cannot store from reg %s.",
                  PlatformArm32::getRegNameSafe(src_reg_no).c_str());
        emit("@ ERROR: store_var called with null dest_var from reg " + PlatformArm32::getRegNameSafe(src_reg_no));
        return;
    }

    // 检查源寄存器是否有效
    std::string src_reg_name = PlatformArm32::getRegNameSafe(src_reg_no);
    if (src_reg_no == -1 || src_reg_name.rfind("REG_ID(", 0) == 0 || src_reg_name.rfind("EMPTY_REG_NAME", 0) == 0) {
        minic_log(LOG_ERROR, "ILocStoreVar: Invalid source register ID %d (Name: %s) for storing to %s.",
                  src_reg_no, src_reg_name.c_str(), dest_var->getIRName().c_str());
        emit("@ ERROR: STORE from invalid source register " + src_reg_name + " to " + dest_var->getIRName());
        return;
    }

    // 检查是否尝试写入常量
    if (dest_var->isConstant()) {
        minic_log(LOG_ERROR, "ILocStoreVar: Attempting to store into a constant value %s from reg %s. This should not happen.",
                  dest_var->getIRName().c_str(), src_reg_name.c_str());
        emit("@ ERROR: STORE into constant " + dest_var->getIRName() + " from " + src_reg_name);
        return;
    }

    // --- 开始实际的存储逻辑 ---

    if (dest_var->getRegId() != -1 && dest_var->getRegId() < PlatformArm32::maxUsableRegNum) {
        // 目标是寄存器变量
        int dest_reg_id = dest_var->getRegId();
        std::string dest_reg_name = PlatformArm32::getRegNameSafe(dest_reg_id);

        // 再次检查目标寄存器名是否有效 (以防万一 getRegId 返回了有效范围内的错误ID)
        if (dest_reg_name.rfind("REG_ID(", 0) == 0 || dest_reg_name.rfind("EMPTY_REG_NAME", 0) == 0) {
             minic_log(LOG_ERROR, "ILocStoreVar: Target register variable %s (IRName: %s) has invalid register ID %d (Name: %s). Cannot store from %s.",
                       dest_var->getName().c_str(), dest_var->getIRName().c_str(), dest_reg_id, dest_reg_name.c_str(), src_reg_name.c_str());
             emit("@ ERROR: STORE to invalid target register variable " + dest_var->getIRName() + " (" + dest_reg_name + ") from " + src_reg_name);
             return;
        }
        
        minic_log(LOG_DEBUG, "ILocStoreVar: Dest %s (IRName: %s) is in reg %s. Source reg is %s.",
                  dest_var->getName().c_str(), dest_var->getIRName().c_str(), dest_reg_name.c_str(), src_reg_name.c_str());

        if (src_reg_no != dest_reg_id) {
            emit("mov", dest_reg_name, src_reg_name);
        }
        // 如果源和目标是同一个寄存器，则不需要操作

    } else if (Instanceof(globalVar, GlobalVariable *, dest_var)) {
        // 目标是全局变量
        minic_log(LOG_DEBUG, "ILocStoreVar: Storing from %s to global variable %s (IRName: %s) (using tmp_reg: %s)",
                  src_reg_name.c_str(), globalVar->getName().c_str(), globalVar->getIRName().c_str(), PlatformArm32::getRegNameSafe(tmp_reg_no).c_str());
        load_symbol(tmp_reg_no, globalVar->getName());
        std::string tmp_reg_name_for_addr = PlatformArm32::getRegNameSafe(tmp_reg_no);
        emit("str", src_reg_name, "[" + tmp_reg_name_for_addr + "]");

    } else {
        // 目标是栈上变量 (局部变量或溢出的临时变量)
        minic_log(LOG_DEBUG, "ILocStoreVar: Attempting stack store for Value (IRName: '%s', OrigName: '%s', DynType: %s, Ptr: %p, current regId: %d) from reg %s",
                  dest_var->getIRName().c_str(),
                  dest_var->getName().c_str(),
                  typeid(*dest_var).name(),
                  (void*)dest_var,
                  dest_var->getRegId(), // 这个 getRegId() 应该是 dest_var 的通用寄存器ID
                  src_reg_name.c_str());

        // ---- A2: 针对 LocalVariable 的详细日志 ----
        LocalVariable* lv_debug = dynamic_cast<LocalVariable*>(dest_var); // 用于日志的临时指针
        if (lv_debug) {
            minic_log(LOG_DEBUG, "ILocStoreVar: dest_var IS LocalVariable. Ptr: %p, IRName: '%s'. Internal state BEFORE getMemoryAddr: baseRegNo=%d, offset=%d.",
                      (void*)lv_debug, 
                      lv_debug->getIRName().c_str(), 
                      lv_debug->getBaseRegNoForDebug(),
                      lv_debug->getOffsetForDebug());
        } 
        // ---- 结束 A2 LocalVariable 日志 ----
        else if (dynamic_cast<TempVariable*>(dest_var)) {
            minic_log(LOG_DEBUG, "ILocStoreVar: dest_var IS TempVariable. Ptr: %p, IRName: '%s'.", (void*)dest_var, dest_var->getIRName().c_str());
        } else if (dynamic_cast<Instruction*>(dest_var)) { 
             minic_log(LOG_DEBUG, "ILocStoreVar: dest_var IS Instruction result. Ptr: %p, IRName: '%s'.", (void*)dest_var, dest_var->getIRName().c_str());
        }


        int32_t dest_baseRegId = -1;
        int64_t dest_offset = 0; 
        bool has_mem_addr = dest_var->getMemoryAddr(&dest_baseRegId, &dest_offset);

        // 在调用 getMemoryAddr 之后，再次检查 LocalVariable 的内部状态
        if (lv_debug) { // 使用之前 dynamic_cast 的结果
            minic_log(LOG_DEBUG, "ILocStoreVar: LocalVariable (Ptr: %p, IRName: '%s') AFTER getMemoryAddr. Return: %s. Effective baseRegId: %d, offset: %lld. Internal state AFTER: baseRegNo=%d, internal offset=%d.",
                      (void*)lv_debug, lv_debug->getIRName().c_str(),
                      has_mem_addr ? "true" : "false", dest_baseRegId, (long long)dest_offset,
                      lv_debug->getBaseRegNoForDebug(), lv_debug->getOffsetForDebug());
        } else {
             minic_log(LOG_DEBUG, "ILocStoreVar: Value (IRName: '%s', Ptr: %p, DynType: %s) AFTER getMemoryAddr. Return: %s. Effective baseRegId: %d, offset: %lld.",
                      dest_var->getIRName().c_str(), (void*)dest_var, typeid(*dest_var).name(),
                      has_mem_addr ? "true" : "false", dest_baseRegId, (long long)dest_offset);
        }

        if (has_mem_addr && dest_baseRegId != -1 && dest_baseRegId < PlatformArm32::maxRegNum) {
            minic_log(LOG_DEBUG, "ILocStoreVar: Proceeding to store_base for %s into [%s, #%lld] from %s",
                dest_var->getIRName().c_str(), PlatformArm32::getRegNameSafe(dest_baseRegId).c_str(), (long long)dest_offset, src_reg_name.c_str());
            store_base(src_reg_no, dest_baseRegId, dest_offset, tmp_reg_no);
        } else {
            minic_log(LOG_ERROR, "ILocStoreVar: FINAL DECISION - Value %s (DynType: %s, Ptr: %p) -> getMemoryAddr FAILED or returned invalid baseId. Actual has_mem_addr: %s, actual dest_baseRegId: %d. Cannot STR from %s.",
                       dest_var->getIRName().c_str(), typeid(*dest_var).name(), (void*)dest_var,
                       has_mem_addr ? "true":"false", dest_baseRegId, src_reg_name.c_str());
            // 生成错误标记的汇编指令
            emit("str", src_reg_name, "[NO_VALID_MEM_ADDR_FOR_" + dest_var->getIRName() + "]");
        }
    }
}

/// @brief 加载栈内变量地址
/// @param rsReg 结果寄存器号
/// @param base_reg_no 基址寄存器
/// @param off 偏移
void ILocArm32::leaStack(int rs_reg_no, int base_reg_no, int off)
{
    std::string rs_reg_name = PlatformArm32::getRegNameSafe(rs_reg_no);
	std::string base_reg_name = PlatformArm32::getRegNameSafe(base_reg_no);

    if (PlatformArm32::constExpr(off))
        // add r8,fp,#-16
        emit("add", rs_reg_name, base_reg_name, toStr(off));
    else {
        // ldr r8,=-257
        load_imm(rs_reg_no, off);

        // add r8,fp,r8
        emit("add", rs_reg_name, base_reg_name, rs_reg_name);
    }
}

/// @brief 函数内栈内空间分配（局部变量、形参变量、函数参数传值，或不能寄存器分配的临时变量等）
/// @param func 函数
/// @param tmp_reg_No
void ILocArm32::allocStack(Function * func, int tmp_reg_no)
{
    // 计算栈帧大小
    int off = func->getMaxDep();

    // 不需要在栈内额外分配空间，则什么都不做
    if (0 == off) {
        return;
    }

    // 保存SP寄存器到FP寄存器中
    mov_reg(ARM32_FP_REG_NO, ARM32_SP_REG_NO);

    if (PlatformArm32::constExpr(off)) {
        // sub sp,sp,#16
        emit("sub", "sp", "sp", toStr(off));
    } else {
        // ldr r8,=257
        load_imm(tmp_reg_no, off);

        // sub sp,sp,r8
        emit("sub", "sp", "sp", PlatformArm32::getRegNameSafe(tmp_reg_no));
    }
}

/// @brief 调用函数fun
/// @param fun
void ILocArm32::call_fun(std::string name)
{
    // 函数返回值在r0,不需要保护
    emit("bl", name);
}

/// @brief NOP操作
void ILocArm32::nop()
{
    // FIXME 无操作符，要确认是否用nop指令
    emit("");
}

///
/// @brief 无条件跳转指令
/// @param label 目标Label名称
///
// backend/arm32/ILocArm32.cpp
void ILocArm32::jump(std::string label) {
    if (!label.empty()) { // 确保标签名非空
        referencedLabelNames.insert(label); // <--- 记录使用的标签
    } else {
        minic_log(LOG_WARNING, "ILocArm32::jump called with empty label.");
    }
    emit("b", label);
}

void ILocArm32::conditional_jump(std::string cond_op, std::string label) {
    if (!label.empty()) {
        referencedLabelNames.insert(label);
        minic_log(LOG_DEBUG, "ILocArm32::conditional_jump: Referenced label '%s' from op '%s'", label.c_str(), cond_op.c_str());
    } else {
        minic_log(LOG_WARNING, "ILocArm32::conditional_jump called with empty label for op %s.", cond_op.c_str());
    }
    // 假设 emit(op, target) 是正确的 ArmInst 构造方式
    emit(cond_op, label); 
}
