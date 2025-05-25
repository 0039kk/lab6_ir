///
/// @file InstSelectorArm32.cpp
/// @brief 指令选择器-ARM32的实现
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
#include <typeinfo>

#include "Common.h"
#include "ILocArm32.h"
#include "InstSelectorArm32.h"
#include "PlatformArm32.h"

#include "PointerType.h"
#include "RegVariable.h"
#include "Function.h"
#include "BinaryInstruction.h"
#include "LabelInstruction.h"
#include "GotoInstruction.h"
#include "FuncCallInstruction.h"
#include "MoveInstruction.h"
#include "CmpInstruction.h"
#include "BranchConditionalInstruction.h"
#include <iostream>
// ---- 确保这个辅助函数在这里定义 ----
static std::string getValueDetailsForInstSelector(Value* var) { // 我之前建议用这个名字
    if (!var) return "null_Value_ptr"; // 更明确一点
    std::string ir_name_str = var->getIRName(); 
    std::string name_str = var->getName();
    std::string type_str = "null_Type_ptr";
    if (var->getType()) {
        type_str = var->getType()->toString();
    }
    int32_t reg_id_val = var->getLoadRegId(); // 或 getRegId()

    // 使用更安全的字符串拼接或 ostringstream 以避免复杂的 + 操作
    std::string details = "'";
    details += ir_name_str;
    details += "' (OrigName: '";
    details += name_str;
    details += "', DynType: ";
    details += typeid(*var).name(); // 注意：如果 var 是 nullptr，这里会抛出 std::bad_typeid
                                   // 所以上面的 if (!var) 检查很重要
    details += ", Ptr: " + std::to_string(reinterpret_cast<uintptr_t>(var));
    details += ", LoadRegId: " + std::to_string(reg_id_val);
    details += ", TypeStr: " + type_str + ")";
    return details;
}
// ---- 结束辅助函数定义 ----
/// @brief 构造函数
/// @param _irCode 指令
/// @param _iloc ILoc
/// @param _func 函数
InstSelectorArm32::InstSelectorArm32(vector<Instruction *> & _irCode,
                                     ILocArm32 & _iloc,
                                     Function * _func,
                                     SimpleRegisterAllocator & allocator)
    : ir(_irCode), iloc(_iloc), func(_func), simpleRegisterAllocator(allocator)
{
    translator_handlers[IRInstOperator::IRINST_OP_ENTRY] = &InstSelectorArm32::translate_entry;
    translator_handlers[IRInstOperator::IRINST_OP_EXIT] = &InstSelectorArm32::translate_exit;

    translator_handlers[IRInstOperator::IRINST_OP_LABEL] = &InstSelectorArm32::translate_label;
    translator_handlers[IRInstOperator::IRINST_OP_GOTO] = &InstSelectorArm32::translate_goto;

    translator_handlers[IRInstOperator::IRINST_OP_ASSIGN] = &InstSelectorArm32::translate_assign;

    translator_handlers[IRInstOperator::IRINST_OP_ADD_I] = &InstSelectorArm32::translate_add_int32;
    translator_handlers[IRInstOperator::IRINST_OP_SUB_I] = &InstSelectorArm32::translate_sub_int32;
    translator_handlers[IRInstOperator::IRINST_OP_MUL_I] = &InstSelectorArm32::translate_mul_int32;
    translator_handlers[IRInstOperator::IRINST_OP_DIV_I] = &InstSelectorArm32::translate_div_int32;
    translator_handlers[IRInstOperator::IRINST_OP_MOD_I] = &InstSelectorArm32::translate_mod_int32;
    translator_handlers[IRInstOperator::IRINST_OP_NEG_I] = &InstSelectorArm32::translate_neg_int32;

	// 新增映射
    translator_handlers[IRInstOperator::IRINST_OP_CMP] = &InstSelectorArm32::translate_cmp;
    translator_handlers[IRInstOperator::IRINST_OP_BRANCH_COND] = &InstSelectorArm32::translate_branch_cond;

    
    translator_handlers[IRInstOperator::IRINST_OP_FUNC_CALL] = &InstSelectorArm32::translate_call;
    translator_handlers[IRInstOperator::IRINST_OP_ARG] = &InstSelectorArm32::translate_arg;
}

