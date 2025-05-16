///
/// @file LabelInstruction.h
/// @brief Label指令
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

#include <string>

#include "Instruction.h"

class Function;

///
/// @brief Label指令
///
// ir/Instructions/LabelInstruction.h
class LabelInstruction : public Instruction {
	public:
		explicit LabelInstruction(Function * _func, const std::string& unique_ir_name);
		[[nodiscard]] std::string toString() const override;
		// getName() 和 getIRName() 将使用 Value 基类的默认实现
	};