
// Generated from MiniC.g4 by ANTLR 4.12.0

#pragma once


#include "antlr4-runtime.h"
#include "MiniCListener.h"


/**
 * This class provides an empty implementation of MiniCListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  MiniCBaseListener : public MiniCListener {
public:

  virtual void enterCompileUnit(MiniCParser::CompileUnitContext * /*ctx*/) override { }
  virtual void exitCompileUnit(MiniCParser::CompileUnitContext * /*ctx*/) override { }

  virtual void enterFuncDef(MiniCParser::FuncDefContext * /*ctx*/) override { }
  virtual void exitFuncDef(MiniCParser::FuncDefContext * /*ctx*/) override { }

  virtual void enterBlock(MiniCParser::BlockContext * /*ctx*/) override { }
  virtual void exitBlock(MiniCParser::BlockContext * /*ctx*/) override { }

  virtual void enterBlockItemList(MiniCParser::BlockItemListContext * /*ctx*/) override { }
  virtual void exitBlockItemList(MiniCParser::BlockItemListContext * /*ctx*/) override { }

  virtual void enterBlockItem(MiniCParser::BlockItemContext * /*ctx*/) override { }
  virtual void exitBlockItem(MiniCParser::BlockItemContext * /*ctx*/) override { }

  virtual void enterVarDecl(MiniCParser::VarDeclContext * /*ctx*/) override { }
  virtual void exitVarDecl(MiniCParser::VarDeclContext * /*ctx*/) override { }

  virtual void enterBasicType(MiniCParser::BasicTypeContext * /*ctx*/) override { }
  virtual void exitBasicType(MiniCParser::BasicTypeContext * /*ctx*/) override { }

  virtual void enterVarDef(MiniCParser::VarDefContext * /*ctx*/) override { }
  virtual void exitVarDef(MiniCParser::VarDefContext * /*ctx*/) override { }

  virtual void enterReturnStatement(MiniCParser::ReturnStatementContext * /*ctx*/) override { }
  virtual void exitReturnStatement(MiniCParser::ReturnStatementContext * /*ctx*/) override { }

  virtual void enterAssignStatement(MiniCParser::AssignStatementContext * /*ctx*/) override { }
  virtual void exitAssignStatement(MiniCParser::AssignStatementContext * /*ctx*/) override { }

  virtual void enterBlockStatement(MiniCParser::BlockStatementContext * /*ctx*/) override { }
  virtual void exitBlockStatement(MiniCParser::BlockStatementContext * /*ctx*/) override { }

  virtual void enterExpressionStatement(MiniCParser::ExpressionStatementContext * /*ctx*/) override { }
  virtual void exitExpressionStatement(MiniCParser::ExpressionStatementContext * /*ctx*/) override { }

  virtual void enterExpr(MiniCParser::ExprContext * /*ctx*/) override { }
  virtual void exitExpr(MiniCParser::ExprContext * /*ctx*/) override { }

  virtual void enterAddExp(MiniCParser::AddExpContext * /*ctx*/) override { }
  virtual void exitAddExp(MiniCParser::AddExpContext * /*ctx*/) override { }

  virtual void enterAddOp(MiniCParser::AddOpContext * /*ctx*/) override { }
  virtual void exitAddOp(MiniCParser::AddOpContext * /*ctx*/) override { }

  virtual void enterMulExp(MiniCParser::MulExpContext * /*ctx*/) override { }
  virtual void exitMulExp(MiniCParser::MulExpContext * /*ctx*/) override { }

  virtual void enterMulOp(MiniCParser::MulOpContext * /*ctx*/) override { }
  virtual void exitMulOp(MiniCParser::MulOpContext * /*ctx*/) override { }

  virtual void enterNegativeExpr(MiniCParser::NegativeExprContext * /*ctx*/) override { }
  virtual void exitNegativeExpr(MiniCParser::NegativeExprContext * /*ctx*/) override { }

  virtual void enterPrimaryExpr(MiniCParser::PrimaryExprContext * /*ctx*/) override { }
  virtual void exitPrimaryExpr(MiniCParser::PrimaryExprContext * /*ctx*/) override { }

  virtual void enterFunctionCallExpr(MiniCParser::FunctionCallExprContext * /*ctx*/) override { }
  virtual void exitFunctionCallExpr(MiniCParser::FunctionCallExprContext * /*ctx*/) override { }

  virtual void enterPrimaryExp(MiniCParser::PrimaryExpContext * /*ctx*/) override { }
  virtual void exitPrimaryExp(MiniCParser::PrimaryExpContext * /*ctx*/) override { }

  virtual void enterRealParamList(MiniCParser::RealParamListContext * /*ctx*/) override { }
  virtual void exitRealParamList(MiniCParser::RealParamListContext * /*ctx*/) override { }

  virtual void enterLVal(MiniCParser::LValContext * /*ctx*/) override { }
  virtual void exitLVal(MiniCParser::LValContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

