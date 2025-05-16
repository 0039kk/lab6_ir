///
/// @file IRGenerator.cpp
/// @brief AST遍历产生线性IR的源文件
/// @author zenglj (zenglj@live.com)
/// @version 1.1
/// @date 2024-11-23
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// <tr><td>2024-11-23 <td>1.1     <td>zenglj  <td>表达式版增强
/// </table>
///
#include <cstdint>
#include <cstdio>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "AST.h"
#include "Common.h"
#include "Function.h"
#include "IRCode.h"
#include "IRGenerator.h"
#include "Module.h"
#include "EntryInstruction.h"
#include "LabelInstruction.h"
#include "ExitInstruction.h"
#include "FuncCallInstruction.h"
#include "BinaryInstruction.h"
#include "UnaryInstruction.h"

#include "BranchConditionalInstruction.h"
#include "CmpInstruction.h"

#include "MoveInstruction.h"
#include "GotoInstruction.h"

/// @brief 构造函数
/// @param _root AST的根
/// @param _module 符号表
IRGenerator::IRGenerator(ast_node * _root, Module * _module) : root(_root), module(_module)
{
    /* 叶子节点 */
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_LITERAL_UINT] = &IRGenerator::ir_leaf_node_uint;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_VAR_ID] = &IRGenerator::ir_leaf_node_var_id;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_TYPE] = &IRGenerator::ir_leaf_node_type;

    /* 表达式运算， 加减 */
    ast2ir_handlers[ast_operator_type::AST_OP_SUB] = &IRGenerator::ir_sub;
    ast2ir_handlers[ast_operator_type::AST_OP_ADD] = &IRGenerator::ir_add;
    /* 表达式运算， 乘、除、取模 */
    ast2ir_handlers[ast_operator_type::AST_OP_MUL] = &IRGenerator::ir_mul;
    ast2ir_handlers[ast_operator_type::AST_OP_DIV] = &IRGenerator::ir_div;
    ast2ir_handlers[ast_operator_type::AST_OP_MOD] = &IRGenerator::ir_mod;
    /* 表达式运算， 一元运算 */
    ast2ir_handlers[ast_operator_type::AST_OP_NEG] = &IRGenerator::ir_neg;

    /* 语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_ASSIGN] = &IRGenerator::ir_assign;
    ast2ir_handlers[ast_operator_type::AST_OP_RETURN] = &IRGenerator::ir_return;

    /* 函数调用 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_CALL] = &IRGenerator::ir_function_call;

    /* 函数定义 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_DEF] = &IRGenerator::ir_function_define;
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS] = &IRGenerator::ir_function_formal_params;

    /* 变量定义语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_DECL_STMT] = &IRGenerator::ir_declare_statment;
    ast2ir_handlers[ast_operator_type::AST_OP_VAR_DECL] = &IRGenerator::ir_variable_declare;

    /* 语句块 */
    ast2ir_handlers[ast_operator_type::AST_OP_BLOCK] = &IRGenerator::ir_block;

    /* 编译单元 */
    ast2ir_handlers[ast_operator_type::AST_OP_COMPILE_UNIT] = &IRGenerator::ir_compile_unit;

	/*--- 新增：为语句类型注册处理器 ---
    这些处理器将由 ir_visit_ast_node 调用*/
    ast2ir_handlers[ast_operator_type::AST_OP_IF] = &IRGenerator::ir_if_statement;
    ast2ir_handlers[ast_operator_type::AST_OP_WHILE] = &IRGenerator::ir_while_statement;
    ast2ir_handlers[ast_operator_type::AST_OP_BREAK] = &IRGenerator::ir_break_statement;
    ast2ir_handlers[ast_operator_type::AST_OP_CONTINUE] = &IRGenerator::ir_continue_statement;

  
    ast2ir_handlers[ast_operator_type::AST_OP_LNOT] = &IRGenerator::ir_lnot_expression;


}

/// @brief 遍历抽象语法树产生线性IR，保存到IRCode中
/// @param root 抽象语法树
/// @param IRCode 线性IR
/// @return true: 成功 false: 失败
bool IRGenerator::run()
{
    ast_node * node;

    // 从根节点进行遍历
    node = ir_visit_ast_node(root);

    return node != nullptr;
}

/// @brief 根据AST的节点运算符查找对应的翻译函数并执行翻译动作
/// @param node AST节点
/// @return 成功返回node节点，否则返回nullptr
ast_node * IRGenerator::ir_visit_ast_node(ast_node * node)
{
    // 空节点
    if (nullptr == node) {
        return nullptr;
    }

    bool result;

    std::unordered_map<ast_operator_type, ast2ir_handler_t>::const_iterator pIter;
    pIter = ast2ir_handlers.find(node->node_type);
    if (pIter == ast2ir_handlers.end()) {
        // 没有找到，则说明当前不支持
        result = (this->ir_default)(node);
    } else {
        result = (this->*(pIter->second))(node);
    }

    if (!result) {
        // 语义解析错误，则出错返回
        node = nullptr;
    }

    return node;
}

/// @brief 未知节点类型的节点处理
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_default(ast_node * node)
{
    // 未知的节点
    printf("Unkown node(%d)\n", (int) node->node_type);
    return true;
}

/// @brief 编译单元AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_compile_unit(ast_node * node)
{
    module->setCurrentFunction(nullptr);

    for (auto son: node->sons) {

        // 遍历编译单元，要么是函数定义，要么是语句
        ast_node * son_node = ir_visit_ast_node(son);
        if (!son_node) {
            // TODO 自行追加语义错误处理
            return false;
        }
    }

    return true;
}

