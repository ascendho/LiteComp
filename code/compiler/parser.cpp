#include <iostream>
#include <memory>
#include "parser.hpp"

// 优先级哈希表
std::map<TokenType, Precedence> precedences = {
        {TokenType::EQUAL,     Precedence::EQUALITY},
        {TokenType::NOTEQUAL,  Precedence::EQUALITY},
        {TokenType::LESS,      Precedence::UNEQUALCOMPARISON},
        {TokenType::GREAT,     Precedence::UNEQUALCOMPARISON},
        {TokenType::PLUS,      Precedence::ADDITION},
        {TokenType::MINUS,     Precedence::ADDITION},
        {TokenType::SLASH,     Precedence::MULTIPLICATION},
        {TokenType::ASTERISK,  Precedence::MULTIPLICATION},
        {TokenType::LBRACKET,  Precedence::INVOKE},
        {TokenType::LSBRACKET, Precedence::INDEX},
};

Parser::Parser(Lexer &&lexer) : curToken{}, nextToken{}, l{std::move(lexer)} {
    // 绑定前缀解析函数
    bindPrefixFunc(TokenType::IDENTIFIER, std::mem_fn(&Parser::parseIdentifier));
    bindPrefixFunc(TokenType::INTEGER, std::mem_fn(&Parser::parseIntegerLiteral));
    bindPrefixFunc(TokenType::BANG, std::mem_fn(&Parser::parsePrefixExpression));
    bindPrefixFunc(TokenType::MINUS, std::mem_fn(&Parser::parsePrefixExpression));
    bindPrefixFunc(TokenType::TRUE, std::mem_fn(&Parser::parseBoolean));
    bindPrefixFunc(TokenType::FALSE, std::mem_fn(&Parser::parseBoolean));
    bindPrefixFunc(TokenType::LBRACKET, std::mem_fn(&Parser::parseGroupedExpression));
    bindPrefixFunc(TokenType::IF, std::mem_fn(&Parser::parseConditionExpression));
    bindPrefixFunc(TokenType::FUNCTION, std::mem_fn(&Parser::parseFunctionLiteral));
    bindPrefixFunc(TokenType::STRING, std::mem_fn(&Parser::parseStringLiteral));
    bindPrefixFunc(TokenType::LSBRACKET, std::mem_fn(&Parser::parseArrayLiteral));
    bindPrefixFunc(TokenType::LCBRACKET, std::mem_fn(&Parser::parseHashLiteral));

    // 绑定中缀解析函数
    bindInfixFunc(TokenType::PLUS, std::mem_fn(&Parser::parseInfixExpression));
    bindInfixFunc(TokenType::MINUS, std::mem_fn(&Parser::parseInfixExpression));
    bindInfixFunc(TokenType::SLASH, std::mem_fn(&Parser::parseInfixExpression));
    bindInfixFunc(TokenType::ASTERISK, std::mem_fn(&Parser::parseInfixExpression));
    bindInfixFunc(TokenType::EQUAL, std::mem_fn(&Parser::parseInfixExpression));
    bindInfixFunc(TokenType::NOTEQUAL, std::mem_fn(&Parser::parseInfixExpression));
    bindInfixFunc(TokenType::LESS, std::mem_fn(&Parser::parseInfixExpression));
    bindInfixFunc(TokenType::GREAT, std::mem_fn(&Parser::parseInfixExpression));
    bindInfixFunc(TokenType::LBRACKET, std::mem_fn(&Parser::parseInvokeExpression));
    bindInfixFunc(TokenType::LSBRACKET, std::mem_fn(&Parser::parseIndexExpression));

    // 初始化
    move();
    move();
}

void Parser::move() {
    curToken = nextToken;
    nextToken = l.scan();
}

std::shared_ptr<DeclareStatement> Parser::parseDeclarationStatement() {
    auto stmt = std::make_shared<DeclareStatement>(DeclareStatement{curToken});

    if (!expectPeek(TokenType::IDENTIFIER)) {
        return nullptr;
    }

    stmt->name = std::make_shared<Identifier>(Identifier{curToken, curToken.literal});

    if (!expectPeek(TokenType::ASSIGN)) {
        return nullptr;
    }

    move();

    stmt->value = parseExpression(Precedence::DEFAULT);

    if (auto fl = std::dynamic_pointer_cast<FuncLiteral>(stmt->value)) {
        fl->name = stmt->name->value;
    }

    if (nextTokenIs(TokenType::SEMICOLON)) {
        move();
    }

    return stmt;
}

std::shared_ptr<ReturnStatement> Parser::parseReturnStatement() {
    auto stmt = std::make_shared<ReturnStatement>(ReturnStatement{curToken});

    move();

    stmt->return_value = parseExpression(Precedence::DEFAULT);

    if (nextTokenIs(TokenType::SEMICOLON)) {
        move();
    }

    return stmt;
}

std::shared_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
    auto stmt = std::make_shared<ExpressionStatement>(ExpressionStatement{curToken});

    stmt->expression = parseExpression(Precedence::DEFAULT);

    if (nextTokenIs(TokenType::SEMICOLON)) {
        move();
    }

    return stmt;
}

