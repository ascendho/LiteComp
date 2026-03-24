#include <iostream>
#include <memory>
#include "litecomp/parser.hpp"

/**
 * @brief 优先级表
 * 定义了不同 Token 类型对应的优先级。
 * 只有中缀运算符和部分后缀运算符（如索引、调用）需要在这里定义优先级。
 */
std::map<TokenType, Precedence> precedences = {
        {TokenType::EQUAL,     Precedence::EQUALITY},           // ==
        {TokenType::NOTEQUAL,  Precedence::EQUALITY},           // !=
        {TokenType::LESS,      Precedence::UNEQUALCOMPARISON},  // <
        {TokenType::GREAT,     Precedence::UNEQUALCOMPARISON},  // >
        {TokenType::PLUS,      Precedence::ADDITION},           // +
        {TokenType::MINUS,     Precedence::ADDITION},           // -
        {TokenType::SLASH,     Precedence::MULTIPLICATION},     // /
        {TokenType::ASTERISK,  Precedence::MULTIPLICATION},     // *
        {TokenType::LBRACKET,  Precedence::INVOKE},             // ( 函数调用
        {TokenType::LSBRACKET, Precedence::INDEX},              // [ 索引访问
};

/**
 * @brief 构造函数：初始化解析器并注册解析函数
 */
Parser::Parser(Lexer &&lexer) : curToken{}, nextToken{}, l{std::move(lexer)} {
    // --- 注册前缀解析函数 (Prefix) ---
    // 当这些 Token 出现在表达式开头时，调用相应的处理函数
    bindPrefixFunc(TokenType::IDENTIFIER, std::mem_fn(&Parser::parseIdentifier));       // 变量名
    bindPrefixFunc(TokenType::INTEGER, std::mem_fn(&Parser::parseIntegerLiteral));      // 整数
    bindPrefixFunc(TokenType::BANG, std::mem_fn(&Parser::parsePrefixExpression));       // !
    bindPrefixFunc(TokenType::MINUS, std::mem_fn(&Parser::parsePrefixExpression));      // -
    bindPrefixFunc(TokenType::TRUE, std::mem_fn(&Parser::parseBoolean));                // true
    bindPrefixFunc(TokenType::FALSE, std::mem_fn(&Parser::parseBoolean));               // false
    bindPrefixFunc(TokenType::LBRACKET, std::mem_fn(&Parser::parseGroupedExpression));  // ( 括号分组
    bindPrefixFunc(TokenType::IF, std::mem_fn(&Parser::parseConditionExpression));      // if
    bindPrefixFunc(TokenType::FUNCTION, std::mem_fn(&Parser::parseFunctionLiteral));    // fn
    bindPrefixFunc(TokenType::STRING, std::mem_fn(&Parser::parseStringLiteral));        // "string"
    bindPrefixFunc(TokenType::LSBRACKET, std::mem_fn(&Parser::parseArrayLiteral));      // [ 数组字面量
    bindPrefixFunc(TokenType::LCBRACKET, std::mem_fn(&Parser::parseHashLiteral));       // { 哈希字面量

    // --- 注册中缀解析函数 (Infix) ---
    // 当这些 Token 出现在左侧表达式之后时，调用相应的处理函数
    bindInfixFunc(TokenType::PLUS, std::mem_fn(&Parser::parseInfixExpression));      // +
    bindInfixFunc(TokenType::MINUS, std::mem_fn(&Parser::parseInfixExpression));     // -
    bindInfixFunc(TokenType::SLASH, std::mem_fn(&Parser::parseInfixExpression));     // /
    bindInfixFunc(TokenType::ASTERISK, std::mem_fn(&Parser::parseInfixExpression));  // *
    bindInfixFunc(TokenType::EQUAL, std::mem_fn(&Parser::parseInfixExpression));     // ==
    bindInfixFunc(TokenType::NOTEQUAL, std::mem_fn(&Parser::parseInfixExpression));  // !=
    bindInfixFunc(TokenType::LESS, std::mem_fn(&Parser::parseInfixExpression));      // <
    bindInfixFunc(TokenType::GREAT, std::mem_fn(&Parser::parseInfixExpression));     // >
    bindInfixFunc(TokenType::LBRACKET, std::mem_fn(&Parser::parseInvokeExpression)); // ( 函数调用
    bindInfixFunc(TokenType::LSBRACKET, std::mem_fn(&Parser::parseIndexExpression)); // [ 索引访问

    // 预读两个 Token，填充 curToken 和 nextToken
    move();
    move();
}

/**
 * @brief 推进 Token 流
 */
