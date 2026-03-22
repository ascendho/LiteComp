#ifndef PARSER_HPP
#define PARSER_HPP

#include <functional>
#include <map>
#include "litecomp/ast.hpp"
#include "litecomp/lexer.hpp"
#include "litecomp/token.hpp"

struct Parser;

typedef std::function<std::shared_ptr<Expression>(Parser *)> prefixParseFn;                              // 前缀解析函数
typedef std::function<std::shared_ptr<Expression>(Parser *, std::shared_ptr<Expression>)> infixParseFn;  // 中缀解析函数

// 优先级
enum class Precedence {
    DEFAULT,            // 默认的最低的优先级
    EQUALITY,           // ==
    UNEQUALCOMPARISON,  // > or <
    ADDITION,           // 加法
    MULTIPLICATION,     // 乘法
    PREFIX,             // -X或者!X
    INVOKE,             // 函数调用
    INDEX               // 数组、哈希表
};

struct Parser {
    Token curToken;     // 当前词法单元
    Token nextToken;    // 下一个词法单元

    Parser() = delete;

    explicit Parser(Lexer &&lexer);

    Parser(const Parser &other) = delete;

    Parser(Parser &&other) noexcept = delete;

    Parser &operator=(const Parser &other) = delete;

    Parser &operator=(Parser &&other) noexcept = delete;

    void move();

    std::shared_ptr<DeclareStatement> parseDeclarationStatement();

    std::shared_ptr<ReturnStatement> parseReturnStatement();

    std::shared_ptr<ExpressionStatement> parseExpressionStatement();

    std::shared_ptr<BlockStatement> parseBlockStatement();

    std::shared_ptr<Statement> parseStatement();

    std::shared_ptr<Expression> parseExpression(Precedence precedence);

    std::vector<std::shared_ptr<Expression>> parseExpressionList(TokenType end);

    std::shared_ptr<Expression> parseIdentifier() const;

    std::shared_ptr<Expression> parseIntegerLiteral();

    std::vector<std::shared_ptr<Identifier>> parseFunctionParameters();

    std::shared_ptr<Expression> parseFunctionLiteral();

    std::shared_ptr<Expression> parseBoolean() const;

    std::shared_ptr<Expression> parseStringLiteral() const;

    std::shared_ptr<Expression> parseArrayLiteral();

    std::shared_ptr<Expression> parseHashLiteral();

    std::shared_ptr<Expression> parsePrefixExpression();

    std::shared_ptr<Expression> parseIndexExpression(std::shared_ptr<Expression> left);

    std::shared_ptr<Expression> parseInfixExpression(std::shared_ptr<Expression> left);

    std::shared_ptr<Expression> parseInvokeExpression(std::shared_ptr<Expression> function);

    std::shared_ptr<Expression> parseGroupedExpression();

    std::shared_ptr<Expression> parseConditionExpression();

    std::shared_ptr<Program> parseProgram();

    std::vector<std::string> errors() const;

    bool curTokenIs(TokenType t) const;

    bool nextTokenIs(TokenType t) const;

    bool expectPeek(TokenType t);

    void eyeError(TokenType t);

    Precedence eyePrecedence() const;

    Precedence curPrecedence() const;

    void noPrefixParseFnError(TokenType t);

    // 绑定前缀函数
    void bindPrefixFunc(TokenType token_type, prefixParseFn fn);

    // 绑定中缀函数
    void bindInfixFunc(TokenType token_type, infixParseFn fn);

private:
    Lexer l;                      // 词法分析器
    std::vector<std::string> e;   // 错误信息向量

    std::map<TokenType, prefixParseFn> prefixParseFncs;   // 前缀解析函数哈希表
    std::map<TokenType, infixParseFn> infixParseFncs;     // 中缀解析函数哈希表
};

#endif