#include <iostream>
#include <string>
#include <vector>

enum class TokenType {
    Symbol,
    Number,
    MemoryRef,
    Function,
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

struct Token {
    TokenType type;
    std::string value;
    size_t line = 0; // 行番号
};

// メモリ参照を解析
std::string parseMemoryRef(const std::string& src, size_t& pos) {
    std::string memref = "";
    memref += src[pos++]; // '$'
    
    if (pos < src.size() && (src[pos] == '#' || src[pos] == '@' || src[pos] == '~' || src[pos] == '%')) {
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
    size_t pos = 0;
    while (pos < src.size()) {
        char c = src[pos];
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
                tokens.push_back({TokenType::String, src.substr(start, pos - start)});
                ++pos; // Skip closing quote
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
            tokens.push_back({TokenType::Function, funcCall});
            continue;
        }

        // メモリ参照
        if (src[pos] == '$') {
            size_t startPos = pos;
            std::string memref = parseMemoryRef(src, pos);
            tokens.push_back({TokenType::MemoryRef, memref});
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
            tokens.push_back({TokenType::Function, funcid});
            continue;
        }

        // 数字
        if (isdigit(c)) {
            size_t start = pos;
            while (pos < src.size() && isdigit(src[pos])) {
                ++pos;
            }
            tokens.push_back({TokenType::Number, src.substr(start, pos - start)});
            continue;
        }

        // 記号系
        switch (c) {
            case '{':
                tokens.push_back({TokenType::LBrace, "{"});
                ++pos;
                break;
            case '}':
                tokens.push_back({TokenType::RBrace, "}"});
                ++pos;
                break;
            case '(':
                tokens.push_back({TokenType::LParen, "("});
                ++pos;
                break;
            case ')':
                tokens.push_back({TokenType::RParen, ")"});
                ++pos;
                break;
            case '[':
                tokens.push_back({TokenType::LBracket, "["});
                ++pos;
                break;
            case ']':
                tokens.push_back({TokenType::RBracket, "]"});
                ++pos;
                break;
            case ',':
                tokens.push_back({TokenType::Comma, ","});
                ++pos;
                break;
            case ';':
                tokens.push_back({TokenType::Semicolon, ";"});
                ++pos;
                break;
            case ':':
                tokens.push_back({TokenType::Colon, ":"});
                ++pos;
                break;
            case '&':
                if (pos + 1 < src.size()) {
                    if (src[pos + 1] == '(') {
                        tokens.push_back({TokenType::If, "&"});
                        ++pos;  // & を処理
                    } 
                    else if (src[pos + 1] == '{') {
                        tokens.push_back({TokenType::Loop, "&{"});
                        pos += 2;
                    } 
                    else if (src[pos + 1] == '&') {
                        tokens.push_back({TokenType::And, "&&"});
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
                            tokens.push_back({TokenType::Else, "???"});
                            pos += 3;
                        } 
                        else {
                            tokens.push_back({TokenType::ElseIf, "??"});
                            pos += 2;
                        }
                    } 
                    else if (src[pos + 1] == '(' || src[pos + 1] == '{') {
                        tokens.push_back({TokenType::If, "?"});
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
                    tokens.push_back({TokenType::LessThanOrEqual, "<="});
                    pos += 2;
                } 
                else if (pos + 1 < src.size() && src[pos + 1] == '<') {
                    tokens.push_back({TokenType::DoubleLAngleBracket, "<<"});
                    pos += 2;
                } 
                else if (pos + 1 < src.size() && src[pos + 1] == '!') {
                    tokens.push_back({TokenType::ErrorOutput, "<!"});
                    pos += 2;
                }
                else {
                    tokens.push_back({TokenType::LAngleBracket, "<"});
                    ++pos;
                }
                break;
            case '>':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::GreaterThanOrEqual, ">="});
                    pos += 2;
                } 
                else if (pos + 1 < src.size() && src[pos + 1] == '>') {
                    tokens.push_back({TokenType::DoubleRAngleBracket, ">>"});
                    pos += 2;
                }
                else {
                    tokens.push_back({TokenType::RAngleBracket, ">"});
                    ++pos;
                }
                break;
            case '=':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::EqualTo, "=="});
                    pos += 2;
                } 
                else {
                    tokens.push_back({TokenType::Assign, "="});
                    ++pos;
                }
                break;
            case '+':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::PlusEqual, "+="});
                    pos += 2;
                } 
                else {
                    tokens.push_back({TokenType::Plus, "+"});
                    ++pos;
                }
                break;
            case '-':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::MinusEqual, "-="});
                    pos += 2;
                } 
                else {
                    tokens.push_back({TokenType::Minus, "-"});
                    ++pos;
                }
                break;
            case '*':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::MultiplyEqual, "*="});
                    pos += 2;
                } 
                else {
                    tokens.push_back({TokenType::Multiply, "*"});
                    ++pos;
                }
                break;
            case '/':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::DivideEqual, "/="});
                    pos += 2;
                } 
                else {
                    tokens.push_back({TokenType::Divide, "/"});
                    ++pos;
                }
                break;
            case '#':
                if (pos + 1 < src.size() && src[pos + 1] == ':') {
                    tokens.push_back({TokenType::IntCast, "#:"});
                    pos += 2;
                } else {
                    tokens.push_back({TokenType::Hash, "#"});
                    ++pos;
                }
                break;
            case '@':
                if (pos + 1 < src.size() && src[pos + 1] == ':') {
                    tokens.push_back({TokenType::StrCast, "@:"});
                    pos += 2;
                } else {
                    tokens.push_back({TokenType::At, "@"});
                    ++pos;
                }
                break;
            case '~':
                if (pos + 1 < src.size() && src[pos + 1] == ':') {
                    tokens.push_back({TokenType::FloatCast, "~:"});
                    pos += 2;
                } else {
                    tokens.push_back({TokenType::Tilde, "~"});
                    ++pos;
                }
                break;
            case '%':
                if (pos + 1 < src.size() && src[pos + 1] == ':') {
                    tokens.push_back({TokenType::BoolCast, "%:"});
                    pos += 2;
                } 
                else {
                    if (pos + 1 < src.size() && src[pos + 1] == '=') {
                        tokens.push_back({TokenType::ModulusEqual, "%="});
                        pos += 2;
                    } 
                    else {
                        tokens.push_back({TokenType::Modulus, "%"});
                        ++pos;
                    }
                }
                break;

            case '|':
                if (pos + 1 < src.size() && src[pos + 1] == '|') {
                    tokens.push_back({TokenType::Or, "||"});
                    pos += 2;
                } 
                else {
                    std::cerr << "Error: Unknown character sequence '|" << src[pos + 1] << "'\n";
                    return {};
                }
                break;
            case '!':
                if (pos + 1 < src.size() && src[pos + 1] == '=') {
                    tokens.push_back({TokenType::NotEqualTo, "!="});
                    pos += 2;
                } 
                else {
                    tokens.push_back({TokenType::Not, "!"});
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
                    tokens.push_back({TokenType::Symbol, symbol});
                } 
                else {
                    std::cerr << "Error: Unknown character '" << c << "'\n";
                    return {};
                }
                break;
        }
    }

    // 終端トークンを追加
    tokens.push_back({TokenType::End, ""});
    return tokens;
}

int main() {
    std::string test_code = R"(
    _001{
        ?($#0 % 15 == 0){
            < "FizzBuzz";
        }
        ??($#0 % 3 == 0){
            < "Fizz";
        }
        ??($#0 % 5 == 0){
            < "Buzz";
        }
    }

    _000{
        $#0 = 1;

        &($#0 <= 100){
            $_001;
            $#0 += 1;
        }

        "FileName" >> $@1;
        $#1 = #: $@1;
        $#0 = 0;
        $#1 = 0;
        $@$#0#1 = "Hello, Wolrd!";
        <! "End";
    }
    )";
    auto tokens = tokenize(test_code);
    
    std::cout << "トークン解析結果：\n";
    for (const auto& token : tokens) {
        std::cout << "タイプ: " << static_cast<int>(token.type) << ", 値: \"" << token.value << "\"\n";
    }
    return 0;
}