void Parser::move() {
    curToken = nextToken;
    nextToken = l.scan();
}

/**
 * @brief 解析声明语句：let <标识符> = <表达式>;
 */
std::shared_ptr<DeclareStatement> Parser::parseDeclarationStatement() {
    auto stmt = std::make_shared<DeclareStatement>(DeclareStatement{curToken});

    // 期望下一个是标识符（变量名）
    if (!expectPeek(TokenType::IDENTIFIER)) return nullptr;
    stmt->name = std::make_shared<Identifier>(Identifier{curToken, curToken.literal});

    // 期望下一个是等号
    if (!expectPeek(TokenType::ASSIGN)) return nullptr;

    move(); // 跳过等号

    // 解析右值表达式
    stmt->value = parseExpression(Precedence::DEFAULT);

    // 如果声明的是函数，尝试记录函数名（用于递归支持）
    if (auto fl = std::dynamic_pointer_cast<FuncLiteral>(stmt->value)) {
        fl->name = stmt->name->value;
    }

    // 消耗可选的分号
    if (nextTokenIs(TokenType::SEMICOLON)) move();

    return stmt;
}

/**
 * @brief 解析返回语句：return <表达式>;
 */
std::shared_ptr<ReturnStatement> Parser::parseReturnStatement() {
    auto stmt = std::make_shared<ReturnStatement>(ReturnStatement{curToken});
    move(); // 跳过 return
    stmt->return_value = parseExpression(Precedence::DEFAULT);

    if (nextTokenIs(TokenType::SEMICOLON)) move();
    return stmt;
}

/**
 * @brief 解析表达式语句
 * 很多语言允许直接写表达式作为语句（如函数调用 add(1, 2);）
 */
std::shared_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
    auto stmt = std::make_shared<ExpressionStatement>(ExpressionStatement{curToken});
    stmt->expression = parseExpression(Precedence::DEFAULT);

    if (nextTokenIs(TokenType::SEMICOLON)) move();
    return stmt;
}

/**
 * @brief 解析代码块：{ stmt1; stmt2; }
 */
std::shared_ptr<BlockStatement> Parser::parseBlockStatement() {
    auto block = std::make_shared<BlockStatement>(BlockStatement{curToken});
    move(); // 跳过 {

    // 循环解析直到遇到 } 或文件末尾
    while (!curTokenIs(TokenType::RCBRACKET) && !curTokenIs(TokenType::ENDOFFILE)) {
        auto stmt = parseStatement();
        if (stmt) block->statements.push_back(std::move(stmt));
        move();
    }
    return block;
}

/**
 * @brief 语句解析入口
 */
std::shared_ptr<Statement> Parser::parseStatement() {
    switch (curToken.type) {
        case (TokenType::DECLARE): // let
            return parseDeclarationStatement();
        case (TokenType::RETURN):  // return
            return parseReturnStatement();
        default:
            return parseExpressionStatement();
    }
}

/**
 * @brief Pratt 解析核心逻辑：解析表达式
 * @param precedence 当前解析器的优先级。如果下一个 Token 优先级更高，则继续向右解析。
 */
std::shared_ptr<Expression> Parser::parseExpression(Precedence precedence) {
    // 1. 查找前缀解析函数
    auto pcontains = prefixParseFncs.find(curToken.type);
    if (pcontains == prefixParseFncs.end()) {
        noPrefixParseFnError(curToken.type);
        return nullptr;
    }

    auto prefix = prefixParseFncs[curToken.type];

     // 解析“左侧”部分（如数字、标识符或前缀表达式）
    auto left_exp = prefix(this);

    // 2. 只要下一个 Token 的优先级高于当前优先级，就尝试将其作为中缀表达式处理
    while (!nextTokenIs(TokenType::SEMICOLON) && precedence < eyePrecedence()) {
        auto icontains = infixParseFncs.find(nextToken.type);
        if (icontains == infixParseFncs.end()) return left_exp;

        auto infix = infixParseFncs[nextToken.type];

        // 移动到中缀运算符（如 +）
        move(); 

        // 调用中缀函数，将当前的 left_exp 传入，形成更深的树结构
        left_exp = infix(this, left_exp);
    }

    return left_exp;
}

/**
 * @brief 解析以逗号分隔的表达式列表（用于函数参数、数组元素）
 */
std::vector<std::shared_ptr<Expression>> Parser::parseExpressionList(TokenType end) {
    std::vector<std::shared_ptr<Expression>> list;

    // 空列表
    if (nextTokenIs(end)) { 
        move();
        return list;
    }

    move();
    list.push_back(parseExpression(Precedence::DEFAULT));

    while (nextTokenIs(TokenType::COMMA)) {
        move(); // 消耗逗号
        move(); // 准备解析下一个表达式
        list.push_back(parseExpression(Precedence::DEFAULT));
    }

    if (!expectPeek(end)) return std::vector<std::shared_ptr<Expression>>{};
    return list;
}

