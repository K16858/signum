// SigNum AST

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>

// ASTノードの種類
enum class NodeType {
    Program,
    Function,
    FunctionCall,
    Statement,
    ArithmeticExpression,
    LogicalExpression,
    Factor,
    MemoryRef,
    Number,
    String,
    Symbol,
    Operator,
    Comparison,
    Condition,
    Cast,
    CharCodeCast,
    StringIndex,
    StringLength,
    IfStatement,
    LoopStatement,
    Assignment,
    InputStatement,
    OutputStatement,
    FileInputStatement,
    FileOutputStatement,
    StackOperation,
    MemoryMapRef,
    MapWindowSlide,
    Error, // エラー用ノード
};

// ノード型を文字列に変換
std::string nodeType2String(NodeType type);

// ASTノード
struct ASTNode {
    NodeType type; // ノードの種類
    std::string value; // ノードの値
    std::vector<std::shared_ptr<ASTNode>> children; // 子ノードのリスト

    ASTNode(NodeType type, const std::string& value = "");
    virtual ~ASTNode() = default;

    // デバッグ用表示メソッド
    void print(int indent = 0) const;
    
    // JSON変換
    std::string toJSON(int indent = 0) const;
    
    // JSONエスケープ処理
    static std::string escapeJSON(const std::string& input);
    
    // JSONファイルに保存
    bool saveToJSONFile(const std::string& filename) const;
};
