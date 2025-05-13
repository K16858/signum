// SigNum Semantic Analyzer
#include "semantic.hpp"
#include <iostream>

std::string memoryTypeToString(MemoryType type) {
    switch (type) {
        case MemoryType::Integer: return "Integer";
        case MemoryType::Float: return "Float";
        case MemoryType::String: return "String";
        case MemoryType::Boolean: return "Boolean";
        default: return "Unknown";
    }
}

// メインの解析関数
bool SemanticAnalyzer::analyze(const std::shared_ptr<ASTNode>& root) {
    if (!root) return false;
    // 1パス目：関数定義を収集
    collectFunctionDefinitions(root.get());

    // 2パス目：ノードを巡回して意味解析を行う
    visitNode(root.get());
    return errors.empty();
}

// 関数定義収集
void SemanticAnalyzer::collectFunctionDefinitions(const ASTNode* node) {
    if (!node) return;
    
    // 関数定義を見つけたら登録
    if (node->type == NodeType::Function) {
        std::string funcID = node->value;
        bool idError = false;
        
        try {
            int funcIDInt = std::stoi(funcID);
            if (funcIDInt < 1 || funcIDInt > 999) {
                idError = true;
            }
        } catch (...) {
            idError = true;
        }
        
        // 関数を登録
        functions[funcID] = {funcID, !idError};
    }

    for (const auto& child : node->children) {
        collectFunctionDefinitions(child.get());
    }
}
// ノード巡回関数
MemoryType SemanticAnalyzer::visitNode(const ASTNode* node) {
    if (!node) return MemoryType::Integer; // デフォルト値
    switch (node->type) {
        case NodeType::Program:
            // プログラム全体を処理
            for (const auto& child : node->children) {
                visitNode(child.get());
            }
            return MemoryType::Integer;
            
        case NodeType::Function:
            // 関数定義
            checkFunctionDefinition(node);
            return MemoryType::Integer;
            
        case NodeType::FunctionCall:
            // 関数呼び出し
            checkFunctionCall(node);
            return MemoryType::Integer;
            
        case NodeType::Assignment:
            // 代入処理
            return checkAssignment(node);
            
        case NodeType::ArithmeticExpression:
            // 四則演算式
            return checkExpression(node);
            
        case NodeType::MemoryRef:
            // メモリ参照
            return checkMemoryRef(node);
            
        case NodeType::Number:
            // 数値
            if (node->value.find('.') != std::string::npos) {
                return MemoryType::Float; // 小数点があれば浮動小数点
            }
            return MemoryType::Integer;
            
        case NodeType::String:
            // 文字列リテラル
            return MemoryType::String;
            
        case NodeType::Cast:
            // 型変換
            return checkCast(node);
        
        case NodeType::Comparison:
            checkCondition(node);
            return MemoryType::Boolean;
            
        case NodeType::IfStatement:
            // 条件分岐
            for (const auto& child : node->children) {
                visitNode(child.get());
            }
            return MemoryType::Integer;
            
        case NodeType::LoopStatement:
            // ループ
            for (const auto& child : node->children) {
                visitNode(child.get());
            }
            return MemoryType::Integer;
            
        default:
            // その他のノード
            for (const auto& child : node->children) {
                visitNode(child.get());
            }
            return MemoryType::Integer;
    }
}

// メモリ参照のチェック
MemoryType SemanticAnalyzer::checkMemoryRef(const ASTNode* node) {
    // メモリ参照の形式をチェック
    std::string memRef = node->value;
    if (memRef.size() < 2 || memRef[0] != '$') {
        reportError("Invalid memoryRef: " + memRef);
        return MemoryType::Integer; // エラーの場合はデフォルト値
    }
    
    // メモリ範囲をチェック
    if (!checkMemoryRange(memRef)) {
        reportError("Memory out of range: " + memRef);
        return MemoryType::Integer;
    }
    
    // メモリタイプを取得
    return getTypeFromMemRef(memRef);
}

// メモリ参照から型を取得
MemoryType SemanticAnalyzer::getTypeFromMemRef(const std::string& memRef) {
    if (memRef.size() < 2) return MemoryType::Integer;
    
    char typeChar = memRef[1];
    switch (typeChar) {
        case '#': return MemoryType::Integer;
        case '~': return MemoryType::Float;
        case '@': return MemoryType::String;
        case '?': return MemoryType::Boolean;
        default:
            reportError("Unknown memory type: " + std::string(1, typeChar));
            return MemoryType::Integer;
    }
}

// メモリ番号の範囲チェック
bool SemanticAnalyzer::checkMemoryRange(const std::string& memRef) {
    size_t digitStart = 2;
    while (digitStart < memRef.size() && !isdigit(memRef[digitStart])) {
        digitStart++;
    }
    
    if (digitStart >= memRef.size()) {
        return false; // 数字が見つからない
    }
    
    try {
        int index = std::stoi(memRef.substr(digitStart));
        return index >= 0 && index < 128; // 0～127の範囲内かチェック
    } 
    catch (...) {
        return false; // 数値変換エラー
    }
}

