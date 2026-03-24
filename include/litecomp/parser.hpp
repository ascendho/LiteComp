#pragma once

#include <functional>
#include <map>
#include "litecomp/ast.hpp"
#include "litecomp/lexer.hpp"
#include "litecomp/token.hpp"

struct Parser;

/**
 * @brief 函数类型定义（Pratt 解析的核心）
 * prefixParseFn: 前缀解析函数。当 Token 出现在表达式开头时调用（如：-5, !true, x, (表达式)）。
 * infixParseFn:  中缀解析函数。当 Token 出现在两个表达式中间或后面时调用（如：5 + 5, a[i], call()）。
 *                它接收一个左侧已经解析好的表达式作为参数。
 */
typedef std::function<std::shared_ptr<Expression>(Parser *)> prefixParseFn;
typedef std::function<std::shared_ptr<Expression>(Parser *, std::shared_ptr<Expression>)> infixParseFn;

/**
 * @brief 运算符优先级定义（从低到高）
 */
enum class Precedence {
    DEFAULT,            // 最低优先级
    EQUALITY,           // == 或 !=
    UNEQUALCOMPARISON,  // > 或 < 或 >= 或 <=
    ADDITION,           // + 或 -
    MULTIPLICATION,     // * 或 /
    PREFIX,             // 前缀运算符（-X 或 !X）
    INVOKE,             // 函数调用（func()）
    INDEX               // 索引访问（array[index], hash[key]）
};

/**
 * @brief 语法解析器结构体
 * 负责执行递归下降解析，并将 Token 流转化为 AST。
 */
struct Parser {
    Token curToken;     // 当前正在处理的词法单元
    Token nextToken;    // 下一个词法单元（用于“窥探” Peek）

    // 禁止默认构造，必须传入 Lexer
    Parser() = delete;
    explicit Parser(Lexer &&lexer);

    // 禁止拷贝与移动赋值，确保解析状态唯一
    Parser(const Parser &other) = delete;
    Parser(Parser &&other) noexcept = delete;
    Parser &operator=(const Parser &other) = delete;
    Parser &operator=(Parser &&other) noexcept = delete;

    /**
     * @brief 推进 Token
     * 将 curToken 更新为 nextToken，并将 nextToken 更新为 Lexer 的下一个扫描结果。
     */
    void move();

    // =========================================================================
    // 语句解析 (Statement Parsing)
    // =========================================================================

    std::shared_ptr<DeclareStatement> parseDeclarationStatement();   // 解析变量声明: let x = ...;
    std::shared_ptr<ReturnStatement> parseReturnStatement();         // 解析返回语句: return ...;
    std::shared_ptr<ExpressionStatement> parseExpressionStatement(); // 解析表达式语句: x + 5;
    std::shared_ptr<BlockStatement> parseBlockStatement();           // 解析代码块: { ... }
    std::shared_ptr<Statement> parseStatement();                     // 解析单条语句（分发器）
    std::shared_ptr<Program> parseProgram();                         // 解析整个程序（根节点）

    // =========================================================================
    // 表达式解析 (Expression Parsing - Pratt 算法核心)
    // =========================================================================

    /**
     * @brief 解析表达式的核心入口
     * @param precedence 当前解析环境的优先级，用于决定何时停止解析并返回。
     */
    std::shared_ptr<Expression> parseExpression(Precedence precedence);

    // 解析以特定 TokenType 结尾的表达式列表（用于数组元素或函数参数）
    std::vector<std::shared_ptr<Expression>> parseExpressionList(TokenType end);

    std::shared_ptr<Expression> parseIdentifier() const;           // 解析标识符 (x)
    std::shared_ptr<Expression> parseIntegerLiteral();             // 解析整数 (5)
    std::shared_ptr<Expression> parseBoolean() const;              // 解析布尔值 (true/false)
    std::shared_ptr<Expression> parseStringLiteral() const;        // 解析字符串 ("hello")
    std::shared_ptr<Expression> parseArrayLiteral();               // 解析数组字面量 [1, 2]
    std::shared_ptr<Expression> parseHashLiteral();                // 解析哈希字面量 {key: value}
    std::shared_ptr<Expression> parsePrefixExpression();           // 解析前缀表达式 (!x, -5)
    std::shared_ptr<Expression> parseGroupedExpression();          // 解析括号分组 (1 + 2)
    std::shared_ptr<Expression> parseConditionExpression();        // 解析 If 表达式

    // 解析函数相关
    std::vector<std::shared_ptr<Identifier>> parseFunctionParameters(); // 解析函数形参列表
    std::shared_ptr<Expression> parseFunctionLiteral();                 // 解析函数定义 fn(x) { ... }
    // 解析函数调用 func()
    std::shared_ptr<Expression> parseInvokeExpression(std::shared_ptr<Expression> function); 

    // 中缀解析（处理二元运算）
    std::shared_ptr<Expression> parseInfixExpression(std::shared_ptr<Expression> left); // 解析 5 + 5
    std::shared_ptr<Expression> parseIndexExpression(std::shared_ptr<Expression> left); // 解析 array[0]

    // =========================================================================
    // 辅助工具与错误处理
    // =========================================================================

    std::vector<std::string> errors() const;    // 获取所有解析错误信息
    bool curTokenIs(TokenType t) const;         // 判断当前 Token 类型
    bool nextTokenIs(TokenType t) const;        // 判断下一个 Token 类型

    /**
     * @brief 断言下一个 Token 类型
     * 如果下一个 Token 是期望类型，则 move() 推进并返回 true；
     * 否则记录错误并返回 false。常用于解析如 `let` 后面必须接 `IDENT` 的场景。
     */
    bool expectPeek(TokenType t);

    void eyeError(TokenType t);                 // 记录“期望 Token 与实际不符”的错误
    Precedence eyePrecedence() const;           // 获取 nextToken 的优先级
    Precedence curPrecedence() const;           // 获取 curToken 的优先级
    void noPrefixParseFnError(TokenType t);     // 记录“缺少前缀解析函数”的错误

    /**
     * @brief 注册解析函数
     * 在构造函数中调用，将特定的 Token 类型与对应的解析逻辑绑定。
     */
    void bindPrefixFunc(TokenType token_type, prefixParseFn fn);
    void bindInfixFunc(TokenType token_type, infixParseFn fn);

private:
    Lexer l;                                    // 词法分析器实例
    std::vector<std::string> e;                 // 错误日志

    // Pratt 解析函数映射表
    std::map<TokenType, prefixParseFn> prefixParseFncs; 
    std::map<TokenType, infixParseFn> infixParseFncs;   
};