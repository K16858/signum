// SigNum Interpreter

#include <cstdint>
#include "interpreter.hpp"

// MemoryMapクラス
// ファイルマッピング
void MemoryMap::mapFile(const std::string& path, char type) {
    filePath = path;
    mapType = type;
    windowOffset = 0;
    ensureFileSize();
}

// ファイルサイズの確保・拡張
void MemoryMap::ensureFileSize() {
    std::ifstream checkFile(filePath, std::ios::binary | std::ios::ate);
    size_t currentSize = 0;
    
    if (checkFile.is_open()) {
        currentSize = checkFile.tellg();
        checkFile.close();
    }
    
    // 必要なサイズを計算
    size_t elementSize;
    switch (mapType) {
        case '#': case '~': elementSize = 4; break; // int, float
        case '%': elementSize = 1; break; // bool
        case '@': elementSize = 1; break; // string (char配列)
        default: throw std::runtime_error("Unknown map type for size calculation");
    }
    
    size_t requiredSize = MEMORY_MAP_SIZE * elementSize;
    
    // ファイルサイズが不足している場合は拡張
    if (currentSize < requiredSize) {
        std::ofstream file(filePath, std::ios::binary | std::ios::app);
        if (!file) {
            throw std::runtime_error("Failed to create/extend file: " + filePath);
        }
        
        // 不足分をゼロパディング
        size_t paddingSize = requiredSize - currentSize;
        std::vector<char> padding(paddingSize, 0);
        file.write(padding.data(), paddingSize);
        file.close();
    }
}

// 要素の読み取り
Value MemoryMap::readElement(size_t index) {
    if (index >= MEMORY_MAP_SIZE) {
        throw std::out_of_range("Memory map index out of range: " + std::to_string(index));
    }
    
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for reading: " + filePath);
    }
    
    size_t elementSize;
    size_t fileOffset;
    
    switch (mapType) {
        case '#': // int (4バイト)
            elementSize = 4;
            fileOffset = (windowOffset + index) * elementSize;
            file.seekg(fileOffset);
            {
                int32_t value;
                file.read(reinterpret_cast<char*>(&value), sizeof(value));
                return static_cast<int>(value);
            }
            break;
            
        case '~': // float (4バイト)
            elementSize = 4;
            fileOffset = (windowOffset + index) * elementSize;
            file.seekg(fileOffset);
            {
                float value;
                file.read(reinterpret_cast<char*>(&value), sizeof(value));
                return static_cast<double>(value);
            }
            break;
            
        case '%': // bool (1バイト)
            elementSize = 1;
            fileOffset = (windowOffset + index) * elementSize;
            file.seekg(fileOffset);
            {
                uint8_t value;
                file.read(reinterpret_cast<char*>(&value), sizeof(value));
                return static_cast<bool>(value);
            }
            break;
            
        case '@': // string (UTF-8可変長)
            elementSize = 1;
            fileOffset = windowOffset + index;
            file.seekg(fileOffset);
            {
                char value;
                file.read(&value, 1);
                return std::string(1, value);
            }
            break;
            
        default:
            throw std::runtime_error("Unknown memory map type: " + std::string(1, mapType));
    }
}

// 要素の書き込み
void MemoryMap::writeElement(size_t index, const Value& value) {
    if (index >= MEMORY_MAP_SIZE) {
        throw std::out_of_range("Memory map index out of range: " + std::to_string(index));
    }
    
    std::fstream file(filePath, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing: " + filePath);
    }
    
    size_t elementSize;
    size_t fileOffset;
    
    switch (mapType) {
        case '#': // int (4バイト)
            elementSize = 4;
            fileOffset = (windowOffset + index) * elementSize;
            file.seekp(fileOffset);
            {
                int32_t intVal = std::get<int>(value);
                file.write(reinterpret_cast<const char*>(&intVal), sizeof(intVal));
            }
            break;
            
        case '~': // float (4バイト)
            elementSize = 4;
            fileOffset = (windowOffset + index) * elementSize;
            file.seekp(fileOffset);
            {
                float floatVal = static_cast<float>(std::get<double>(value));
                file.write(reinterpret_cast<const char*>(&floatVal), sizeof(floatVal));
            }
            break;
            
        case '%': // bool (1バイト)
            elementSize = 1;
            fileOffset = (windowOffset + index) * elementSize;
            file.seekp(fileOffset);
            {
                uint8_t boolVal = std::get<bool>(value) ? 1 : 0;
                file.write(reinterpret_cast<const char*>(&boolVal), sizeof(boolVal));
            }
            break;
            
        case '@': // string (UTF-8可変長)
            elementSize = 1;
            fileOffset = windowOffset + index;
            file.seekp(fileOffset);
            {
                std::string strVal = std::get<std::string>(value);
                if (!strVal.empty()) {
                    file.write(&strVal[0], 1);
                } 
                else {
                    char nullChar = '\0';
                    file.write(&nullChar, 1);
                }
            }
            break;
            
        default:
            throw std::runtime_error("Unknown memory map type: " + std::string(1, mapType));
    }
    
    file.flush();
}

