///
/// @file RecursiveDescentFlex.cpp
/// @brief 词法分析的手动实现源文件
/// @author zenglj (zenglj@live.com)
/// @version 1.1
/// @date 2024-11-23
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-21 <td>1.0     <td>zenglj  <td>新做
/// <tr><td>2024-11-23 <td>1.1     <td>zenglj  <td>表达式版增强
/// </table>
///
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>

#include "RecursiveDescentParser.h"
#include "Common.h"

/// @brief 词法分析的行号信息
int64_t rd_line_no = 1;

/// @brief 词法分析的token对应的字符识别
std::string tokenValue;

/// @brief 输入源文件指针
FILE * rd_filein;

/// @brief 关键字与Token类别的数据结构
struct KeywordToken {
    std::string name;
    enum RDTokenType type;
};

/// @brief  关键字与Token对应表
static KeywordToken allKeywords[] = {
    {"int", RDTokenType::T_INT},
    {"return", RDTokenType::T_RETURN},
};

/// @brief 在标识符中检查是否时关键字，若是关键字则返回对应关键字的Token，否则返回T_ID
/// @param id 标识符
/// @return Token
static RDTokenType getKeywordToken(std::string id)
{
    //如果在allkeywords中找到，则说明为关键字
    for (auto & keyword: allKeywords) {
        if (keyword.name == id) {
            return keyword.type;
        }
    }
    // 如果不再allkeywords中，说明是标识符
    return RDTokenType::T_ID;
}

