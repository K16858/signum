// SigNum Lexer

#include "lexer.hpp"
#include <iostream>
#include <string>
#include <vector>

// エラー箇所の周辺コンテキストを取得
std::string Lexer::getContextAroundPosition() const {
    const size_t contextRange = 20; // 前後20文字
    size_t start = (pos > contextRange) ? pos - contextRange : 0;
    size_t end = (pos + contextRange < source.size()) ? pos + contextRange : source.size();
    
    std::string context = source.substr(start, end - start);
    // 改行を表示用に変換
    for (char& c : context) {
        if (c == '\n') c = ' ';
        if (c == '\t') c = ' ';
    }
    return context;
}

// メモリ参照を解析
std::string Lexer::parseMemoryRef() {
    std::string memref = "";
    memref += source[pos++]; // "$"
    
    while (pos < source.size() && (source[pos] == '#' || source[pos] == '@' || source[pos] == '~' || source[pos] == '%')) {
        memref += source[pos++]; // 型記号
        column++;
        
        // ネストされた参照の場合
        if (pos < source.size() && source[pos] == '$') {
            memref += parseMemoryRef(); // 再帰
        } 
        // 通常の数字の場合
        else {
            while (pos < source.size() && isdigit(source[pos])) {
                memref += source[pos++];
                column++;
            }
        }
    }
    
    return memref;
}

