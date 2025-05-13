// SigNum Interpreter

# include "interpreter.hpp"

// メモリ参照を解決する
Value Interpreter::resolveMemoryRef(const std::string& ref) {
    if (ref[0] == '#') {
        return intPool[std::stoi(ref.substr(2))];
    } else if (ref[0] == '@') {
        return stringPool[std::stoi(ref.substr(2))];
    } else if (ref[0] == '~') {
        return floatPool[std::stoi(ref.substr(2))];
    } else if (ref[0] == '%') {
        return boolPool[std::stoi(ref.substr(2))];
    }
    throw std::runtime_error("Invalid memory reference: " + ref);
}

// メモリインデックスを評価する
int Interpreter::evaluateMemoryIndex(const std::string& indexExpr) {
    if (indexExpr[0] == '#') {
        return std::stoi(indexExpr.substr(2));
    } else if (indexExpr[0] == '@') {
        return std::stoi(indexExpr.substr(2));
    } else if (indexExpr[0] == '~') {
        return std::stoi(indexExpr.substr(2));
    } else if (indexExpr[0] == '%') {
        return std::stoi(indexExpr.substr(2));
    }
    throw std::runtime_error("Invalid memory index: " + indexExpr);
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
    setMemoryValue(varName[0], evaluateMemoryIndex(varName), value);
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