// ウィンドウスライド
void MemoryMap::slideWindow(int offset) {
    int newOffset = static_cast<int>(windowOffset) + offset;
    
    // 0未満にならないようにクランプ
    if (newOffset < 0) {
        windowOffset = 0;
    } else {
        windowOffset = static_cast<size_t>(newOffset);
    }
}


// メモリ参照を解決する
Value Interpreter::resolveMemoryRef(const std::string& ref) {
    int startPos = (ref[0] == '$') ? 1 : 0;
    
    // ネストされた参照チェック
    size_t nextDollar = ref.find('$', startPos + 1);
    if (nextDollar != std::string::npos) {
        // ネストされた参照を先に評価
        std::string nestedRef = ref.substr(nextDollar);
        int nestedIndex = std::get<int>(resolveMemoryRef(nestedRef));
        
        // 外側の参照タイプを使って最終的な値を取得
        char type = ref[startPos];
        return getMemoryValue(type, nestedIndex);
    }
    
    // 通常の参照処理
    if (ref[startPos] == '#') {
        return intPool[std::stoi(ref.substr(startPos + 1))];
    } 
    else if (ref[startPos] == '@') {
        return stringPool[std::stoi(ref.substr(startPos + 1))];
    } 
    else if (ref[startPos] == '~') {
        return floatPool[std::stoi(ref.substr(startPos + 1))];
    } 
    else if (ref[startPos] == '%') {
        return boolPool[std::stoi(ref.substr(startPos + 1))];
    }
    throw std::runtime_error("Invalid memory reference: " + ref);
}

// メモリインデックスを評価する
int Interpreter::evaluateMemoryIndex(const std::string& indexExpr) {
    int startPos = (indexExpr[0] == '$') ? 1 : 0;
    
    if (indexExpr[startPos] == '#' || 
        indexExpr[startPos] == '@' || 
        indexExpr[startPos] == '~' || 
        indexExpr[startPos] == '%') {
        
        // ネストされたメモリ参照かチェック
        size_t nextDollar = indexExpr.find('$', startPos + 1);
        if (nextDollar != std::string::npos) {
            // ネストされた参照を先に評価
            std::string nestedRef = indexExpr.substr(nextDollar);
            int nestedValue = std::get<int>(resolveMemoryRef(nestedRef));
            return nestedValue;
        } else {
            // 通常の参照
            return std::stoi(indexExpr.substr(startPos + 1));
        }
    }
    
    throw std::runtime_error("Invalid memory index: " + indexExpr);
}

// メモリ値の取得
Value Interpreter::getMemoryValue(char type, int index) {
    if (index < 0 || index >= MEMORY_POOL_SIZE) {
        throw std::out_of_range("Memory index out of range: " + std::to_string(index));
    }
    switch (type) {
        case '#': return intPool[index];
        case '@': return stringPool[index];
        case '~': return floatPool[index];
        case '%': return boolPool[index];
        default: throw std::runtime_error("Invalid memory type: " + std::string(1, type));
    }
}

// メモリ値の設定
void Interpreter::setMemoryValue(char type, int index, const Value& value) {
    if (index < 0 || index >= MEMORY_POOL_SIZE) {
        throw std::out_of_range("Memory index out of range: " + std::to_string(index));
    }
    switch (type) {
        case '#': intPool[index] = std::get<int>(value); break;
        case '@': stringPool[index] = std::get<std::string>(value); break;
        case '~': floatPool[index] = std::get<double>(value); break;
        case '%': boolPool[index] = std::get<bool>(value); break;
        default: throw std::runtime_error("Invalid memory type: " + std::string(1, type));
    }
}