/// @brief 函数定义AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_define(ast_node * node)
{
    bool result;

    // 创建一个函数，用于当前函数处理
    if (module->getCurrentFunction()) {
        // 函数中嵌套定义函数，这是不允许的，错误退出
        // TODO 自行追加语义错误处理
        return false;
    }

    // 函数定义的AST包含四个孩子
    // 第一个孩子：函数返回类型
    // 第二个孩子：函数名字
    // 第三个孩子：形参列表
    // 第四个孩子：函数体即block
    ast_node * type_node = node->sons[0];
    ast_node * name_node = node->sons[1];
    ast_node * param_node = node->sons[2];
    ast_node * block_node = node->sons[3];

    // 创建一个新的函数定义
    Function * newFunc = module->newFunction(name_node->name, type_node->type);
    if (!newFunc) {
        // 新定义的函数已经存在，则失败返回。
        // TODO 自行追加语义错误处理
        return false;
    }

    // 当前函数设置有效，变更为当前的函数
    module->setCurrentFunction(newFunc);

    // 进入函数的作用域
    module->enterScope();

    // 获取函数的IR代码列表，用于后面追加指令用，注意这里用的是引用传值
    InterCode & irCode = newFunc->getInterCode();

    // 这里也可增加一个函数入口Label指令，便于后续基本块划分
	LabelInstruction* entryLabel = new LabelInstruction(newFunc, ".L" + std::to_string(label_counter_++));
	irCode.addInst(entryLabel);
    // 创建并加入Entry入口指令
    irCode.addInst(new EntryInstruction(newFunc));

    // 创建出口指令并不加入出口指令，等函数内的指令处理完毕后加入出口指令
    std::string exit_label_name = ".L" + std::to_string(label_counter_++); // 或者其他唯一名称生成方式
	LabelInstruction * exitLabelInst = new LabelInstruction(newFunc, exit_label_name);

    // 函数出口指令保存到函数信息中，因为在语义分析函数体时return语句需要跳转到函数尾部，需要这个label指令
    newFunc->setExitLabel(exitLabelInst);

    // 遍历形参，没有IR指令，不需要追加
    result = ir_function_formal_params(param_node);
    if (!result) {
        // 形参解析失败
        // TODO 自行追加语义错误处理
        return false;
    }
    node->blockInsts.addInst(param_node->blockInsts);

    // 新建一个Value，用于保存函数的返回值，如果没有返回值可不用申请
    LocalVariable * retValue = nullptr;
    if (!type_node->type->isVoidType()) {

        // 保存函数返回值变量到函数信息中，在return语句翻译时需要设置值到这个变量中
        retValue = static_cast<LocalVariable *>(module->newVarValue(type_node->type));
    }
    newFunc->setReturnValue(retValue);

    // 这里最好设置返回值变量的初值为0，以便在没有返回值时能够返回0

    // 函数内已经进入作用域，内部不再需要做变量的作用域管理
    block_node->needScope = false;

    // 遍历block
    result = ir_block(block_node);
    if (!result) {
        // block解析失败
        // TODO 自行追加语义错误处理
        return false;
    }

    // IR指令追加到当前的节点中
    node->blockInsts.addInst(block_node->blockInsts);

    // 此时，所有指令都加入到当前函数中，也就是node->blockInsts

    // node节点的指令移动到函数的IR指令列表中
    irCode.addInst(node->blockInsts);

    // 添加函数出口Label指令，主要用于return语句跳转到这里进行函数的退出
    irCode.addInst(exitLabelInst);

    // 函数出口指令
    irCode.addInst(new ExitInstruction(newFunc, retValue));

    // 恢复成外部函数
    module->setCurrentFunction(nullptr);

    // 退出函数的作用域
    module->leaveScope();

    return true;
}

/// @brief 形式参数AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_formal_params(ast_node * node)
{
    // TODO 目前形参还不支持，直接返回true

    // 每个形参变量都创建对应的临时变量，用于表达实参转递的值
    // 而真实的形参则创建函数内的局部变量。
    // 然后产生赋值指令，用于把表达实参值的临时变量拷贝到形参局部变量上。
    // 请注意这些指令要放在Entry指令后面，因此处理的先后上要注意。

    return true;
}

/// @brief 函数调用AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_call(ast_node * node)
{
    std::vector<Value *> realParams;

    // 获取当前正在处理的函数
    Function * currentFunc = module->getCurrentFunction();

    // 函数调用的节点包含两个节点：
    // 第一个节点：函数名节点
    // 第二个节点：实参列表节点

    std::string funcName = node->sons[0]->name;
    int64_t lineno = node->sons[0]->line_no;

    ast_node * paramsNode = node->sons[1];

    // 根据函数名查找函数，看是否存在。若不存在则出错
    // 这里约定函数必须先定义后使用
    auto calledFunction = module->findFunction(funcName);
    if (nullptr == calledFunction) {
        minic_log(LOG_ERROR, "函数(%s)未定义或声明", funcName.c_str());
        return false;
    }

    // 当前函数存在函数调用
    currentFunc->setExistFuncCall(true);

    // 如果没有孩子，也认为是没有参数
    if (!paramsNode->sons.empty()) {

        int32_t argsCount = (int32_t) paramsNode->sons.size();

        // 当前函数中调用函数实参个数最大值统计，实际上是统计实参传参需在栈中分配的大小
        // 因为目前的语言支持的int和float都是四字节的，只统计个数即可
        if (argsCount > currentFunc->getMaxFuncCallArgCnt()) {
            currentFunc->setMaxFuncCallArgCnt(argsCount);
        }

        // 遍历参数列表，孩子是表达式
        // 这里自左往右计算表达式
        for (auto son: paramsNode->sons) {

            // 遍历Block的每个语句，进行显示或者运算
            ast_node * temp = ir_visit_ast_node(son);
            if (!temp) {
                return false;
            }

            realParams.push_back(temp->val);
            node->blockInsts.addInst(temp->blockInsts);
        }
    }

    // TODO 这里请追加函数调用的语义错误检查，这里只进行了函数参数的个数检查等，其它请自行追加。
    if (realParams.size() != calledFunction->getParams().size()) {
        // 函数参数的个数不一致，语义错误
        minic_log(LOG_ERROR, "第%lld行的被调用函数(%s)未定义或声明", (long long) lineno, funcName.c_str());
        return false;
    }

    // 返回调用有返回值，则需要分配临时变量，用于保存函数调用的返回值
    Type * return_type = calledFunction->getReturnType();

    FuncCallInstruction * funcCallInst = new FuncCallInstruction(currentFunc, calledFunction, realParams, return_type);

    // 创建函数调用指令
    node->blockInsts.addInst(funcCallInst);
    
	if (return_type && !return_type->isVoidType()) {
        currentFunc->addTempVar(funcCallInst); // <--- 确保这行被执行
    }
    // 函数调用结果Value保存到node中，可能为空，上层节点可利用这个值
    node->val = funcCallInst;

    return true;
}

/// @brief 语句块（含函数体）AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_block(ast_node * node)
{
    // 进入作用域
    if (node->needScope) {
        module->enterScope();
    }

    std::vector<ast_node *>::iterator pIter;
    for (pIter = node->sons.begin(); pIter != node->sons.end(); ++pIter) {

        // 遍历Block的每个语句，进行显示或者运算
        ast_node * temp = ir_visit_ast_node(*pIter);
        if (!temp) {
            return false;
        }

        node->blockInsts.addInst(temp->blockInsts);
    }

    // 离开作用域
    if (node->needScope) {
        module->leaveScope();
    }

    return true;
}

/// @brief 整数加法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_add(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 加法节点，左结合，先计算左节点，后计算右节点

    // 加法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 加法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_ADD_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(addInst);
	module->getCurrentFunction()->addTempVar(addInst);
    node->val = addInst;

    return true;
}

/// @brief 整数减法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_sub(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 加法节点，左结合，先计算左节点，后计算右节点

    // 加法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 加法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * subInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_SUB_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(subInst);
	module->getCurrentFunction()->addTempVar(subInst);
    node->val = subInst;

    return true;
}
/// @brief 整数乘法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mul(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];
    // 乘法节点，左结合，先计算左节点，后计算右节点
    // 乘法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }
    // 乘法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }
    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理
    BinaryInstruction * mulInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MUL_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());
    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(mulInst);
    module->getCurrentFunction()->addTempVar(mulInst);
    node->val = mulInst;
    return true;
}

