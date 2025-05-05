// SigNum Parser
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"

// 構文解析器
class Parser {
private: 
    std::vector<Token> tokens; // トークンのリスト
    size_t pos = 0; // 解析位置
    bool hasError = false; // エラーフラグ

public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens) {} // コンストラクタ

    std::unique_ptr<ASTNode> parseProgram(); // コード全体を解析
    std::unique_ptr<ASTNode> parseFunction(); // 関数の解析
    std::unique_ptr<ASTNode> parseFunctionCall(); // 関数呼び出しの解析
    std::unique_ptr<ASTNode> parseStatement(); // ステートメントの解析
    std::unique_ptr<ASTNode> parseExpression(); // 加減算式の解析
    std::unique_ptr<ASTNode> parseTerm(); // 乗除算式の解析
    std::unique_ptr<ASTNode> parseFactor(); // 因子の解析
    std::unique_ptr<ASTNode> parseMemoryRef(); // メモリ参照の解析
    std::unique_ptr<ASTNode> parseCondition(); // 条件式の解析
    std::unique_ptr<ASTNode> parseComparison(); // 比較演算子の解析
    std::unique_ptr<ASTNode> parseIfStatement(); // 条件分岐の解析
    std::unique_ptr<ASTNode> parseLoopStatement(); // ループの解析
    std::unique_ptr<ASTNode> parseAssignment(); // 代入文の解析
    std::unique_ptr<ASTNode> parseInputStatement(); // 入力文の解析
    std::unique_ptr<ASTNode> parseOutputStatement(); // 出力文の解析
    std::unique_ptr<ASTNode> parseFileInputStatement(); // ファイル入力文の解析
    std::unique_ptr<ASTNode> parseFileOutputStatement(); // ファイル出力文の解析
    std::unique_ptr<ASTNode> parseCast(); // 型変換の解析

private:
    Token& getToken() { return tokens[pos]; } // 現在のトークンを取得
    void advance() { pos++; } // 次のトークンに進む
    void reportError(const std::string& message) { // エラーレポート
        std::cerr << "Error: " << message << " at token " << pos << std::endl;
        hasError = true;
    }
};
