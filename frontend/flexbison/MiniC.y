%{
#include <cstdio>
#include <cstring>

// 词法分析头文件
#include "FlexLexer.h"

// bison生成的头文件
//#include "BisonParser.h"

// 抽象语法树函数定义原型头文件
#include "AST.h"

#include "IntegerType.h"

#include "AttrType.h"
// LR分析失败时所调用函数的原型声明
void yyerror(char * msg);

%}

// 联合体声明，用于后续终结符和非终结符号属性指定使用
%union {
    class ast_node * node;

    struct digit_int_attr integer_num;
    struct digit_real_attr float_num;
    struct var_id_attr var_id;
    struct type_attr type;
    int op_class;
};


// --- 新的优先级和结合性规则 ---
%right T_ASSIGN                  // 赋值 = (右结合)
%left  T_LOR                     // 逻辑或 || (左结合)
%left  T_LAND                    // 逻辑与 && (左结合)
%left  T_EQ T_NE                 // 等于 ==, 不等于 != (左结合)
%left  T_LT T_LE T_GT T_GE       // 关系 < <= > >= (左结合)
// --- 结束新的优先级规则 ---


%left T_ADD T_SUB
%left T_MUL T_DIV T_MOD


// --- 新的单目操作符优先级 ---
%right T_LNOT                    // 逻辑非 ! (右结合)
// --- 结束新的单目操作符优先级 ---
%right UNARY_MINUS


// 悬挂else (Dangling else) 处理:
// T_ELSE 的优先级高于没有else的if语句的 "then" 部分。
%nonassoc T_IFX     // 代表 "if (Expr) Statement" (没有 else 的 if)
%nonassoc T_ELSE    // T_ELSE 本身

// 文法的开始符号
%start  CompileUnit

// 指定文法的终结符号，<>可指定文法属性
// 对于单个字符的算符或者分隔符，在词法分析时可直返返回对应的ASCII码值，bison预留了255以内的值
// %token开始的符号称之为终结符，需要词法分析工具如flex识别后返回
// %type开始的符号称之为非终结符，需要通过文法产生式来定义
// %token或%type之后的<>括住的内容成为文法符号的属性，定义在前面的%union中的成员名字。
%token <integer_num> T_DIGIT
%token <var_id> T_ID
%token <type> T_INT

// 关键或保留字 一词一类 不需要赋予语义属性
%token T_RETURN

// 分隔符 一词一类 不需要赋予语义属性
%token T_SEMICOLON T_L_PAREN T_R_PAREN T_L_BRACE T_R_BRACE
%token T_COMMA

// --- 新的操作符 Token ---
%token T_LT T_LE T_GT T_GE T_EQ T_NE // 关系和相等运算符
%token T_LAND T_LOR T_LNOT          // 逻辑运算符
%token T_IF T_ELSE T_WHILE T_BREAK T_CONTINUE
// --- 结束新的操作符 Token ---

// 运算符
%token T_ASSIGN T_SUB T_ADD
%token T_MUL T_DIV T_MOD 
// 非终结符
// %type指定文法的非终结符号，<>可指定文法属性
%type <node> CompileUnit
%type <node> FuncDef
%type <node> Block
%type <node> BlockItemList
%type <node> BlockItem
%type <node> Statement
%type <node> IfStmt WhileStmt BreakStmt ContinueStmt // 特定的语句类型
%type <node> Expr AssignExpr LOrExp LAndExp EqExp RelExp // 新的表达式层级
%type <node> LVal
%type <node> VarDecl VarDeclExpr VarDef
%type <node> AddExp UnaryExp PrimaryExp 
%type <node> MulExp
%type <node> RealParamList
%type <type> BasicType
%type <op_class> AddOp 
%type <op_class> MulOp
// --- 新的用于操作符的非终结符 (可选, 也可以直接在动作中使用 token) ---
%type <op_class> RelOp EqOp
// --- 结束新的操作符非终结符 ---
%%