/**
 * @brief 解析标识符 (前缀)
 */
std::shared_ptr<Expression> Parser::parseIdentifier() const {
    return std::make_shared<Identifier>(Identifier{curToken, curToken.literal});
}

/**
 * @brief 解析整数字面量 (前缀)
 */
std::shared_ptr<Expression> Parser::parseIntegerLiteral() {
    auto lit = std::make_shared<IntegerLiteral>(IntegerLiteral{curToken});

    std::size_t pos{};
    try {
        lit->value = std::stoi(curToken.literal, &pos);
    } catch (...) {
        e.push_back("Could not parse " + curToken.literal + " as integer.");
        return nullptr;
    }
    return lit;
}

/**
 * @brief 解析函数形式参数：(a, b, c)
 */
std::vector<std::shared_ptr<Identifier>> Parser::parseFunctionParameters() {
    std::vector<std::shared_ptr<Identifier>> identifiers;

    if (nextTokenIs(TokenType::RBRACKET)) {
        move();
        return identifiers;
    }

    move();
    identifiers.push_back(std::make_shared<Identifier>(Identifier{curToken, curToken.literal}));

    while (nextTokenIs(TokenType::COMMA)) {
        move(); move();
        identifiers.push_back(std::make_shared<Identifier>(Identifier{curToken, curToken.literal}));
    }

    if (!expectPeek(TokenType::RBRACKET)) return {};
    return identifiers;
}

/**
 * @brief 解析函数字面量：fn(params) { body }
 */
std::shared_ptr<Expression> Parser::parseFunctionLiteral() {
    auto lit = std::make_shared<FuncLiteral>(FuncLiteral{curToken});

    if (!expectPeek(TokenType::LBRACKET)) return nullptr;
    lit->parameters = parseFunctionParameters();

    if (!expectPeek(TokenType::LCBRACKET)) return nullptr;
    lit->body = parseBlockStatement();

    return lit;
}

/**
 * @brief 解析布尔值 (前缀)
 */
std::shared_ptr<Expression> Parser::parseBoolean() const {
    return std::make_shared<BooleanLiteral>(BooleanLiteral{curToken, curTokenIs(TokenType::TRUE)});
}

/**
 * @brief 解析字符串 (前缀)
 */
std::shared_ptr<Expression> Parser::parseStringLiteral() const {
    return std::make_shared<StringLiteral>(StringLiteral{curToken, curToken.literal});
}

/**
 * @brief 解析数组字面量 (前缀)：[1, 2, 3]
 */
std::shared_ptr<Expression> Parser::parseArrayLiteral() {
    auto array = std::make_shared<ArrayLiteral>(ArrayLiteral{curToken});
    array->elements = parseExpressionList(TokenType::RSBRACKET);
    return array;
}

/**
 * @brief 解析哈希/对象字面量 (前缀)：{ "key": value }
 */
std::shared_ptr<Expression> Parser::parseHashLiteral() {
    auto hash = std::make_shared<HashLiteral>(HashLiteral{curToken});

    while (!nextTokenIs(TokenType::RCBRACKET)) {
        // 移动到键并解析
        move(); 
        auto key = parseExpression(Precedence::DEFAULT);

        // 期待冒号
        if (!expectPeek(TokenType::COLON)) return nullptr; 

        // 移动到值并解析
        move(); 
        auto value = parseExpression(Precedence::DEFAULT);
        hash->pairs[std::move(key)] = std::move(value);

        // 如果后面不是 }，则必须有逗号
        if (!nextTokenIs(TokenType::RCBRACKET) && !expectPeek(TokenType::COMMA)) {
            return nullptr;
        }
    }

    if (!expectPeek(TokenType::RCBRACKET)) return nullptr;
    return hash;
}

/**
 * @brief 解析前缀表达式 (前缀)：!true, -5
 */
std::shared_ptr<Expression> Parser::parsePrefixExpression() {
    auto expression = std::make_shared<PrefixExpression>(PrefixExpression{curToken, curToken.literal});
    move();
    // 使用 PREFIX 优先级解析右侧内容
    expression->right = parseExpression(Precedence::PREFIX);
    return expression;
}

/**
 * @brief 解析索引表达式 (中缀)：array[0]
 */