// 値を文字列に変換
std::string Interpreter::valueToString(const Value& val) {
    if (std::holds_alternative<int>(val)) {
        return std::to_string(std::get<int>(val));
    }
    else if (std::holds_alternative<double>(val)) {
        return std::to_string(std::get<double>(val));
    }
    else if (std::holds_alternative<bool>(val)) {
        return std::get<bool>(val) ? "true" : "false";
    }
    else if (std::holds_alternative<std::string>(val)) {
        return std::get<std::string>(val);
    }
    return ""; // 未対応の型の場合
}

// 実行
void Interpreter::interpret(const std::shared_ptr<ASTNode>& program) {
    evaluateNode(program);
}

// ノード評価
Value Interpreter::evaluateNode(const std::shared_ptr<ASTNode>& node) {
    switch (node->type) {
        case NodeType::Program:
            return evaluateProgram(node);
        case NodeType::Function:
            return evaluateFunction(node);
        case NodeType::FunctionCall:
            return evaluateFunctionCall(node);
        case NodeType::Statement:
            for (const auto& child : node->children) {
                evaluateNode(child);
            }
            return Value();
        case NodeType::Assignment:
            return evaluateAssignment(node);
        case NodeType::ArithmeticExpression:
            return evaluateArithmeticExpression(node);
        case NodeType::LogicalExpression:
            return evaluateLogicalExpression(node);
        case NodeType::MemoryRef:
            return evaluateMemoryRef(node);
        case NodeType::Number:
            return evaluateNumber(node);
        case NodeType::String:
            return evaluateString(node);
        case NodeType::Comparison:
            return evaluateComparison(node);
        case NodeType::Cast:
            return evaluateCast(node);
        case NodeType::CharCodeCast:
            return evaluateCharCodeCast(node);
        case NodeType::StringIndex:
            return evaluateStringIndex(node);
        case NodeType::StringLength:
            return evaluateStringLength(node);
        case NodeType::IfStatement:
            return evaluateIfStatement(node);
        case NodeType::LoopStatement:
            return evaluateLoopStatement(node);
        case NodeType::InputStatement:
            return evaluateInputStatement(node);
        case NodeType::OutputStatement:
            return evaluateOutputStatement(node);
        case NodeType::FileInputStatement:
            return evaluateFileInputStatement(node);
        case NodeType::FileOutputStatement:
            return evaluateFileOutputStatement(node);
        case NodeType::StackOperation:
            return evaluateStackOperation(node);
        case NodeType::MemoryMapRef:
            return evaluateMemoryMapRef(node);
        case NodeType::MapWindowSlide:
            return evaluateMapWindowSlide(node);
        case NodeType::Error:
            throw std::runtime_error("Parse error encountered: " + node->value);
        default:
            throw std::runtime_error("Unknown node type: " + std::to_string(static_cast<int>(node->type)));
    }
}

// ルートノード評価
Value Interpreter::evaluateProgram(const std::shared_ptr<ASTNode>& program) {
    for (const std::shared_ptr<ASTNode>& child : program->children) {
        evaluateNode(child);
    }
    return Value();
}

// 関数ノード評価
Value Interpreter::evaluateFunction(const std::shared_ptr<ASTNode>& node) {
    // 関数の定義を保存
    functions[std::stoi(node->value)] = node;
    return Value();
}

Value Interpreter::evaluateFunctionCall(const std::shared_ptr<ASTNode>& node) {
    // 関数の呼び出しを評価
    auto it = functions.find(std::stoi(node->value));
    if (it != functions.end()) {
        // 関数の中身（子ノード）を順番に実行
        for (const auto& child : it->second->children) {
            evaluateNode(child);
        }
        return Value();
    }
    throw std::runtime_error("Function not found: " + node->value);
}

// 代入ノード評価
Value Interpreter::evaluateAssignment(const std::shared_ptr<ASTNode>& node) {
    std::string varName = node->children[0]->value;
    Value value = evaluateNode(node->children[1]);

    // メモリマップ参照かチェック
    if (varName.size() >= 3 && varName.substr(0, 2) == "$^") {
        char mapType = varName[2];
        MemoryMap& memMap = getMemoryMap(mapType);
        
        if (!memMap.isMapped()) {
            throw std::runtime_error("Memory map not initialized for assignment: " + varName);
        }
        
        // インデックス取得
        size_t index = 0;
        if (varName.size() > 3) {
            index = std::stoi(varName.substr(3));
        }
        
        // string型の特殊処理
        if (mapType == '@' && std::holds_alternative<std::string>(value)) {
            std::string strValue = std::get<std::string>(value);
            // 文字列を1文字ずつ連続配置
            for (size_t i = 0; i < strValue.size() && (index + i) < MEMORY_MAP_SIZE; ++i) {
                memMap.writeElement(index + i, std::string(1, strValue[i]));
            }
        } 
        else {
            memMap.writeElement(index, value);
        }
        
        return value;
    } else {
        // 通常のメモリ参照への代入
        int startPos = (varName[0] == '$') ? 1 : 0;
        setMemoryValue(varName[startPos], evaluateMemoryIndex(varName), value);
        return value;
    }
}