// 编译单元可包含若干个函数与全局变量定义。要在语义分析时检查main函数存在
// compileUnit: (funcDef | varDecl)* EOF;
// bison不支持闭包运算，为便于追加修改成左递归方式
// compileUnit: funcDef | varDecl | compileUnit funcDef | compileUnit varDecl
CompileUnit : FuncDef {

		// 创建一个编译单元的节点AST_OP_COMPILE_UNIT
		$$ = create_contain_node(ast_operator_type::AST_OP_COMPILE_UNIT, $1);

		// 设置到全局变量中
		ast_root = $$;
	}
	| VarDecl {

		// 创建一个编译单元的节点AST_OP_COMPILE_UNIT
		$$ = create_contain_node(ast_operator_type::AST_OP_COMPILE_UNIT, $1);
		ast_root = $$;
	}
	| CompileUnit FuncDef {

		// 把函数定义的节点作为编译单元的孩子
		$$ = $1->insert_son_node($2);
	}
	| CompileUnit VarDecl {
		// 把变量定义的节点作为编译单元的孩子
		$$ = $1->insert_son_node($2);
	}
	;

// 函数定义，目前支持整数返回类型，不支持形参
FuncDef : BasicType T_ID T_L_PAREN T_R_PAREN Block  {

		// 函数返回类型
		type_attr funcReturnType = $1;

		// 函数名
		var_id_attr funcId = $2;

		// 函数体节点即Block，即$5
		ast_node * blockNode = $5;

		// 形参结点没有，设置为空指针
		ast_node * formalParamsNode = nullptr;

		// 创建函数定义的节点，孩子有类型，函数名，语句块和形参(实际上无)
		// create_func_def函数内会释放funcId中指向的标识符空间，切记，之后不要再释放，之前一定要是通过strdup函数或者malloc分配的空间
		$$ = create_func_def(funcReturnType, funcId, blockNode, formalParamsNode);
	}
	;

// 语句块的文法Block ： T_L_BRACE BlockItemList? T_R_BRACE
// 其中?代表可有可无，在bison中不支持，需要拆分成两个产生式
// Block ： T_L_BRACE T_R_BRACE | T_L_BRACE BlockItemList T_R_BRACE
Block : T_L_BRACE T_R_BRACE {
		// 语句块没有语句

		// 为了方便创建一个空的Block节点
		$$ = create_contain_node(ast_operator_type::AST_OP_BLOCK);
	}
	| T_L_BRACE BlockItemList T_R_BRACE {
		// 语句块含有语句

		// BlockItemList归约时内部创建Block节点，并把语句加入，这里不创建Block节点
		$$ = $2;
	}
	;

// 语句块内语句列表的文法：BlockItemList : BlockItem+
// Bison不支持正闭包，需修改成左递归形式，便于属性的传递与孩子节点的追加
// 左递归形式的文法为：BlockItemList : BlockItem | BlockItemList BlockItem
BlockItemList : BlockItem {
		// 第一个左侧的孩子节点归约成Block节点，后续语句可持续作为孩子追加到Block节点中
		// 创建一个AST_OP_BLOCK类型的中间节点，孩子为Statement($1)
		$$ = create_contain_node(ast_operator_type::AST_OP_BLOCK, $1);
	}
	| BlockItemList BlockItem {
		// 把BlockItem归约的节点加入到BlockItemList的节点中
		$$ = $1->insert_son_node($2);
	}
	;


// 语句块中子项的文法：BlockItem : Statement
// 目前只支持语句,后续可增加支持变量定义
BlockItem : Statement  {
		// 语句节点传递给归约后的节点上，综合属性
		$$ = $1;
	}
	| VarDecl {
		// 变量声明节点传递给归约后的节点上，综合属性
		$$ = $1;
	}
	;

// 变量声明语句
// 语法：varDecl: basicType varDef (T_COMMA varDef)* T_SEMICOLON
// 因Bison不支持闭包运算符，因此需要修改成左递归，修改后的文法为：
// VarDecl : VarDeclExpr T_SEMICOLON
// VarDeclExpr: BasicType VarDef | VarDeclExpr T_COMMA varDef
VarDecl : VarDeclExpr T_SEMICOLON {
		$$ = $1;
	}
	;

