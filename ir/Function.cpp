///
/// @file Function.cpp
/// @brief 函数实现
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

#include <cstdlib>
#include <string>

#include "IRConstant.h"
#include "Function.h"

/// @brief 指定函数名字、函数类型的构造函数
/// @param _name 函数名称
/// @param _type 函数类型
/// @param _builtin 是否是内置函数
Function::Function(std::string _name, FunctionType * _type, bool _builtin)
    : GlobalValue(_type, _name), builtIn(_builtin)
{
    returnType = _type->getReturnType();

    // 设置对齐大小
    setAlignment(1);
}

///
/// @brief 析构函数
/// @brief 释放函数占用的内存和IR指令代码
/// @brief 注意：IR指令代码并未释放，需要手动释放
Function::~Function()
{
    Delete();
}

/// @brief 获取函数返回类型
/// @return 返回类型
Type * Function::getReturnType() const
{
    return returnType;
}

/// @brief 获取函数的形参列表
/// @return 形参列表
std::vector<FormalParam *> & Function::getParams()
{
    return params;
}

/// @brief 获取函数内的IR指令代码
/// @return IR指令代码
InterCode & Function::getInterCode()
{
    return code;
}

/// @brief 判断该函数是否是内置函数
/// @return true: 内置函数，false：用户自定义
bool Function::isBuiltin()
{
    return builtIn;
}

/// @brief 函数指令信息输出
/// @param str 函数指令
// ir/Function.cpp
#include "Function.h"
#include "Type.h"        // For Type::toString()
#include "Value.h"       // For Value::getIRName(), Value::getType(), etc.
#include "Instruction.h" // For Instruction::getOp(), Instruction::toString()
#include "FormalParam.h" // For FormalParam
#include "LocalVariable.h"// For LocalVariable
#include <set>           // For std::set to handle unique temporary declarations

// ... (其他 Function 成员函数保持不变) ...

// 修改 toString 的定义
std::string Function::toString() const { // <--- 添加 const，修改返回类型，移除参数
    std::string func_ir_str; // 用于构建结果

    if (builtIn) {
        // 对于内置函数，通常只输出一个声明（如果需要的话），而不是完整的定义
        // 或者根据你的 IR 规范，可能什么都不输出，或者输出 "declare ..."
        // 例如: func_ir_str = "declare " + getReturnType()->toString() + " @" + getName() + "(";
        // (参数列表也需要正确格式化)
        // func_ir_str += ");\n";
        return func_ir_str; // 暂时返回空字符串，你需要确定内置函数的正确 IR 表示
    }

    // 输出函数头 "define <return_type> @<func_name>(<param_list>)"
    // 确保 getReturnType(), getName(), param->getType(), param->getIRName() 都是 const 方法
    func_ir_str = "define " + getReturnType()->toString() + " @" + getName() + "(";

    bool firstParam = true;
    for (const auto & param : params) { // 使用 const auto&
        if (!firstParam) {
            func_ir_str += ", ";
        }
        if (param && param->getType()) { // 添加空指针检查
            func_ir_str += param->getType()->toString() + " " + param->getIRName();
        } else {
            func_ir_str += "<invalid_param>";
        }
        firstParam = false;
    }
    func_ir_str += "){\n"; // 添加函数体开始的 '{'

    // --- 局部变量声明 ---
    // 确保 var->getType()->toString(), var->getIRName(), var->getName(), var->getScopeLevel() 是 const
    if (!varsVector.empty()) {
        for (const auto & var : varsVector) { // 使用 const auto&
            if (var && var->getType()) { // 添加空指针检查
                func_ir_str += "\tdeclare " + var->getType()->toString() + " " + var->getIRName();
                std::string realName = var->getName();
                if (!realName.empty()) {
                    func_ir_str += " ; " + std::to_string(var->getScopeLevel()) + ":" + realName;
                }
                func_ir_str += "\n";
            }
        }
        
    }

    if (!this->tempVars.empty()) { // 检查 tempVars 是否为空
        for (const auto& temp_val_ptr : this->tempVars) { // 遍历 tempVars 向量
            if (temp_val_ptr && temp_val_ptr->getType() && !temp_val_ptr->getIRName().empty()) { // 确保 Value 有效、有类型、有IR名
                // 调试打印 (可选):
                // std::cout << "toString declaring temp: " << temp_val_ptr->getIRName()
                //           << " of type: " << temp_val_ptr->getType()->toString() << std::endl;

                func_ir_str += "\tdeclare " + temp_val_ptr->getType()->toString() + " " + temp_val_ptr->getIRName() + "\n";
            }
        }
        func_ir_str += "\n"; // 在所有临时变量声明之后加一个换行
    }

    // --- 指令列表 ---
    // 确保 inst->toString() 和 inst->getOp() 是 const
    for (const auto & inst : code.getInsts()) { // 使用 const auto&
        if (inst) { // 添加空指针检查
            std::string current_inst_str = inst->toString();
            if (!current_inst_str.empty()) {
                if (inst->getOp() == IRInstOperator::IRINST_OP_LABEL) {
                    func_ir_str += current_inst_str + "\n";
                } else {
                    func_ir_str += "\t" + current_inst_str + "\n";
                }
            }
        }
    }

    // 输出函数尾部
    func_ir_str += "}\n\n"; // 添加函数体结束的 '}' 和一个空行
    return func_ir_str;    // 返回构建好的字符串
}

