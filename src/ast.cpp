#include "litecomp/ast.hpp"

// =============================================================================
// DeclareStatement (变量声明语句，如：let x = 5;)
// =============================================================================

std::string DeclareStatement::token_literal() const {
    return token.literal;
}

std::string DeclareStatement::string() const {
    std::string out;
    out.append(token_literal() + " "); // 通常是 "let"
    out.append(name->string());        // 变量名
    out.append(" = ");
    out.append(value->string());       // 变量值表达式
    out.append(";");
    return out;
}

// 克隆当前的声明语句节点
std::shared_ptr<Node> DeclareStatement::clone() const {
    return std::make_shared<DeclareStatement>(DeclareStatement{*this});
}

DeclareStatement::DeclareStatement(const Token& t) : token{t} {}

// 拷贝构造函数：实现深拷贝，确保子节点（name 和 value）也被克隆
DeclareStatement::DeclareStatement(const DeclareStatement& other) : token{other.token} {
    // 使用 clone() 确保递归地复制整个子树，而不是只复制指针
    name = std::dynamic_pointer_cast<Identifier>(other.name->clone());
    value = std::dynamic_pointer_cast<Expression>(other.value->clone());
}

// 移动构造函数：高效转移资源
DeclareStatement::DeclareStatement(DeclareStatement&& other) noexcept {
    token = std::move(other.token);
    name = std::move(other.name);
    value = std::move(other.value);

    // 重置源对象
    other.name = nullptr;
    other.value = nullptr;
    other.token.type = TokenType::ILLEGAL;
    other.token.literal = "";
}

// 赋值运算符重载（深拷贝版本）
DeclareStatement& DeclareStatement::operator=(const DeclareStatement& other) {
    if (this == &other) return *this;
    token = other.token;
    name = std::dynamic_pointer_cast<Identifier>(other.name->clone());
    value = std::dynamic_pointer_cast<Expression>(other.value->clone());
    return *this;
}

// 赋值运算符重载（移动语义版本）
DeclareStatement& DeclareStatement::operator=(DeclareStatement&& other) noexcept {
    if (this == &other) return *this;
    token = std::move(other.token);
    name = std::move(other.name);
    value = std::move(other.value);
    other.name = nullptr;
    other.value = nullptr;
    return *this;
}

// =============================================================================
// ReturnStatement (返回语句，如：return 5;)
// =============================================================================

std::string ReturnStatement::token_literal() const {
    return token.literal;
}

std::string ReturnStatement::string() const {
    std::string out;
    out.append(token_literal() + " ");
    out.append(return_value->string());
    out.append(";");
    return out;
}

std::shared_ptr<Node> ReturnStatement::clone() const {
    return std::make_shared<ReturnStatement>(ReturnStatement{*this});
}

ReturnStatement::ReturnStatement(const Token& t) : token(t) {}

ReturnStatement::ReturnStatement(const ReturnStatement& other) : token{other.token} {
    return_value = std::dynamic_pointer_cast<Expression>(other.return_value->clone());
}

ReturnStatement::ReturnStatement(ReturnStatement&& other) noexcept {
    token = std::move(other.token);
    return_value = std::move(other.return_value);
    other.return_value = nullptr;
}

// ... (ReturnStatement 的赋值运算符实现逻辑与 DeclareStatement 类似)

// =============================================================================
// ExpressionStatement (表达式语句，如直接调用函数或单纯的运算：add(1, 2);)
// =============================================================================

std::string ExpressionStatement::token_literal() const {
    return token.literal;
}

std::string ExpressionStatement::string() const {
    std::string out;
    if (expression) {
        out.append(expression->string());
    }
    return out;
}

std::shared_ptr<Node> ExpressionStatement::clone() const {
    return std::make_shared<ExpressionStatement>(ExpressionStatement{*this});
}

// ... (ExpressionStatement 的构造与赋值实现，确保 expression 子节点的深度复制)

// =============================================================================
// Identifier (标识符节点)
// =============================================================================

std::string Identifier::string() const {
    return value; // 返回标识符名称
}

std::shared_ptr<Node> Identifier::clone() const {
    return std::make_shared<Identifier>(Identifier{*this});
}

// Identifier 内部是简单的 string 和 Token，不涉及复杂的深拷贝逻辑
Identifier::Identifier(const Token& t, const std::string& v) : token{t}, value{v} {}

// =============================================================================
// IntegerLiteral (整数型字面量)
// =============================================================================

std::string IntegerLiteral::string() const {
    return std::to_string(value);
}

std::shared_ptr<Node> IntegerLiteral::clone() const {
    return std::make_shared<IntegerLiteral>(IntegerLiteral{*this});
}

// =============================================================================
// PrefixExpression (前缀表达式，如：!true, -5)
// =============================================================================

std::string PrefixExpression::string() const {
    // 加上括号以确保在打印 AST 时优先级清晰
    return "(" + op + right->string() + ")";
}

std::shared_ptr<Node> PrefixExpression::clone() const {
    return std::make_shared<PrefixExpression>(PrefixExpression{*this});
}