// 变量声明表达式，可支持逗号分隔定义多个
VarDeclExpr: BasicType VarDef {

		// 创建类型节点
		ast_node * type_node = create_type_node($1);

		// 创建变量定义节点
		ast_node * decl_node = create_contain_node(ast_operator_type::AST_OP_VAR_DECL, type_node, $2);
		decl_node->type = type_node->type;

		// 创建变量声明语句，并加入第一个变量
		$$ = create_var_decl_stmt_node(decl_node);
	}
	| VarDeclExpr T_COMMA VarDef {

		// 创建类型节点，这里从VarDeclExpr获取类型，前面已经设置
		ast_node * type_node = ast_node::New($1->type);

		// 创建变量定义节点
		ast_node * decl_node = create_contain_node(ast_operator_type::AST_OP_VAR_DECL, type_node, $3);

		// 插入到变量声明语句
		$$ = $1->insert_son_node(decl_node);
	}
	;

// 变量定义包含变量名，实际上还有初值，这里没有实现。
VarDef : T_ID {
		// 变量ID

		$$ = ast_node::New(var_id_attr{$1.id, $1.lineno});

		// 对于字符型字面量的字符串空间需要释放，因词法用到了strdup进行了字符串复制
		free($1.id);
	}
	;

// 基本类型，目前只支持整型
BasicType: T_INT {
		$$ = $1;
	}
	;

// 语句文法：statement:T_RETURN expr T_SEMICOLON | lVal T_ASSIGN expr T_SEMICOLON
// | block | expr? T_SEMICOLON
// 支持返回语句、赋值语句、语句块、表达式语句
// 其中表达式语句可支持空语句，由于bison不支持?，修改成两条
// --- 修改后的 Statement 规则 ---
Statement : T_RETURN Expr T_SEMICOLON {
        // 假设 create_unary_op_node(op, child)
        $$ = create_contain_node(ast_operator_type::AST_OP_RETURN, $2);
    }
    // | LVal T_ASSIGN Expr T_SEMICOLON { // 原来的赋值语句，现在赋值是表达式的一部分
    //     // $$ = create_contain_node(ast_operator_type::AST_OP_ASSIGN, $1, $3);
    // }
    | Block { $$ = $1; }
    | IfStmt    { $$ = $1; }      // 新增
    | WhileStmt { $$ = $1; }      // 新增
    | BreakStmt { $$ = $1; }      // 新增
    | ContinueStmt { $$ = $1; }   // 新增
    | Expr T_SEMICOLON {          // 表达式语句 (可能包含赋值表达式)
        $$ = $1;
    }
    | T_SEMICOLON {               // 空语句
        // 用一个特殊的节点表示空语句，或者直接返回 nullptr
        // $$ = create_simple_stmt_node(ast_operator_type::AST_OP_EMPTY_STMT, yylineno);
        $$ = nullptr; // 在 BlockItemList 中处理 nullptr
    }
    ;

IfStmt  : T_IF T_L_PAREN Expr T_R_PAREN Statement %prec T_IFX {
        $$ = create_contain_node(ast_operator_type::AST_OP_IF, $3, $5, nullptr);
        // 手动设置行号，如果 create_contain_node 不会自动从第一个有效子节点获取的话
        if ($$ && $3) $$->line_no = $3->line_no; // 以条件表达式的行号为准
    }
    | T_IF T_L_PAREN Expr T_R_PAREN Statement T_ELSE Statement {
        $$ = create_contain_node(ast_operator_type::AST_OP_IF, $3, $5, $7);
        if ($$ && $3) $$->line_no = $3->line_no; // 以条件表达式的行号为准
    }
    ;

WhileStmt : T_WHILE T_L_PAREN Expr T_R_PAREN Statement {
        // 假设 create_while_stmt_node(cond_expr, body_stmt)
        // $$ = create_while_stmt_node($3, $5);
        // 使用通用创建函数
        $$ = create_contain_node(ast_operator_type::AST_OP_WHILE, $3, $5); // $3是条件, $5是循环体
    }
    ;

BreakStmt : T_BREAK T_SEMICOLON {
        $$ = create_contain_node(ast_operator_type::AST_OP_BREAK);
        if ($$) $$->line_no = yylineno; // 确保 $$ 非空后设置行号
    }
    ;