/// @brief 整数除法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_div(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];
    // 除法节点，左结合，先计算左节点，后计算右节点
    // 除法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }
    // 除法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {

        // 某个变量没有定值
        return false;
    }
    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理
    BinaryInstruction * divInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_DIV_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());
    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(divInst);
    module->getCurrentFunction()->addTempVar(divInst);
    node->val = divInst;
    return true;
}

/// @brief 整数取模AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
// IRGenerator.cpp
bool IRGenerator::ir_mod(ast_node * node) {
    if (!node || node->sons.size() != 2) {
        minic_log(LOG_ERROR, "MOD node is null or does not have 2 operands.");
        return false;
    }

    ast_node * src1_node = node->sons[0]; // a
    ast_node * src2_node = node->sons[1]; // b

    // --- 访问左操作数 (a) ---
    ast_node * visited_left = ir_visit_ast_node(src1_node);
    if (!visited_left || !visited_left->val) {
        minic_log(LOG_ERROR, "MOD: Failed to visit or get value for left operand.");
        return false;
    }
    appendInstructionsToNode(node, visited_left->blockInsts); // 添加计算 a 的指令
    Value* val_a = visited_left->val;

    // --- 访问右操作数 (b) ---
    ast_node * visited_right = ir_visit_ast_node(src2_node);
    if (!visited_right || !visited_right->val) {
        minic_log(LOG_ERROR, "MOD: Failed to visit or get value for right operand.");
        return false;
    }
    appendInstructionsToNode(node, visited_right->blockInsts); // 添加计算 b 的指令
    Value* val_b = visited_right->val;

    Function* current_func = module->getCurrentFunction();
    if (!current_func) { // 防御性检查
        minic_log(LOG_ERROR, "MOD: currentFunc is null.");
        return false;
    }

    // --- 1. 计算 t_div = a / b ---
    BinaryInstruction* divInst = new BinaryInstruction(
        current_func,
        IRInstOperator::IRINST_OP_DIV_I,
        val_a,
        val_b,
        IntegerType::getTypeInt() // 假设除法结果是 i32
    );
    node->blockInsts.addInst(divInst);
    current_func->addTempVar(divInst); // <--- 重要：将 divInst 注册为临时变量

    // --- 2. 计算 t_mul = t_div * b ---
    // divInst (作为 Value*) 现在是乘法的第一个操作数
    BinaryInstruction* mulInst = new BinaryInstruction(
        current_func,
        IRInstOperator::IRINST_OP_MUL_I,
        divInst,  // 使用上一步的 divInst 作为源操作数
        val_b,
        IntegerType::getTypeInt() // 假设乘法结果是 i32
    );
    node->blockInsts.addInst(mulInst);
    current_func->addTempVar(mulInst); // <--- 重要：将 mulInst 注册为临时变量

    // --- 3. 计算 result_mod = a - t_mul ---
    // mulInst (作为 Value*) 现在是减法的第二个操作数
    BinaryInstruction* modInst = new BinaryInstruction(
        current_func,
        IRInstOperator::IRINST_OP_SUB_I,
        val_a,
        mulInst, // 使用上一步的 mulInst 作为源操作数
        IntegerType::getTypeInt() // 假设减法/取模结果是 i32
    );
    node->blockInsts.addInst(modInst);
    current_func->addTempVar(modInst); // <--- 重要：将最终的 modInst (即 sub 指令) 注册为临时变量

    // 设置当前 AST 节点 (AST_OP_MOD) 的值为最终的取模结果指令
    node->val = modInst;

    return true;
}


/// @brief 整数取负AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
 // IRGenerator.cpp
