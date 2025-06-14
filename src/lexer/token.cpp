// SigNum Tokens

#include "token.hpp"

// トークン型を文字列に変換する関数
std::string tokenType2String(TokenType type) {
    switch (type) {
        case TokenType::Symbol: return "Symbol";            // シンボル
        case TokenType::Integer: return "Integer";          // 整数値
        case TokenType::Float: return "Float";              // 浮動小数点数
        case TokenType::Boolean: return "Boolean";          // ブール値
        case TokenType::MemoryRef: return "MemoryRef";      // メモリ参照
        case TokenType::Function: return "Function";        // 関数
        case TokenType::FunctionCall: return "FunctionCall";// 関数呼び出し
        case TokenType::String: return "String";            // 文字列
        case TokenType::LBrace: return "{";                 // 左中括弧
        case TokenType::RBrace: return "}";                 // 右中括弧
        case TokenType::LParen: return "(";                 // 左括弧
        case TokenType::RParen: return ")";                 // 右括弧
        case TokenType::LBracket: return "[";               // 左角括弧
        case TokenType::RBracket: return "]";               // 右角括弧
        case TokenType::LAngleBracket: return "<";          // 左山括弧
        case TokenType::RAngleBracket: return ">";          // 右山括弧
        case TokenType::DoubleLAngleBracket: return "<<";   // 左山括弧2つ
        case TokenType::DoubleRAngleBracket: return ">>";   // 右山括弧2つ
        case TokenType::ErrorOutput: return "<!";           // エラー出力
        case TokenType::Comma: return ",";                  // カンマ
        case TokenType::Colon: return ":";                  // コロン
        case TokenType::Semicolon: return ";";              // セミコロン
        case TokenType::If: return "if";                    // if
        case TokenType::ElseIf: return "elseif";            // elseif
        case TokenType::Else: return "else";                // else
        case TokenType::Loop: return "loop";                // loop
        case TokenType::Assign: return "=";                 // 代入
        case TokenType::Plus: return "+";                   // 足し算
        case TokenType::Minus: return "-";                  // 引き算
        case TokenType::PlusEqual: return "+=";             // 足し算代入
        case TokenType::MinusEqual: return "-=";            // 引き算代入
        case TokenType::Multiply: return "*";               // 掛け算
        case TokenType::Divide: return "/";                 // 割り算
        case TokenType::MultiplyEqual: return "*=";         // 掛け算代入
        case TokenType::DivideEqual: return "/=";           // 割り算代入
        case TokenType::Modulus: return "%";                // 割り算余り
        case TokenType::ModulusEqual: return "%=";          // 割り算余り代入
        case TokenType::And: return "And";                  // 論理積
        case TokenType::Or: return "Or";                    // 論理和
        case TokenType::Not: return "Not";                  // 否定
        case TokenType::LessThanOrEqual: return "<=";       // 以下
        case TokenType::GreaterThanOrEqual: return ">=";    // 以上
        case TokenType::EqualTo: return "==";               // 等価
        case TokenType::NotEqualTo: return "!=";            // 非等価
        case TokenType::IntCast: return "IntCast";          // 整数キャスト
        case TokenType::FloatCast: return "FloatCast";      // 浮動小数点キャスト
        case TokenType::StrCast: return "StrCast";          // 文字列キャスト
        case TokenType::BoolCast: return "BoolCast";        // ブールキャスト
        case TokenType::Hash: return "#";                   // ハッシュ
        case TokenType::At: return "@";                     // アットマーク
        case TokenType::Dollar: return "$";                 // ドル
        case TokenType::Tilde: return "~";                  // チルダ
        case TokenType::End: return "END";                  // 終端トークン
        default: return "Unknown";                             // 不明なトークン
    }
}
