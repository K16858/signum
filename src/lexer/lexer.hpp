// SigNum Lexer
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "token.hpp"

class Lexer {
private:
    std::string source; // ソースコード
    size_t pos;         // 現在の解析位置
    size_t line;        // 現在の行番号
    size_t column;      // 現在の列番号
public:
    Lexer(const std::string& src)
        : source(src), pos(0), line(1), column(1) {}

    // メモリ参照を解析
    std::string parseMemoryRef();

    // 字句解析関数
    std::vector<Token> tokenize();
    
    size_t getLine() const { return line; }
    size_t getColumn() const { return column; }

    void reset();
};

// トークン表示関数
void printTokens(const std::vector<Token>& tokens);
