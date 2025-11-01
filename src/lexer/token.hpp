// SigNum Tokens

#pragma once

#include <string>

// トークン
enum class TokenType {
    Symbol,
    Integer,
    Float,
    Boolean,
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
    DoubleColon,
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
    CharCodeToInt,
    IntToCharCode,
    Hash,
    At,
    Dollar,
    Tilde,
    End,
    IntegerStackPush,
    IntegerStackPop,
    FloatStackPush,
    FloatStackPop,
    StringStackPush,
    StringStackPop,
    BooleanStackPush,
    BooleanStackPop,
    MemoryMapRef,
    MapWindowSlide
};

// トークン構造体
struct Token {
    TokenType type;
    std::string value;
    size_t line = 0; // 行番号
};

// トークン型を文字列に変換
std::string tokenType2String(TokenType type);
