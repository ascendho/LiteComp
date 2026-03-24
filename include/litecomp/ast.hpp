#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>
#include "litecomp/token.hpp"

/**
 * @brief AST 节点的基类
 * 所有语法树节点（语句、表达式、程序）都必须继承自此接口。
 */
struct Node {
    virtual ~Node() = default;

    // 返回节点对应的词法单元字面量（用于调试或错误提示）
    virtual std::string token_literal() const = 0;

    // 返回节点的字符串表示（用于重新生成代码或调试打印）
    virtual std::string string() const = 0;

    // 实现节点的深拷贝
    virtual std::shared_ptr<Node> clone() const = 0;
};

/**
 * @brief 语句基类
 * 语句通常不产生值（例如：let x = 5;）
 */
struct Statement : public Node {
    std::string token_literal() const = 0;
    std::string string() const = 0;
    virtual std::shared_ptr<Node> clone() const = 0;
};

/**
 * @brief 表达式基类
 * 表达式通常会求得一个值（例如：5 + 5;）
 */
struct Expression : public Node {
    std::string token_literal() const = 0;
    std::string string() const = 0;
    virtual std::shared_ptr<Node> clone() const = 0;
};

// =============================================================================
// 表达式实现 (Expressions)
// =============================================================================

/**
 * @brief 标识符 (例如：变量名 x, myVar)
 */
struct Identifier : public Expression {
    Identifier(const Token& t, const std::string& v);
    // 拷贝与移动构造函数及其赋值运算符（用于处理深拷贝）
    Identifier(const Identifier& other);
    Identifier(Identifier&& other) noexcept;
    Identifier& operator=(const Identifier& other);
    Identifier& operator=(Identifier&& other) noexcept;

    Token token;        // 词法单元 (IDENT)
    std::string value;  // 标识符的具体名称

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief 整数型字面量 (例如：5, 100)
 */
struct IntegerLiteral : public Expression {
    explicit IntegerLiteral(const Token& t);
    IntegerLiteral(const IntegerLiteral& other);
    IntegerLiteral(IntegerLiteral&& other) noexcept;
    IntegerLiteral& operator=(const IntegerLiteral& other);
    IntegerLiteral& operator=(IntegerLiteral&& other) noexcept;

    Token token;

    // 转换后的整数值
    int value; 

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief 布尔型字面量 (true, false)
 */
struct BooleanLiteral : public Expression {
    BooleanLiteral(const Token& t, bool v);
    BooleanLiteral(const BooleanLiteral& other);
    BooleanLiteral(BooleanLiteral&& other) noexcept;
    BooleanLiteral& operator=(const BooleanLiteral& other);
    BooleanLiteral& operator=(BooleanLiteral&& other) noexcept;

    Token token;
    bool value;

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief 字符串字面量 (例如："hello world")
 */
struct StringLiteral : public Expression {
    StringLiteral(const Token& t, std::string v);
    StringLiteral(const StringLiteral& other);
    StringLiteral(StringLiteral&& other) noexcept;
    StringLiteral& operator=(const StringLiteral& other);
    StringLiteral& operator=(StringLiteral&& other) noexcept;

    Token token;
    std::string value;

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief 前缀表达式 (例如：-5, !true)
 */
struct PrefixExpression : public Expression {
    PrefixExpression(const Token& t, const std::string& o);
    PrefixExpression(const PrefixExpression& other);
    PrefixExpression(PrefixExpression&& other) noexcept;
    PrefixExpression& operator=(const PrefixExpression& other);
    PrefixExpression& operator=(PrefixExpression&& other) noexcept;

    Token token;
    std::string op;                     // 运算符 (如 "-" 或 "!")
    std::shared_ptr<Expression> right;  // 右侧的操作数

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief 中缀表达式 (例如：5 + 5, x * y)
 */
struct InfixExpression : public Expression {
    InfixExpression(const Token& t, const std::string& o);
    InfixExpression(const InfixExpression& other);
    InfixExpression(InfixExpression&& other) noexcept;
    InfixExpression& operator=(const InfixExpression& other);
    InfixExpression& operator=(InfixExpression&& other) noexcept;

    Token token;
    std::shared_ptr<Expression> left;   // 左操作数
    std::string op;                     // 运算符 (如 "+", "<", "==")
    std::shared_ptr<Expression> right;  // 右操作数

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief 代码块语句 (例如：{ stmt1; stmt2; })
 */
struct BlockStatement : public Statement {
    explicit BlockStatement(const Token& t);
    BlockStatement(const BlockStatement& other);
    BlockStatement(BlockStatement&& other) noexcept;
    BlockStatement& operator=(const BlockStatement& other);
    BlockStatement& operator=(BlockStatement&& other) noexcept;

    // 词法单元 '{'
    Token token; 

    // 块内的语句列表
    std::vector<std::shared_ptr<Node>> statements; 

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief If 表达式 (例如：if (x > y) { x } else { y })
 */
struct IfExpression : public Expression {
    explicit IfExpression(const Token& t);
    IfExpression(const IfExpression& other);
    IfExpression(IfExpression&& other) noexcept;
    IfExpression& operator=(const IfExpression& other);
    IfExpression& operator=(IfExpression&& other) noexcept;

