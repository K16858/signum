// SigNum Interpreter

#pragma once

#include <iostream>
#include <unordered_map>
#include <array>
#include <variant>
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include "../ast/ast.hpp"

// 値の型
using Value = std::variant<int, double, std::string, bool>;

// メモリプールのサイズ
constexpr size_t MEMORY_POOL_SIZE = 64;
constexpr size_t ARGS_START = 48;
constexpr size_t RETURN_START = 56;
constexpr size_t SYSTEM_START = 60;

// スタックのサイズ
constexpr size_t STACK_MAX_SIZE = 1024;

// メモリマップのサイズ
constexpr size_t MEMORY_MAP_SIZE = 1024;

// メモリマップ管理クラス
class MemoryMap {
private:
    std::string filePath;
    size_t windowOffset;
    char mapType; // '#', '@', '~', '%'
    
public:
    MemoryMap() : windowOffset(0), mapType('\0') {}
    MemoryMap(const std::string& path, char type) : filePath(path), windowOffset(0), mapType(type) {}
    
    // ファイルマッピング
    void mapFile(const std::string& path, char type);
    
    // 要素の読み書き
    Value readElement(size_t index);
    void writeElement(size_t index, const Value& value);
    
    // ウィンドウスライド
    void slideWindow(int offset);
    
    // ファイル初期化・拡張
    void ensureFileSize();
    
    // getter
    bool isMapped() const { return !filePath.empty(); }
    size_t getWindowOffset() const { return windowOffset; }
    const std::string& getFilePath() const { return filePath; }
    char getMapType() const { return mapType; }
};

class Interpreter {
private:
    // 各型のメモリプール
    std::array<Value, MEMORY_POOL_SIZE> intPool;    // # (整数)
    std::array<Value, MEMORY_POOL_SIZE> stringPool; // @ (文字列)
    std::array<Value, MEMORY_POOL_SIZE> floatPool;  // ~ (浮動小数点)
    std::array<Value, MEMORY_POOL_SIZE> boolPool;   // % (真偽値)

    // 各型のスタック
    std::vector<int> intStack;
    std::vector<double> floatStack;
    std::vector<std::string> stringStack;
    std::vector<bool> booleanStack;

    // 関数テーブル
    std::unordered_map<int, std::shared_ptr<ASTNode>> functions;
    
    // メモリマップ
    MemoryMap intMemoryMap;    // ^#
    MemoryMap stringMemoryMap; // ^@
    MemoryMap floatMemoryMap;  // ^~
    MemoryMap boolMemoryMap;   // ^%
    
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

        // スタックの初期化
        intStack.reserve(STACK_MAX_SIZE);
        floatStack.reserve(STACK_MAX_SIZE);
        stringStack.reserve(STACK_MAX_SIZE);
        booleanStack.reserve(STACK_MAX_SIZE);
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
    Value evaluateCharCodeCast(const std::shared_ptr<ASTNode>& node);
    Value evaluateStringIndex(const std::shared_ptr<ASTNode>& node);
    Value evaluateStringLength(const std::shared_ptr<ASTNode>& node);
    Value evaluateIfStatement(const std::shared_ptr<ASTNode>& node);
    Value evaluateLoopStatement(const std::shared_ptr<ASTNode>& node);
    Value evaluateInputStatement(const std::shared_ptr<ASTNode>& node);
    Value evaluateOutputStatement(const std::shared_ptr<ASTNode>& node);
    Value evaluateFileInputStatement(const std::shared_ptr<ASTNode>& node);
    Value evaluateFileOutputStatement(const std::shared_ptr<ASTNode>& node);
    Value evaluateStackOperation(const std::shared_ptr<ASTNode>& node);
    Value evaluateMemoryMapRef(const std::shared_ptr<ASTNode>& node);
    Value evaluateMapWindowSlide(const std::shared_ptr<ASTNode>& node);
    
    // 変数の取得と設定
    Value getMemoryValue(char type, int index);
    void setMemoryValue(char type, int index, const Value& value);
    
    // メモリマップの取得
    MemoryMap& getMemoryMap(char type);
};
