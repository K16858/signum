// SigNum Parser
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "lexer.hpp"

// ASTノードの種類
enum class NodeType {
    Program,
    Function,
    Statement,
    Expression,
    MemoryRef,
    Number,
    String,
    Symbol,
    Operator,
    Cast,
    IfStatement,
    LoopStatement,
    Assignment,
    IOStatement,
};

// ASTノード
struct ASTNode {
    NodeType type; // ノードの種類
    std::string value; // ノードの値
    std::vector<std::unique_ptr<ASTNode>> children; // 子ノードのリスト

    ASTNode(NodeType type, const std::string& value = "") : type(type), value(value) {} // コンストラクタ
    virtual ~ASTNode() = default; // デストラクタ

    // デバッグ用
    void print(int indent = 0) const {
        for (int i = 0; i < indent; i++) std::cout << "  ";
        std::cout << "ノード: " << static_cast<int>(type) << ", 値: " << value << std::endl;
        
        for (const auto& child : children) {
            child->print(indent + 1);
        }
    }
};

// 構文解析器
class Parser {
private: 
    std::vector<Token> tokens; // トークンのリスト
    size_t pos = 0; // 解析位置

public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens) {} // コンストラクタ

    std::unique_ptr<ASTNode> parseCode(); // コード全体を解析
    std::unique_ptr<ASTNode> parseFunction(); // 関数の解析
    std::unique_ptr<ASTNode> parseStatement(); // ステートメントの解析
    std::unique_ptr<ASTNode> parseExpression(); // 式の解析
    std::unique_ptr<ASTNode> parseMemoryRef(); // メモリ参照の解析
};