// 算術式ノード評価
Value Interpreter::evaluateArithmeticExpression(const std::shared_ptr<ASTNode>& node) {
    // 単項式
    if (node->children.size() == 1) {
        return evaluateNode(node->children[0]);
    } 
    // 二項式
    else if (node->children.size() == 2) {
        Value left = evaluateNode(node->children[0]);
        Value right = evaluateNode(node->children[1]);
        std::string op = node->value;

        // int-int
        if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
            int lval = std::get<int>(left);
            int rval = std::get<int>(right);
            
            if (op == "+") return lval + rval;
            if (op == "-") return lval - rval;
            if (op == "*") return lval * rval;
            if (op == "/") {
                if (rval == 0) throw std::runtime_error("Division by zero");
                return lval / rval;
            }
            if (op == "%") {
                if (rval == 0) throw std::runtime_error("Modulo by zero");
                return lval % rval;
            }
        } 
        // double-double
        else if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
            double lval = std::get<double>(left);
            double rval = std::get<double>(right);
            
            if (op == "+") return lval + rval;
            if (op == "-") return lval - rval;
            if (op == "*") return lval * rval;
            if (op == "/") {
                if (rval == 0.0) throw std::runtime_error("Division by zero");
                return lval / rval;
            }
        }
        // int-double
        else if (std::holds_alternative<int>(left) && std::holds_alternative<double>(right) 
                || std::holds_alternative<double>(left) && std::holds_alternative<int>(right)) {
            int lval = std::get<int>(left);
            double rval = std::get<double>(right);
            
            if (op == "+") return lval + rval;
            if (op == "-") return lval - rval;
            if (op == "*") return lval * rval;
            if (op == "/") {
                if (rval == 0.0) throw std::runtime_error("Division by zero");
                return lval / rval;
            }
        }
        // int-bool
        else if (std::holds_alternative<int>(left) && std::holds_alternative<bool>(right)) {
            int lval = std::get<int>(left);
            bool rval = std::get<bool>(right);
            
            if (op == "+") return lval + (rval ? 1 : 0);
            if (op == "-") return lval - (rval ? 1 : 0);
            if (op == "*") return lval * (rval ? 1 : 0);
            if (op == "/") {
                if (rval) return lval / 1;
                throw std::runtime_error("Division by zero");
            }
            if (op == "%") {
                if (rval == 0) throw std::runtime_error("Modulo by zero");
                return lval % rval;
            }
        }
        // double-bool
        else if (std::holds_alternative<double>(left) && std::holds_alternative<bool>(right)) {
            double lval = std::get<double>(left);
            bool rval = std::get<bool>(right);
            
            if (op == "+") return lval + (rval ? 1.0 : 0.0);
            if (op == "-") return lval - (rval ? 1.0 : 0.0);
            if (op == "*") return lval * (rval ? 1.0 : 0.0);
            if (op == "/") {
                if (rval) return lval / 1.0;
                throw std::runtime_error("Division by zero");
            }
        }
        // bool-int
        else if (std::holds_alternative<bool>(left) && std::holds_alternative<int>(right)) {
            bool lval = std::get<bool>(left);
            int rval = std::get<int>(right);
            
            if (op == "+") return (lval ? 1 : 0) + rval;
            if (op == "-") return (lval ? 1 : 0) - rval;
            if (op == "*") return (lval ? 1 : 0) * rval;
            if (op == "/") {
                if (rval == 0) throw std::runtime_error("Division by zero");
                return (lval ? 1 : 0) / rval;
            }
            if (op == "%") {
                if (rval == 0) throw std::runtime_error("Modulo by zero");
                return lval % rval;
            }
        }
        // bool-double
        else if (std::holds_alternative<bool>(left) && std::holds_alternative<double>(right)) {
            bool lval = std::get<bool>(left);
            double rval = std::get<double>(right);
            
            if (op == "+") return (lval ? 1.0 : 0.0) + rval;
            if (op == "-") return (lval ? 1.0 : 0.0) - rval;
            if (op == "*") return (lval ? 1.0 : 0.0) * rval;
            if (op == "/") {
                if (rval == 0.0) throw std::runtime_error("Division by zero");
                return (lval ? 1.0 : 0.0) / rval;
            }
        }
        // bool-bool
        else if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
            bool lval = std::get<bool>(left);
            bool rval = std::get<bool>(right);
            
            if (op == "+") return (lval ? 1 : 0) + (rval ? 1 : 0);
            if (op == "-") return (lval ? 1 : 0) - (rval ? 1 : 0);
            if (op == "*") return (lval ? 1 : 0) * (rval ? 1 : 0);
            if (op == "/") {
                if (!rval) throw std::runtime_error("Division by zero");
                return (lval ? 1 : 0) / (rval ? 1 : 0);
            }
            if (op == "%") {
                if (!rval) throw std::runtime_error("Modulo by zero");
                return (lval ? 1 : 0) % (rval ? 1 : 0);
            }
        }
        // str-任意の型
        else if ((std::holds_alternative<std::string>(left) || 
                  std::holds_alternative<std::string>(right)) && op == "+") {
            return valueToString(left) + valueToString(right);
        }
    }
    throw std::runtime_error("Invalid arithmetic expression: " + node->toJSON());
}

