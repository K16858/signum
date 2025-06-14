// SigNum Lexer

#include "lexer.hpp"
#include <iostream>
#include <string>
#include <vector>

// メモリ参照を解析
std::string parseMemoryRef(const std::string& src, size_t& pos) {
    std::string memref = "";
    memref += src[pos++]; // "$"
    
    while (pos < src.size() && (src[pos] == '#' || src[pos] == '@' || src[pos] == '~' || src[pos] == '%')) {
        memref += src[pos++]; // 型記号
        
        // ネストされた参照の場合
        if (pos < src.size() && src[pos] == '$') {
            memref += parseMemoryRef(src, pos); // 再帰
        } 
        // 通常の数字の場合
        else {
            while (pos < src.size() && isdigit(src[pos])) {
                memref += src[pos++];
            }
        }
    }
    
    return memref;
}

// 字句解析関数
std::vector<Token> tokenize(const std::string& src) {
    std::vector<Token> tokens;
    size_t pos = 0; // 解析位置
    size_t line = 1; // 行番号

    while (pos < src.size()) {
        char c = src[pos];

        // 改行を検出
        if (c == '\n') {
            ++line;
            ++pos;
            continue;
        }
        // 空白をスキップ
        if (isspace(c)) {
            ++pos;
            continue;
        }

        // 文字列リテラル
        if (c == '"') {
            size_t start = ++pos;
            while (pos < src.size() && src[pos] != '"') {
                ++pos;
            }
            if (pos < src.size()) {
                tokens.push_back({TokenType::String, src.substr(start, pos - start), line});
                ++pos; // '"' をスキップ
            } 
            else {
                std::cerr << "Error: Unmatched double quote\n";
                return {};
            }
            continue;
        }

        // 関数呼び出し
        if (src[pos] == '$' && pos + 1 < src.size() && src[pos + 1] == '_') {
            std::string funcCall;
            funcCall += src[pos++]; // '$'
            funcCall += src[pos++]; // '_'
            while (pos < src.size() && isdigit(src[pos])) {
                funcCall += src[pos++];
            }
            tokens.push_back({TokenType::FunctionCall, funcCall, line});
            continue;
        }

        // メモリ参照
        if (src[pos] == '$') {
            size_t startPos = pos;
            std::string memref = parseMemoryRef(src, pos);
            tokens.push_back({TokenType::MemoryRef, memref, line});
            continue;
        }

        // 関数割り当て
        // 関数IDは '_' で始まり、次に数字が続く（少なくとも3文字続く）
        if (src[pos] == '_' && pos + 3 < src.size() && isdigit(src[pos + 1])) {
            std::string funcid;
            funcid += src[pos++]; // '_'
            while (pos < src.size() && isdigit(src[pos])) {
                funcid += src[pos++];
            }
            tokens.push_back({TokenType::Function, funcid, line});
            continue;
        }

        // 数字
        if (isdigit(c)) {
            size_t start = pos;
            bool isFloat = false;
            
            // 整数部分を読み取り
            while (pos < src.size() && isdigit(src[pos])) {
                ++pos;
            }
            
            // 小数点があれば小数部分も読み取り
            if (pos < src.size() && src[pos] == '.') {
                isFloat = true;
                ++pos;  // 小数点
                
                // 少なくとも1桁は必要
                if (pos < src.size() && isdigit(src[pos])) {
                    while (pos < src.size() && isdigit(src[pos])) {
                        ++pos;
                    }
                } 
                else {
                    std::cerr << "Error";
                    return {};
                }
            }
            
            TokenType type = isFloat ? TokenType::Float : TokenType::Integer;
            tokens.push_back({type, src.substr(start, pos - start), line});
            continue;
        }

        // 記号系
        switch (c) {
            case '{':
                tokens.push_back({TokenType::LBrace, "{", line});
                ++pos;
                break;
            case '}':
                tokens.push_back({TokenType::RBrace, "}", line});
                ++pos;
                break;
            case '(':
                tokens.push_back({TokenType::LParen, "(", line});
                ++pos;
                break;
            case ')':
                tokens.push_back({TokenType::RParen, ")", line});
                ++pos;
                break;
            case '[':
                tokens.push_back({TokenType::LBracket, "[", line});
                ++pos;
                break;
            case ']':
                tokens.push_back({TokenType::RBracket, "]", line});
                ++pos;
                break;
            case ',':
                tokens.push_back({TokenType::Comma, ",", line});
                ++pos;
                break;
            case ';':
                tokens.push_back({TokenType::Semicolon, ";", line});
                ++pos;
                break;
            case ':':
                tokens.push_back({TokenType::Colon, ":", line});
                ++pos;
                break;
            case '&':
                if (pos + 1 < src.size()) {
                    if (src[pos + 1] == '&') {
                        tokens.push_back({TokenType::And, "&&", line});
                        pos += 2;
                    }
                    else {
                        tokens.push_back({TokenType::Loop, "&", line});
                        ++pos;  // & を処理
                    }
                }
                break;
            case '?':
                if (pos + 1 < src.size()) {
                    if (src[pos + 1] == '?') {
                        // ??
                        if (pos + 2 < src.size() && src[pos + 2] == '?') {
                            tokens.push_back({TokenType::Else, "???", line});
                            pos += 3;
                        } 
                        else {
                            tokens.push_back({TokenType::ElseIf, "??", line});
                            pos += 2;
                        }
                    } 
                    else {
                        tokens.push_back({TokenType::If, "?", line});
                        ++pos;
                    } 
                }
                break;
            case '<':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::LessThanOrEqual, "<=", line});
                    pos += 2;
                } 
                else if (pos + 1 < src.size() && src[pos + 1] == '<') {
                    tokens.push_back({TokenType::DoubleLAngleBracket, "<<", line});
                    pos += 2;
                } 
                else if (pos + 1 < src.size() && src[pos + 1] == '!') {
                    tokens.push_back({TokenType::ErrorOutput, "<!", line});
                    pos += 2;
                }
                else {
                    tokens.push_back({TokenType::LAngleBracket, "<", line});
                    ++pos;
                }
                break;
            case '>':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::GreaterThanOrEqual, ">=", line});
                    pos += 2;
                } 
                else if (pos + 1 < src.size() && src[pos + 1] == '>') {
                    tokens.push_back({TokenType::DoubleRAngleBracket, ">>", line});
                    pos += 2;
                }
                else {
                    tokens.push_back({TokenType::RAngleBracket, ">", line});
                    ++pos;
                }
                break;
            case '=':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::EqualTo, "==", line});
                    pos += 2;
                } 
                else {
                    tokens.push_back({TokenType::Assign, "=", line});
                    ++pos;
                }
                break;
            case '+':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::PlusEqual, "+=", line});
                    pos += 2;
                } 
                else {
                    tokens.push_back({TokenType::Plus, "+", line});
                    ++pos;
                }
                break;
            case '-':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::MinusEqual, "-=", line});
                    pos += 2;
                } 
                else {
                    tokens.push_back({TokenType::Minus, "-", line});
                    ++pos;
                }
                break;
            case '*':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::MultiplyEqual, "*=", line});
                    pos += 2;
                } 
                else {
                    tokens.push_back({TokenType::Multiply, "*", line});
                    ++pos;
                }
                break;
            case '/':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::DivideEqual, "/=", line});
                    pos += 2;
                } 
                else {
                    tokens.push_back({TokenType::Divide, "/", line});
                    ++pos;
                }
                break;
            case '#':
                if (pos + 1 < src.size() && src[pos + 1] == ':') {
                    tokens.push_back({TokenType::IntCast, "#:", line});
                    pos += 2;
                } else {
                    tokens.push_back({TokenType::Hash, "#", line});
                    ++pos;
                }
                break;
            case '@':
                if (pos + 1 < src.size() && src[pos + 1] == ':') {
                    tokens.push_back({TokenType::StrCast, "@:", line});
                    pos += 2;
                } else {
                    tokens.push_back({TokenType::At, "@", line});
                    ++pos;
                }
                break;
            case '~':
                if (pos + 1 < src.size() && src[pos + 1] == ':') {
                    tokens.push_back({TokenType::FloatCast, "~:", line});
                    pos += 2;
                } else {
                    tokens.push_back({TokenType::Tilde, "~", line});
                    ++pos;
                }
                break;
            case '%':
                if (pos + 1 < src.size() && src[pos + 1] == ':') {
                    tokens.push_back({TokenType::BoolCast, "%:", line});
                    pos += 2;
                }
                else if (pos + 1 < src.size() && (src[pos + 1] == '0' || src[pos + 1] == '1')) {
                    tokens.push_back({TokenType::Boolean, src.substr(pos, 2), line});
                    pos += 2;
                } 
                else {
                    if (pos + 1 < src.size() && src[pos + 1] == '=') {
                        tokens.push_back({TokenType::ModulusEqual, "%=", line});
                        pos += 2;
                    } 
                    else {
                        tokens.push_back({TokenType::Modulus, "%", line});
                        ++pos;
                    }
                }
                break;

            case '|':
                if (pos + 1 < src.size() && src[pos + 1] == '|') {
                    tokens.push_back({TokenType::Or, "||", line});
                    pos += 2;
                } 
                else {
                    std::cerr << "Error: Unknown character sequence '|" << src[pos + 1] << "'\n";
                    return {};
                }
                break;
            case '!':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::NotEqualTo, "!=", line});
                    pos += 2;
                } 
                else {
                    tokens.push_back({TokenType::Not, "!", line});
                    ++pos;
                }
                break;
            default:
                if (isalpha(c)) {
                    std::string symbol(1, c);
                    ++pos;
                    while (pos < src.size() && (isalnum(src[pos]) || src[pos] == '_')) {
                        symbol += src[pos++];
                    }
                    tokens.push_back({TokenType::Symbol, symbol, line});
                } 
                else {
                    std::cerr << "Error: Unknown character '" << c << "'\n";
                    return {};
                }
                break;
        }
    }

    // 終端トークンを追加
    tokens.push_back({TokenType::End, "", line});
    return tokens;
}

void printTokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        std::cout << "Token: " << tokenType2String(token.type)
                  << ", Value: " << token.value 
                  << ", Line: " << token.line << std::endl;
    }
}
