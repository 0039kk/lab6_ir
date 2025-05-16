// ir/Instructions/CmpInstruction.cpp
#include "CmpInstruction.h"
#include "Function.h" // 如果构造函数需要
#include "Type.h"     // 如果需要检查类型
#include "IntegerType.h"  
#include <stdexcept>  // For std::invalid_argument
#include <iostream>      // <--- 这里，用于 std::cerr
/*CmpInstruction::CmpInstruction(Value *dest, CmpOp op_enum, Value *op1, Value *op2, Function *parent_func)
    : Instruction(parent_func, IRInstOperator::IRINST_OP_CMP, IntegerType::get(1)), // 使用新增的枚举，结果类型是 i1
      dest_reg_(dest),
      cmp_operator_(op_enum),
      operand1_(op1),
      operand2_(op2) {
    if (!dest || !op1 || !op2) {
        throw std::invalid_argument("CmpInstruction 操作数不能为空。");
    }
    // 验证 dest 的类型是否为 i1
    if (!dest->getType() || dest->getType() != IntegerType::get(1)) { // 假设 IntegerType::get(1) 获取 i1 类型
         throw std::invalid_argument("CmpInstruction 的目标寄存器必须是 i1 类型。");
    }
}*/
CmpInstruction::CmpInstruction(Value *dest, CmpOp op_enum, Value *op1, Value *op2, Function *parent_func)
    : Instruction(parent_func, IRInstOperator::IRINST_OP_CMP, IntegerType::getTypeBool()), // 结果类型总是 i1
      dest_reg_(dest),
      cmp_operator_(op_enum),
      operand1_(op1),
      operand2_(op2) {

    // 1. 检查操作数和目标是否为空指针
    if (!dest) {
        std::cerr << "[CMP_CTOR_FATAL] CRITICAL ERROR: 'dest' operand is nullptr." << std::endl;
        throw std::invalid_argument("CmpInstruction 'dest' 操作数不能为空。");
    }
    if (!op1) {
        std::cerr << "[CMP_CTOR_FATAL] CRITICAL ERROR: 'op1' operand is nullptr." << std::endl;
        throw std::invalid_argument("CmpInstruction 'op1' 操作数不能为空。");
    }
    if (!op2) {
        std::cerr << "[CMP_CTOR_FATAL] CRITICAL ERROR: 'op2' operand is nullptr." << std::endl;
        throw std::invalid_argument("CmpInstruction 'op2' 操作数不能为空。");
    }

    // 2. 检查目标操作数的类型是否为空指针
    Type* actual_dest_type = dest->getType();
    if (!actual_dest_type) {
         std::cerr << "[CMP_CTOR_FATAL] CRITICAL ERROR: dest->getType() returned nullptr. dest IRName: " << dest->getIRName() << std::endl;
         throw std::invalid_argument("CmpInstruction 的目标寄存器类型为 null。");
    }

    // 3. 获取 isInt1Byte() 的结果并进行调试打印
    bool is_i1_type = actual_dest_type->isInt1Byte();

    // --- 详细调试打印 ---
    std::cerr << "-----------------------------------------------------" << std::endl;
    std::cerr << "[CMP_CTOR_DEBUG] Entering CmpInstruction Constructor" << std::endl;
    std::cerr << "[CMP_CTOR_DEBUG] dest Value Pointer: " << dest << std::endl;
    std::cerr << "[CMP_CTOR_DEBUG] dest IRName: " << dest->getIRName() << std::endl;
    std::cerr << "[CMP_CTOR_DEBUG] dest->getType() (actual_dest_type) Pointer: " << actual_dest_type << std::endl;
    std::cerr << "[CMP_CTOR_DEBUG] actual_dest_type->toString(): " << actual_dest_type->toString() << std::endl;
    std::cerr << "[CMP_CTOR_DEBUG] actual_dest_type->getTypeID(): " << actual_dest_type->getTypeID() << std::endl; // 假设 TypeID 是可打印的枚举或整数
    if (actual_dest_type->isIntegerType()) {
         IntegerType* int_type = static_cast<IntegerType*>(actual_dest_type);
         std::cerr << "[CMP_CTOR_DEBUG] actual_dest_type Bitwidth: " << int_type->getBitWidth() << std::endl;
         std::cerr << "[CMP_CTOR_DEBUG] actual_dest_type (using internal bit_width_ check for isInt1Byte): " << (int_type->getBitWidth() == 1 ? "true (matches bit_width_==1)" : "false (bit_width_!=1)") << std::endl;
    }
    std::cerr << "[CMP_CTOR_DEBUG] Result of actual_dest_type->isInt1Byte() stored in is_i1_type: " << (is_i1_type ? "true" : "false") << std::endl;

    Type* expected_i1_singleton = IntegerType::getTypeBool();
    std::cerr << "[CMP_CTOR_DEBUG] IntegerType::getTypeBool() Pointer: " << expected_i1_singleton << std::endl;
    if (expected_i1_singleton) {
        std::cerr << "[CMP_CTOR_DEBUG] IntegerType::getTypeBool()->toString(): " << expected_i1_singleton->toString() << std::endl;
        std::cerr << "[CMP_CTOR_DEBUG] IntegerType::getTypeBool()->isInt1Byte(): " << (expected_i1_singleton->isInt1Byte() ? "true" : "false") << std::endl;
        if (expected_i1_singleton->isIntegerType()) {
            std::cerr << "[CMP_CTOR_DEBUG] IntegerType::getTypeBool() Bitwidth: " << static_cast<IntegerType*>(expected_i1_singleton)->getBitWidth() << std::endl;
        }
    }
    std::cerr << "-----------------------------------------------------" << std::endl;
    // --- 调试打印结束 ---


    // 4. 使用局部变量进行最终的类型检查并抛出异常
    if (!is_i1_type) {
         std::cerr << "[CMP_CTOR_THROW] THROWING EXCEPTION because !is_i1_type is true." << std::endl;
         std::string error_msg = "CmpInstruction 的目标寄存器必须是 i1 类型. Actual type: " + actual_dest_type->toString()
                               + " (IRName: " + dest->getIRName() + ")";
         throw std::invalid_argument(error_msg);
    }

    // 如果所有检查通过，构造继续...
}
std::string CmpInstruction::CmpOpToString(CmpOp op) {
    switch (op) {
        case EQ: return "eq";
        case NE: return "ne";
        case GT: return "gt";
        case GE: return "ge";
        case LT: return "lt";
        case LE: return "le";
        default: return "unknown_cmp_op";
    }
}

CmpInstruction::CmpOp CmpInstruction::StringToCmpOp(const std::string& op_str) {
    if (op_str == "eq") return EQ;
    if (op_str == "ne") return NE;
    if (op_str == "gt") return GT;
    if (op_str == "ge") return GE;
    if (op_str == "lt") return LT;
    if (op_str == "le") return LE;
    throw std::invalid_argument("Unknown CmpOperator string: " + op_str);
}


std::string CmpInstruction::toString() const {
    std::string result_str;
    if (dest_reg_ && operand1_ && operand2_) {
        result_str = dest_reg_->getIRName() + " = icmp " + CmpOpToString(cmp_operator_) + " " +
                     operand1_->getIRName() + ", " + operand2_->getIRName();
    } else {
        result_str = "; <Error: CmpInstruction has null operands or destination>";
    }
    return result_str;
}