// SigNum Parser
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include "lexer.hpp"

// ASTノードの種類
enum class NodeType {
    Program,
    Function,
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
    IfStatement,
    LoopStatement,
    Assignment,
    IOStatement,
};

inline std::string nodeType2String(NodeType type) {
    switch (type) {
        case NodeType::Program: return "ルート";
        case NodeType::Function: return "関数定義";
        case NodeType::Statement: return "ステートメント";
        case NodeType::ArithmeticExpression: return "四則演算式";
        case NodeType::LogicalExpression: return "論理式";
        case NodeType::Factor: return "因子";
        case NodeType::MemoryRef: return "メモリ参照";
        case NodeType::Number: return "数値";
        case NodeType::String: return "文字列";
        case NodeType::Symbol: return "シンボル";
        case NodeType::Operator: return "演算子";
        case NodeType::Comparison: return "比較演算式";
        case NodeType::Condition: return "条件式";
        case NodeType::Cast: return "型変換";
        case NodeType::IfStatement: return "条件分岐";
        case NodeType::LoopStatement: return "ループ";
        case NodeType::Assignment: return "代入";
        case NodeType::IOStatement: return "入出力";
        default: return "不明";
    }
}

// ASTノード
struct ASTNode {
    NodeType type; // ノードの種類
    std::string value; // ノードの値
    std::vector<std::unique_ptr<ASTNode>> children; // 子ノードのリスト
    // LSTとRSTが必要かも

    ASTNode(NodeType type, const std::string& value = "") : type(type), value(value) {} // コンストラクタ
    virtual ~ASTNode() = default; // デストラクタ

    // デバッグ用
    void print(int indent = 0) const {
        for (int i = 0; i < indent; i++) std::cout << "  ";
        std::cout << "ノード: " << nodeType2String(type) << ", 値: " << value << std::endl;
        
        for (const auto& child : children) {
            child->print(indent + 1);
        }
    }

    std::string toJSON(int indent = 0) const {
        std::string result = "{\n";
        std::string indentStr(indent + 2, ' ');
        std::string indentEndStr(indent, ' ');
        
        // タイプ
        result += indentStr + "\"type\": \"" + nodeType2String(type) + "\",\n";
        
        // 値（エスケープ処理）
        result += indentStr + "\"value\": \"" + escapeJSON(value) + "\"";
        
        // 子ノードがあれば追加
        if (!children.empty()) {
            result += ",\n" + indentStr + "\"children\": [\n";
            
            for (size_t i = 0; i < children.size(); ++i) {
                result += indentStr + "  " + children[i]->toJSON(indent + 2);
                if (i < children.size() - 1) {
                    result += ",";
                }
                result += "\n";
            }
            
            result += indentStr + "]\n";
        } else {
            result += "\n";
        }
        
        result += indentEndStr + "}";
        return result;
    }

    // JSONエスケープ処理
    static std::string escapeJSON(const std::string& input) {
        std::string result;
        for (char c : input) {
            switch (c) {
                case '\"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        char buf[7];
                        snprintf(buf, sizeof(buf), "\\u%04x", c);
                        result += buf;
                    } else {
                        result += c;
                    }
            }
        }
        return result;
    }

    // JSONファイルに保存
    bool saveToJSONFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "File cannot open: " << filename << std::endl;
            return false;
        }
        
        file << toJSON();
        file.close();
        return true;
    }
};

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
    std::unique_ptr<ASTNode> parseIOStatement(); // 入出力文の解析
    std::unique_ptr<ASTNode> parseCast(); // 型変換の解析

private:
    Token& getToken() { return tokens[pos]; } // 現在のトークンを取得
    void advance() { pos++; } // 次のトークンに進む
    void reportError(const std::string& message) { // エラーレポート
        std::cerr << "Error: " << message << " at token " << pos << std::endl;
        hasError = true;
    }
};
