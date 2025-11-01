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
            
        case NodeType::MemoryMapRef:
            // メモリマップ参照
            return checkMemoryMapRef(node);
            
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
        
        case NodeType::CharCodeCast:
            // 文字コード変換
            return checkCharCodeCast(node);
        
        case NodeType::StringIndex:
            // 文字列インデックスアクセス
            return checkStringIndex(node);
        
        case NodeType::StringLength:
            // 文字列長取得
            return checkStringLength(node);
        
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

        case NodeType::FileInputStatement:
            checkFileInputOutput(node);
            return MemoryType::Integer;
        
        case NodeType::FileOutputStatement:
            checkFileInputOutput(node);
            return MemoryType::Integer;

        case NodeType::StackOperation:
            return checkStackOperation(node);

        case NodeType::MapWindowSlide:
            return checkMapWindowSlide(node);

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

// メモリマップ参照のチェック
MemoryType SemanticAnalyzer::checkMemoryMapRef(const ASTNode* node) {
    // メモリマップ参照の形式をチェック
    std::string mapRef = node->value;
    if (mapRef.size() < 3 || mapRef.substr(0, 2) != "$^") {
        reportError("Invalid memory map reference: " + mapRef);
        return MemoryType::Integer;
    }
    
    // メモリマップタイプを取得
    char typeChar = mapRef[2];
    MemoryType mapType;
    switch (typeChar) {
        case '#': mapType = MemoryType::Integer; break;
        case '@': mapType = MemoryType::String; break;
        case '~': mapType = MemoryType::Float; break;
        case '%': mapType = MemoryType::Boolean; break;
        default:
            reportError("Unknown memory map type: " + std::string(1, typeChar));
            return MemoryType::Integer;
    }
    
    // インデックスがある場合はチェック
    if (mapRef.size() > 3) {
        std::string indexStr = mapRef.substr(3);
        try {
            int index = std::stoi(indexStr);
            if (index < 0) {
                reportError("Memory map index must be non-negative: " + mapRef);
            }
            // 最大1024要素なので、インデックスは0-1023まで有効
            if (index >= 1024) {
                reportError("Memory map index out of range (max 1023): " + mapRef);
            }
            
            // 既に型が記録されている場合は、その型を返す
            if (memoryMapTypes.count(mapRef) > 0) {
                MemoryType recordedType = memoryMapTypes[mapRef];
                // 期待される型と記録された型が一致するかチェック
                if (!isCompatible(mapType, recordedType)) {
                    reportError("Memory map element type inconsistency: " + mapRef + 
                                " expected " + memoryTypeToString(mapType) + 
                                " but previously used as " + memoryTypeToString(recordedType));
                }
                return recordedType;
            }
        } catch (...) {
            reportError("Invalid memory map index: " + mapRef);
        }
    } else {
        // インデックスなしのメモリマップ参照（$^#など）
        // 全体のメモリマップを参照する場合（スライド操作などで使用）
    }
    
    return mapType;
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
        return index >= 0 && index < 64; // 0～63の範囲内かチェック
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
        std::string leftSide = "";
        std::string rightSide = "";
        
        // 左辺の詳細情報を取得
        if (node->children[0]->type == NodeType::MemoryRef) {
            leftSide = "memory reference " + node->children[0]->value;
        } else if (node->children[0]->type == NodeType::MemoryMapRef) {
            leftSide = "memory map reference " + node->children[0]->value;
        } else {
            leftSide = "left side";
        }
        
        // 右辺の詳細情報を取得
        if (node->children[1]->type == NodeType::Number) {
            rightSide = "number " + node->children[1]->value;
        } else if (node->children[1]->type == NodeType::String) {
            rightSide = "string " + node->children[1]->value;
        } else if (node->children[1]->type == NodeType::MemoryRef) {
            rightSide = "memory reference " + node->children[1]->value;
        } else if (node->children[1]->type == NodeType::MemoryMapRef) {
            rightSide = "memory map reference " + node->children[1]->value;
        } else {
            rightSide = "right side";
        }
        
        reportError("Type mismatch in assignment: " + leftSide + " (" + 
                    memoryTypeToString(leftType) + ") vs " + rightSide + " (" + 
                    memoryTypeToString(rightType) + ")");
    }
    
    // メモリ参照なら型を記録
    if (node->children[0]->type == NodeType::MemoryRef) {
        memoryTypes[node->children[0]->value] = leftType;
    }
    else if (node->children[0]->type == NodeType::MemoryMapRef) {
        // メモリマップ参照の場合も型を記録
        std::string mapRef = node->children[0]->value;
        memoryMapTypes[mapRef] = leftType;
        
        // メモリマップ要素への代入の特別なチェック
        if (mapRef.size() > 3) {
            // インデックス付きメモリマップ参照（$^#1など）への代入
            // 型の厳密なチェック
            char typeChar = mapRef[2];
            MemoryType expectedType;
            switch (typeChar) {
                case '#': expectedType = MemoryType::Integer; break;
                case '@': expectedType = MemoryType::String; break;
                case '~': expectedType = MemoryType::Float; break;
                case '%': expectedType = MemoryType::Boolean; break;
                default: expectedType = MemoryType::Integer; break;
            }
            
            if (!isCompatible(expectedType, rightType)) {
                reportError("Memory map element type mismatch: " + mapRef + " expects " + 
                            memoryTypeToString(expectedType) + " but got " + 
                            memoryTypeToString(rightType));
            }
        }
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

// 文字コード変換のチェック
MemoryType SemanticAnalyzer::checkCharCodeCast(const ASTNode* node) {
    if (node->children.empty()) {
        reportError("Empty character code cast expression");
        return MemoryType::Integer;
    }
    
    // 変換対象の式をチェック
    MemoryType sourceType = visitNode(node->children[0].get());
    
    // 変換の種類を取得
    std::string castType = node->value;
    if (castType == "charToInt") {
        // 文字列 → 整数
        if (sourceType != MemoryType::String) {
            reportError("Character to int cast expects string type");
        }
        return MemoryType::Integer;
    }
    else if (castType == "intToChar") {
        // 整数 → 文字列
        if (sourceType != MemoryType::Integer) {
            reportError("Int to character cast expects integer type");
        }
        return MemoryType::String;
    }
    else {
        reportError("Unknown character code cast type: " + castType);
        return MemoryType::Integer;
    }
}

// インデックスアクセスのチェック
MemoryType SemanticAnalyzer::checkStringIndex(const ASTNode* node) {
    if (node->children.size() < 2) {
        reportError("String index requires memory reference and index expression");
        return MemoryType::String;
    }
    
    // メモリ参照の型をチェック
    MemoryType memType = visitNode(node->children[0].get());
    if (memType != MemoryType::String) {
        reportError("String index can only be used on string type memory");
    }
    
    // インデックス式の型をチェック
    MemoryType indexType = visitNode(node->children[1].get());
    if (indexType != MemoryType::Integer) {
        reportError("String index must be integer type");
    }
    
    // 結果は1文字の文字列
    return MemoryType::String;
}

// 文字列長取得のチェック
MemoryType SemanticAnalyzer::checkStringLength(const ASTNode* node) {
    if (node->children.empty()) {
        reportError("String length requires an expression");
        return MemoryType::Integer;
    }
    
    // 式の型をチェック
    MemoryType exprType = visitNode(node->children[0].get());
    if (exprType != MemoryType::String) {
        reportError("String length can only be used on string type");
    }
    
    // 結果は整数
    return MemoryType::Integer;
}

// 関数定義のチェック
void SemanticAnalyzer::checkFunctionDefinition(const ASTNode* node) {
    if (node->value.empty()) {
        reportError("Function ID is empty");
        return;
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

    if (functions.count(funcID) > 0) {
        // 関数内のステートメントだけチェック
        for (const auto& child : node->children) {
            visitNode(child.get());
        }
        return;
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

// ファイル入出力のチェック
void SemanticAnalyzer::checkFileInputOutput(const ASTNode* node) {
    if (node->children.size() < 2) {
        reportError("File I/O statement has too few children");
        return;
    }
    
    // ファイル名の取得
    auto fileNameNode = node->children[0].get();
    if (fileNameNode->type != NodeType::String && fileNameNode->type != NodeType::MemoryRef) {
        reportError("Invalid file name type: " + fileNameNode->value);
        return;
    }
    
    // 入出力対象のチェック（メモリ参照またはメモリマップ参照）
    auto targetNode = node->children[1].get();
    auto targetType = visitNode(targetNode);
    
    // メモリマップ入出力の場合の特別なチェック
    if (targetNode->type == NodeType::MemoryMapRef) {
        if (node->type == NodeType::FileInputStatement) {
            // ファイル→メモリマップ：ファイルからメモリマップにロード
            // ファイル形式とメモリマップ型の整合性をチェック（将来拡張）
        } else if (node->type == NodeType::FileOutputStatement) {
            // メモリマップ→ファイル：メモリマップをファイルに書き出し
            // メモリマップが初期化されているかチェック（将来拡張）
        }
    }
}

// スタック操作のチェック
MemoryType SemanticAnalyzer::checkStackOperation(const ASTNode* node) {
    if (node->children.empty()) {
        reportError("Stack operation missing operand");
        return MemoryType::Integer;
    }

    // 操作種別
    std::string op = node->value;
    auto operandType = visitNode(node->children[0].get());

    // 操作ごとに型チェック
    if (op == "IntegerStackPush") {
        if (operandType != MemoryType::Integer) {
            reportError("Stack operation expects integer type");
        }
        if (intStackSize >= 1024) {
            reportError("Integer stack overflow (max 1024)");
        } else {
            intStackSize++;
        }
        return MemoryType::Integer;
    }
    if (op == "IntegerStackPop") {
        if (operandType != MemoryType::Integer) {
            reportError("Stack operation expects integer type");
        }
        if (intStackSize == 0) {
            reportError("Integer stack underflow");
        } else {
            intStackSize--;
        }
        return MemoryType::Integer;
    }
    if (op == "FloatStackPush") {
        if (operandType != MemoryType::Float) {
            reportError("Stack operation expects float type");
        }
        if (floatStackSize >= 1024) {
            reportError("Float stack overflow (max 1024)");
        } else {
            floatStackSize++;
        }
        return MemoryType::Float;
    }
    if (op == "FloatStackPop") {
        if (operandType != MemoryType::Float) {
            reportError("Stack operation expects float type");
        }
        if (floatStackSize == 0) {
            reportError("Float stack underflow");
        } else {
            floatStackSize--;
        }
        return MemoryType::Float;
    }
    if (op == "StringStackPush") {
        if (operandType != MemoryType::String) {
            reportError("Stack operation expects string type");
        }
        if (stringStackSize >= 1024) {
            reportError("String stack overflow (max 1024)");
        } else {
            stringStackSize++;
        }
        return MemoryType::String;
    }
    if (op == "StringStackPop") {
        if (operandType != MemoryType::String) {
            reportError("Stack operation expects string type");
        }
        if (stringStackSize == 0) {
            reportError("String stack underflow");
        } else {
            stringStackSize--;
        }
        return MemoryType::String;
    }
    if (op == "BooleanStackPush") {
        if (operandType != MemoryType::Boolean) {
            reportError("Stack operation expects boolean type");
        }
        if (booleanStackSize >= 1024) {
            reportError("Boolean stack overflow (max 1024)");
        } else {
            booleanStackSize++;
        }
        return MemoryType::Boolean;
    }
    if (op == "BooleanStackPop") {
        if (operandType != MemoryType::Boolean) {
            reportError("Stack operation expects boolean type");
        }
        if (booleanStackSize == 0) {
            reportError("Boolean stack underflow");
        } else {
            booleanStackSize--;
        }
        return MemoryType::Boolean;
    }

    reportError("Unknown stack operation: " + op);
    return MemoryType::Integer;
}

// マップウィンドウスライドのチェック
MemoryType SemanticAnalyzer::checkMapWindowSlide(const ASTNode* node) {
    if (node->children.size() < 2) {
        reportError("Map window slide has too few children");
        return MemoryType::Integer;
    }

    // スライド量をチェック
    auto slideAmountType = visitNode(node->children[0].get());
    if (slideAmountType != MemoryType::Integer) {
        reportError("Map window slide amount must be integer type");
    }

    // メモリマップ参照をチェック
    auto mapRefType = visitNode(node->children[1].get());
    if (node->children[1]->type != NodeType::MemoryMapRef) {
        reportError("Map window slide target must be memory map reference");
        return MemoryType::Integer;
    }

    // メモリマップ参照がインデックスなしであることを確認
    std::string mapRef = node->children[1]->value;
    if (mapRef.size() > 3) {
        reportError("Map window slide requires unindexed memory map reference (like $^#, not $^#0): " + mapRef);
    }

    return mapRefType;
}

// エラー報告
void SemanticAnalyzer::reportError(const std::string& message) {
    errors.push_back(message);
    std::cerr << "Semantic error: " << message << std::endl;
}
