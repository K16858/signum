// SigNum Semantic Analyzer
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "../ast/ast.hpp"

// メモリタイプの定義
enum class MemoryType {
    Integer,  // $#
    Float,    // $~
    String,   // $@
    Boolean   // $?
};

// 関数情報の構造体
struct FunctionInfo {
    std::string id;       // 関数ID
    bool isDefined;       // 定義済みかどうか
};

// メモリタイプを文字列に変換
std::string memoryTypeToString(MemoryType type);

// 意味解析器クラス
class SemanticAnalyzer {
private:
    // メモリ参照の型情報を記録
    std::unordered_map<std::string, MemoryType> memoryTypes;
    
    // メモリマップ参照の型情報を記録
    std::unordered_map<std::string, MemoryType> memoryMapTypes;
    
    // 関数の定義情報を記録
    std::unordered_map<std::string, FunctionInfo> functions;
    
    // エラーメッセージを保存
    std::vector<std::string> errors;

    // スタックカウント
    size_t intStackSize = 0;
    size_t floatStackSize = 0;
    size_t stringStackSize = 0;
    size_t booleanStackSize = 0;

public:
    // コンストラクタ
    SemanticAnalyzer() = default;
    
    // 意味解析関数
    bool analyze(const std::shared_ptr<ASTNode>& root);
    
    // エラー取得
    const std::vector<std::string>& getErrors() const { return errors; }
    
private:
    // ノード巡回
    MemoryType visitNode(const ASTNode* node);
    
    // 代入のチェック
    MemoryType checkAssignment(const ASTNode* node);
    
    // メモリ参照のチェック
    MemoryType checkMemoryRef(const ASTNode* node);
    
    // メモリマップ参照のチェック
    MemoryType checkMemoryMapRef(const ASTNode* node);
    
    // 式のチェック
    MemoryType checkExpression(const ASTNode* node);
    
    // キャストのチェック
    MemoryType checkCast(const ASTNode* node);
    
    // 文字コード変換のチェック
    MemoryType checkCharCodeCast(const ASTNode* node);
    
    // 文字列インデックスアクセスのチェック
    MemoryType checkStringIndex(const ASTNode* node);
    
    // 文字列長取得のチェック
    MemoryType checkStringLength(const ASTNode* node);
    
    // 関数定義のチェック
    void checkFunctionDefinition(const ASTNode* node);
    
    // 関数呼び出しのチェック
    void checkFunctionCall(const ASTNode* node);
    
    // 条件式のチェック
    bool checkCondition(const ASTNode* node);

    // ファイル入出力のチェック
    void checkFileInputOutput(const ASTNode* node);

    // スタック操作のチェック
    MemoryType checkStackOperation(const ASTNode* node);

    // マップウィンドウスライドのチェック
    MemoryType checkMapWindowSlide(const ASTNode* node);
    
    // エラー報告
    void reportError(const std::string& message);
    
    // メモリ参照から型を取得
    MemoryType getTypeFromMemRef(const std::string& memRef);
    
    // メモリ番号の範囲チェック
    bool checkMemoryRange(const std::string& memRef);
    
    // 型の互換性チェック
    bool isCompatible(MemoryType lhs, MemoryType rhs);
    
    // 関数定義を集める（1パス目）
    void collectFunctionDefinitions(const ASTNode* node);
};