///
/// @brief 析构函数
///
InstSelectorArm32::~InstSelectorArm32()
{}

/// @brief 指令选择执行
void InstSelectorArm32::run()
{
    for (auto inst: ir) {

        // 逐个指令进行翻译
        if (!inst->isDead()) {
            translate(inst);
        }
    }
}

/// @brief 指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate(Instruction * inst)
{
    // --- 新增的详细日志 ---
    if (inst) {
        std::string inst_str_repr = inst->toString(); 
        minic_log(LOG_DEBUG, "InstSelector::translate: ENTRY for IR: %s (Ptr: %p, DynType: %s)",
                  inst_str_repr.c_str(), (void*)inst, typeid(*inst).name());

        // 特别关注 BinaryInstruction (以及其他你感兴趣的类型)
        // 首先声明用于 getMemoryAddr 的变量
        int32_t temp_instr_base_reg; 
        int64_t temp_instr_offset;
        bool instr_has_addr = inst->getMemoryAddr(&temp_instr_base_reg, &temp_instr_offset);

        minic_log(LOG_DEBUG, "  InstSelector::translate (Inst Ptr: %p): Initial MemAddr check -> HasAddr: %s, Base: %d, Offset: %lld",
                  (void*)inst,
                  instr_has_addr ? "true" : "false", 
                  instr_has_addr ? temp_instr_base_reg : -1, // 只在有地址时打印有效值
                  instr_has_addr ? (long long)temp_instr_offset : 0 // 只在有地址时打印有效值
                  );
        
        // 如果你想针对特定类型做更详细的检查，可以继续用 dynamic_cast
        // if (dynamic_cast<BinaryInstruction*>(inst)) {
        //     // 这里的 instr_has_addr, temp_instr_base_reg, temp_instr_offset 已经包含了信息
        //     minic_log(LOG_DEBUG, "    (Specifically a BinaryInstruction)");
        // }
    }
    // --- 结束新增日志 ---
    // 操作符
    IRInstOperator op = inst->getOp();

    map<IRInstOperator, translate_handler>::const_iterator pIter;
    pIter = translator_handlers.find(op);
    if (pIter == translator_handlers.end()) {
        // 没有找到，则说明当前不支持
        printf("Translate: Operator(%d) not support", (int) op);
        return;
    }

    // 开启时输出IR指令作为注释
    if (showLinearIR) {
        outputIRInstruction(inst);
    }

    (this->*(pIter->second))(inst);
}

///
/// @brief 输出IR指令
///
void InstSelectorArm32::outputIRInstruction(Instruction * inst)
{
    std::string irStr = inst->toString();
    if (!irStr.empty()) {
        iloc.comment(irStr);
    }
}

/// @brief NOP翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_nop(Instruction * inst)
{
    (void) inst;
    iloc.nop();
}

/// @brief Label指令指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_label(Instruction * inst)
{
    Instanceof(labelInst, LabelInstruction *, inst);

    minic_log(LOG_DEBUG, "InstSelector: Translating LABEL IR: %s. ASM output will be: %s:", 
		labelInst->toString().c_str(), labelInst->getName().c_str()); // getName() 应该是 IR 中的标签名，如 .L4
    iloc.label(labelInst->getName());
}