// 論理式ノード評価
Value Interpreter::evaluateLogicalExpression(const std::shared_ptr<ASTNode>& node) {
    // 単項式
    if (node->children.size() == 1) {
        // 単項否定
        if (node->value == "!") {
            Value childValue = evaluateNode(node->children[0]);
            if (std::holds_alternative<bool>(childValue)) {
                return !std::get<bool>(childValue);
            }
            throw std::runtime_error("Invalid logical negation: " + node->toJSON());
        }
        return evaluateNode(node->children[0]);
    } 
    // 二項式
    else if (node->children.size() == 2) {
        Value left = evaluateNode(node->children[0]);
        Value right = evaluateNode(node->children[1]);
        std::string op = node->value;

        if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
            bool lval = std::get<bool>(left);
            bool rval = std::get<bool>(right);
            
            if (op == "&&") return lval && rval;
            if (op == "||") return lval || rval;
        }
    }
    throw std::runtime_error("Invalid logical expression: " + node->toJSON());
}

// 比較式ノード評価
Value Interpreter::evaluateComparison(const std::shared_ptr<ASTNode>& node) {
    Value left = evaluateNode(node->children[0]);
    Value right = evaluateNode(node->children[1]);
    std::string op = node->value;

    if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
        int lval = std::get<int>(left);
        int rval = std::get<int>(right);
        
        if (op == "==") return lval == rval;
        if (op == "!=") return lval != rval;
        if (op == "<") return lval < rval;
        if (op == "<=") return lval <= rval;
        if (op == ">") return lval > rval;
        if (op == ">=") return lval >= rval;
    }
    else if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
        double lval = std::get<double>(left);
        double rval = std::get<double>(right);
        
        if (op == "==") return lval == rval;
        if (op == "!=") return lval != rval;
        if (op == "<") return lval < rval;
        if (op == "<=") return lval <= rval;
        if (op == ">") return lval > rval;
        if (op == ">=") return lval >= rval;
    } 
    else if (std::holds_alternative<int>(left) && std::holds_alternative<double>(right)) {
        int lval = std::get<int>(left);
        double rval = std::get<double>(right);
        
        if (op == "==") return lval == rval;
        if (op == "!=") return lval != rval;
        if (op == "<") return lval < rval;
        if (op == "<=") return lval <= rval;
        if (op == ">") return lval > rval;
        if (op == ">=") return lval >= rval;
    } 
    else if (std::holds_alternative<double>(left) && std::holds_alternative<int>(right)) {
        double lval = std::get<double>(left);
        int rval = std::get<int>(right);
        
        if (op == "==") return lval == rval;
        if (op == "!=") return lval != rval;
        if (op == "<") return lval < rval;
        if (op == "<=") return lval <= rval;
        if (op == ">") return lval > rval;
        if (op == ">=") return lval >= rval;
    }
    else if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
        bool lval = std::get<bool>(left);
        bool rval = std::get<bool>(right);
        
        if (op == "==") return lval == rval;
        if (op == "!=") return lval != rval;
    }
    else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
        std::string lval = std::get<std::string>(left);
        std::string rval = std::get<std::string>(right);
        
        if (op == "==") return lval == rval;
        if (op == "!=") return lval != rval;
    }

    if (op == "==" || op == "!=") {
        std::string lstr = valueToString(left);
        std::string rstr = valueToString(right);
        
        if (op == "==") return lstr == rstr;
        if (op == "!=") return lstr != rstr;
    }

    throw std::runtime_error("Invalid comparison: " + node->toJSON());
}