/// @brief 设置函数出口指令
/// @param inst 出口Label指令
void Function::setExitLabel(Instruction * inst)
{
    exitLabel = inst;
}

/// @brief 获取函数出口指令
/// @return 出口Label指令
Instruction * Function::getExitLabel()
{
    return exitLabel;
}

/// @brief 设置函数返回值变量
/// @param val 返回值变量，要求必须是局部变量，不能是临时变量
void Function::setReturnValue(LocalVariable * val)
{
    returnValue = val;
}

/// @brief 获取函数返回值变量
/// @return 返回值变量
LocalVariable * Function::getReturnValue()
{
    return returnValue;
}

/// @brief 获取最大栈帧深度
/// @return 栈帧深度
int Function::getMaxDep()
{
    return maxDepth;
}

/// @brief 设置最大栈帧深度
/// @param dep 栈帧深度
void Function::setMaxDep(int dep)
{
    maxDepth = dep;

    // 设置函数栈帧被重定位标记，用于生成不同的栈帧保护代码
    relocated = true;
}

/// @brief 获取本函数需要保护的寄存器
/// @return 要保护的寄存器
std::vector<int32_t> & Function::getProtectedReg()
{
    return protectedRegs;
}

/// @brief 获取本函数需要保护的寄存器字符串
/// @return 要保护的寄存器
std::string & Function::getProtectedRegStr()
{
    return protectedRegStr;
}

/// @brief 获取函数调用参数个数的最大值
/// @return 函数调用参数个数的最大值
int Function::getMaxFuncCallArgCnt()
{
    return maxFuncCallArgCnt;
}

/// @brief 设置函数调用参数个数的最大值
/// @param count 函数调用参数个数的最大值
void Function::setMaxFuncCallArgCnt(int count)
{
    maxFuncCallArgCnt = count;
}

/// @brief 函数内是否存在函数调用
/// @return 是否存在函调用
bool Function::getExistFuncCall()
{
    return funcCallExist;
}

/// @brief 设置函数是否存在函数调用
/// @param exist true: 存在 false: 不存在
void Function::setExistFuncCall(bool exist)
{
    funcCallExist = exist;
}

/// @brief 新建变量型Value。先检查是否存在，不存在则创建，否则失败
/// @param name 变量ID
/// @param type 变量类型
/// @param scope_level 局部变量的作用域层级
LocalVariable * Function::newLocalVarValue(Type * type, std::string name, int32_t scope_level)
{
    // 创建变量并加入符号表
    LocalVariable * varValue = new LocalVariable(type, name, scope_level);

    // varsVector表中可能存在变量重名的信息
    varsVector.push_back(varValue);

    return varValue;
}

