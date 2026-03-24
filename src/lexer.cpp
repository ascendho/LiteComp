#include "litecomp/lexer.hpp"

// 匿名空间：其中的函数仅在当前文件内可见，类似于静态全局函数
namespace {

/**
 * @brief 判断字符是否为合法的标识符组成部分
 * 标识符规则：字母 (a-z, A-Z) 或下划线 (_)
 */
bool isCharacter(char ch) {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_';
}

/**
 * @brief 判断字符是否为数字 (0-9)
 */
bool isDigit(char ch) {
    return '0' <= ch && ch <= '9';
}

}

/**
 * @brief 构造函数：初始化源代码并准备第一个字符
 */
Lexer::Lexer(const std::string &input_in) : src{input_in}, curPos{}, nextPos{}, ch{} {
    // 启动扫描器，将第一个字符读入 ch
    move();
}

/**
 * @brief 移动构造函数：实现资源的高效转移
 */
Lexer::Lexer(Lexer &&other) noexcept: src(std::move(other.src)), curPos(other.curPos),
                                      nextPos(other.nextPos), ch(other.ch) {
    // 重置原对象状态
    other.src = "";
    other.curPos = 0;
    other.nextPos = 0;
    other.ch = 0;
}

/**
 * @brief 前进指针：读取下一个字符到 ch
 */
void Lexer::move() {
    if (nextPos >= static_cast<int>(src.length())) {
        ch = 0; // 0 代表读取到文件末尾 (EOF)
    } else {
        ch = src[nextPos];
    }

    // 更新当前位置 curPos，并将 nextPos 后移
    curPos = nextPos++;
}

/**
 * @brief 跳过空白字符
 * 编程语言通常忽略空格、制表符和换行符
 */
void Lexer::ignoreWhitespace() {
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        move();
    }
}

/**
 * @brief 连续读取字符直到非标识符字符为止
 * 用于提取变量名或关键字
 */
std::string Lexer::readIdentifier() {
    int startPos = curPos; // 记录开始位置

    // 只要是字符或数字（标识符中间可以含数字），就一直移动
    do {
        move();
    } while (isCharacter(ch) || isDigit(ch)); // 修正逻辑：标识符通常允许数字出现在非首位

    // 利用 string 的 substr 截取整段内容
    return src.substr(startPos, curPos - startPos);
}

/**
 * @brief 连续读取数字字符
 */
std::string Lexer::readWholeNumber() {
    int startPos = curPos;

    do {
        move();
    } while (isDigit(ch));

    return src.substr(startPos, curPos - startPos);
}

/**
 * @brief 前瞻：查看下一个字符是什么，但不移动指针
 * 这在处理 "==" 和 "!=" 等双字符操作符时非常关键
 */
char Lexer::eyeNextChar() {
    if (nextPos >= static_cast<int>(src.length())) {
        return 0;
    } else {
        return src[nextPos];
    }
}

/**
 * @brief 读取字符串字面量（处理引号内的内容）
 */
std::string Lexer::readString() {
    // curPos 当前在左引号 " 上，跳过它开始读取
    int startPos = curPos + 1;
    while (true) {
        move();

        // 遇到右引号或文件末尾则停止
        if (ch == '"' || ch == 0) {
            break;
        }
    }

    // 截取引号之间的内容
    return src.substr(startPos, (curPos - startPos));
}

/**
 * @brief 核心扫描函数：识别并返回下一个 Token
 */
Token Lexer::scan() {
    // 1. 首先跳过无意义的空白
    ignoreWhitespace();

    // 默认创建一个 ILLEGAL (非法) 类型的 Token，后续再根据 ch 修改
    auto tok = Token{TokenType::ILLEGAL, std::string{ch}};

    switch (ch) {
        case ('='): {
            // 检查是 "=" (赋值) 还是 "==" (相等判定)
            if (eyeNextChar() == '=') {
                // 消耗第一个 =
                move(); 
                tok.type = TokenType::EQUAL;

                // 拼接成 "=="
                tok.literal += std::string{ch}; 
            } else {
                tok.type = TokenType::ASSIGN;
            }
        }
            break;

        case ('+'): { tok.type = TokenType::PLUS; } break;
        case ('-'): { tok.type = TokenType::MINUS; } break;

        case ('!'): {
            // 检查是 "!" (非) 还是 "!=" (不等)
            if (eyeNextChar() == '=') {
                move();
                tok.type = TokenType::NOTEQUAL;
                tok.literal += std::string{ch};
            } else {
                tok.type = TokenType::BANG;
            }
        }
            break;

        case ('*'): { tok.type = TokenType::ASTERISK; } break;
        case ('/'): { tok.type = TokenType::SLASH; } break;
        case ('<'): { tok.type = TokenType::LESS; } break;
        case ('>'): { tok.type = TokenType::GREAT; } break;
        case (';'): { tok.type = TokenType::SEMICOLON; } break;
        case (':'): { tok.type = TokenType::COLON; } break;
        case (','): { tok.type = TokenType::COMMA; } break;
        case ('('): { tok.type = TokenType::LBRACKET; } break;
        case (')'): { tok.type = TokenType::RBRACKET; } break;
        case ('{'): { tok.type = TokenType::LCBRACKET; } break;
        case ('}'): { tok.type = TokenType::RCBRACKET; } break;
        case ('['): { tok.type = TokenType::LSBRACKET; } break;
        case (']'): { tok.type = TokenType::RSBRACKET; } break;

        case ('"'): {
            // 识别字符串字面量
            tok.type = TokenType::STRING;
            tok.literal = readString();
        }
            break;

        case (0): {
            // 文件末尾
            tok.type = TokenType::ENDOFFILE;
            tok.literal = "";
        }
            break;

        default: {
            // 如果 ch 是字母，可能是关键字 (let, fn) 或标识符 (变量名)
            if (isCharacter(ch)) {
                tok.literal = readIdentifier();
                // searchIdentifier 函数（通常定义在 token.cpp 中）
                // 会检查该字符串是否在关键字映射表中，如果是则返回关键字类型，否则返回 IDENT
                tok.type = searchIdentifier(tok.literal);
                // 注意：readIdentifier 内部已经 move 过了，直接返回
                return tok; 
            } 
            // 如果 ch 是数字
            else if (isDigit(ch)) {
                tok.type = TokenType::INTEGER;
                tok.literal = readWholeNumber();
                // 同理，直接返回，因为 readWholeNumber 内部已经移动过指针了
                return tok; 
            }
            // 如果既不是符号也不是字母数字，保持默认的 ILLEGAL 类型
        }
    }

    // 消耗当前字符，为下一次 scan 做准备
    move();

    return tok;
}