// 型変換ノード評価
Value Interpreter::evaluateCast(const std::shared_ptr<ASTNode>& node) {
    Value value = evaluateNode(node->children[0]);
    std::string targetType = node->value;

    if (targetType == "int") {
        if (std::holds_alternative<double>(value)) {
            return static_cast<int>(std::get<double>(value));
        } 
        else if (std::holds_alternative<std::string>(value)) {
            return std::stoi(std::get<std::string>(value));
        }
    } 
    else if (targetType == "double") {
        if (std::holds_alternative<int>(value)) {
            return static_cast<double>(std::get<int>(value));
        } 
        else if (std::holds_alternative<std::string>(value)) {
            return std::stod(std::get<std::string>(value));
        }
    } 
    else if (targetType == "string") {
        return valueToString(value);
    } 
    else if (targetType == "bool") {
        if (std::holds_alternative<int>(value)) {
            return std::get<int>(value) != 0;
        } 
        else if (std::holds_alternative<double>(value)) {
            return std::get<double>(value) != 0.0;
        } 
        else if (std::holds_alternative<std::string>(value)) {
            return !std::get<std::string>(value).empty();
        }
    }

    throw std::runtime_error("Invalid cast: " + node->toJSON());
}

// 文字コード変換ノード評価
Value Interpreter::evaluateCharCodeCast(const std::shared_ptr<ASTNode>& node) {
    Value value = evaluateNode(node->children[0]);
    std::string castType = node->value;

    if (castType == "charToInt") {
        if (std::holds_alternative<std::string>(value)) {
            std::string str = std::get<std::string>(value);
            if (str.length() == 1) {
                return static_cast<int>(static_cast<unsigned char>(str[0]));
            }
            throw std::runtime_error("Character code cast requires single character, got: " + str);
        }
        throw std::runtime_error("Character code cast (charToInt) requires string type");
    }
    else if (castType == "intToChar") {
        if (std::holds_alternative<int>(value)) {
            int code = std::get<int>(value);
            if (code >= 0 && code <= 127) {  // ASCII範囲
                char c = static_cast<char>(code);
                return std::string(1, c);
            }
            throw std::runtime_error("Character code must be in range 0-127, got: " + std::to_string(code));
        }
        throw std::runtime_error("Character code cast (intToChar) requires int type");
    }

    throw std::runtime_error("Invalid character code cast: " + node->toJSON());
}

// インデックスアクセスノード評価
Value Interpreter::evaluateStringIndex(const std::shared_ptr<ASTNode>& node) {
    if (node->children.size() < 2) {
        throw std::runtime_error("String index requires memory reference and index");
    }
    
    // メモリ参照を評価
    Value memValue = evaluateNode(node->children[0]);
    
    // インデックスを評価
    Value indexValue = evaluateNode(node->children[1]);
    
    if (!std::holds_alternative<std::string>(memValue)) {
        throw std::runtime_error("String index can only be used on string type");
    }
    
    if (!std::holds_alternative<int>(indexValue)) {
        throw std::runtime_error("String index must be integer type");
    }
    
    std::string str = std::get<std::string>(memValue);
    int index = std::get<int>(indexValue);
    
    // 範囲チェック
    if (index < 0 || index >= static_cast<int>(str.length())) {
        throw std::out_of_range("String index out of range: " + std::to_string(index) + 
                                " (string length: " + std::to_string(str.length()) + ")");
    }
    
    // 1文字を文字列として返す
    return std::string(1, str[index]);
}

// 文字列長取得ノード評価
Value Interpreter::evaluateStringLength(const std::shared_ptr<ASTNode>& node) {
    if (node->children.empty()) {
        throw std::runtime_error("String length requires an expression");
    }
    
    // 式を評価
    Value value = evaluateNode(node->children[0]);
    
    if (!std::holds_alternative<std::string>(value)) {
        throw std::runtime_error("String length can only be used on string type");
    }
    
    std::string str = std::get<std::string>(value);
    return static_cast<int>(str.length());
}