/// @brief goto指令指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_goto(Instruction * inst)
{
    Instanceof(gotoInst, GotoInstruction *, inst); // 使用你的 Instanceof 宏

    if (!gotoInst) { // 总是好的做法是检查 dynamic_cast/Instanceof 的结果
        minic_log(LOG_ERROR, "InstSelector: translate_goto called with non-GotoInstruction or null instruction.");
        if (inst) { // 如果 inst 不是 null，但 dynamic_cast 失败
             minic_log(LOG_ERROR, "InstSelector: Instruction was: %s", inst->toString().c_str());
        }
        return;
    }

    LabelInstruction* targetLabel = gotoInst->getTarget(); // 获取目标 LabelInstruction

    if (!targetLabel) {
        minic_log(LOG_ERROR, "InstSelector: GotoInstruction %s has a null target label.",
                  gotoInst->getIRName().c_str()); // 假设 Instruction 有 getIRName()
        return;
    }

    std::string targetLabelName = targetLabel->getName(); // 获取标签名 (例如 ".L1")
    // 或者 targetLabel->getIRName()，取决于哪个存储的是你期望在汇编中使用的标签名

    // --- 添加日志 ---
    minic_log(LOG_DEBUG, "InstSelector: Translating GOTO. From IR: %s. To Target Label: '%s' (Label IRName: '%s'). ASM will be: B %s",
              inst->toString().c_str(),    // 打印原始的 GOTO IR 指令字符串
              targetLabelName.c_str(),     // 打印目标标签的名称
              targetLabel->getIRName().c_str(), // 打印目标标签的IR名称 (可能与getName相同)
              targetLabelName.c_str()      // 打印将要生成的汇编跳转目标
              );
    // --- 结束日志 ---

    // 无条件跳转
    iloc.jump(targetLabelName); // 使用获取到的标签名
}

/// @brief 函数入口指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_entry(Instruction * inst)
{
    // 查看保护的寄存器
    auto & protectedRegNo = func->getProtectedReg();
    auto & protectedRegStr = func->getProtectedRegStr();

    bool first = true;
    for (auto regno: protectedRegNo) {
        if (first) {
            protectedRegStr = PlatformArm32::regName[regno];
            first = false;
        } else {
            protectedRegStr += "," + PlatformArm32::regName[regno];
        }
    }

    if (!protectedRegStr.empty()) {
        iloc.inst("push", "{" + protectedRegStr + "}");
    }

    // 为fun分配栈帧，含局部变量、函数调用值传递的空间等
    iloc.allocStack(func, ARM32_TMP_REG_NO);
}

/// @brief 函数出口指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_exit(Instruction * inst)
{
    if (inst->getOperandsNum()) {
        // 存在返回值
        Value * retVal = inst->getOperand(0);

        // 赋值给寄存器R0
        iloc.load_var(0, retVal);
    }

    // 恢复栈空间
    iloc.inst("mov", "sp", "fp");

    // 保护寄存器的恢复
    auto & protectedRegStr = func->getProtectedRegStr();
    if (!protectedRegStr.empty()) {
        iloc.inst("pop", "{" + protectedRegStr + "}");
    }

    iloc.inst("bx", "lr");
}

/// @brief 赋值指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_assign(Instruction * inst)
{
    Value * result = inst->getOperand(0);
    Value * arg1 = inst->getOperand(1);

    int32_t arg1_regId = arg1->getRegId();
    int32_t result_regId = result->getRegId();

    if (arg1_regId != -1) {
        // 寄存器 => 内存
        // 寄存器 => 寄存器

        // r8 -> rs 可能用到r9
        iloc.store_var(arg1_regId, result, ARM32_TMP_REG_NO);
    } else if (result_regId != -1) {
        // 内存变量 => 寄存器

        iloc.load_var(result_regId, arg1);
    } else {
        // 内存变量 => 内存变量

        int32_t temp_regno = simpleRegisterAllocator.Allocate();

        // arg1 -> r8
        iloc.load_var(temp_regno, arg1);

        // r8 -> rs 可能用到r9
        iloc.store_var(temp_regno, result, ARM32_TMP_REG_NO);

        simpleRegisterAllocator.free(temp_regno);
    }
}

