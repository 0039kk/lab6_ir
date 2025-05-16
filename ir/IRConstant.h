///
/// @file IRConstant.h
/// @brief DragonIR定义的符号常量
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

#define IR_GLOBAL_VARNAME_PREFIX "@"
#define IR_LOCAL_VARNAME_PREFIX "%l"
#define IR_TEMP_VARNAME_PREFIX "%t"
#define IR_MEM_VARNAME_PREFIX "%m"
#define IR_LABEL_PREFIX ".L"

#define IR_KEYWORD_DECLARE "declare"
#define IR_KEYWORD_DEFINE "define"

#define IR_KEYWORD_ADD_I "add"
#define IR_KEYWORD_SUB_I "sub"
#define IR_KEYWORD_MUL_I "mul"
#define IR_KEYWORD_DIV_I "div"
#define IR_KEYWORD_MOD_I "mod"
#define IR_KEYWORD_NEG_I "neg"

#define IR_KEYWORD_ASSIGN "assign"
// --- 比较运算关键字 ---
#define IR_KEYWORD_CMP "cmp"
#define IR_CMP_OP_EQ "eq"
#define IR_CMP_OP_NE "ne"
#define IR_CMP_OP_GT "gt"
#define IR_CMP_OP_GE "ge"
#define IR_CMP_OP_LT "lt"
#define IR_CMP_OP_LE "le"

// --- 跳转和标签关键字 ---
#define IR_KEYWORD_BRANCH_COND "bc"     // 条件跳转
#define IR_KEYWORD_BRANCH_UNCOND "br"   // 无条件跳转
#define IR_KEYWORD_GOTO "goto"          // 等同于 br，有时也用 goto
#define IR_KEYWORD_LABEL "label"        // 可选的，在跳转指令中 "label .L0"

// --- 函数相关关键字 ---
#define IR_KEYWORD_CALL "call"      // 函数调用
#define IR_KEYWORD_RETURN "ret"     // 函数返回 (你需要一个ReturnInstruction)
#define IR_KEYWORD_ARG "arg"        // 函数参数传递 (如果ARG指令用于此)
#define IR_KEYWORD_ENTRY "entry"    // 函数入口
#define IR_KEYWORD_EXIT "exit"      // 函数出口