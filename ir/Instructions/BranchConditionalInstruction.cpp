// ir/Instructions/BranchConditionalInstruction.cpp
#include "BranchConditionalInstruction.h"
#include "LabelInstruction.h" // 需要知道 LabelInstruction 的 getName()
#include "Function.h"
#include "Type.h"
#include "IntegerType.h"  // 假设你有 IRType.h 定义了 i1 类型
#include <stdexcept>
#include "VoidType.h"
#include <iostream>

BranchConditionalInstruction::BranchConditionalInstruction(Value *cond, LabelInstruction *true_label, LabelInstruction *false_label, Function *parent_func)
    : Instruction(parent_func, IRInstOperator::IRINST_OP_BRANCH_COND, VoidType::getType()), // 条件跳转指令本身不产生值，所以类型可以是 void
      condition_reg_(cond),
      true_target_(true_label),
      false_target_(false_label) {
    if (!cond || !true_label || !false_label) {
        throw std::invalid_argument("BranchConditionalInstruction 操作数不能为空。");
    }
    // 验证 cond 的类型是否为 i1
    if (!cond->getType()) { // 先检查类型指针是否为空
		throw std::invalid_argument("BranchConditionalInstruction 条件操作数的类型为 null。");
   }
   // 使用类型系统的方法进行可靠的类型判断
   if (!cond->getType()->isInt1Byte()) { // isInt1Byte() 应该检查 bit_width_ == 1
		// 为了调试，先不要立即 throw，而是打印信息，看看 isInt1Byte() 返回什么
		std::cout << "[BC_CTOR_TYPE_CHECK] cond IRName: " << cond->getIRName() << std::endl;
		std::cout << "[BC_CTOR_TYPE_CHECK] cond->getType()->toString(): " << cond->getType()->toString() << std::endl;
		std::cout << "[BC_CTOR_TYPE_CHECK] cond->getType()->isInt1Byte() returned: "
				  << (cond->getType()->isInt1Byte() ? "true" : "false") << std::endl;
		if (cond->getType()->isIntegerType()) {
		   std::cout << "[BC_CTOR_TYPE_CHECK] Bitwidth: " << static_cast<IntegerType*>(cond->getType())->getBitWidth() << std::endl;
		}
		// 只有在调试确认 isInt1Byte() 确实按预期工作但这里仍然判断为 false 时才恢复 throw
		throw std::invalid_argument("BranchConditionalInstruction 的条件必须是 i1 类型. Actual: " + cond->getType()->toString());
   }
}
std::string BranchConditionalInstruction::toString() const {
    // 格式：bc condvar, label X, label Y
    // 例如: bc %t1, label .LTrue, label .LFalse
    return "bc " + condition_reg_->getIRName() + ", label " +
           true_target_->getIRName() + ", label " + false_target_->getIRName();
}