std::shared_ptr<BlockStatement> Parser::parseBlockStatement() {
    auto block = std::make_shared<BlockStatement>(BlockStatement{curToken});

    move();

    while (!curTokenIs(TokenType::RCBRACKET) && !curTokenIs(TokenType::ENDOFFILE)) {
        auto stmt = parseStatement();

        if (stmt) {
            block->statements.push_back(std::move(stmt));
        }

        move();
    }

    return block;
}

std::shared_ptr<Statement> Parser::parseStatement() {
    switch (curToken.type) {
        case (TokenType::DECLARE):
            return parseDeclarationStatement();
        case (TokenType::RETURN):
            return parseReturnStatement();
        default:
            return parseExpressionStatement();
    }
}

std::shared_ptr<Expression> Parser::parseExpression(Precedence precedence) {
    // 搜索前缀解析函数
    auto pcontains = prefixParseFncs.find(curToken.type);
    if (pcontains == prefixParseFncs.end()) {
//        noPrefixParseFnError(curToken.type);
        return nullptr;
    }

    auto prefix = prefixParseFncs[curToken.type];

    // 调用前缀解析函数
    auto left_exp = prefix(this);

    while (!nextTokenIs(TokenType::SEMICOLON) && precedence < eyePrecedence()) {
        // 查找中缀解析函数
        auto icontains = infixParseFncs.find(nextToken.type);

        // 无匹配的中缀解析函数
        if (icontains == infixParseFncs.end()) {
            return left_exp;
        }

        // 搜索中缀解析函数
        auto infix = infixParseFncs[nextToken.type];

        move();

        // 调用中缀解析函数
        left_exp = infix(this, left_exp);
    }

    return left_exp;
}

std::vector<std::shared_ptr<Expression>> Parser::parseExpressionList(TokenType end) {
    std::vector<std::shared_ptr<Expression>> list;

    if (nextTokenIs(end)) {
        move();
        return list;
    }

    move();
    list.push_back(parseExpression(Precedence::DEFAULT));

    while (nextTokenIs(TokenType::COMMA)) {
        move();
        move();
        list.push_back(parseExpression(Precedence::DEFAULT));
    }

    if (!expectPeek(end)) {
        return std::vector<std::shared_ptr<Expression>>{};
    }

    return list;
}

std::shared_ptr<Expression> Parser::parseIdentifier() const {
    return std::make_shared<Identifier>(Identifier{curToken, curToken.literal});
}

std::shared_ptr<Expression> Parser::parseIntegerLiteral() {
    auto lit = std::make_shared<IntegerLiteral>(IntegerLiteral{curToken});

    // 尝试解析整数字面量
    std::size_t pos{};
    try {
        // 字符串转化为整型
        lit->value = std::stoi(curToken.literal, &pos);
    } catch (std::invalid_argument const &ex) {
        std::string msg = "Invalid Argument Parsing " + std::string(ex.what()) + " as INT.";
        e.push_back(msg);
        return nullptr;
    } catch (std::out_of_range const &ex) {
        // 整型太大了，超出了计算机能支持的范围
        std::string msg = "Integer Literal " + std::string(ex.what()) + " to be parsed out of range.";
        e.push_back(msg);
        return nullptr;
    }

    return lit;
}

std::vector<std::shared_ptr<Identifier>> Parser::parseFunctionParameters() {
    std::vector<std::shared_ptr<Identifier>> identifiers;

    if (nextTokenIs(TokenType::RBRACKET)) {
        move();
        return identifiers;
    }

    move();

    auto ident = std::make_shared<Identifier>(Identifier{curToken, curToken.literal});
    identifiers.push_back(ident);

    while (nextTokenIs(TokenType::COMMA)) {
        move();
        move();
        ident = std::make_shared<Identifier>(Identifier{curToken, curToken.literal});
        identifiers.push_back(ident);
    }

    if (!expectPeek(TokenType::RBRACKET)) {
        return std::vector<std::shared_ptr<Identifier>>{};
    }

    return identifiers;
}

std::shared_ptr<Expression> Parser::parseFunctionLiteral() {
    auto lit = std::make_shared<FuncLiteral>(FuncLiteral{curToken});

    if (!expectPeek(TokenType::LBRACKET)) {
        return nullptr;
    }

    lit->parameters = parseFunctionParameters();

    if (!expectPeek(TokenType::LCBRACKET)) {
        return nullptr;
    }

    lit->body = parseBlockStatement();

    return lit;
}

std::shared_ptr<Expression> Parser::parseBoolean() const {
    return std::make_shared<BooleanLiteral>(BooleanLiteral{curToken, curTokenIs(TokenType::TRUE)});
}

std::shared_ptr<Expression> Parser::parseStringLiteral() const {
    return std::make_shared<StringLiteral>(StringLiteral{curToken, curToken.literal});
}

std::shared_ptr<Expression> Parser::parseArrayLiteral() {
    auto array = std::make_shared<ArrayLiteral>(ArrayLiteral{curToken});

    array->elements = parseExpressionList(TokenType::RSBRACKET);

    return array;
}