std::shared_ptr<Expression> Parser::parseIndexExpression(std::shared_ptr<Expression> left) {
    auto exp = std::make_shared<IndexExpression>(IndexExpression{curToken});
    exp->left = std::move(left);
    move(); // 跳过 [
    exp->index = parseExpression(Precedence::DEFAULT);

    if (!expectPeek(TokenType::RSBRACKET)) return nullptr;
    return exp;
}

/**
 * @brief 解析二元中缀表达式 (中缀)：5 + 5
 */
std::shared_ptr<Expression> Parser::parseInfixExpression(std::shared_ptr<Expression> left) {
    auto expression = std::make_shared<InfixExpression>(InfixExpression{curToken, curToken.literal});
    expression->left = std::move(left);

    auto precedence = curPrecedence();
    move();
    // 递归调用，并将当前优先级传入，处理运算符结合性
    expression->right = parseExpression(precedence);

    return expression;
}

/**
 * @brief 解析函数调用 (中缀)：add(1, 2)
 */
std::shared_ptr<Expression> Parser::parseInvokeExpression(std::shared_ptr<Expression> function) {
    auto exp = std::make_shared<CallExpression>(CallExpression{curToken, function});
    exp->arguments = parseExpressionList(TokenType::RBRACKET);
    return exp;
}

/**
 * @brief 解析括号分组表达式 (前缀)：(1 + 2)
 */
std::shared_ptr<Expression> Parser::parseGroupedExpression() {
    // 跳过 (
    move(); 
    auto exp = parseExpression(Precedence::DEFAULT);
    if (!expectPeek(TokenType::RBRACKET)) return nullptr;
    return exp;
}

/**
 * @brief 解析 If 表达式 (前缀)：if (condition) { consequence } else { alternative }
 */
std::shared_ptr<Expression> Parser::parseConditionExpression() {
    auto expression = std::make_shared<IfExpression>(IfExpression{curToken});

    if (!expectPeek(TokenType::LBRACKET)) return nullptr; // (
    move();
    expression->condition = parseExpression(Precedence::DEFAULT);
    if (!expectPeek(TokenType::RBRACKET)) return nullptr; // )

    if (!expectPeek(TokenType::LCBRACKET)) return nullptr; // {
    expression->consequence = parseBlockStatement();

    if (nextTokenIs(TokenType::ELSE)) {
        // 跳过 else
        move(); 
        if (!expectPeek(TokenType::LCBRACKET)) return nullptr; // {
        expression->alternative = parseBlockStatement();
    }

    return expression;
}

/**
 * @brief 解析整个程序入口
 */
std::shared_ptr<Program> Parser::parseProgram() {
    auto program = std::make_shared<Program>(Program{});
    while (curToken.type != TokenType::ENDOFFILE) {
        auto stmt = parseStatement();
        if (stmt) program->statements.push_back(std::move(stmt));
        move();
    }
    return program;
}

// =========================================================================
// 辅助函数
// =========================================================================

/**
 * @brief 断言并推进
 * 如果下一个 Token 类型匹配，则推进到该 Token 并返回 true；否则报错返回 false。
 */
bool Parser::expectPeek(TokenType t) {
    if (nextTokenIs(t)) {
        move();
        return true;
    } else {
        eyeError(t);
        return false;
    }
}

/**
 * @brief 记录“期望 Token 与实际不符”的错误
 */
void Parser::eyeError(TokenType t) {
    std::string msg = "Expected " + searchToken(t) + ", got " + searchToken(nextToken.type);
    e.push_back(msg);
}

/**
 * @brief 查看下一个 Token 的优先级
 */
Precedence Parser::eyePrecedence() const {
    auto precedence = precedences.find(nextToken.type);
    if (precedence == precedences.end()) return Precedence::DEFAULT;
    return precedence->second;
}

/**
 * @brief 查看当前 Token 的优先级
 */
Precedence Parser::curPrecedence() const {
    auto precedence = precedences.find(curToken.type);
    if (precedence == precedences.end()) return Precedence::DEFAULT;
    return precedence->second;
}

void Parser::noPrefixParseFnError(TokenType t) {
    e.push_back("No prefix parse function for " + searchToken(t));
}

void Parser::bindPrefixFunc(TokenType token_type, prefixParseFn fn) {
    prefixParseFncs[token_type] = std::move(fn);
}

void Parser::bindInfixFunc(TokenType token_type, infixParseFn fn) {
    infixParseFncs[token_type] = std::move(fn);
}

std::vector<std::string> Parser::errors() const { return e; }
bool Parser::curTokenIs(TokenType t) const { return curToken.type == t; }
bool Parser::nextTokenIs(TokenType t) const { return nextToken.type == t; }