/// @brief 一元操作指令翻译成ARM32汇编
/// @param inst IR指令
/// @param operator_name 操作码（如 "neg"）
void InstSelectorArm32::translate_one_operator(Instruction * inst, const std::string & operator_name)
{
    Value * result = inst;
    Value * arg = inst->getOperand(0); // 一元操作只有一个操作数

    // 检查操作数有效性
    if (!arg) {
        minic_log(LOG_ERROR, "Unary operator '%s' has no operand", operator_name.c_str());
        return;
    }

    // 分配或获取寄存器
    int32_t arg_reg_no = arg->getRegId();
    int32_t result_reg_no = inst->getRegId();
    int32_t load_arg_reg_no, load_result_reg_no;

    // 处理源操作数
    if (arg_reg_no == -1) {
        load_arg_reg_no = simpleRegisterAllocator.Allocate(arg);
        iloc.load_var(load_arg_reg_no, arg);
    } else {
        load_arg_reg_no = arg_reg_no;
    }

    // 处理目标操作数
    if (result_reg_no == -1) {
        load_result_reg_no = simpleRegisterAllocator.Allocate(result);
    } else {
        load_result_reg_no = result_reg_no;
    }

    // 生成汇编指令（一元操作格式：op dst, src）
    iloc.inst(operator_name,
              PlatformArm32::regName[load_result_reg_no],
              PlatformArm32::regName[load_arg_reg_no]);

    // 存储结果（若目标不是寄存器）
    if (result_reg_no == -1) {
        iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
    }

    // 释放寄存器
    simpleRegisterAllocator.free(arg);
    simpleRegisterAllocator.free(result);
}

/// @brief 二元操作指令翻译成ARM32汇编
/// @param inst IR指令
/// @param operator_name 操作码
/// @param rs_reg_no 结果寄存器号
/// @param op1_reg_no 源操作数1寄存器号
/// @param op2_reg_no 源操作数2寄存器号
void InstSelectorArm32::translate_two_operator(Instruction * inst, string operator_name)
{
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t result_reg_no = inst->getRegId();
    int32_t load_result_reg_no, load_arg1_reg_no, load_arg2_reg_no;
	  // --- 打印 Value* 地址 ---
    std::cout << "  translate_two_operator: inst (result) (Value*) at address: " << static_cast<void*>(result)
              << ", IRName: " << result->getIRName() << std::endl;
    if (arg1) { // 使用 arg1
        std::cout << "  translate_two_operator: arg1 (Value*) at address: " << static_cast<void*>(arg1)
                  << ", IRName: " << arg1->getIRName() << std::endl;
    } else {
        std::cout << "  translate_two_operator: arg1 is nullptr" << std::endl;
    }
    if (arg2) { // 使用 arg2
        std::cout << "  translate_two_operator: arg2 (Value*) at address: " << static_cast<void*>(arg2)
                  << ", IRName: " << arg2->getIRName() << std::endl;
    } else {
        std::cout << "  translate_two_operator: arg2 is nullptr" << std::endl;
    }
    // --- 结束打印 ---
    // 看arg1是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg1_reg_no == -1) {

        // 分配一个寄存器r8
        load_arg1_reg_no = simpleRegisterAllocator.Allocate(arg1);

        // arg1 -> r8，这里可能由于偏移不满足指令的要求，需要额外分配寄存器
        iloc.load_var(load_arg1_reg_no, arg1);
    } else {
        load_arg1_reg_no = arg1_reg_no;
    }

    // 看arg2是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg2_reg_no == -1) {

        // 分配一个寄存器r9
        load_arg2_reg_no = simpleRegisterAllocator.Allocate(arg2);

        // arg2 -> r9
        iloc.load_var(load_arg2_reg_no, arg2);
    } else {
        load_arg2_reg_no = arg2_reg_no;
    }

    // 看结果变量是否是寄存器，若不是则需要分配一个新的寄存器来保存运算的结果
    if (result_reg_no == -1) {
        // 分配一个寄存器r10，用于暂存结果
        load_result_reg_no = simpleRegisterAllocator.Allocate(result);
    } else {
        load_result_reg_no = result_reg_no;
    }

    // r8 + r9 -> r10
    iloc.inst(operator_name,
              PlatformArm32::regName[load_result_reg_no],
              PlatformArm32::regName[load_arg1_reg_no],
              PlatformArm32::regName[load_arg2_reg_no]);

    // 结果不是寄存器，则需要把rs_reg_name保存到结果变量中
    if (result_reg_no == -1) {

        // 这里使用预留的临时寄存器，因为立即数可能过大，必须借助寄存器才可操作。

        // r10 -> result
        iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
    }

    // 释放寄存器
    simpleRegisterAllocator.free(arg1);
    simpleRegisterAllocator.free(arg2);
    simpleRegisterAllocator.free(result);
}

/// @brief 整数加法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_add_int32(Instruction * inst)
{
    translate_two_operator(inst, "add");
}