// IRGenerator.cpp
bool IRGenerator::ir_neg(ast_node * node) {
    if (!node || node->sons.empty()) {
        std::cerr << "[IR_NEG_FAIL] Called with null node or no operand." << std::endl;
        minic_log(LOG_ERROR, "NEG node is null or has no operand.");
        return false;
    }
    ast_node * src1_node = node->sons[0];
    std::cerr << "[IR_NEG_TRACE] Visiting operand for NEG. Operand AST node type: " << static_cast<int>(src1_node->node_type) << std::endl;
    ast_node * src1 = ir_visit_ast_node(src1_node);

    if (!src1) {
        std::cerr << "[IR_NEG_FAIL] ir_visit_ast_node(src1_node) returned nullptr." << std::endl;
        minic_log(LOG_ERROR, "Operand of NEG: visiting child node failed.");
        return false;
    }
    if (!src1->val) {
        std::cerr << "[IR_NEG_FAIL] src1->val is nullptr after visiting child. Child AST node type was: " << static_cast<int>(src1_node->node_type) << std::endl;
        minic_log(LOG_ERROR, "Operand of NEG is null after visiting child (node->val is null).");
        return false;
    }
    appendInstructionsToNode(node, src1->blockInsts);

    Value* operand_val = src1->val;
    Type* operand_type = operand_val->getType();
    if (!operand_type) {
        std::cerr << "[IR_NEG_FAIL] Operand of NEG has a null type. Operand IRName: " << operand_val->getIRName() << std::endl;
        minic_log(LOG_ERROR, "Operand of NEG has a null type.");
        return false;
    }
    std::cerr << "[IR_NEG_TRACE] Operand for NEG has type: " << operand_type->toString() << " (IRName: " << operand_val->getIRName() << ")" << std::endl;

    Type* neg_result_type = nullptr;

    if (operand_type->isIntegerType()) {
        IntegerType* int_operand_type = static_cast<IntegerType*>(operand_type);
        int bitwidth = int_operand_type->getBitWidth();
        std::cerr << "[IR_NEG_TRACE] Operand is IntegerType with bitwidth: " << bitwidth << std::endl;

        if (bitwidth == 32) {
            std::cerr << "[IR_NEG_TRACE] Operand is i32. Setting result type to i32." << std::endl;
            neg_result_type = IntegerType::getTypeInt();
        } else if (bitwidth == 1) {
            std::cerr << "[IR_NEG_TRACE] Operand is i1. Setting result type to i32." << std::endl;
            neg_result_type = IntegerType::getTypeInt();
        } else {
            std::cerr << "[IR_NEG_FAIL] Operand of NEG is an integer type with unsupported bitwidth: " << bitwidth << std::endl;
            minic_log(LOG_ERROR, "Operand of NEG is an integer type with unsupported bitwidth: %d", bitwidth);
            return false;
        }
    } else {
        std::cerr << "[IR_NEG_FAIL] Operand of NEG is not an integer type. Actual type: " << operand_type->toString() << std::endl;
        minic_log(LOG_ERROR, "Operand of NEG is not an integer type. Actual type: %s", operand_type->toString().c_str());
        return false;
    }

    if (!neg_result_type) { // 额外检查，理论上如果上面逻辑正确，这里不应该为null
        std::cerr << "[IR_NEG_FAIL] neg_result_type is unexpectedly null." << std::endl;
        minic_log(LOG_ERROR, "NEG: Failed to determine result type.");
        return false;
    }
    std::cerr << "[IR_NEG_TRACE] Determined neg_result_type: " << neg_result_type->toString() << std::endl;

    UnaryInstruction * negInst = new UnaryInstruction(
        module->getCurrentFunction(),
        IRInstOperator::IRINST_OP_NEG_I,
        operand_val, // 使用 operand_val 而不是 src1->val 以确保一致性
        neg_result_type
    );
    
    node->blockInsts.addInst(negInst);
    module->getCurrentFunction()->addTempVar(negInst);
    node->val = negInst;
    std::cerr << "[IR_NEG_SUCCESS] Successfully processed NEG. node->val set to " << negInst->getIRName() << " (type " << negInst->getType()->toString() << ")" << std::endl;
    return true;
}
/// @brief 赋值AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_assign(ast_node * node)
{
    ast_node * son1_node = node->sons[0];
    ast_node * son2_node = node->sons[1];

    // 赋值节点，自右往左运算

    // 赋值运算符的左侧操作数
    ast_node * left = ir_visit_ast_node(son1_node);
    if (!left) {
        // 某个变量没有定值
        // 这里缺省设置变量不存在则创建，因此这里不会错误
        return false;
    }

    // 赋值运算符的右侧操作数
    ast_node * right = ir_visit_ast_node(son2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    MoveInstruction * movInst = new MoveInstruction(module->getCurrentFunction(), left->val, right->val);

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(movInst);

    // 这里假定赋值的类型是一致的
    node->val = movInst;

    return true;
}

/// @brief return节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_return(ast_node * node)
{
    ast_node * right = nullptr;

    // return语句可能没有没有表达式，也可能有，因此这里必须进行区分判断
    if (!node->sons.empty()) {

        ast_node * son_node = node->sons[0];

        // 返回的表达式的指令保存在right节点中
        right = ir_visit_ast_node(son_node);
        if (!right) {

            // 某个变量没有定值
            return false;
        }
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理
    Function * currentFunc = module->getCurrentFunction();

    // 返回值存在时则移动指令到node中
    if (right) {

        // 创建临时变量保存IR的值，以及线性IR指令
        node->blockInsts.addInst(right->blockInsts);

        // 返回值赋值到函数返回值变量上，然后跳转到函数的尾部
        node->blockInsts.addInst(new MoveInstruction(currentFunc, currentFunc->getReturnValue(), right->val));

        node->val = right->val;
    } else {
        // 没有返回值
        node->val = nullptr;
    }
    Instruction* exit_instruction_base_ptr = currentFunc->getExitLabel();

    if (!exit_instruction_base_ptr) {
        minic_log(LOG_ERROR, "Function @%s has no exit label set!", currentFunc->getName().c_str());
        return false; // 或者其他错误处理
    }

    // 安全地转换为 LabelInstruction*
    LabelInstruction* exit_label_derived_ptr = dynamic_cast<LabelInstruction*>(exit_instruction_base_ptr);

    if (exit_label_derived_ptr) {
        // 跳转到函数的尾部出口指令上
        node->blockInsts.addInst(new GotoInstruction(currentFunc, exit_label_derived_ptr)); // <--- 传递正确的 LabelInstruction*
    } else {
        // 如果 dynamic_cast 失败，说明 getExitLabel() 返回的不是一个 LabelInstruction
        // 这通常意味着在 ir_function_define 中设置出口标签时逻辑有误
        minic_log(LOG_ERROR, "Function @%s exit label is not a LabelInstruction (type mismatch)!", currentFunc->getName().c_str());
        return false; // 或者其他错误处理方式
    }

    return true;
}

/// @brief 类型叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_type(ast_node * node)
{
    // 不需要做什么，直接从节点中获取即可。

    return true;
}

/// @brief 标识符叶子节点翻译成线性中间IR，变量声明的不走这个语句
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_var_id(ast_node * node)
{
	
    Value * val;

    // 查找ID型Value
    // 变量，则需要在符号表中查找对应的值

    val = module->findVarValue(node->name);

    node->val = val;

    return true;
}

/// @brief 无符号整数字面量叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_uint(ast_node * node)
{
	
    ConstInt * val;

    // 新建一个整数常量Value
    val = module->newConstInt((int32_t) node->integer_val);

    node->val = val;

    return true;
}

/// @brief 变量声明语句节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_declare_statment(ast_node * node)
{
    bool result = false;

    for (auto & child: node->sons) {

        // 遍历每个变量声明
        result = ir_variable_declare(child);
        if (!result) {
            break;
        }
    }

    return result;
}

/// @brief 变量定声明节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_variable_declare(ast_node * node)
{
    // 共有两个孩子，第一个类型，第二个变量名

    // TODO 这里可强化类型等检查

    node->val = module->newVarValue(node->sons[0]->type, node->sons[1]->name);

    return true;
}
// IRGenerator.cpp

// 辅助函数：创建新的唯一标签
LabelInstruction* IRGenerator::newLabel() {
    std::string label_ir_name = ".L" + std::to_string(label_counter_++); // 例如 ".L0"
    return new LabelInstruction(getCurrentFunction(), label_ir_name);
}

// 辅助函数：获取当前函数
Function* IRGenerator::getCurrentFunction() {
    return module->getCurrentFunction();
}

/* // 如果需要，可以取消注释并实现这些辅助函数
// 辅助函数：将指令添加到当前函数
void IRGenerator::addInstructionToCurrentFunction(Instruction* inst) {
    if (getCurrentFunction() && inst) {
        getCurrentFunction()->getInterCode().addInst(inst);
    }
}
*/

// 辅助函数：将指令添加到指定AST节点的指令列表 (blockInsts)
void IRGenerator::addInstructionToNode(ast_node* node, Instruction* inst) {
    if (node && inst) {
        node->blockInsts.addInst(inst);
    }
}

// 辅助函数：将一个指令列表 (InterCode) 追加到指定AST节点的指令列表
void IRGenerator::appendInstructionsToNode(ast_node* node, InterCode& instructions) {
    if (node) {
        node->blockInsts.addInst(instructions); // 假设 InterCode 有 addInst(InterCode&) 或类似方法
    }
}


// 用于条件上下文的访问器
// 本函数假设当 'node' 的结果被用来控制一个分支时被调用。
// 它将生成代码：如果 'node' 为真，则跳转到 true_label；如果为假，则跳转到 false_label。
bool IRGenerator::ir_visit_conditional_node(ast_node* node, LabelInstruction* true_label, LabelInstruction* false_label) {
    if (!node || !true_label || !false_label) return false;

    node->val = nullptr;

    switch (node->node_type) {
        case ast_operator_type::AST_OP_LT:
        case ast_operator_type::AST_OP_LE:
        case ast_operator_type::AST_OP_GT:
        case ast_operator_type::AST_OP_GE:
        case ast_operator_type::AST_OP_EQ:
        case ast_operator_type::AST_OP_NE:
            return ir_relational_op(node, true_label, false_label);

        case ast_operator_type::AST_OP_LAND:
            return ir_logical_and(node, true_label, false_label);
        case ast_operator_type::AST_OP_LOR:
            return ir_logical_or(node, true_label, false_label);
        case ast_operator_type::AST_OP_LNOT:
            return ir_logical_not(node, true_label, false_label);

        default: {
            std::cerr << "[COND_NODE_DEFAULT_TRACE] Entered default case for AST node type: "
                      << static_cast<int>(node->node_type)
                      << " (processing as a value-condition)" << std::endl;

            // 步骤 1: 访问节点以获取其计算值和产生的指令
            ast_node* visited_node = ir_visit_ast_node(node);

            // 检查访问是否成功，以及是否产生了值
            if (!visited_node) {
                std::cerr << "[COND_NODE_DEFAULT_FAIL] ir_visit_ast_node(node) returned nullptr for node type: "
                          << static_cast<int>(node->node_type) << std::endl;
                minic_log(LOG_ERROR, "Default conditional: visited_node is null after ir_visit_ast_node.");
                return false;
            }
            if (!visited_node->val) {
                std::cerr << "[COND_NODE_DEFAULT_FAIL] visited_node->val is nullptr. AST Node type was: "
                          << static_cast<int>(node->node_type) << std::endl;
                minic_log(LOG_ERROR, "Default conditional: visited_node->val is null after ir_visit_ast_node.");
                return false;
            }

            // 步骤 2: 将子表达式（visited_node）产生的指令追加到当前条件节点（node）的指令列表中
            // 这些指令是计算 cond_val (即 visited_node->val) 所必需的
            std::cerr << "[COND_NODE_DEFAULT_TRACE] Appending instructions from visited_node (type "
                      << static_cast<int>(visited_node->node_type) << ", val "
                      << (visited_node->val ? visited_node->val->getIRName() : "null_val")
                      << ") into current conditional node (type " << static_cast<int>(node->node_type) << ")" << std::endl;

            // --- 调试打印 visited_node->blockInsts 的内容 ---
            std::cerr << "    --- Instructions in visited_node->blockInsts (BEFORE append): ---" << std::endl;
            if (visited_node->blockInsts.getInsts().empty()) { // 假设 getInsts() 返回 const std::vector<Instruction*>&
                 std::cerr << "        (empty)" << std::endl;
            } else {
                for (const auto& inst_in_visited : visited_node->blockInsts.getInsts()) {
                    if (inst_in_visited) {
                        std::cerr << "        " << inst_in_visited->toString() << std::endl;
                    } else {
                        std::cerr << "        (null instruction in visited_node->blockInsts)" << std::endl;
                    }
                }
            }
            std::cerr << "    --- End of instructions in visited_node->blockInsts ---" << std::endl;
            // --- 调试打印结束 ---

            

            // --- 调试打印追加后的 node->blockInsts 的内容 ---
            std::cerr << "    --- Instructions in current node->blockInsts (AFTER append): ---" << std::endl;
             if (node->blockInsts.getInsts().empty()) {
                 std::cerr << "        (empty)" << std::endl;
            } else {
                for (const auto& inst_in_current : node->blockInsts.getInsts()) {
                    if (inst_in_current) {
                        std::cerr << "        " << inst_in_current->toString() << std::endl;
                    } else {
                        std::cerr << "        (null instruction in current node->blockInsts after append)" << std::endl;
                    }
                }
            }
            std::cerr << "    --- End of instructions in current node->blockInsts ---" << std::endl;
            // --- 调试打印结束 ---


            // 步骤 3: 获取计算出的条件值及其类型
            Value* cond_val = visited_node->val; // cond_val 现在是表达式 node 的计算结果 (例如 -!!!a 的结果)
            Type* cond_type = cond_val->getType();

            if (!cond_type) {
                std::cerr << "[COND_NODE_DEFAULT_FAIL] cond_val->getType() is nullptr. cond_val IRName: "
                          << cond_val->getIRName() << std::endl;
                minic_log(LOG_ERROR, "Default conditional: cond_val has null type.");
                return false;
            }
            std::cerr << "[COND_NODE_DEFAULT_TRACE] cond_val (result of visited_node) type: "
                      << cond_type->toString() << " (IRName: " << cond_val->getIRName() << ")" << std::endl;
            std::cerr << "[COND_NODE_DEFAULT_TRACE] Checking cond_type->isInt32Type(): "
                      << (cond_type->isInt32Type() ? "true" : "false") << std::endl;
            std::cerr << "[COND_NODE_DEFAULT_TRACE] Checking cond_type->isInt1Byte(): "
                      << (cond_type->isInt1Byte() ? "true" : "false") << std::endl;


            // 步骤 4: 根据条件值的类型生成相应的比较和分支指令
            if (cond_type->isInt32Type()) {
                std::cerr << "[COND_NODE_DEFAULT_TRACE] cond_type is i32. Creating Cmp NE with zero." << std::endl;
                Value* zero_i32 = module->newConstInt(0); // 默认 i32 类型
                Value* temp_i1_result = module->newTemporary(IntegerType::getTypeBool(), "cond_bool_res"); // 存储比较结果的 i1 临时变量

                if (!zero_i32 || !temp_i1_result) {
                     std::cerr << "[COND_NODE_DEFAULT_FAIL] Failed to create zero_i32 or temp_i1_result." << std::endl;
                     return false;
                }
                std::cerr << "[COND_NODE_DEFAULT_TRACE] zero_i32 IRName: " << zero_i32->getIRName() << std::endl;
                std::cerr << "[COND_NODE_DEFAULT_TRACE] temp_i1_result IRName: " << temp_i1_result->getIRName()
                          << ", Type: " << temp_i1_result->getType()->toString() << std::endl;


                addInstructionToNode(node, new CmpInstruction(temp_i1_result, CmpInstruction::NE, cond_val, zero_i32, getCurrentFunction()));
                addInstructionToNode(node, new BranchConditionalInstruction(temp_i1_result, true_label, false_label, getCurrentFunction()));
                
                std::cerr << "[COND_NODE_DEFAULT_SUCCESS] Processed i32 condition. Generated: "
                          << temp_i1_result->getIRName() << " = icmp ne " << cond_val->getIRName() << ", " << zero_i32->getIRName()
                          << "; then bc " << temp_i1_result->getIRName() << ", ..." << std::endl;
                return true;

            } else if (cond_type->isInt1Byte()) {
                std::cerr << "[COND_NODE_DEFAULT_TRACE] cond_type is i1. Creating direct BranchConditional." << std::endl;
                addInstructionToNode(node, new BranchConditionalInstruction(cond_val, true_label, false_label, getCurrentFunction()));
                std::cerr << "[COND_NODE_DEFAULT_SUCCESS] Processed i1 condition. Generated: bc " << cond_val->getIRName() << ", ..." << std::endl;
                return true;

            } else {
                std::cerr << "[COND_NODE_DEFAULT_FAIL] Expression type for condition is not i32 or i1. Actual type: "
                          << cond_type->toString() << " (IRName: " << cond_val->getIRName() << ")" << std::endl;
                minic_log(LOG_ERROR, "Default conditional: expression type is not i32 or i1. Actual: %s", cond_type->toString().c_str());
                return false;
            }
        } // end default case
    }
}

// IRGenerator.cpp

// 处理关系运算符
bool IRGenerator::ir_relational_op(ast_node * node, LabelInstruction* true_label, LabelInstruction* false_label) {
    node->blockInsts.clear(); // 每个处理器开始时清空当前节点的指令列表 (或者不由它收集，而是返回 InterCode)
    ast_node* left_child = node->sons[0];
    ast_node* right_child = node->sons[1];

	if (!ir_visit_ast_node(left_child) || !left_child->val) return false;
	if (!ir_visit_ast_node(right_child) || !right_child->val) return false;

	appendInstructionsToNode(node, left_child->blockInsts);
	appendInstructionsToNode(node, right_child->blockInsts);

	Value* lhs_val = left_child->val;
	Value* rhs_val = right_child->val;

	// Value* cmp_result_temp = module->newTemporary(IRType::getI1Type()); // 使用你的类型系统
	Value* cmp_result_temp = module->newTemporary(IntegerType::get(1)); // 获取 i1 类型

	CmpInstruction::CmpOp cmp_op_enum; // 使用新指令定义的枚举
	switch (node->node_type) {
		case ast_operator_type::AST_OP_LT: cmp_op_enum = CmpInstruction::LT; break;
		case ast_operator_type::AST_OP_LE: cmp_op_enum = CmpInstruction::LE; break;
		case ast_operator_type::AST_OP_GT: cmp_op_enum = CmpInstruction::GT; break;
		case ast_operator_type::AST_OP_GE: cmp_op_enum = CmpInstruction::GE; break;
		case ast_operator_type::AST_OP_EQ: cmp_op_enum = CmpInstruction::EQ; break;
		case ast_operator_type::AST_OP_NE: cmp_op_enum = CmpInstruction::NE; break;
		default: return false;
	}

	addInstructionToNode(node, new CmpInstruction(cmp_result_temp, cmp_op_enum, lhs_val, rhs_val, getCurrentFunction()));
	addInstructionToNode(node, new BranchConditionalInstruction(cmp_result_temp, true_label, false_label, getCurrentFunction()));
	return true;
}
// 处理逻辑与 &&
bool IRGenerator::ir_logical_and(ast_node * node, LabelInstruction* true_label, LabelInstruction* false_label) {
    node->blockInsts.clear();
    ast_node* expr1 = node->sons[0];
    ast_node* expr2 = node->sons[1];
    LabelInstruction* eval_expr2_label = newLabel();

    if (!ir_visit_conditional_node(expr1, eval_expr2_label, false_label)) return false;
    appendInstructionsToNode(node, expr1->blockInsts);
    addInstructionToNode(node, eval_expr2_label);
    if (!ir_visit_conditional_node(expr2, true_label, false_label)) return false;
    appendInstructionsToNode(node, expr2->blockInsts);
    return true;
}

// 处理逻辑或 ||
bool IRGenerator::ir_logical_or(ast_node * node, LabelInstruction* true_label, LabelInstruction* false_label) {
    node->blockInsts.clear();
    ast_node* expr1 = node->sons[0];
    ast_node* expr2 = node->sons[1];
    LabelInstruction* eval_expr2_label = newLabel();

    if (!ir_visit_conditional_node(expr1, true_label, eval_expr2_label)) return false;
    appendInstructionsToNode(node, expr1->blockInsts);
    addInstructionToNode(node, eval_expr2_label);
    if (!ir_visit_conditional_node(expr2, true_label, false_label)) return false;
    appendInstructionsToNode(node, expr2->blockInsts);
    return true;
}

// 处理逻辑非 !
bool IRGenerator::ir_logical_not(ast_node * node, LabelInstruction* true_label, LabelInstruction* false_label) {
    node->blockInsts.clear();
    ast_node* expr = node->sons[0];
    if (!ir_visit_conditional_node(expr, true_label, false_label)) return false;
    appendInstructionsToNode(node, expr->blockInsts);
    return true;
}

// IRGenerator.cpp
bool IRGenerator::ir_lnot_expression(ast_node * node) {
    if (!node) {
        std::cerr << "[LNOT_EXPR_FAIL] Called with null node." << std::endl;
        minic_log(LOG_ERROR, "LNOT expression: node is null.");
        return false;
    }
    if (node->sons.empty()) {
        std::cerr << "[LNOT_EXPR_FAIL] Node (type " << static_cast<int>(node->node_type) << ") has no operand for LNOT." << std::endl;
        minic_log(LOG_ERROR, "LNOT expression: node has no operand.");
        return false;
    }

    ast_node* operand_node_ptr = node->sons[0];
    if (!operand_node_ptr) {
        std::cerr << "[LNOT_EXPR_FAIL] Operand node pointer (node->sons[0]) is null." << std::endl;
        minic_log(LOG_ERROR, "LNOT expression: operand node is null.");
        return false;
    }

    std::cerr << "[LNOT_EXPR_TRACE] Visiting operand for LNOT. Operand AST node type: " << static_cast<int>(operand_node_ptr->node_type) << std::endl;
    ast_node* visited_operand_node = ir_visit_ast_node(operand_node_ptr);

    if (!visited_operand_node) {
        std::cerr << "[LNOT_EXPR_FAIL] ir_visit_ast_node(operand_node) returned nullptr." << std::endl;
        minic_log(LOG_ERROR, "LNOT expression: Failed to visit operand node.");
        return false;
    }
    if (!visited_operand_node->val) {
        std::cerr << "[LNOT_EXPR_FAIL] visited_operand_node->val is nullptr after visit. Operand AST node type was: " << static_cast<int>(operand_node_ptr->node_type) << std::endl;
        minic_log(LOG_ERROR, "LNOT expression: Failed to get value for LNOT operand.");
        return false;
    }
    
    // 将子节点产生的指令追加到当前 LNOT 节点的指令列表中
    // 只有在 operand_node_ptr 和 visited_operand_node 是同一个对象时，blockInsts 才有意义
    // 通常 ir_visit_ast_node 返回的是其参数 node (除非出错返回 nullptr)
    // 所以 operand_node_ptr->blockInsts 就是 visited_operand_node->blockInsts
    appendInstructionsToNode(node, operand_node_ptr->blockInsts);

    Value* operand_val = visited_operand_node->val; // 使用访问后的节点的 val
    Type* operand_type = operand_val->getType();

    if (!operand_type) {
        std::cerr << "[LNOT_EXPR_FAIL] Operand of LNOT has a null type. Operand IRName: " << operand_val->getIRName() << std::endl;
        minic_log(LOG_ERROR, "LNOT expression: Operand has a null type.");
        return false;
    }
    std::cerr << "[LNOT_EXPR_TRACE] Operand for LNOT has type: " << operand_type->toString() << " (IRName: " << operand_val->getIRName() << ")" << std::endl;

    // 逻辑非的结果总是 i1 (布尔) 类型
    Value* result_i1_val = module->newTemporary(IntegerType::getTypeBool(), "lnot_res");
    if (!result_i1_val || !result_i1_val->getType() || !result_i1_val->getType()->isInt1Byte()) {
         std::cerr << "[LNOT_EXPR_FATAL] Failed to create a valid i1 temporary for LNOT result." << std::endl;
         minic_log(LOG_ERROR, "LNOT expression: Failed to create i1 temporary for result.");
         return false; // 如果无法创建有效的i1临时变量
    }
    std::cerr << "[LNOT_EXPR_TRACE] Created temporary for LNOT result: " << result_i1_val->getIRName() << " type: " << result_i1_val->getType()->toString() << std::endl;


    if (operand_type->isIntegerType()) {
        IntegerType* int_operand_type = static_cast<IntegerType*>(operand_type);
        int bitwidth = int_operand_type->getBitWidth();
        std::cerr << "[LNOT_EXPR_TRACE] Operand is IntegerType with bitwidth: " << bitwidth << std::endl;

        if (bitwidth == 32) {
            std::cerr << "[LNOT_EXPR_TRACE] Operand is i32. Comparing with i32 zero." << std::endl;
            Value* zero_i32 = module->newConstInt(0); // 默认创建 i32 类型的0
            if(!zero_i32 || !zero_i32->getType()){ std::cerr<<"[LNOT_EXPR_FATAL] zero_i32 or its type is null"<<std::endl; return false;}
            std::cerr << "[LNOT_EXPR_TRACE] zero_i32 type: " << zero_i32->getType()->toString() << std::endl;
            addInstructionToNode(node, new CmpInstruction(result_i1_val, CmpInstruction::EQ, operand_val, zero_i32, getCurrentFunction()));
        } else if (bitwidth == 1) {
            std::cerr << "[LNOT_EXPR_TRACE] Operand is i1. Comparing with i1 zero." << std::endl;
            Value* zero_i1 = module->newConstInt(0, IntegerType::getTypeBool()); // 明确创建 i1 类型的0
            if(!zero_i1 || !zero_i1->getType()){ std::cerr<<"[LNOT_EXPR_FATAL] zero_i1 or its type is null"<<std::endl; return false;}
            std::cerr << "[LNOT_EXPR_TRACE] zero_i1 type: " << zero_i1->getType()->toString() << std::endl;
            addInstructionToNode(node, new CmpInstruction(result_i1_val, CmpInstruction::EQ, operand_val, zero_i1, getCurrentFunction()));
        } else {
            std::cerr << "[LNOT_EXPR_FAIL] Operand of LNOT is an integer type with unsupported bitwidth: " << bitwidth << std::endl;
            minic_log(LOG_ERROR, "LNOT expression: Operand is an integer with unsupported bitwidth: %d", bitwidth);
            return false;
        }
    } else {
        std::cerr << "[LNOT_EXPR_FAIL] Operand of LNOT is not an integer type. Actual type: " << operand_type->toString() << std::endl;
        minic_log(LOG_ERROR, "LNOT expression: Operand is not an integer type. Actual type: %s", operand_type->toString().c_str());
        return false;
    }

    node->val = result_i1_val;
    std::cerr << "[LNOT_EXPR_SUCCESS] Successfully processed LNOT. node->val set to " << node->val->getIRName() << " (type " << node->val->getType()->toString() << ")" << std::endl;
    return true;
}

// 处理 if 语句
// ir/Generator/IRGenerator.cpp
/*bool IRGenerator::ir_if_statement(ast_node * node) {
    node->blockInsts.clear();

    ast_node * cond_expr_node = node->sons[0];
    ast_node * then_block_node = node->sons[1];
    ast_node * else_block_node = (node->sons.size() > 2) ? node->sons[2] : nullptr;

    Function* current_func = getCurrentFunction();

    if (!else_block_node) {
        LabelInstruction* then_entry_label = newLabel();
        LabelInstruction* after_if_label = newLabel();

        if (!ir_visit_conditional_node(cond_expr_node, then_entry_label, after_if_label)) return false;
        appendInstructionsToNode(node, cond_expr_node->blockInsts);

        addInstructionToNode(node, then_entry_label);
        if (!ir_visit_ast_node(then_block_node)) return false;
        appendInstructionsToNode(node, then_block_node->blockInsts);

        // 只在不是终结指令时跳转
        if (!then_block_node->blockInsts.empty()) {
            auto last = then_block_node->blockInsts.getLastInst();
            if (!last || !last->isTerminator()) {
                addInstructionToNode(node, new GotoInstruction(current_func, after_if_label));
            }
        }

        addInstructionToNode(node, after_if_label);
    } else {
        LabelInstruction* then_entry_label = newLabel();
        LabelInstruction* else_entry_label = newLabel();
        LabelInstruction* end_if_label = newLabel();

        if (!ir_visit_conditional_node(cond_expr_node, then_entry_label, else_entry_label)) return false;
        appendInstructionsToNode(node, cond_expr_node->blockInsts);

        addInstructionToNode(node, then_entry_label);
        if (!ir_visit_ast_node(then_block_node)) return false;
        appendInstructionsToNode(node, then_block_node->blockInsts);

        if (!then_block_node->blockInsts.empty()) {
            auto last = then_block_node->blockInsts.getLastInst();
            if (!last || !last->isTerminator()) {
                addInstructionToNode(node, new GotoInstruction(current_func, end_if_label));
            }
        }

        addInstructionToNode(node, else_entry_label);
        if (!ir_visit_ast_node(else_block_node)) return false;
        appendInstructionsToNode(node, else_block_node->blockInsts);

        if (!else_block_node->blockInsts.empty()) {
            auto last = else_block_node->blockInsts.getLastInst();
            if (!last || !last->isTerminator()) {
                addInstructionToNode(node, new GotoInstruction(current_func, end_if_label));
            }
        }

        addInstructionToNode(node, end_if_label);
    }

    node->val = nullptr;
    return true;
}
*/

// IRGenerator.cpp

bool IRGenerator::ir_if_statement(ast_node * node) {
    node->blockInsts.clear();

    ast_node * cond_expr_node = node->sons[0];
    ast_node * then_block_node = node->sons[1]; // 现在会用到这个
    ast_node * else_block_node = (node->sons.size() > 2) ? node->sons[2] : nullptr; // 处理可选的 else

    Function* current_func = getCurrentFunction();
    if (!current_func) return false;

    // 1. 获取函数已定义的出口标签 (这将是我们的 .L2 或最终的合并点)
    Instruction* exit_label_inst_base = current_func->getExitLabel();
    if (!exit_label_inst_base) {
        minic_log(LOG_ERROR, "Function @%s has no exit label set!", current_func->getName().c_str());
        return false;
    }
    LabelInstruction* merge_label_for_if = dynamic_cast<LabelInstruction*>(exit_label_inst_base);
    if (!merge_label_for_if && else_block_node) { // 如果有else, 我们需要一个if自己的合并点
        merge_label_for_if = newLabel(); // If has else, create a specific merge label for the if construct
    } else if (!merge_label_for_if) { // No else, merge directly to function exit
         minic_log(LOG_ERROR, "Function @%s exit label is not a LabelInstruction!", current_func->getName().c_str());
        return false;
    }
    // 对于只有if没有else的情况，如果then块不是以return结束，它应该跳转到if语句后的代码。
    // 如果有else，则then和else块都会跳转到if语句自己的merge_label_for_if。

    // 2. 为 if 结构创建新的标签
    LabelInstruction* then_entry_label = newLabel();      // .L3 (or next available)
    LabelInstruction* else_entry_label = newLabel();      // .L4 (or next after then_label, only if else_block_node exists)
                                                        // else_entry_label will be the target for 'bc' if false
                                                        // or the label after the then-block if no explicit else.
    LabelInstruction* actual_merge_target;

    if (else_block_node) {
        actual_merge_target = newLabel(); // This will be the label after both then and else blocks.
                                          // Corresponds to .L2 in the standard IR if L2 is the specific merge for if-else.
                                          // Let's rename merge_label_for_if to actual_merge_target for clarity if else exists.
    } else {
        actual_merge_target = else_entry_label; // If no else, false branch of 'bc' jumps directly after then-block.
                                                // And then-block, if not terminated, also jumps here.
    }


    // 3. 处理条件表达式
    // ir_visit_conditional_node 会生成 icmp 和 bc。
    // 如果条件为 true, 跳转到 then_entry_label.
    // 如果条件为 false, 跳转到 (else_block_node ? else_entry_label : actual_merge_target)
    if (!ir_visit_conditional_node(cond_expr_node, then_entry_label, (else_block_node ? else_entry_label : actual_merge_target) )) {
        return false;
    }
    appendInstructionsToNode(node, cond_expr_node->blockInsts);

    // 4. "then" 分支
    addInstructionToNode(node, then_entry_label);
    if (!ir_visit_ast_node(then_block_node)) { // 递归访问 then_block_node
        return false;
    }
    appendInstructionsToNode(node, then_block_node->blockInsts);

    // 如果 then_block 不是以终结指令结束，则跳转到 if 语句的合并点
    bool then_is_terminated = false;
    if (!then_block_node->blockInsts.empty()) {
        Instruction* last_then_inst = then_block_node->blockInsts.getLastInst();
        if (last_then_inst && last_then_inst->isTerminator()) {
            then_is_terminated = true;
        }
    }
    if (!then_is_terminated) {
        addInstructionToNode(node, new GotoInstruction(current_func, actual_merge_target));
    }

    // 5. "else" 分支 (如果存在)
    if (else_block_node) {
        addInstructionToNode(node, else_entry_label); // else 代码块的入口
        if (!ir_visit_ast_node(else_block_node)) { // 递归访问 else_block_node
            return false;
        }
        appendInstructionsToNode(node, else_block_node->blockInsts);

        // 如果 else_block 不是以终结指令结束，则跳转到 if 语句的合并点
        bool else_is_terminated = false;
        if (!else_block_node->blockInsts.empty()) {
            Instruction* last_else_inst = else_block_node->blockInsts.getLastInst();
            if (last_else_inst && last_else_inst->isTerminator()) {
                else_is_terminated = true;
            }
        }
        if (!else_is_terminated) {
            addInstructionToNode(node, new GotoInstruction(current_func, actual_merge_target));
        }
    }

    // 6. 添加 if 语句的合并点标签 (actual_merge_target)
    // 这个标签是 then (和 else, 如果存在) 分支执行完毕后的汇合点。
    addInstructionToNode(node, actual_merge_target);

    node->val = nullptr;
    return true;
}
// 处理 while 语句
bool IRGenerator::ir_while_statement(ast_node * node) {
    node->blockInsts.clear();
    ast_node* cond_expr_node = node->sons[0];
    ast_node* body_block_node = node->sons[1];

    LabelInstruction* loop_condition_label = newLabel(); // L1
    LabelInstruction* loop_body_label = newLabel();      // L2
    LabelInstruction* loop_exit_label = newLabel();      // L3

    continue_target_stack_.push_back(loop_condition_label);
    break_target_stack_.push_back(loop_exit_label);

    addInstructionToNode(node, new GotoInstruction(getCurrentFunction(), loop_condition_label)); // unconditional jump to L1
    addInstructionToNode(node, loop_condition_label); // L1:
    if (!ir_visit_conditional_node(cond_expr_node, loop_body_label, loop_exit_label)) { // if cond goto L2; else goto L3
        break_target_stack_.pop_back(); continue_target_stack_.pop_back(); return false;
    }
    appendInstructionsToNode(node, cond_expr_node->blockInsts);

    addInstructionToNode(node, loop_body_label); // L2:
    if (!ir_visit_ast_node(body_block_node)) {
        break_target_stack_.pop_back(); continue_target_stack_.pop_back(); return false;
    }
    appendInstructionsToNode(node, body_block_node->blockInsts);
    addInstructionToNode(node, new GotoInstruction(getCurrentFunction(), loop_condition_label)); // goto L1

    addInstructionToNode(node, loop_exit_label); // L3:

    break_target_stack_.pop_back();
    continue_target_stack_.pop_back();
    node->val = nullptr;
    return true;
}

// 处理 break 语句
bool IRGenerator::ir_break_statement(ast_node * node) {
    node->blockInsts.clear();
    if (break_target_stack_.empty()) {
        minic_log(LOG_ERROR, "Line %lld: 'break' 语句不在循环体内部。", (long long)node->line_no);
        return false;
    }
    addInstructionToNode(node, new GotoInstruction(getCurrentFunction(), break_target_stack_.back()));
    node->val = nullptr;
    return true;
}

// 处理 continue 语句
bool IRGenerator::ir_continue_statement(ast_node * node) {
    node->blockInsts.clear();
    if (continue_target_stack_.empty()) {
        minic_log(LOG_ERROR, "Line %lld: 'continue' 语句不在循环体内部。", (long long)node->line_no);
        return false;
    }
    addInstructionToNode(node, new GotoInstruction(getCurrentFunction(), continue_target_stack_.back()));
    node->val = nullptr;
    return true;
}