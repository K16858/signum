// SigNum Lexer
#pragma once

#include <iostream>
#include <string>
#include <vector>

// トークン
enum class TokenType {
    Symbol,
    Integer,
    Float,
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

std::string tokenType2String(TokenType type);
std::string parseMemoryRef(const std::string& src, size_t& pos);
std::vector<Token> tokenize(const std::string& src);
void printTokens(const std::vector<Token>& tokens);