/// @brief 新建一个内存型的Value，并加入到符号表，用于后续释放空间
/// \param type 变量类型
/// \return 临时变量Value
MemVariable * Function::newMemVariable(Type * type)
{
    // 肯定唯一存在，直接插入即可
    MemVariable * memValue = new MemVariable(type);

    memVector.push_back(memValue);

    return memValue;
}

/// @brief 清理函数内申请的资源
void Function::Delete()
{
    // 清理IR指令
    code.Delete();

    // 清理Value
    for (auto & var: varsVector) {
        delete var;
    }

    varsVector.clear();
}

///
/// @brief 函数内的Value重命名
///
// Function.cpp
void Function::renameIR()
{
    if (isBuiltin()) {
        return;
    }

    int32_t nameIndex = 0; // 本函数内统一的命名计数器

    // 1. 重命名形式参数
    for (auto & param : this->params) {
        param->setIRName(IR_TEMP_VARNAME_PREFIX + std::to_string(nameIndex++));
    }

    // 2. 重命名局部变量 (varsVector)
    for (auto & var : this->varsVector) {
        var->setIRName(IR_LOCAL_VARNAME_PREFIX + std::to_string(nameIndex++));
    }

    // 3. 重命名临时变量 (tempVars) - 这是关键新增部分
    for (auto & temp_val : this->tempVars) {
        temp_val->setIRName(IR_TEMP_VARNAME_PREFIX + std::to_string(nameIndex++));
    }

    // 4. 重命名标签 (如果需要，你的是在创建时命名，通常可以)
    for (auto inst : this->getInterCode().getInsts()) {
        if (inst->getOp() == IRInstOperator::IRINST_OP_LABEL) {
            // 你的注释说标签名在 newLabel() 中创建时即最终确定，这很好。
        }
        // 注意：不再需要在这里通过遍历指令去命名指令结果为临时变量。
        // 因为所有临时变量（包括指令结果）都应该通过 Module::newTemporary 创建，
        // 加入到 tempVars，并在上面的第3步被统一命名。
        // 原来的 else if (inst->hasResultValue()) ... inst->setIRName(...) 部分可以考虑移除或修改，
        // 确保它不会与第3步的命名冲突，或者只处理那些不由 newTemporary 管理的特殊结果值（如果有的话）。
        // 最安全的是，如果一个指令的结果是一个 Value*，那个 Value* 应该就是 tempVars 中的一员，
        // 其名字已在第3步被设置。
    }
}

///
	/// @brief 获取统计的ARG指令的个数
	/// @return int32_t 个数
	///
	int32_t Function::getRealArgcount()
	{
		return this->realArgCount;
	}

	///
	/// @brief 用于统计ARG指令个数的自增函数，个数加1
	///
	void Function::realArgCountInc()
	{
		this->realArgCount++;
	}

	///
	/// @brief 用于统计ARG指令个数的清零
	///
	void Function::realArgCountReset()
	{
		this->realArgCount = 0;
    }
    
	void Function::addTempVar(Value* val)
{
    // 检查传入的指针是否有效，避免向 vector 中添加空指针
    if (val != nullptr) {
        // 可选：防止重复添加同一个临时变量
        // 如果你确定在调用 addTempVar 之前不会有重复添加的情况，可以省略这个检查以提高一点点性能。
        // 但为了健壮性，加上检查通常是好的。
        bool already_exists = false;
        for (const auto& existing_temp_var : tempVars) {
            if (existing_temp_var == val) {
                already_exists = true;
                break;
            }
        }

        if (!already_exists) {
            tempVars.push_back(val);
        }
        // 如果不需要去重，可以直接写：
        // tempVars.push_back(val);
    }
    // else {
        // 可选：如果 val 为 nullptr，可以记录一个警告或错误
        // minic_log(LOG_WARNING, "Attempted to add a null Value as a temporary variable to function %s", getName().c_str());
    // }
}