    // 'if' 词法单元
    Token token; 
    std::shared_ptr<Expression> condition;       // 条件部分
    std::shared_ptr<BlockStatement> consequence; // if 分支代码块
    std::shared_ptr<BlockStatement> alternative; // else 分支代码块 (可选)

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief 函数字面量 (例如：fn(x, y) { x + y; })
 */
struct FuncLiteral : public Expression {
    explicit FuncLiteral(const Token& t);
    FuncLiteral(const FuncLiteral& other);
    FuncLiteral(FuncLiteral&& other) noexcept;
    FuncLiteral& operator=(const FuncLiteral& other);
    FuncLiteral& operator=(FuncLiteral&& other) noexcept;

    // 'fn' 词法单元
    Token token;
    std::vector<std::shared_ptr<Identifier>> parameters; // 参数列表
    std::shared_ptr<BlockStatement> body;                // 函数体
    std::string name;                                    // 函数名（匿名函数则为空）

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief 数组字面量 (例如：[1, 2, 3])
 */
struct ArrayLiteral : public Expression {
    explicit ArrayLiteral(const Token& t);
    ArrayLiteral(const ArrayLiteral& other);
    ArrayLiteral(ArrayLiteral&& other) noexcept;
    ArrayLiteral& operator=(const ArrayLiteral& other);
    ArrayLiteral& operator=(ArrayLiteral&& other) noexcept;

    // '[' 词法单元
    Token token;
    // 数组元素
    std::vector<std::shared_ptr<Expression>> elements;

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief HashLiteral 的自定义比较器
 * 因为 map 的键是智能指针，我们需要根据表达式的字符串表示来排序键，确保 map 的稳定性。
 */
struct comp_hashliteral {
    bool operator()(std::shared_ptr<Expression> key1, std::shared_ptr<Expression> key2) const;
};

/**
 * @brief 哈希表/对象字面量 (例如：{"key": "value"})
 */
struct HashLiteral : public Expression {
    explicit HashLiteral(const Token& t);
    HashLiteral(const HashLiteral& other);
    HashLiteral(HashLiteral&& other) noexcept;
    HashLiteral& operator=(const HashLiteral& other);
    HashLiteral& operator=(HashLiteral&& other) noexcept;

    // '{' 词法单元
    Token token;
    // 键值对
    std::map<std::shared_ptr<Expression>, std::shared_ptr<Expression>, comp_hashliteral> pairs;

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief 索引表达式 (例如：array[index] 或 hash["key"])
 */
struct IndexExpression : public Expression {
    explicit IndexExpression(const Token& t);
    IndexExpression(const IndexExpression& other);
    IndexExpression(IndexExpression&& other) noexcept;
    IndexExpression& operator=(const IndexExpression& other);
    IndexExpression& operator=(IndexExpression&& other) noexcept;

    // '[' 词法单元
    Token token;
    // 被索引的对象
    std::shared_ptr<Expression> left;
    // 索引值
    std::shared_ptr<Expression> index;

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief 调用表达式 (例如：add(1, 2))
 */
struct CallExpression : public Expression {
    explicit CallExpression(const Token& t, std::shared_ptr<Expression> f);
    CallExpression(const CallExpression& other);
    CallExpression(CallExpression&& other) noexcept;
    CallExpression& operator=(const CallExpression& other);
    CallExpression& operator=(CallExpression&& other) noexcept;

    // '(' 词法单元
    Token token; 
    // 被调用的函数 (标识符或函数字面量)
    std::shared_ptr<Expression> function;    
    // 参数列表          
    std::vector<std::shared_ptr<Expression>> arguments; 

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

// =============================================================================
// 语句实现 (Statements)
// =============================================================================

/**
 * @brief 变量声明语句 (例如：let x = 5;)
 */
struct DeclareStatement : public Statement {
    explicit DeclareStatement(const Token& t);
    DeclareStatement(const DeclareStatement& other);
    DeclareStatement(DeclareStatement&& other) noexcept;
    DeclareStatement& operator=(const DeclareStatement& other);
    DeclareStatement& operator=(DeclareStatement&& other) noexcept;

    // 'let' 词法单元
    Token token;
    // 变量名
    std::shared_ptr<Identifier> name;
    // 初始化的值
    std::shared_ptr<Expression> value;

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief 返回语句 (例如：return 5;)
 */
struct ReturnStatement : public Statement {
    explicit ReturnStatement(const Token& t);
    ReturnStatement(const ReturnStatement& other);
    ReturnStatement(ReturnStatement&& other) noexcept;
    ReturnStatement& operator=(const ReturnStatement& other);
    ReturnStatement& operator=(ReturnStatement&& other) noexcept;

    // 'return' 词法单元
    Token token;
    // 返回值
    std::shared_ptr<Expression> return_value;

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief 表达式语句
 * 包装一个表达式使其可以作为语句存在（例如：x + 5;）
 */
struct ExpressionStatement : public Statement {
    explicit ExpressionStatement(const Token& t);
    ExpressionStatement(const ExpressionStatement& other);
    ExpressionStatement(ExpressionStatement&& other) noexcept;
    ExpressionStatement& operator=(const ExpressionStatement& other);
    ExpressionStatement& operator=(ExpressionStatement&& other) noexcept;

    // 表达式的第一个词法单元
    Token token;
    // 内部封装的表达式
    std::shared_ptr<Expression> expression;

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};

/**
 * @brief 程序根节点
 * 这是 AST 的顶层节点，包含整个文件的语句。
 */
struct Program : public Node {
    Program() = default;
    Program(const Program& other);
    Program(Program&& other) noexcept;
    Program& operator=(const Program& other);
    Program& operator=(Program&& other) noexcept;

    // 所有的顶层语句
    std::vector<std::shared_ptr<Node>> statements; 

    std::string token_literal() const override;
    std::string string() const override;
    std::shared_ptr<Node> clone() const override;
};