// SigNum Lexer
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "token.hpp"

// エラー情報用構造体
struct LexerError {
    std::string message;
    size_t line;
    size_t column;
    std::string context; // エラー箇所の周辺コード
    
    std::string toString() const {
        return "Line " + std::to_string(line) + ":" + std::to_string(column) + 
               " - Lexer Error: " + message + "\n" + context;
    }
};

class Lexer {
private:
    std::string source; // ソースコード
    size_t pos;         // 現在の解析位置
    size_t line;        // 現在の行番号
    size_t column;      // 現在の列番号
    std::vector<LexerError> errors; // エラー情報のリスト

    void addError(const std::string& message, const std::string& context) {
        errors.push_back({message, line, column, context});
    }
    
    std::string getContextAroundPosition() const;
    
public:
    Lexer(const std::string& src)
        : source(src), pos(0), line(1), column(1) {}

    // メモリ参照を解析
    std::string parseMemoryRef();

    // 字句解析関数
    std::vector<Token> tokenize();
    
    size_t getLine() const { return line; }
    size_t getColumn() const { return column; }
    
    // エラー関連
    bool hasErrors() const { return !errors.empty(); }
    const std::vector<LexerError>& getErrors() const { return errors; }
    void printErrors() const;

    void reset() {
        pos = 0;
        line = 1;
        column = 1;
        errors.clear();
    }
};

// トークン表示関数
void printTokens(const std::vector<Token>& tokens);
