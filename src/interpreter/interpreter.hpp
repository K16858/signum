// SigNum Interpreter

#pragma once

#include <iostream>
#include <unordered_map>
#include <array>
#include <variant>
#include <string>
#include <memory>
#include <vector>
#include "../ast/ast.hpp"

// 値の型
using Value = std::variant<int, double, std::string, bool>;

// メモリプールのサイズ
constexpr size_t MEMORY_POOL_SIZE = 64;
constexpr size_t ARGS_START = 48;
constexpr size_t RETURN_START = 56;
constexpr size_t SYSTEM_START = 60;

class Interpreter {
private:
    // 各型のメモリプール
    std::array<Value, MEMORY_POOL_SIZE> intPool;    // # (整数)
    std::array<Value, MEMORY_POOL_SIZE> stringPool; // @ (文字列)
    std::array<Value, MEMORY_POOL_SIZE> floatPool;  // ~ (浮動小数点)
    std::array<Value, MEMORY_POOL_SIZE> boolPool;   // % (真偽値)
    
    // 関数テーブル
    std::unordered_map<int, std::shared_ptr<ASTNode>> functions;
    
    // メモリ参照を解決する
    Value resolveMemoryRef(const std::string& ref);
    int evaluateMemoryIndex(const std::string& indexExpr);
    
    // 値を文字列に変換
    static std::string valueToString(const Value& val);

public:
    Interpreter(){
        // メモリプールの初期化
        intPool.fill(0);
        stringPool.fill("");
        floatPool.fill(0.0);
        boolPool.fill(false);
    }
    ~Interpreter() = default;
    
    // 実行
    void interpret(const std::shared_ptr<ASTNode>& program);
    
    // 評価
    Value evaluateNode(const std::shared_ptr<ASTNode>& node);
    Value evaluateProgram(const std::shared_ptr<ASTNode>& program);
    Value evaluateFunction(const std::shared_ptr<ASTNode>& node);
    Value evaluateFunctionCall(const std::shared_ptr<ASTNode>& node);
    Value evaluateAssignment(const std::shared_ptr<ASTNode>& node);
    Value evaluateArithmeticExpression(const std::shared_ptr<ASTNode>& node);
    Value evaluateLogicalExpression(const std::shared_ptr<ASTNode>& node);
    Value evaluateMemoryRef(const std::shared_ptr<ASTNode>& node);
    Value evaluateNumber(const std::shared_ptr<ASTNode>& node);
    Value evaluateString(const std::shared_ptr<ASTNode>& node);
    Value evaluateComparison(const std::shared_ptr<ASTNode>& node);
    Value evaluateCast(const std::shared_ptr<ASTNode>& node);
    Value evaluateIfStatement(const std::shared_ptr<ASTNode>& node);
    Value evaluateLoopStatement(const std::shared_ptr<ASTNode>& node);
    Value evaluateInputStatement(const std::shared_ptr<ASTNode>& node);
    Value evaluateOutputStatement(const std::shared_ptr<ASTNode>& node);
    Value evaluateFileInputStatement(const std::shared_ptr<ASTNode>& node);
    Value evaluateFileOutputStatement(const std::shared_ptr<ASTNode>& node);
    
    // 変数の取得と設定
    Value getMemoryValue(char type, int index);
    void setMemoryValue(char type, int index, const Value& value);
};
