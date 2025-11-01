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
    std::vector<Token> tokens;  // トークンのリスト
    size_t pos = 0;             // 解析位置
    bool hasError = false;      // エラーフラグ
    bool debugMode = false;    // デバッグモード
    std::vector<std::string> errors; // エラーリスト

public:
    Parser(const std::vector<Token>& tokens, bool debug = false) 
    : tokens(tokens), pos(0), debugMode(debug) {} // コンストラクタ

     // コード全体を解析
    std::shared_ptr<ASTNode> parseProgram();

    // 関数の解析
    std::shared_ptr<ASTNode> parseFunction();

    // 関数呼び出しの解析 
    std::shared_ptr<ASTNode> parseFunctionCall();

    // ステートメントの解析
    std::shared_ptr<ASTNode> parseStatement();

    // 加減算式の解析
    std::shared_ptr<ASTNode> parseExpression();

    // 乗除算式の解析           
    std::shared_ptr<ASTNode> parseTerm();

    // 因子の解析
    std::shared_ptr<ASTNode> parseFactor();

    // メモリ参照の解析
    std::shared_ptr<ASTNode> parseMemoryRef();

    // 条件式の解析
    std::shared_ptr<ASTNode> parseCondition();

    // 比較演算子の解析
    std::shared_ptr<ASTNode> parseComparison();

    // 条件分岐の解析
    std::shared_ptr<ASTNode> parseIfStatement();

     // ループの解析
    std::shared_ptr<ASTNode> parseLoopStatement();

    // 代入文の解析
    std::shared_ptr<ASTNode> parseAssignment();

    // 入力文の解析
    std::shared_ptr<ASTNode> parseInputStatement();

    // 出力文の解析
    std::shared_ptr<ASTNode> parseOutputStatement();

    // ファイル入力文の解析
    std::shared_ptr<ASTNode> parseFileInputStatement();

    // ファイル出力文の解析
    std::shared_ptr<ASTNode> parseFileOutputStatement();

    // 型変換の解析
    std::shared_ptr<ASTNode> parseCast();

    // 文字コード変換の解析
    std::shared_ptr<ASTNode> parseCharCodeCast();

    // スタック操作の解析
    std::shared_ptr<ASTNode> parseStackOperation();

    // メモリマップ参照の解析
    std::shared_ptr<ASTNode> parseMemoryMapRef();

    // マップウィンドウスライドの解析
    std::shared_ptr<ASTNode> parseMapWindowSlide();

    // エラー関連
    bool hasErrors() const { return !errors.empty(); }
    void printErrors() const {
        for (const auto& error : errors) {
            std::cerr << error << std::endl;
        }
    }

private:
    Token& getToken() { return tokens[pos]; }       // 現在のトークンを取得
    void advance() { pos++; }                       // 次のトークンに進む
    void reportError(const std::string& message) {  // エラーレポート
        if (pos < tokens.size()) {
            std::string errorMsg = "Parse Error at line " + std::to_string(tokens[pos].line) + 
                                 ": " + message + " (token: '" + tokens[pos].value + "')";
            errors.push_back(errorMsg);
        } else {
            errors.push_back("Parse Error: " + message + " (at end of input)");
        }
        hasError = true;
    }
    std::shared_ptr<ASTNode> recoverFromError(const std::string& message); // エラーから回復
    void synchronize();
    void debugLog(const std::string& message) {
        if (debugMode) {
            std::cout << message << std::endl;
        }
    }
};
