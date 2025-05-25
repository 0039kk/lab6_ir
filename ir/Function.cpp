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
#include "PlatformArm32.h"
#include "Common.h"
/// @brief 指定函数名字、函数类型的构造函数
/// @param _name 函数名称
/// @param _type 函数类型
/// @param _builtin 是否是内置函数
Function::Function(std::string _name, FunctionType * _type, bool _builtin)
    : GlobalValue(_type, _name), builtIn(_builtin),currentNegativeStackOffsetSize_(0)
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
LocalVariable *Function::newLocalVarValue(Type *type, std::string name, int32_t scope_level)
{
    if (!type) {
        minic_log(LOG_ERROR, "Function '%s': newLocalVarValue called with null Type for var name '%s'.", 
                  this->getName().c_str(), name.c_str());
        return nullptr;
    }

    // 1. 创建 LocalVariable 对象
    //    你需要根据你的 LocalVariable.h 决定如何创建。
    //    假设 LocalVariable 构造函数是 public 或者 Function 是 friend:
    LocalVariable *localVar = new LocalVariable(type, name, scope_level);
    
    if (!localVar) { // 理论上 new 不会返回 nullptr 除非 std::nothrow，但以防万一
        minic_log(LOG_ERROR, "Function '%s': Failed to allocate LocalVariable for name '%s'.",
                  this->getName().c_str(), name.c_str());
        return nullptr;
    }

    // 确保 IRName 被设置 (如果 name 为空，LocalVariable 构造函数或 Value 基类应处理)
    if (name.empty() && localVar->getIRName().find("UNNAMED_VALUE") != std::string::npos) {
        // 如果需要，可以给一个更特定的临时局部变量名，但这通常由 Value 基类处理
    } else if (!name.empty() && (localVar->getIRName().empty() || localVar->getIRName().find("UNNAMED_VALUE") != std::string::npos) ) {
        localVar->setIRName(name + "_lv"); // 例如 %a_lv
    }


    // 2. 计算大小和对齐
    int32_t var_size = type->getSize();
    if (var_size <= 0) {
         minic_log(LOG_WARNING, "Function '%s': LocalVariable '%s' (type: %s) has size %d. Defaulting to 4 bytes for offset calculation.",
                   this->getName().c_str(), name.c_str(), type->toString().c_str(), var_size);
         var_size = 4; // 至少分配一个字，避免0或负大小
    }
    int32_t aligned_size = (var_size + 3) & ~3; // 4字节对齐

    // 3. 更新 currentNegativeStackOffsetSize_ (存储已分配的局部变量总大小的绝对值)
    this->currentNegativeStackOffsetSize_ += aligned_size;

    // 4. 计算新局部变量的栈偏移 (相对于 FP 的负偏移)
    int32_t new_offset_for_var = -this->currentNegativeStackOffsetSize_;

    // 5. 为 LocalVariable 设置内存地址
    localVar->setMemoryAddr(ARM32_FP_REG_NO, new_offset_for_var);
    
    // 6. 将新创建的局部变量添加到函数的变量列表中
    varsVector.push_back(localVar);

    // --- 日志 ---
    int32_t check_base; 
    int64_t check_off;
    bool has_addr = localVar->getMemoryAddr(&check_base, &check_off); // 调用 getMemoryAddr 验证

    minic_log(LOG_DEBUG, "Function '%s'::newLocalVarValue: Created LocalVar '%s' (IR: '%s', Ptr: %p). "
                         "Type: %s, Size: %d (Aligned: %d). "
                         "CumulativeNegOffsetSize: %d. Assigned Offset: %d. "
                         "Verification -> HasAddr: %s, BaseRead: %d, OffsetRead: %lld",
              this->getName().c_str(),
              name.c_str(), localVar->getIRName().c_str(), static_cast<void*>(localVar),
              type->toString().c_str(), var_size, aligned_size,
              this->currentNegativeStackOffsetSize_, new_offset_for_var,
              has_addr ? "true" : "false", check_base, static_cast<long long>(check_off));
    // --- 结束日志 ---
    
    // 更新函数的总栈帧深度 (maxDepth 通常用于 CodeGeneratorArm32::allocStack)
    // 注意：这里的 maxDepth 与 CodeGeneratorArm32::stackAlloc 中的 setMaxDep(sp_esp)
    //       的 sp_esp 含义可能不同。
    //       这里的 currentNegativeStackOffsetSize_ 只包含局部变量。
    //       而 CodeGeneratorArm32::stackAlloc 中的 sp_esp 最终会包含局部变量+临时变量+调用参数空间。
    //       通常，Function::maxDepth 由 CodeGeneratorArm32::stackAlloc 在最后设置。
    //       所以，这里可能不需要直接更新 this->maxDepth，除非你有特定逻辑。
    //       如果 newLocalVarValue 是早期分配，而 stackAlloc 是后期，那么 stackAlloc
    //       会使用 getCurrentFuncFrameSizeNegative() 作为起点。

    return localVar;
}


// --- 实现新增的 getCurrentFuncFrameSizeNegative ---
[[nodiscard]] int32_t Function::getCurrentFuncFrameSizeNegative() const {
    minic_log(LOG_DEBUG, "Function '%s': getCurrentFuncFrameSizeNegative() returning %d (abs size of local vars stack)",
              this->getName().c_str(), this->currentNegativeStackOffsetSize_);
    return this->currentNegativeStackOffsetSize_;
}
// --- 结束实现 ---


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

std::string Function::newTempName() {
    // 假设 IR_TEMP_VARNAME_PREFIX 是在某处定义的宏或常量，例如 "%t"
    // 如果没有定义，你需要定义它，例如：
    // const std::string IR_TEMP_VARNAME_PREFIX = "%t"; (或者在 Common.h)
    return IR_TEMP_VARNAME_PREFIX + std::to_string(tempNameCounter_++);
}