/// @brief 整数减法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_sub_int32(Instruction * inst)
{
    translate_two_operator(inst, "sub");
}

/// @brief 整数乘法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_mul_int32(Instruction * inst)
{
    translate_two_operator(inst, "mul");
}

/// @brief 整数除法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_div_int32(Instruction * inst)
{
	translate_two_operator(inst, "sdiv");
}

/// @brief 整数取模指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_mod_int32(Instruction * inst)
{
	translate_two_operator(inst, "s");
}

/// @brief 整数取反指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_neg_int32(Instruction * inst)
{
    translate_one_operator(inst, "neg");
}


/// @brief 函数调用指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_call(Instruction * inst)
{
    FuncCallInstruction * callInst = dynamic_cast<FuncCallInstruction *>(inst);
	if (!callInst) {
        minic_log(LOG_ERROR, "Translate CALL: Instruction is not FuncCallInstruction.");
        return;
    }
    int32_t operandNum = callInst->getOperandsNum();

    if (operandNum != realArgCount) {

        // 两者不一致 也可能没有ARG指令，正常
        if (realArgCount != 0) {

            minic_log(LOG_ERROR, "ARG指令的个数与调用函数个数不一致");
        }
    }

    if (operandNum) {

        // 强制占用这几个寄存器参数传递的寄存器
        simpleRegisterAllocator.Allocate(0);
        simpleRegisterAllocator.Allocate(1);
        simpleRegisterAllocator.Allocate(2);
        simpleRegisterAllocator.Allocate(3);

        // 前四个的后面参数采用栈传递
        int esp = 0;
        for (int32_t k = 4; k < operandNum; k++) {

            auto arg = callInst->getOperand(k);

            // 新建一个内存变量，用于栈传值到形参变量中
            MemVariable * newVal = func->newMemVariable((Type *) PointerType::get(arg->getType()));
            newVal->setMemoryAddr(ARM32_SP_REG_NO, esp);
            esp += 4;

            Instruction * assignInst = new MoveInstruction(func, newVal, arg);

            // 翻译赋值指令
            translate_assign(assignInst);

            delete assignInst;
        }

        for (int32_t k = 0; k < operandNum && k < 4; k++) {

            auto arg = callInst->getOperand(k);

            // 检查实参的类型是否是临时变量。
            // 如果是临时变量，该变量可更改为寄存器变量即可，或者设置寄存器号
            // 如果不是，则必须开辟一个寄存器变量，然后赋值即可

            Instruction * assignInst = new MoveInstruction(func, PlatformArm32::intRegVal[k], arg);

            // 翻译赋值指令
            translate_assign(assignInst);

            delete assignInst;
        }
    }

    iloc.call_fun(callInst->getName());

    if (operandNum) {
        simpleRegisterAllocator.free(0);
        simpleRegisterAllocator.free(1);
        simpleRegisterAllocator.free(2);
        simpleRegisterAllocator.free(3);
    }

    // 赋值指令
    if (callInst->hasResultValue()) {

        // 新建一个赋值操作
        Instruction * assignInst = new MoveInstruction(func, callInst, PlatformArm32::intRegVal[0]);

        // 翻译赋值指令
        translate_assign(assignInst);

        delete assignInst;
    }

    // 函数调用后清零，使得下次可正常统计
    realArgCount = 0;
}

///
/// @brief 实参指令翻译成ARM32汇编
/// @param inst
///
void InstSelectorArm32::translate_arg(Instruction * inst)
{
    // 翻译之前必须确保源操作数要么是寄存器，要么是内存，否则出错。
    Value * src = inst->getOperand(0);

    // 当前统计的ARG指令个数
    int32_t regId = src->getRegId();

    if (realArgCount < 4) {
        // 前四个参数
        if (regId != -1) {
            if (regId != realArgCount) {
                // 肯定寄存器分配有误
                minic_log(LOG_ERROR, "第%d个ARG指令对象寄存器分配有误: %d", argCount + 1, regId);
            }
        } else {
            minic_log(LOG_ERROR, "第%d个ARG指令对象不是寄存器", argCount + 1);
        }
    } else {
        // 必须是内存分配，若不是则出错
        int32_t baseRegId;
        bool result = src->getMemoryAddr(&baseRegId);
        if ((!result) || (baseRegId != ARM32_SP_REG_NO)) {

            minic_log(LOG_ERROR, "第%d个ARG指令对象不是SP寄存器寻址", argCount + 1);
        }
    }

    realArgCount++;
}



