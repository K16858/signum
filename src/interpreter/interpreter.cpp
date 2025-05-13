// SigNum Interpreter

# include "interpreter.hpp"


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
        return evaluateNode(it->second);
    }
    throw std::runtime_error("Function not found: " + node->value);
}

// 代入ノード評価
Value Interpreter::evaluateAssignment(const std::shared_ptr<ASTNode>& node) {
    std::string varName = node->children[0]->value;
    Value value = evaluateNode(node->children[1]);

    int startPos = (varName[0] == '$') ? 1 : 0;

    setMemoryValue(varName[startPos], evaluateMemoryIndex(varName), value);
    return value;
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
            
            if (op == "&&") return lval && rval;
            if (op == "||") return lval || rval;
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

// if文ノード評価
Value Interpreter::evaluateIfStatement(const std::shared_ptr<ASTNode>& node) {
    Value condition = evaluateNode(node->children[0]);
    if (std::holds_alternative<bool>(condition) && std::get<bool>(condition)) {
        return evaluateNode(node->children[1]);
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
    std::string input;
    std::cout << "Input " << varName << ": ";
    std::cin >> input;
    setMemoryValue(varName[0], evaluateMemoryIndex(varName), input);
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
    std::string varName = node->children[1]->value;

    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    setMemoryValue(varName[0], evaluateMemoryIndex(varName), content);
    return Value();
}

// ファイル出力文ノード評価
Value Interpreter::evaluateFileOutputStatement(const std::shared_ptr<ASTNode>& node) {
    std::string filename = std::get<std::string>(evaluateNode(node->children[0]));
    Value value = evaluateNode(node->children[1]);

    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    file << valueToString(value);
    return Value();
}

// メモリ参照ノード評価
Value Interpreter::evaluateMemoryRef(const std::shared_ptr<ASTNode>& node) {
    return resolveMemoryRef(node->value);
}

Value Interpreter::evaluateNumber(const std::shared_ptr<ASTNode>& node) {
    return std::stoi(node->value);
}

Value Interpreter::evaluateString(const std::shared_ptr<ASTNode>& node) {
    return node->value;
}

