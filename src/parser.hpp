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
};