void InstSelectorArm32::translate_cmp(Instruction * inst) {
    CmpInstruction *cmpInst = dynamic_cast<CmpInstruction *>(inst);
    if (!cmpInst) {
        minic_log(LOG_ERROR, "Translate CMP: Instruction is not a CmpInstruction.");
        return;
    }

    Value *dest_val = cmpInst->getDest();     // i1 结果，例如 %t1
    Value *src1_val = cmpInst->getOperand1(); // 例如 %a
    Value *src2_val = cmpInst->getOperand2(); // 例如 %b
    CmpInstruction::CmpOp op = cmpInst->getOperator();

    if (!dest_val || !src1_val || !src2_val) {
        minic_log(LOG_ERROR, "Translate CMP: Operands or destination is null.");
        return;
    }
    if (!dest_val->getType() || !dest_val->getType()->isInt1Byte()) {
         minic_log(LOG_ERROR, "Translate CMP: Destination is not i1 type.");
         return;
    }

    // 1. 确保源操作数在寄存器中
    int32_t src1_reg_id = src1_val->getRegId(); // 使用你统一的 getRegId()
    bool src1_is_temp_alloc = false;
    if (src1_reg_id == -1) {
        src1_reg_id = simpleRegisterAllocator.Allocate(src1_val);
        if (src1_reg_id == -1) { minic_log(LOG_ERROR, "Translate CMP: Failed to allocate register for src1."); return; }
        iloc.load_var(src1_reg_id, src1_val);
        src1_is_temp_alloc = true;
    }

    int32_t src2_reg_id = -1;
    bool src2_is_temp_alloc = false;
    bool src2_is_immediate_for_cmp = false;

    // 尝试优化：如果 src2 是常量且可用于 ARM CMP 的立即数
    if (src2_val->isConstant()) {
        int32_t imm_val = static_cast<ConstInt*>(src2_val)->getVal();
        // ARM CMP 指令的立即数通常是受限的（例如，可旋转的8位值）
        // 你需要一个 PlatformArm32::isValidCmpImmediate(imm_val) 函数
        if (PlatformArm32::isValidCmpImmediate(imm_val)) { // 假设此函数存在
            iloc.inst("cmp", PlatformArm32::regName[src1_reg_id], "#" + std::to_string(imm_val));
            src2_is_immediate_for_cmp = true;
        }
    }

    if (!src2_is_immediate_for_cmp) {
        src2_reg_id = src2_val->getRegId();
        if (src2_reg_id == -1) {
            src2_reg_id = simpleRegisterAllocator.Allocate(src2_val);
            if (src2_reg_id == -1) { 
                minic_log(LOG_ERROR, "Translate CMP: Failed to allocate register for src2."); 
                if(src1_is_temp_alloc) simpleRegisterAllocator.free(src1_val);
                return; 
            }
            iloc.load_var(src2_reg_id, src2_val);
            src2_is_temp_alloc = true;
        }
        iloc.inst("cmp", PlatformArm32::regName[src1_reg_id], PlatformArm32::regName[src2_reg_id]);
    }

    // 2. 为目标 dest_val (i1 结果) 分配寄存器
    int32_t dest_reg_id = simpleRegisterAllocator.Allocate(dest_val);
    if (dest_reg_id == -1) { 
        minic_log(LOG_ERROR, "Translate CMP: Failed to allocate register for destination."); 
        if(src1_is_temp_alloc) simpleRegisterAllocator.free(src1_val);
        if(src2_is_temp_alloc && !src2_is_immediate_for_cmp) simpleRegisterAllocator.free(src2_val);
        return; 
    }

    std::string arm_cond_code_true;  // 条件为真时的 ARM 条件码后缀
    std::string arm_cond_code_false; // 条件为假时的 ARM 条件码后缀 (通常是相反的条件)

    switch (op) {
        case CmpInstruction::EQ: arm_cond_code_true = "eq"; arm_cond_code_false = "ne"; break;
        case CmpInstruction::NE: arm_cond_code_true = "ne"; arm_cond_code_false = "eq"; break;
        case CmpInstruction::GT: arm_cond_code_true = "gt"; arm_cond_code_false = "le"; break; // Signed >
        case CmpInstruction::GE: arm_cond_code_true = "ge"; arm_cond_code_false = "lt"; break; // Signed >=
        case CmpInstruction::LT: arm_cond_code_true = "lt"; arm_cond_code_false = "ge"; break; // Signed <
        case CmpInstruction::LE: arm_cond_code_true = "le"; arm_cond_code_false = "gt"; break; // Signed <=
        default:
            minic_log(LOG_ERROR, "Translate CMP: Unknown CmpOp.");
            if(src1_is_temp_alloc) simpleRegisterAllocator.free(src1_val);
            if(src2_is_temp_alloc && !src2_is_immediate_for_cmp) simpleRegisterAllocator.free(src2_val);
            simpleRegisterAllocator.free(dest_val); // 释放为 dest 新分配的
            return;
    }

    // 3. 根据 CMP 设置的条件码，将 1 或 0 移动到目标寄存器
    // MOV<cond_true> Rd, #1
    // MOV<cond_false> Rd, #0
    iloc.inst("mov" + arm_cond_code_true, PlatformArm32::regName[dest_reg_id], "#1");
    iloc.inst("mov" + arm_cond_code_false, PlatformArm32::regName[dest_reg_id], "#0");

    // 4. 释放为源操作数临时分配的寄存器
    if (src1_is_temp_alloc) {
        simpleRegisterAllocator.free(src1_val);
    }
    if (src2_is_temp_alloc && !src2_is_immediate_for_cmp) {
        simpleRegisterAllocator.free(src2_val);
    }
    // dest_val 的寄存器 dest_reg_id 的生命周期由其后续使用决定。
    // 通常，cmp 的结果会被紧接着的 bc 使用，bc 使用完后可以释放。
}



