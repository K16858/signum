// SigNum Lexer

#include <iostream>
#include <string>
#include <vector>

// トークン
enum class TokenType {
    Symbol,
    Number,
    MemoryRef,
    Function,
    FunctionCall,
    String,
    LBrace,
    RBrace,
    LParen,
    RParen,
    LBracket,
    RBracket,
    LAngleBracket,
    RAngleBracket,
    DoubleLAngleBracket,
    DoubleRAngleBracket,
    ErrorOutput,
    Comma,
    Colon,
    Cast,
    Semicolon,
    If,
    ElseIf,
    Else,
    Loop,
    Assign,
    Plus,
    Minus,
    PlusEqual,
    MinusEqual,
    Multiply,
    Divide,
    MultiplyEqual,
    DivideEqual,
    Modulus,
    ModulusEqual,
    And,
    Or,
    Not,
    LessThanOrEqual,
    GreaterThanOrEqual,
    EqualTo,
    NotEqualTo,
    IntCast,
    FloatCast,
    StrCast,
    BoolCast,
    Hash,
    At,
    Dollar,
    Tilde,
    End,
};

// トークン構造体
struct Token {
    TokenType type;
    std::string value;
    size_t line = 0; // 行番号
};

std::string tokenType2String(TokenType type) {
    switch (type) {
        case TokenType::Symbol: return "シンボル";
        case TokenType::Number: return "数値";
        case TokenType::MemoryRef: return "メモリ参照";
        case TokenType::Function: return "関数宣言";
        case TokenType::FunctionCall: return "関数呼び出し";
        case TokenType::String: return "文字列";
        case TokenType::LBrace: return "{"; // 左中括弧
        case TokenType::RBrace: return "}"; // 右中括弧
        case TokenType::LParen: return "("; // 左括弧
        case TokenType::RParen: return ")"; // 右括弧
        case TokenType::LBracket: return "["; // 左角括弧
        case TokenType::RBracket: return "]"; // 右角括弧
        case TokenType::LAngleBracket: return "<"; // 左山括弧
        case TokenType::RAngleBracket: return ">"; // 右山括弧
        case TokenType::DoubleLAngleBracket: return "<<"; // 左山括弧2つ
        case TokenType::DoubleRAngleBracket: return ">>"; // 右山括弧2つ
        case TokenType::ErrorOutput: return "<!"; // エラー出力
        case TokenType::Comma: return ","; // カンマ
        case TokenType::Colon: return ":"; // コロン
        case TokenType::Semicolon: return ";"; // セミコロン
        case TokenType::If: return "if"; // if
        case TokenType::ElseIf: return "elseif"; // elseif
        case TokenType::Else: return "else"; // else
        case TokenType::Loop: return "loop"; // loop
        case TokenType::Assign: return "="; // 代入
        case TokenType::Plus: return "+"; // 足し算
        case TokenType::Minus: return "-"; // 引き算
        case TokenType::PlusEqual: return "+="; // 足し算代入
        case TokenType::MinusEqual: return "-="; // 引き算代入
        case TokenType::Multiply: return "*"; // 掛け算
        case TokenType::Divide: return "/"; // 割り算
        case TokenType::MultiplyEqual: return "*="; // 掛け算代入
        case TokenType::DivideEqual: return "/="; // 割り算代入
        case TokenType::Modulus: return "%"; // 割り算余り
        case TokenType::ModulusEqual: return "%="; // 割り算余り代入
        case TokenType::And: return "&&"; // 論理積
        case TokenType::Or: return "||"; // 論理和
        case TokenType::Not: return "!"; // 否定
        case TokenType::LessThanOrEqual: return "<="; // 以下
        case TokenType::GreaterThanOrEqual: return ">="; // 以上
        case TokenType::EqualTo: return "=="; // 等価
        case TokenType::NotEqualTo: return "!="; // 等しくない
        case TokenType::IntCast: return "#:"; // 整数キャスト
        case TokenType::FloatCast: return "~:"; // 浮動小数点キャスト
        case TokenType::StrCast: return "@:"; // 文字列キャスト
        case TokenType::BoolCast: return "%:"; // ブールキャスト
        case TokenType::Hash: return "#"; // ハッシュ
        case TokenType::At: return "@"; // アットマーク
        case TokenType::Dollar: return "$"; // ドル
        case TokenType::Tilde: return "~"; // チルダ
        case TokenType::End: return "END"; // 終端トークン
        default: return "不明"; // 不明なトークン
    }
}

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
            while (pos < src.size() && isdigit(src[pos])) {
                ++pos;
            }
            tokens.push_back({TokenType::Number, src.substr(start, pos - start), line});
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
                    if (src[pos + 1] == '(') {
                        tokens.push_back({TokenType::If, "&", line});
                        ++pos;  // & を処理
                    } 
                    else if (src[pos + 1] == '{') {
                        tokens.push_back({TokenType::Loop, "&{", line});
                        pos += 2;
                    } 
                    else if (src[pos + 1] == '&') {
                        tokens.push_back({TokenType::And, "&&", line});
                        pos += 2;
                    }
                    else {
                        std::cerr << "Error: Unknown character sequence '&" << src[pos + 1] << "'\n";
                        return {};
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
                    else if (src[pos + 1] == '(' || src[pos + 1] == '{') {
                        tokens.push_back({TokenType::If, "?", line});
                        ++pos;
                    } 
                    else {
                        std::cerr << "Error: Unknown character sequence '?" << src[pos + 1] << "'\n";
                        return {};
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
        std::cout << "トークン: " << tokenType2String(token.type)
                  << ", 値: " << token.value 
                  << ", 行: " << token.line << std::endl;
    }
}
