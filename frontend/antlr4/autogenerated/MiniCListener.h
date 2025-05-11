
// Generated from MiniC.g4 by ANTLR 4.12.0

#pragma once


#include "antlr4-runtime.h"
#include "MiniCParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by MiniCParser.
 */
class  MiniCListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterCompileUnit(MiniCParser::CompileUnitContext *ctx) = 0;
  virtual void exitCompileUnit(MiniCParser::CompileUnitContext *ctx) = 0;

  virtual void enterFuncDef(MiniCParser::FuncDefContext *ctx) = 0;
  virtual void exitFuncDef(MiniCParser::FuncDefContext *ctx) = 0;

  virtual void enterBlock(MiniCParser::BlockContext *ctx) = 0;
  virtual void exitBlock(MiniCParser::BlockContext *ctx) = 0;

  virtual void enterBlockItemList(MiniCParser::BlockItemListContext *ctx) = 0;
  virtual void exitBlockItemList(MiniCParser::BlockItemListContext *ctx) = 0;

  virtual void enterBlockItem(MiniCParser::BlockItemContext *ctx) = 0;
  virtual void exitBlockItem(MiniCParser::BlockItemContext *ctx) = 0;

  virtual void enterVarDecl(MiniCParser::VarDeclContext *ctx) = 0;
  virtual void exitVarDecl(MiniCParser::VarDeclContext *ctx) = 0;

  virtual void enterBasicType(MiniCParser::BasicTypeContext *ctx) = 0;
  virtual void exitBasicType(MiniCParser::BasicTypeContext *ctx) = 0;

  virtual void enterVarDef(MiniCParser::VarDefContext *ctx) = 0;
  virtual void exitVarDef(MiniCParser::VarDefContext *ctx) = 0;

  virtual void enterReturnStatement(MiniCParser::ReturnStatementContext *ctx) = 0;
  virtual void exitReturnStatement(MiniCParser::ReturnStatementContext *ctx) = 0;

  virtual void enterAssignStatement(MiniCParser::AssignStatementContext *ctx) = 0;
  virtual void exitAssignStatement(MiniCParser::AssignStatementContext *ctx) = 0;

  virtual void enterBlockStatement(MiniCParser::BlockStatementContext *ctx) = 0;
  virtual void exitBlockStatement(MiniCParser::BlockStatementContext *ctx) = 0;

  virtual void enterExpressionStatement(MiniCParser::ExpressionStatementContext *ctx) = 0;
  virtual void exitExpressionStatement(MiniCParser::ExpressionStatementContext *ctx) = 0;

  virtual void enterExpr(MiniCParser::ExprContext *ctx) = 0;
  virtual void exitExpr(MiniCParser::ExprContext *ctx) = 0;

  virtual void enterAddExp(MiniCParser::AddExpContext *ctx) = 0;
  virtual void exitAddExp(MiniCParser::AddExpContext *ctx) = 0;

  virtual void enterAddOp(MiniCParser::AddOpContext *ctx) = 0;
  virtual void exitAddOp(MiniCParser::AddOpContext *ctx) = 0;

  virtual void enterMulExp(MiniCParser::MulExpContext *ctx) = 0;
  virtual void exitMulExp(MiniCParser::MulExpContext *ctx) = 0;

  virtual void enterMulOp(MiniCParser::MulOpContext *ctx) = 0;
  virtual void exitMulOp(MiniCParser::MulOpContext *ctx) = 0;

  virtual void enterNegativeExpr(MiniCParser::NegativeExprContext *ctx) = 0;
  virtual void exitNegativeExpr(MiniCParser::NegativeExprContext *ctx) = 0;

  virtual void enterPrimaryExpr(MiniCParser::PrimaryExprContext *ctx) = 0;
  virtual void exitPrimaryExpr(MiniCParser::PrimaryExprContext *ctx) = 0;

  virtual void enterFunctionCallExpr(MiniCParser::FunctionCallExprContext *ctx) = 0;
  virtual void exitFunctionCallExpr(MiniCParser::FunctionCallExprContext *ctx) = 0;

  virtual void enterPrimaryExp(MiniCParser::PrimaryExpContext *ctx) = 0;
  virtual void exitPrimaryExp(MiniCParser::PrimaryExpContext *ctx) = 0;

  virtual void enterRealParamList(MiniCParser::RealParamListContext *ctx) = 0;
  virtual void exitRealParamList(MiniCParser::RealParamListContext *ctx) = 0;

  virtual void enterLVal(MiniCParser::LValContext *ctx) = 0;
  virtual void exitLVal(MiniCParser::LValContext *ctx) = 0;


};