// if文ノード評価
Value Interpreter::evaluateIfStatement(const std::shared_ptr<ASTNode>& node) {
    Value condition = evaluateNode(node->children[0]);
    if (std::holds_alternative<bool>(condition) && std::get<bool>(condition)) {
        // ifが成立したら、if本体を実行して終了
        return evaluateNode(node->children[1]);
    } 
    else if (node->children.size() > 2) {
        // else ifがあるか（2つ置きにノードが条件と本体になってる）
        for (size_t i = 2; i < node->children.size(); i += 2) {
            // 最後の1つはelseブロック
            if (i == node->children.size() - 1) {
                return evaluateNode(node->children[i]);
            }
            
            // それ以外はelse if条件を評価
            condition = evaluateNode(node->children[i]);
            if (std::holds_alternative<bool>(condition) && std::get<bool>(condition)) {
                return evaluateNode(node->children[i+1]);
            }
        }
    }

    return Value();
}

// ループ文ノード評価
Value Interpreter::evaluateLoopStatement(const std::shared_ptr<ASTNode>& node) {
    while (true) {
        Value condition = evaluateNode(node->children[0]);
        if (std::holds_alternative<bool>(condition) && !std::get<bool>(condition)) {
            break;
        }
        evaluateNode(node->children[1]);
    }
    return Value();
}

// 入力文ノード評価
Value Interpreter::evaluateInputStatement(const std::shared_ptr<ASTNode>& node) {
    std::string varName = node->children[0]->value;
    int startPos = (varName[0] == '$') ? 1 : 0;
    char memType = varName[startPos];
    std::string input;
    std::cout << "Input " << varName << ": ";
    std::cin >> input;
    
    // メモリタイプに応じて適切な型に変換
    Value convertedValue;
    switch (memType) {
        case '#': // int
            convertedValue = std::stoi(input);
            break;
        case '~': // double
            convertedValue = std::stod(input);
            break;
        case '%': // bool
            convertedValue = (input == "true" || input == "1");
            break;
        case '@': // string
            convertedValue = input;
            break;
        default:
            throw std::runtime_error("Unknown memory type for input: " + std::string(1, memType));
    }
    
    setMemoryValue(memType, evaluateMemoryIndex(varName), convertedValue);
    return Value();
}

// 出力文ノード評価
Value Interpreter::evaluateOutputStatement(const std::shared_ptr<ASTNode>& node) {
    Value value = evaluateNode(node->children[0]);
    std::cout << valueToString(value) << std::endl;
    return Value();
}

// ファイル入力文ノード評価
Value Interpreter::evaluateFileInputStatement(const std::shared_ptr<ASTNode>& node) {
    std::string filename = std::get<std::string>(evaluateNode(node->children[0]));
    std::string targetName = node->children[1]->value;

    // メモリマップ参照かチェック
    if (targetName.size() >= 3 && targetName.substr(0, 2) == "$^") {
        // メモリマップにファイルをマッピング
        char mapType = targetName[2];
        MemoryMap& memMap = getMemoryMap(mapType);
        memMap.mapFile(filename, mapType);
        return Value();
    } else {
        // 通常のメモリ参照への読み込み
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        int startPos = (targetName[0] == '$') ? 1 : 0;
        setMemoryValue(targetName[startPos], evaluateMemoryIndex(targetName), content);
        return Value();
    }
}

// ファイル出力文ノード評価
Value Interpreter::evaluateFileOutputStatement(const std::shared_ptr<ASTNode>& node) {
    std::string filename = std::get<std::string>(evaluateNode(node->children[0]));
    std::string sourceName = node->children[1]->value;

    // メモリマップ参照かチェック
    if (sourceName.size() >= 3 && sourceName.substr(0, 2) == "$^") {
        // メモリマップからファイルに書き出し
        char mapType = sourceName[2];
        MemoryMap& memMap = getMemoryMap(mapType);
        
        if (!memMap.isMapped()) {
            throw std::runtime_error("Memory map not initialized for output");
        }
        
        return Value();
    } 
    else {
        // 通常のメモリ参照からファイルへの出力
        Value value = evaluateNode(node->children[1]);

        std::ofstream file(filename);
        if (!file) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        
        file << valueToString(value);
        return Value();
    }
}