// 字句解析関数
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (pos < source.size()) {
        char c = source[pos];

        // 改行を検出
        if (c == '\n') {
            ++line;
            column = 1;
            ++pos;
            continue;
        }
        // 空白をスキップ
        if (isspace(c)) {
            ++pos;
            ++column;
            continue;
        }

        // 文字列リテラル
        if (c == '"') {
            size_t start = ++pos;
            ++column;
            while (pos < source.size() && source[pos] != '"') {
                ++pos;
                ++column;
            }
            if (pos < source.size()) {
                tokens.push_back({TokenType::String, source.substr(start, pos - start), line});
                ++pos; // '"' をスキップ
                ++column;
            } 
            else {
                addError("Unmatched double quote", getContextAroundPosition());
                return tokens;
            }
            continue;
        }

        // 関数呼び出し
        if (source[pos] == '$' && pos + 1 < source.size() && source[pos + 1] == '_') {
            std::string funcCall;
            funcCall += source[pos++]; // '$'
            funcCall += source[pos++]; // '_'
            column += 2;
            while (pos < source.size() && isdigit(source[pos])) {
                funcCall += source[pos++];
                ++column;
            }
            tokens.push_back({TokenType::FunctionCall, funcCall, line});
            continue;
        }

        // メモリ参照
        if (source[pos] == '$') {
            std::string memref = parseMemoryRef();
            tokens.push_back({TokenType::MemoryRef, memref, line});
            continue;
        }

        // 関数割り当て
        if (source[pos] == '_' && pos + 3 < source.size() && isdigit(source[pos + 1])) {
            std::string funcid;
            funcid += source[pos++]; // '_'
            ++column;
            while (pos < source.size() && isdigit(source[pos])) {
                funcid += source[pos++];
                ++column;
            }
            tokens.push_back({TokenType::Function, funcid, line});
            continue;
        }

        // 数字
        if (isdigit(c)) {
            size_t start = pos;
            bool isFloat = false;
            
            // 整数部分を読み取り
            while (pos < source.size() && isdigit(source[pos])) {
                ++pos;
                ++column;
            }
            
            // 小数点があれば小数部分も読み取り
            if (pos < source.size() && source[pos] == '.') {
                isFloat = true;
                ++pos;  // 小数点
                ++column;
                
                // 少なくとも1桁は必要
                if (pos < source.size() && isdigit(source[pos])) {
                    while (pos < source.size() && isdigit(source[pos])) {
                        ++pos;
                        ++column;
                    }
                } 
                else {
                    addError("Invalid float format: decimal point must be followed by digits", getContextAroundPosition());
                    return tokens;
                }
            }
            
            TokenType type = isFloat ? TokenType::Float : TokenType::Integer;
            tokens.push_back({type, source.substr(start, pos - start), line});
            continue;
        }

        // 記号系
        switch (c) {
            case '{':
                tokens.push_back({TokenType::LBrace, "{", line});
                ++pos;
                ++column;
                break;
            case '}':
                tokens.push_back({TokenType::RBrace, "}", line});
                ++pos;
                ++column;
                break;
            case '(':
                tokens.push_back({TokenType::LParen, "(", line});
                ++pos;
                ++column;
                break;
            case ')':
                tokens.push_back({TokenType::RParen, ")", line});
                ++pos;
                ++column;
                break;
            case '[':
                tokens.push_back({TokenType::LBracket, "[", line});
                ++pos;
                ++column;
                break;
            case ']':
                tokens.push_back({TokenType::RBracket, "]", line});
                ++pos;
                ++column;
                break;
            case ',':
                tokens.push_back({TokenType::Comma, ",", line});
                ++pos;
                ++column;
                break;
            case ';':
                tokens.push_back({TokenType::Semicolon, ";", line});
                ++pos;
                ++column;
                break;
            case ':':
                tokens.push_back({TokenType::Colon, ":", line});
                ++pos;
                ++column;
                break;
            case '&':
                if (pos + 1 < source.size()) {
                    if (source[pos + 1] == '&') {
                        tokens.push_back({TokenType::And, "&&", line});
                        pos += 2;
                        column += 2;
                    }
                    else {
                        tokens.push_back({TokenType::Loop, "&", line});
                        ++pos;
                        ++column;
                    }
                }
                break;
            case '?':
                if (pos + 1 < source.size()) {
                    if (source[pos + 1] == '?') {
                        // ??
                        if (pos + 2 < source.size() && source[pos + 2] == '?') {
                            tokens.push_back({TokenType::Else, "???", line});
                            pos += 3;
                            column += 3;
                        } 
                        else {
                            tokens.push_back({TokenType::ElseIf, "??", line});
                            pos += 2;
                            column += 2;
                        }
                    } 
                    else {
                        tokens.push_back({TokenType::If, "?", line});
                        ++pos;
                        ++column;
                    } 
                }
                break;
            case '<':
                if (pos + 1 < source.size() && source[pos + 1] == '=') {
                    tokens.push_back({TokenType::LessThanOrEqual, "<=", line});
                    pos += 2;
                    column += 2;
                } 
                else if (pos + 1 < source.size() && source[pos + 1] == '<') {
                    tokens.push_back({TokenType::DoubleLAngleBracket, "<<", line});
                    pos += 2;
                    column += 2;
                } 
                else if (pos + 1 < source.size() && source[pos + 1] == '!') {
                    tokens.push_back({TokenType::ErrorOutput, "<!", line});
                    pos += 2;
                    column += 2;
                }
                else if (pos + 1 < source.size() && source[pos + 1] == '|') {
                    tokens.push_back({TokenType::Pop, "<|", line});
                    pos += 2;
                    column += 2;
                }
                else {
                    tokens.push_back({TokenType::LAngleBracket, "<", line});
                    ++pos;
                    ++column;
                }
                break;
            case '>':
                if (pos + 1 < source.size() && source[pos + 1] == '=') {
                    tokens.push_back({TokenType::GreaterThanOrEqual, ">=", line});
                    pos += 2;
                    column += 2;
                } 
                else if (pos + 1 < source.size() && source[pos + 1] == '>') {
                    tokens.push_back({TokenType::DoubleRAngleBracket, ">>", line});
                    pos += 2;
                    column += 2;
                }
                else {
                    tokens.push_back({TokenType::RAngleBracket, ">", line});
                    ++pos;
                    ++column;
                }
                break;
            case '=':
                if (pos + 1 < source.size() && source[pos + 1] == '=') {
                    tokens.push_back({TokenType::EqualTo, "==", line});
                    pos += 2;
                    column += 2;
                } 
                else {
                    tokens.push_back({TokenType::Assign, "=", line});
                    ++pos;
                    ++column;
                }
                break;
            case '+':
                if (pos + 1 < source.size() && source[pos + 1] == '=') {
                    tokens.push_back({TokenType::PlusEqual, "+=", line});
                    pos += 2;
                    column += 2;
                } 
                else {
                    tokens.push_back({TokenType::Plus, "+", line});
                    ++pos;
                    ++column;
                }
                break;
            case '-':
                if (pos + 1 < source.size() && source[pos + 1] == '=') {
                    tokens.push_back({TokenType::MinusEqual, "-=", line});
                    pos += 2;
                    column += 2;
                } 
                else {
                    tokens.push_back({TokenType::Minus, "-", line});
                    ++pos;
                    ++column;
                }
                break;
            case '*':
                if (pos + 1 < source.size() && source[pos + 1] == '=') {
                    tokens.push_back({TokenType::MultiplyEqual, "*=", line});
                    pos += 2;
                    column += 2;
                } 
                else {
                    tokens.push_back({TokenType::Multiply, "*", line});
                    ++pos;
                    ++column;
                }
                break;
            case '/':
                if (pos + 1 < source.size() && source[pos + 1] == '=') {
                    tokens.push_back({TokenType::DivideEqual, "/=", line});
                    pos += 2;
                    column += 2;
                } 
                else {
                    tokens.push_back({TokenType::Divide, "/", line});
                    ++pos;
                    ++column;
                }
                break;
            case '#':
                if (pos + 1 < source.size() && source[pos + 1] == ':') {
                    tokens.push_back({TokenType::IntCast, "#:", line});
                    pos += 2;
                    column += 2;
                } else {
                    tokens.push_back({TokenType::Hash, "#", line});
                    ++pos;
                    ++column;
                }
                break;
            case '@':
                if (pos + 1 < source.size() && source[pos + 1] == ':') {
                    tokens.push_back({TokenType::StrCast, "@:", line});
                    pos += 2;
                    column += 2;
                } else {
                    tokens.push_back({TokenType::At, "@", line});
                    ++pos;
                    ++column;
                }
                break;
            case '~':
                if (pos + 1 < source.size() && source[pos + 1] == ':') {
                    tokens.push_back({TokenType::FloatCast, "~:", line});
                    pos += 2;
                    column += 2;
                } else {
                    tokens.push_back({TokenType::Tilde, "~", line});
                    ++pos;
                    ++column;
                }
                break;
            case '%':
                if (pos + 1 < source.size() && source[pos + 1] == ':') {
                    tokens.push_back({TokenType::BoolCast, "%:", line});
                    pos += 2;
                    column += 2;
                }
                else if (pos + 1 < source.size() && (source[pos + 1] == '0' || source[pos + 1] == '1')) {
                    tokens.push_back({TokenType::Boolean, source.substr(pos, 2), line});
                    pos += 2;
                    column += 2;
                } 
                else {
                    if (pos + 1 < source.size() && source[pos + 1] == '=') {
                        tokens.push_back({TokenType::ModulusEqual, "%=", line});
                        pos += 2;
                        column += 2;
                    } 
                    else {
                        tokens.push_back({TokenType::Modulus, "%", line});
                        ++pos;
                        ++column;
                    }
                }
                break;

            case '|':
                if (pos + 1 < source.size() && source[pos + 1] == '|') {
                    tokens.push_back({TokenType::Or, "||", line});
                    pos += 2;
                    column += 2;
                } 
                else {
                    addError("Unknown character sequence '|" + std::string(1, source[pos + 1]) + "'", getContextAroundPosition());
                    return tokens;
                }
                break;
            case '!':
                if (pos + 1 < source.size() && source[pos + 1] == '=') {
                    tokens.push_back({TokenType::NotEqualTo, "!=", line});
                    pos += 2;
                    column += 2;
                } 
                else {
                    tokens.push_back({TokenType::Not, "!", line});
                    ++pos;
                    ++column;
                }
                break;
            default:
                if (isalpha(c)) {
                    std::string symbol(1, c);
                    ++pos;
                    ++column;
                    while (pos < source.size() && (isalnum(source[pos]) || source[pos] == '_')) {
                        symbol += source[pos++];
                        ++column;
                    }
                    tokens.push_back({TokenType::Symbol, symbol, line});
                } 
                else {
                    addError("Unknown character '" + std::string(1, c) + "'", getContextAroundPosition());
                    return tokens;
                }
                break;
        }
    }

    // 終端トークンを追加
    tokens.push_back({TokenType::End, "", line});
    return tokens;
}

// エラー情報を出力
void Lexer::printErrors() const {
    for (const auto& error : errors) {
        std::cerr << error.toString() << std::endl;
    }
}

void printTokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        std::cout << "Token: " << tokenType2String(token.type)
                  << ", Value: " << token.value 
                  << ", Line: " << token.line << std::endl;
    }
}