PrefixExpression::PrefixExpression(const PrefixExpression& other) : token{other.token}, op{other.op} {
    // 递归克隆右侧的操作数表达式
    right = std::dynamic_pointer_cast<Expression>(other.right->clone());
}

// =============================================================================
// InfixExpression (中缀表达式，如：5 + 5)
// =============================================================================

std::string InfixExpression::string() const {
    return "(" + left->string() + " " + op + " " + right->string() + ")";
}

std::shared_ptr<Node> InfixExpression::clone() const {
    return std::make_shared<InfixExpression>(InfixExpression{*this});
}

InfixExpression::InfixExpression(const InfixExpression& other) : token{other.token}, op{other.op} {
    // 左右两个子节点都需要深度克隆
    left = std::dynamic_pointer_cast<Expression>(other.left->clone());
    right = std::dynamic_pointer_cast<Expression>(other.right->clone());
}

// =============================================================================
// BlockStatement (代码块，{} 内的一组语句)
// =============================================================================

std::string BlockStatement::string() const {
    std::string msg;
    for (const auto &stmt: statements) {
        msg.append(stmt->string());
    }
    return msg;
}

BlockStatement::BlockStatement(const BlockStatement& other) : token{other.token}, statements{} {
    // 遍历所有语句，并逐一克隆
    for (const auto &s: other.statements) {
        statements.push_back(s->clone());
    }
}

// =============================================================================
// IfExpression (If 条件分支)
// =============================================================================

std::string IfExpression::string() const {
    std::string msg = "if" + condition->string() + " " + consequence->string();
    // 如果有 else 分支
    if (alternative) { 
        msg += "else ";
        msg += alternative->string();
    }
    return msg;
}

// ... (IfExpression 的深拷贝需要克隆条件、Consequence块和可选的 Alternative块)

// =============================================================================
// FuncLiteral (函数定义，如：fn(x) { x + 1; })
// =============================================================================

std::string FuncLiteral::string() const {
    std::string msg = token.literal;
    // 如果是具名函数
    if (name != "") { msg += "<" + name + ">"; } 
    msg += "(";
    // 拼接参数列表
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i != 0) msg += ", ";
        msg += parameters[i]->string();
    }
    msg += ") ";
    // 函数体
    msg += body->string(); 
    return msg;
}

// =============================================================================
// ArrayLiteral (数组，如：[1, 2, a + b])
// =============================================================================

std::string ArrayLiteral::string() const {
    std::string out = "[";
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i != 0) out += ", ";
        out += elements[i]->string();
    }
    out += "]";
    return out;
}

// =============================================================================
// HashLiteral (哈希/字典，如：{"key": value})
// =============================================================================

// 用于 map 的 key 排序逻辑：根据表达式转换出的字符串进行排序
bool comp_hashliteral::operator()(std::shared_ptr<Expression> key1, std::shared_ptr<Expression> key2) const {
    return key1->string() < key2->string();
}

std::string HashLiteral::string() const {
    std::string out = "{";
    int counter = 0;
    for (const auto &p: pairs) {
        if (counter++ != 0) out += ", ";
        // C++17 结构化绑定
        const auto[key, value] = p; 
        out += key->string() + ":" + value->string();
    }
    out += "}";
    return out;
}

// =============================================================================
// IndexExpression (索引访问，如：myArray[0])
// =============================================================================

std::string IndexExpression::string() const {
    std::string out = "(";
    out += left->string();  // 数组或哈希对象
    out += "[";
    out += index->string(); // 索引表达式
    out += "])";
    return out;
}

// =============================================================================
// CallExpression (函数调用，如：add(2, 3))
// =============================================================================

std::string CallExpression::string() const {
    std::string msg = function->string() + "(";
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i != 0) msg += ", ";
        msg += arguments[i]->string();
    }
    msg += ")";
    return msg;
}

// =============================================================================
// Program (AST 根节点，代表整个源文件)
// =============================================================================

std::string Program::token_literal() const {
    if (!statements.empty()) {
        return statements.at(0)->token_literal();
    }
    return "";
}

std::string Program::string() const {
    std::string out;
    for (const auto &s: statements) {
        out.append(s->string());
    }
    return out;
}

std::shared_ptr<Node> Program::clone() const {
    return std::make_shared<Program>(Program{*this});
}

Program::Program(const Program& other) : statements{} {
    // 拷贝整个程序意味着克隆其下的所有语句
    for (const auto &s: other.statements) {
        statements.push_back(s->clone());
    }
}

Program::Program(Program&& other) noexcept {
    // 交换向量内容，极快
    statements.swap(other.statements); 
}

Program& Program::operator=(const Program& other) {
    if (this == &other) return *this;
    statements.clear();
    for (const auto &s: other.statements) {
        statements.push_back(s->clone());
    }
    return *this;
}

Program& Program::operator=(Program&& other) noexcept {
    if (this == &other) return *this;
    statements.swap(other.statements);
    return *this;
}