// スタック操作ノード評価
Value Interpreter::evaluateStackOperation(const std::shared_ptr<ASTNode>& node) {
    std::string op = node->value;
    Value val = evaluateNode(node->children[0]);

    if (op == "IntegerStackPush") {
        if (intStack.size() >= 1024) throw std::runtime_error("Integer stack overflow");
        intStack.push_back(std::get<int>(val));
        return Value();
    }
    if (op == "IntegerStackPop") {
        if (intStack.empty()) throw std::runtime_error("Integer stack underflow");
        int result = intStack.back();
        intStack.pop_back();
        return result;
    }
    if (op == "FloatStackPush") {
        if (floatStack.size() >= 1024) throw std::runtime_error("Float stack overflow");
        floatStack.push_back(std::get<double>(val));
        return Value();
    }
    if (op == "FloatStackPop") {
        if (floatStack.empty()) throw std::runtime_error("Float stack underflow");
        double result = floatStack.back();
        floatStack.pop_back();
        return result;
    }
    if (op == "StringStackPush") {
        if (stringStack.size() >= 1024) throw std::runtime_error("String stack overflow");
        stringStack.push_back(std::get<std::string>(val));
        return Value();
    }
    if (op == "StringStackPop") {
        if (stringStack.empty()) throw std::runtime_error("String stack underflow");
        std::string result = stringStack.back();
        stringStack.pop_back();
        return result;
    }
    if (op == "BooleanStackPush") {
        if (booleanStack.size() >= 1024) throw std::runtime_error("Boolean stack overflow");
        booleanStack.push_back(std::get<bool>(val));
        return Value();
    }
    if (op == "BooleanStackPop") {
        if (booleanStack.empty()) throw std::runtime_error("Boolean stack underflow");
        bool result = booleanStack.back();
        booleanStack.pop_back();
        return result;
    }
    throw std::runtime_error("Unknown stack operation: " + op);
}

// メモリ参照ノード評価
Value Interpreter::evaluateMemoryRef(const std::shared_ptr<ASTNode>& node) {
    return resolveMemoryRef(node->value);
}

// 数値ノード評価
Value Interpreter::evaluateNumber(const std::shared_ptr<ASTNode>& node) {
    return std::stoi(node->value);
}

// 文字列ノード評価
Value Interpreter::evaluateString(const std::shared_ptr<ASTNode>& node) {
    return node->value;
}

// メモリマップ参照ノード評価
Value Interpreter::evaluateMemoryMapRef(const std::shared_ptr<ASTNode>& node) {
    std::string mapRef = node->value;
    if (mapRef.size() < 3 || mapRef.substr(0, 2) != "$^") {
        throw std::runtime_error("Invalid memory map reference: " + mapRef);
    }
    
    char mapType = mapRef[2];
    MemoryMap& memMap = getMemoryMap(mapType);
    
    if (!memMap.isMapped()) {
        throw std::runtime_error("Memory map not initialized for type: " + std::string(1, mapType));
    }
    
    // インデックス取得
    size_t index = 0;
    if (mapRef.size() > 3) {
        index = std::stoi(mapRef.substr(3));
    }
    
    return memMap.readElement(index);
}

// マップウィンドウスライドノード評価
Value Interpreter::evaluateMapWindowSlide(const std::shared_ptr<ASTNode>& node) {
    if (node->children.size() < 2) {
        throw std::runtime_error("Map window slide missing arguments");
    }
    
    // スライド量を取得
    Value slideAmountValue = evaluateNode(node->children[0]);
    int slideAmount = std::get<int>(slideAmountValue);
    
    // メモリマップ参照を取得
    std::string mapRefStr = node->children[1]->value;
    if (mapRefStr.size() < 3 || mapRefStr.substr(0, 2) != "$^") {
        throw std::runtime_error("Invalid memory map reference in slide: " + mapRefStr);
    }
    
    char mapType = mapRefStr[2];
    MemoryMap& memMap = getMemoryMap(mapType);
    
    if (!memMap.isMapped()) {
        throw std::runtime_error("Memory map not initialized for slide operation");
    }
    
    memMap.slideWindow(slideAmount);
    return Value();
}

// メモリマップの取得
MemoryMap& Interpreter::getMemoryMap(char type) {
    switch (type) {
        case '#': return intMemoryMap;
        case '@': return stringMemoryMap;
        case '~': return floatMemoryMap;
        case '%': return boolMemoryMap;
        default: throw std::runtime_error("Unknown memory map type: " + std::string(1, type));
    }
}