ContinueStmt : T_CONTINUE T_SEMICOLON {
        $$ = create_contain_node(ast_operator_type::AST_OP_CONTINUE);
        if ($$) $$->line_no = yylineno; // 确保 $$ 非空后设置行号
    }
    ;




// --- 新的表达式层级结构 ---
// Expr 是最通用的表达式。赋值现在可以是表达式。
Expr    : AssignExpr { $$ = $1; } // 所有表达式最终都归结为 Expr
        // 如果不希望赋值作为表达式，而是语句，则 Expr: LOrExp; 并在Statement中保留赋值语句规则
        ;

// 赋值表达式 (右结合)
AssignExpr : LVal T_ASSIGN AssignExpr { // 右结合: a = b = c  解析为 a = (b = c)
               // 假设 create_binary_op_node(op, left, right)
               $$ = create_contain_node(ast_operator_type::AST_OP_ASSIGN, $1, $3);
           }
           | LOrExp { $$ = $1; } // 一个赋值表达式也可以是一个逻辑或表达式 (优先级更低)
           ;

// 逻辑或表达式 (左结合)
LOrExp  : LOrExp T_LOR LAndExp {
            $$ = create_contain_node(ast_operator_type::AST_OP_LOR, $1, $3);
        }
        | LAndExp { $$ = $1; } // 一个逻辑或表达式也可以是一个逻辑与表达式
        ;

// 逻辑与表达式 (左结合)
LAndExp : LAndExp T_LAND EqExp { // 注意这里下一级是 EqExp (相等表达式)
            $$ = create_contain_node(ast_operator_type::AST_OP_LAND, $1, $3);
        }
        | EqExp { $$ = $1; }   // 一个逻辑与表达式也可以是一个相等表达式
        ;

// 相等性表达式 (左结合)
EqExp   : EqExp EqOp RelExp { // 注意这里下一级是 RelExp (关系表达式)
            $$ = create_contain_node((ast_operator_type)$2, $1, $3);
        }
        | RelExp { $$ = $1; }   // 一个相等表达式也可以是一个关系表达式
        ;

// 相等性操作符
EqOp    : T_EQ  { $$ = (int)ast_operator_type::AST_OP_EQ; }
        | T_NE  { $$ = (int)ast_operator_type::AST_OP_NE; }
        ;

// 关系表达式 (左结合)
RelExp  : RelExp RelOp AddExp { // 注意这里下一级是 AddExp (加法表达式)
            $$ = create_contain_node((ast_operator_type)$2, $1, $3);
        }
        | AddExp { $$ = $1; }   // 一个关系表达式也可以是一个加法表达式
        ;

// 关系操作符
RelOp   : T_LT  { $$ = (int)ast_operator_type::AST_OP_LT; }
        | T_LE  { $$ = (int)ast_operator_type::AST_OP_LE; }
        | T_GT  { $$ = (int)ast_operator_type::AST_OP_GT; }
        | T_GE  { $$ = (int)ast_operator_type::AST_OP_GE; }
        ;


// 加减表达式文法：addExp: unaryExp (addOp unaryExp)*
// 由于bison不支持用闭包表达，因此需要拆分成左递归的形式
// 改造后的左递归文法：
// addExp : unaryExp | unaryExp addOp unaryExp | addExp addOp unaryExp
AddExp  : AddExp AddOp MulExp { // <--- 修改为左递归
            $$ = create_contain_node((ast_operator_type)$2, $1, $3);
        }
        | MulExp { $$ = $1; }   // 一个加法表达式也可以是一个乘法表达式
        ;

MulExp  : MulExp MulOp UnaryExp { // <--- 修改为左递归
            $$ = create_contain_node((ast_operator_type)$2, $1, $3);
        }
        | UnaryExp { $$ = $1; } // 一个乘法表达式也可以是一个一元表达式
        ;



// 加减运算符
AddOp: T_ADD {
		$$ = (int)ast_operator_type::AST_OP_ADD;
	}
	| T_SUB {
		$$ = (int)ast_operator_type::AST_OP_SUB;
	}
	;