void InstSelectorArm32::translate_branch_cond(Instruction * inst) {
    BranchConditionalInstruction *bcInst = dynamic_cast<BranchConditionalInstruction *>(inst);
    if (!bcInst) {
        minic_log(LOG_ERROR, "Translate BC: Instruction (ptr %p, IR: %s) is not a BranchConditionalInstruction.",
                  (void*)inst, inst ? inst->toString().c_str() : "null_instr");
        return;
    }

    Value *cond_val = bcInst->getCondition();
    LabelInstruction *true_target = bcInst->getTrueTarget();
    LabelInstruction *false_target = bcInst->getFalseTarget();

    // --- 详细日志：打印指令和操作数信息 ---
    std::string cond_val_str = cond_val ? getValueDetailsForInstSelector(cond_val) : "null_cond_val"; // 使用 getValueDetails 辅助函数
    std::string true_target_str = true_target ? "'" + true_target->getName() + "' (IR: '" + true_target->getIRName() + "')" : "null_true_target";
    std::string false_target_str = false_target ? "'" + false_target->getName() + "' (IR: '" + false_target->getIRName() + "')" : "null_false_target";

    minic_log(LOG_DEBUG, "InstSelector: Translating BRANCH_COND. IR: %s", inst->toString().c_str());
    minic_log(LOG_DEBUG, "  Condition Value: %s", cond_val_str.c_str());
    minic_log(LOG_DEBUG, "  True Target: %s", true_target_str.c_str());
    minic_log(LOG_DEBUG, "  False Target: %s", false_target_str.c_str());
    // --- 结束日志 ---

    if (!cond_val || !true_target || !false_target) {
        minic_log(LOG_ERROR, "Translate BC: Condition, TrueTarget, or FalseTarget is null for IR: %s", inst->toString().c_str());
        return;
    }
    if (!cond_val->getType() || !cond_val->getType()->isInt1Byte()) { // isInt1Byte 假设代表布尔类型 (i1)
         minic_log(LOG_ERROR, "Translate BC: Condition value %s is not i1 type for IR: %s",
                   cond_val->getIRName().c_str(), inst->toString().c_str());
         return;
    }

    // 1. 确保条件值 cond_val (i1, 应该是 0 或 1) 在寄存器中
    int32_t cond_reg_id = cond_val->getRegId(); // 尝试获取已分配的寄存器
                                             // (现在 Value 基类应该能正确返回 allocatedRegisterId_)
    bool allocated_now = false;

    if (cond_reg_id == -1 || cond_reg_id >= PlatformArm32::maxUsableRegNum) { // 如果未分配或ID无效
        if (cond_reg_id != -1) { // 如果ID无效但不是-1
            minic_log(LOG_WARNING, "Translate BC: Condition %s had invalid pre-assigned regId %d. Attempting new allocation.",
                      cond_val->getIRName().c_str(), cond_reg_id);
        }
        minic_log(LOG_DEBUG, "Translate BC: Condition %s not in usable register. Allocating...", cond_val->getIRName().c_str());
        cond_reg_id = simpleRegisterAllocator.Allocate(cond_val);
        allocated_now = true;

        if (cond_reg_id == -1) { 
            minic_log(LOG_ERROR, "Translate BC: Failed to allocate register for condition %s (IR: %s). Skipping instruction.",
                      cond_val->getIRName().c_str(), inst->toString().c_str());
            iloc.comment("@ ERROR: BC failed, cannot allocate register for condition " + cond_val->getIRName());
            return; // 必须返回，否则后续使用 cond_reg_id 会出错
        }
        minic_log(LOG_DEBUG, "Translate BC: Allocated reg %s for condition %s. Loading var...",
                  PlatformArm32::getRegNameSafe(cond_reg_id).c_str(), cond_val->getIRName().c_str());
        iloc.load_var(cond_reg_id, cond_val); // 从内存加载条件值 (如果它不在寄存器中)
    } else {
        minic_log(LOG_DEBUG, "Translate BC: Condition %s is already in reg %s.",
                  cond_val->getIRName().c_str(), PlatformArm32::getRegNameSafe(cond_reg_id).c_str());
    }

    std::string cond_reg_name = PlatformArm32::getRegNameSafe(cond_reg_id);

    // 2. 将条件寄存器与 #0 比较以设置标志位
    minic_log(LOG_DEBUG, "Translate BC: Emitting CMP %s, #0", cond_reg_name.c_str());
    iloc.inst("cmp", cond_reg_name, "#0");
    
    // 3. 根据比较结果进行条件跳转
    // 如果 cond_reg != 0 (即 cond_val 为 1, 条件为真), 跳转到 true_target
    // 确保使用正确的标签名称属性 (getName() 或 getIRName())
    std::string true_label_asm_name = true_target->getName(); // 或者 getIRName()，取决于你的约定
    minic_log(LOG_DEBUG, "Translate BC: Emitting BNE %s", true_label_asm_name.c_str());
    iloc.inst("bne", true_label_asm_name); 
    
    // 4. 否则 (cond_reg == 0, 即 cond_val 为 0, 条件为假), 无条件跳转到 false_target
    std::string false_label_asm_name = false_target->getName(); // 或者 getIRName()
    minic_log(LOG_DEBUG, "Translate BC: Emitting B %s", false_label_asm_name.c_str());
    iloc.jump(false_label_asm_name);

    // 5. 释放为条件变量 cond_val 分配/使用的寄存器
    // 只有当这个寄存器是为此 bc 指令临时分配的，或者我们确定 cond_val 之后不再需要时才释放。
    // 如果 cond_val 是 CmpInstruction 的结果，并且可能被其他指令使用，这里释放可能过早。
    // 但如果 SimpleRegisterAllocator 的 free(Value*) 是安全的（例如基于引用计数或活跃性），
    // 或者如果 cond_val 的生命周期确实在此结束，则可以释放。
    // 假设这里的 cond_val 在分支后不再需要其寄存器。
    minic_log(LOG_DEBUG, "Translate BC: Freeing register for condition %s (Reg: %s, Allocated now: %s)",
              cond_val->getIRName().c_str(), cond_reg_name.c_str(), allocated_now ? "yes" : "no (was pre-allocated)");
    simpleRegisterAllocator.free(cond_val); 
}