/// @brief 词法文法，获取下一个Token
/// @return  Token，值保存在rd_lval中
int rd_flex()
{
    int c;              // 扫描的字符
    int tokenKind = -1; // Token的值

    // 忽略空白符号，主要有空格，TAB键和换行符
    while ((c = fgetc(rd_filein)) == ' ' || c == '\t' || c == '\n' || c == '\r') {

        // 支持Linux/Windows/Mac系统的行号分析
        // Windows：\r\n
        // Mac: \n
        // Unix(Linux): \r
        if (c == '\r') {
            c = fgetc(rd_filein);
            rd_line_no++;
            if (c != '\n') {
                // 不是\n，则回退
                ungetc(c, rd_filein);
            }
        } else if (c == '\n') {
            rd_line_no++;
        }
    }
	// 注释处理
	if (c == '/') {
    int next = fgetc(rd_filein);
    if (next == '/') {
        // 单行注释
        while ((c = fgetc(rd_filein)) != '\n' && c != EOF);
        rd_line_no++;
        return rd_flex();  // 递归调用继续扫描下一个有效token
    } else if (next == '*') {
        // 多行注释
        int prev = 0;
        while ((c = fgetc(rd_filein)) != EOF) {
            if (c == '\n') rd_line_no++;
            if (prev == '*' && c == '/') break;
            prev = c;
        }
        return rd_flex();
    } else {
        // 除号
        ungetc(next, rd_filein);
    }
}
    // 文件结束符
    if (c == EOF) {
        // 返回文件结束符
        return RDTokenType::T_EOF;
    }

    // TODO 请自行实现删除源文件中的注释，含单行注释和多行注释等

    // 处理数字
    // 处理数字
	if (isdigit(c)) {
    	rd_lval.integer_num.lineno = rd_line_no;

		if (c == '0') {
			c = fgetc(rd_filein);
			if (c == 'x' || c == 'X') {
				// 十六进制
				int val = 0;
				while (isxdigit(c = fgetc(rd_filein))) {
					val = val * 16 + (isdigit(c) ? c - '0' : (tolower(c) - 'a' + 10));
				}
				rd_lval.integer_num.val = val;
				tokenValue = "0x" + tokenValue;
				ungetc(c, rd_filein);
			} else if (isdigit(c)) {
				// 八进制
				int val = 0;
				do {
					val = val * 8 + (c - '0');
					c = fgetc(rd_filein);
				} while (c >= '0' && c <= '7');
				rd_lval.integer_num.val = val;
				tokenValue = "0" + tokenValue;
				ungetc(c, rd_filein);
			} else {
				// just zero
				rd_lval.integer_num.val = 0;
				tokenValue = "0";
				ungetc(c, rd_filein);
			}
			} else {
				// 十进制
				rd_lval.integer_num.val = c - '0';
				while (isdigit(c = fgetc(rd_filein))) {
					rd_lval.integer_num.val = rd_lval.integer_num.val * 10 + c - '0';
				}
				tokenValue = std::to_string(rd_lval.integer_num.val);
				ungetc(c, rd_filein);
			}

    tokenKind = RDTokenType::T_DIGIT;
    } else if (c == '(') {
        // 识别字符(
        tokenKind = RDTokenType::T_L_PAREN;
        // 存储字符(
        tokenValue = "(";
    } else if (c == ')') {
        // 识别字符)
        tokenKind = RDTokenType::T_R_PAREN;
        // 存储字符)
        tokenValue = ")";
    } else if (c == '{') {
        // 识别字符{
        tokenKind = RDTokenType::T_L_BRACE;
        // 存储字符{
        tokenValue = "{";
    } else if (c == '}') {
        // 识别字符}
        tokenKind = RDTokenType::T_R_BRACE;
        // 存储字符}
        tokenValue = "}";
    } else if (c == ';') {
        // 识别字符;
        tokenKind = RDTokenType::T_SEMICOLON;
        // 存储字符;
        tokenValue = ";";
    } else if (c == '+') {
        // 识别字符+
        tokenKind = RDTokenType::T_ADD;
		// 存储字符+
        tokenValue = "+";
    } else if (c == '-') {
        // 识别字符-
        tokenKind = RDTokenType::T_SUB;
		// 存储字符-
        tokenValue = "-";
    } else if (c == '/') {
        // 识别字符/
        tokenKind = RDTokenType::T_DIV;
        // 存储字符/
        tokenValue = "/";
    } else if (c == '*') {
        // 识别字符*
        tokenKind = RDTokenType::T_MUL;
		// 存储字符*
		tokenValue = "*";
	} else if (c == '%') {
		// 识别字符%
		tokenKind = RDTokenType::T_MOD;
		// 存储字符%
        tokenValue = "%";
    } else if (c == '=') {
        // 识别字符=
        tokenKind = RDTokenType::T_ASSIGN;
    }  else if (c == ',') {
        // 识别字符;
        tokenKind = RDTokenType::T_COMMA;
		// 存储字符,
        tokenValue = ",";
    } else if (isLetterUnderLine(c)) {
        // 识别标识符，包含关键字/保留字或自定义标识符

        // 最长匹配标识符
        std::string name;

        do {
            // 记录字符
            name.push_back(c);
            c = fgetc(rd_filein);
        } while (isLetterDigitalUnderLine(c));

        // 存储标识符
        tokenValue = name;

        // 多读的字符恢复，下次可继续读到该字符
        ungetc(c, rd_filein);

        // 检查是否是关键字，若是则返回对应的Token，否则返回T_ID
        tokenKind = getKeywordToken(name);
        if (tokenKind == RDTokenType::T_ID) {
            // 自定义标识符

            // 设置ID的值
            rd_lval.var_id.id = strdup(name.c_str());

            // 设置行号
            rd_lval.var_id.lineno = rd_line_no;
        } else if (tokenKind == RDTokenType::T_INT) {
            // int关键字

            // 设置类型与行号
            rd_lval.type.type = BasicType::TYPE_INT;
            rd_lval.type.lineno = rd_line_no;
        }
    } else {
        printf("Line(%lld): Invalid char %s\n", (long long) rd_line_no, tokenValue.c_str());
        tokenKind = RDTokenType::T_ERR;
    }

    // Token的类别
    return tokenKind;
}
