#include "lexer.hpp"

bool isCharacter(char ch) {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_';
}

bool isDigit(char ch) {
    return '0' <= ch && ch <= '9';
}

Lexer::Lexer(const std::string &input_in) : src{input_in}, curPos{}, nextPos{}, ch{} {
    move();
}

Lexer::Lexer(Lexer &&other) noexcept: src(std::move(other.src)), curPos(other.curPos),
                                      nextPos(other.nextPos), ch(other.ch) {
    other.src = "";
    other.curPos = 0;
    other.nextPos = 0;
    other.ch = 0;
}

void Lexer::move() {
    if (nextPos >= static_cast<int>(src.length())) {
        ch = 0;
    } else {
        ch = src[nextPos];
    }

    // 扫描指针更新
    curPos = nextPos++;
}

void Lexer::ignoreWhitespace() {
    // 空格、制表符、换行符、回车键
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        move();
    }
}

std::string Lexer::readIdentifier() {
    // 暂存当前位置
    int startPos = curPos;

    // 一直移动词法单元直到扫描的字符不再是字母
    do {
        move();
    } while (isCharacter(ch));

    // 返回标识符
    return src.substr(startPos, curPos - startPos);
}

std::string Lexer::readWholeNumber() {
    // 暂存当前位置
    int startPos = curPos;

    // 一直移动词法单元直到扫描的字符不再是数字
    do {
        move();
    } while (isDigit(ch));

    // 返回整个数
    return src.substr(startPos, curPos - startPos);
}

char Lexer::eyeNextChar() {
    if (nextPos >= static_cast<int>(src.length())) {
        return 0;
    } else {
        return src[nextPos];
    }
}

std::string Lexer::readString() {
    // 跳过第一个"，所以+1
    int startPos = curPos + 1;
    while (true) {
        move();

        // 读取到"或者EOF
        if (ch == '"' || ch == 0) {
            break;
        }
    }

    // 返回字符串
    return src.substr(startPos, (curPos - startPos));
}

Token Lexer::scan() {
    // 忽略掉空白字符
    ignoreWhitespace();

    // 先预设为非法词法单元
    auto tok = Token{TokenType::ILLEGAL, std::string{ch}};

    switch (ch) {
        case ('='): {
            if (eyeNextChar() == '=') {
                move();
                tok.type = TokenType::EQUAL;
                tok.literal += std::string{ch};
            } else {
                tok.type = TokenType::ASSIGN;
            }
        }
            break;

        case ('+'): {
            tok.type = TokenType::PLUS;
        }
            break;

        case ('-'): {
            tok.type = TokenType::MINUS;
        }
            break;

        case ('!'): {
            if (eyeNextChar() == '=') {
                move();
                tok.type = TokenType::NOTEQUAL;
                tok.literal += std::string{ch};
            } else {
                tok.type = TokenType::BANG;
            }
        }
            break;

        case ('*'): {
            tok.type = TokenType::ASTERISK;
        }
            break;

        case ('/'): {
            tok.type = TokenType::SLASH;
        }
            break;

        case ('<'): {
            tok.type = TokenType::LESS;
        }
            break;

        case ('>'): {
            tok.type = TokenType::GREAT;
        }
            break;

        case (';'): {
            tok.type = TokenType::SEMICOLON;
        }
            break;

        case (':'): {
            tok.type = TokenType::COLON;
        }
            break;

        case (','): {
            tok.type = TokenType::COMMA;
        }
            break;

        case ('('): {
            tok.type = TokenType::LBRACKET;
        }
            break;

        case (')'): {
            tok.type = TokenType::RBRACKET;
        }
            break;

        case ('"'): {
            tok.type = TokenType::STRING;
            tok.literal = readString();
        }
            break;

        case ('{'): {
            tok.type = TokenType::LCBRACKET;
        }
            break;

        case ('}'): {
            tok.type = TokenType::RCBRACKET;
        }
            break;

        case ('['): {
            tok.type = TokenType::LSBRACKET;
        }
            break;

        case (']'): {
            tok.type = TokenType::RSBRACKET;
        }
            break;

        case (0): {
            tok.type = TokenType::ENDOFFILE;
            tok.literal = std::string{""};
        }
            break;

        default: {
            if (isCharacter(ch)) {
                tok.literal = readIdentifier();
                tok.type = searchIdentifier(tok.literal);
                return tok;
            } else if (isDigit(ch)) {
                tok.type = TokenType::INTEGER;
                tok.literal = readWholeNumber();
                return tok;
            }
        }
    }

    move();

    return tok;
}