// 乘除法操作符
MulOp : T_MUL {
            $$ = (int)ast_operator_type::AST_OP_MUL;
        }
      | T_DIV {
            $$ = (int)ast_operator_type::AST_OP_DIV;
        }
      | T_MOD {
            $$ = (int)ast_operator_type::AST_OP_MOD;
        };
// 目前一元表达式可以为基本表达式、函数调用，其中函数调用的实参可有可无
// 其文法为：unaryExp: primaryExp | T_ID T_L_PAREN realParamList? T_R_PAREN
// 由于bison不支持？表达，因此变更后的文法为：
// unaryExp: primaryExp | T_ID T_L_PAREN T_R_PAREN | T_ID T_L_PAREN realParamList T_R_PAREN
UnaryExp : PrimaryExp {
		// 基本表达式

		// 传递到归约后的UnaryExp上
		$$ = $1;
	}
	| T_SUB UnaryExp %prec UNARY_MINUS { 
		$$ = create_contain_node(ast_operator_type::AST_OP_NEG, $2);
	}
	 // --- 新增逻辑非 ---
    | T_LNOT UnaryExp { // 逻辑非! 优先级由 %right T_LNOT 控制
        $$ = create_contain_node(ast_operator_type::AST_OP_LNOT, $2);
    }
    // --- 结束新增逻辑非 ---
	| T_ID T_L_PAREN T_R_PAREN {
		// 没有实参的函数调用

		// 创建函数调用名终结符节点
		ast_node * name_node = ast_node::New(std::string($1.id), $1.lineno);

		// 对于字符型字面量的字符串空间需要释放，因词法用到了strdup进行了字符串复制
		free($1.id);

		// 实参列表
		ast_node * paramListNode = nullptr;

		// 创建函数调用节点，其孩子为被调用函数名和实参，实参为空，但函数内部会创建实参列表节点，无孩子
		$$ = create_func_call(name_node, paramListNode);

	}
	| T_ID T_L_PAREN RealParamList T_R_PAREN {
		// 含有实参的函数调用

		// 创建函数调用名终结符节点
		ast_node * name_node = ast_node::New(std::string($1.id), $1.lineno);

		// 对于字符型字面量的字符串空间需要释放，因词法用到了strdup进行了字符串复制
		free($1.id);

		// 实参列表
		ast_node * paramListNode = $3;

		// 创建函数调用节点，其孩子为被调用函数名和实参，实参不为空
		$$ = create_func_call(name_node, paramListNode);
	}
	;

// 基本表达式支持无符号整型字面量、带括号的表达式、具有左值属性的表达式
// 其文法为：primaryExp: T_L_PAREN expr T_R_PAREN | T_DIGIT | lVal
PrimaryExp :  T_L_PAREN Expr T_R_PAREN {
		// 带有括号的表达式
		$$ = $2;
	}
	| T_DIGIT {
        	// 无符号整型字面量

		// 创建一个无符号整型的终结符节点
		$$ = ast_node::New($1);
	}
	| LVal  {
		// 具有左值的表达式

		// 直接传递到归约后的非终结符号PrimaryExp
		$$ = $1;
	}
	;

// 实参表达式支持逗号分隔的若干个表达式
// 其文法为：realParamList: expr (T_COMMA expr)*
// 由于Bison不支持闭包运算符表达，修改成左递归形式的文法
// 左递归文法为：RealParamList : Expr | 左递归文法为：RealParamList T_COMMA expr
RealParamList : Expr {
		// 创建实参列表节点，并把当前的Expr节点加入
		$$ = create_contain_node(ast_operator_type::AST_OP_FUNC_REAL_PARAMS, $1);
	}
	| RealParamList T_COMMA Expr {
		// 左递归增加实参表达式
		$$ = $1->insert_son_node($3);
	}
	;

// 左值表达式，目前只支持变量名，实际上还有下标变量
LVal : T_ID {
		// 变量名终结符

		// 创建变量名终结符节点
		$$ = ast_node::New($1);

		// 对于字符型字面量的字符串空间需要释放，因词法用到了strdup进行了字符串复制
		free($1.id);
	}
	;

%%

// 语法识别错误要调用函数的定义
void yyerror(char * msg)
{
    printf("Line %d: %s\n", yylineno, msg);
}