std::shared_ptr<Expression> Parser::parseHashLiteral() {
    auto hash = std::make_shared<HashLiteral>(HashLiteral{curToken});

    while (!nextTokenIs(TokenType::RCBRACKET)) {
        move();
        auto key = parseExpression(Precedence::DEFAULT);

        if (!expectPeek(TokenType::COLON)) {
            return nullptr;
        }

        move();
        auto value = parseExpression(Precedence::DEFAULT);

        hash->pairs[std::move(key)] = std::move(value);

        if (!nextTokenIs(TokenType::RCBRACKET) && !expectPeek(TokenType::COMMA)) {
            return nullptr;
        }
    }

    if (!expectPeek(TokenType::RCBRACKET)) {
        return nullptr;
    }

    return hash;
}

std::shared_ptr<Expression> Parser::parsePrefixExpression() {
    auto expression = std::make_shared<PrefixExpression>(PrefixExpression{curToken, curToken.literal});

    move();

    expression->right = parseExpression(Precedence::PREFIX);

    return expression;
}

std::shared_ptr<Expression> Parser::parseIndexExpression(std::shared_ptr<Expression> left) {
    auto exp = std::make_shared<IndexExpression>(IndexExpression{curToken});

    exp->left = std::move(left);

    move();

    exp->index = parseExpression(Precedence::DEFAULT);

    if (!expectPeek(TokenType::RSBRACKET)) {
        return nullptr;
    }

    return exp;
}

std::shared_ptr<Expression> Parser::parseInfixExpression(std::shared_ptr<Expression> left) {
    auto expression = std::make_shared<InfixExpression>(InfixExpression{curToken, curToken.literal});

    expression->left = std::move(left);

    auto precedence = curPrecedence();
    move();

    expression->right = parseExpression(precedence);

    return expression;
}

std::shared_ptr<Expression> Parser::parseInvokeExpression(std::shared_ptr<Expression> function) {
    auto exp = std::make_shared<CallExpression>(CallExpression{curToken, function});

    exp->arguments = parseExpressionList(TokenType::RBRACKET);

    return exp;
}

std::shared_ptr<Expression> Parser::parseGroupedExpression() {
    move();

    auto exp = parseExpression(Precedence::DEFAULT);

    if (!expectPeek(TokenType::RBRACKET)) {
        return nullptr;
    }

    return exp;
}

std::shared_ptr<Expression> Parser::parseConditionExpression() {
    auto expression = std::make_shared<IfExpression>(IfExpression{curToken});

    if (!expectPeek(TokenType::LBRACKET)) {
        return nullptr;
    }

    move();
    expression->condition = parseExpression(Precedence::DEFAULT);

    if (!expectPeek(TokenType::RBRACKET)) {
        return nullptr;
    }

    if (!expectPeek(TokenType::LCBRACKET)) {
        return nullptr;
    }

    expression->consequence = parseBlockStatement();

    if (nextTokenIs(TokenType::ELSE)) {
        move();

        if (!expectPeek(TokenType::LCBRACKET)) {
            return nullptr;
        }

        expression->alternative = parseBlockStatement();
    }

    return expression;
}

std::shared_ptr<Program> Parser::parseProgram() {
    auto program = std::make_shared<Program>(Program{});

    while (curToken.type != TokenType::ENDOFFILE) {
        auto stmt = parseStatement();

        if (stmt) {
            program->statements.push_back(std::move(stmt));
        }

        move();
    }

    return program;
}

std::vector<std::string> Parser::errors() const {
    return e;
}

bool Parser::curTokenIs(TokenType t) const {
    return curToken.type == t;
}

bool Parser::nextTokenIs(TokenType t) const {
    return nextToken.type == t;
}

bool Parser::expectPeek(TokenType t) {
    if (nextTokenIs(t)) {
        move();
        return true;
    } else {
        eyeError(t);
        return false;
    }
}

void Parser::eyeError(TokenType t) {
    std::string msg =
            "Expected token should be " + searchToken(t) + ", while got " + searchToken(nextToken.type) +
            " instead";
    e.push_back(msg);
}

Precedence Parser::eyePrecedence() const {
    auto precedence = precedences.find(nextToken.type);

    // 没有匹配的优先级，则返回default
    if (precedence == precedences.end()) {
        return Precedence::DEFAULT;
    }
    return precedences[nextToken.type];
}

Precedence Parser::curPrecedence() const {
    auto precedence = precedences.find(curToken.type);

    // 没有匹配的优先级，则返回default
    if (precedence == precedences.end()) {
        return Precedence::DEFAULT;
    }

    return precedences[curToken.type];
}

void Parser::noPrefixParseFnError(TokenType t) {
    std::string msg = "No Bounded Parse Function Found for " + searchToken(t) + " token";

    e.push_back(msg);
}

void Parser::bindPrefixFunc(TokenType token_type, prefixParseFn fn) {
    prefixParseFncs[token_type] = std::move(fn);
}

void Parser::bindInfixFunc(TokenType token_type, infixParseFn fn) {
    infixParseFncs[token_type] = std::move(fn);
}