// 代入のチェック
MemoryType SemanticAnalyzer::checkAssignment(const ASTNode* node) {
    if (node->children.size() < 2) {
        reportError("Assignment has too few children");
        return MemoryType::Integer;
    }
    
    // 左辺と右辺のチェック
    auto leftType = visitNode(node->children[0].get());
    auto rightType = visitNode(node->children[1].get());
    
    // 型の互換性をチェック
    if (!isCompatible(leftType, rightType)) {
        reportError("Expression type mismatch: " +
                    memoryTypeToString(leftType) + " vs " + 
                    memoryTypeToString(rightType));
    }
    
    // メモリ参照なら型を記録
    if (node->children[0]->type == NodeType::MemoryRef) {
        memoryTypes[node->children[0]->value] = leftType;
    }
    
    return leftType;
}

// 型の互換性チェック
bool SemanticAnalyzer::isCompatible(MemoryType lhs, MemoryType rhs) {
    // 同じ型なら互換性あり
    if (lhs == rhs) return true;
    
    // 数値型同士なら互換性あり（暗黙の型変換）
    if ((lhs == MemoryType::Integer || lhs == MemoryType::Float) &&
        (rhs == MemoryType::Integer || rhs == MemoryType::Float)) {
        return true;
    }
    
    // それ以外は互換性なし
    return false;
}

// 式のチェック
MemoryType SemanticAnalyzer::checkExpression(const ASTNode* node) {
    if (node->children.size() < 2) {
        reportError("Expression has too few children");
        return MemoryType::Integer;
    }
    
    auto leftType = visitNode(node->children[0].get());
    auto rightType = visitNode(node->children[1].get());
    
    // 演算子による型チェック
    std::string op = node->value;

    if (op == "+") {
        // 文字列の連結処理
        if (leftType == MemoryType::String || rightType == MemoryType::String) {
            // 文字列 + 任意の型は文字列になる
            return MemoryType::String;
        }
    }

    if ((op == "+" || op == "-" || op == "*" || op == "/" || op == "%") &&
             (leftType == MemoryType::Integer || leftType == MemoryType::Float) &&
             (rightType == MemoryType::Integer || rightType == MemoryType::Float)) {
        // 数値演算はOK
        if (op == "%" && (leftType == MemoryType::Float || rightType == MemoryType::Float)) {
            reportError("Modulo operator requires integer operands");
            return MemoryType::Integer;
        }
        return (leftType == MemoryType::Float || rightType == MemoryType::Float) ?
               MemoryType::Float : MemoryType::Integer;
    }

    if ((op == "&&" || op == "||") &&
        leftType == MemoryType::Boolean && rightType == MemoryType::Boolean) {
        return MemoryType::Boolean;
    }

    reportError("Expression type mismatch: " + op + " with types " + 
                memoryTypeToString(leftType) + " and " + memoryTypeToString(rightType));
    return MemoryType::Integer;
}

// 条件式のチェック
bool SemanticAnalyzer::checkCondition(const ASTNode* node) {
    if (node->children.size() < 2) {
        reportError("Condition has too few children");
        return false;
    }
    
    auto leftType = visitNode(node->children[0].get());
    auto rightType = visitNode(node->children[1].get());
    
    // 比較演算子による型チェック
    std::string op = node->value;
    if ((op == "==" || op == "!=" || op == "<" || op == ">" ||
         op == "<=" || op == ">=") &&
        (leftType == MemoryType::Integer || leftType == MemoryType::Float) &&
        (rightType == MemoryType::Integer || rightType == MemoryType::Float)) {
        return true;
    }
    
    reportError("Condition type mismatch: " + op);
    return false;
}

// キャストのチェック
MemoryType SemanticAnalyzer::checkCast(const ASTNode* node) {
    if (node->children.empty()) {
        reportError("Empty cast expression");
        return MemoryType::Integer;
    }
    
    // キャスト対象の式をチェック
    visitNode(node->children[0].get());
    
    // キャスト先の型を取得
    std::string castType = node->value;
    if (castType == "int") return MemoryType::Integer;
    else if (castType == "float") return MemoryType::Float;
    else if (castType == "string") return MemoryType::String;
    else if (castType == "bool") return MemoryType::Boolean;
    else {
        reportError("Unknown cast type: " + castType);
        return MemoryType::Integer;
    }
}

// 関数定義のチェック
void SemanticAnalyzer::checkFunctionDefinition(const ASTNode* node) {
    if (node->value.empty()) {
        reportError("Function ID is empty");
    }

    std::string funcID = node->value;
    bool idError = false;
    
    // 数字のみで構成されているかチェック
    try {
        int funcIDInt = std::stoi(funcID);
        if (funcIDInt < 1 || funcIDInt > 999) {
            reportError("Function ID: " + funcID + " is not in range 001-999");
            idError = true;
        }
    } 
    catch (const std::exception& e) {
        reportError("Invalid function ID number: " + funcID);
        idError = true;
    }
    
    // すでに定義済みの関数か確認
    if (functions.count(funcID) > 0 && functions[funcID].isDefined) {
        reportError("Function " + funcID + " is already defined");
    }
    
    // 関数を登録
    functions[funcID] = {funcID, !idError};
    
    // 関数内のステートメントをチェック
    for (const auto& child : node->children) {
        visitNode(child.get());
    }
}

// 関数呼び出しのチェック
void SemanticAnalyzer::checkFunctionCall(const ASTNode* node) {
    std::string funcID = node->value;
    
    // 関数が定義されているかチェック
    if (functions.count(funcID) == 0 || !functions[funcID].isDefined) {
        reportError("Function " + funcID + " is not defined");
    }
}

// エラー報告
void SemanticAnalyzer::reportError(const std::string& message) {
    errors.push_back(message);
    std::cerr << "Semantic error: " << message << std::endl;
}
