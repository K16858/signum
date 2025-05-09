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
        // str-str
        else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right) && op == "+") {
            return std::get<std::string>(left) + std::get<std::string>(right);
        }
        // str-int
        else if (std::holds_alternative<std::string>(left) && std::holds_alternative<int>(right) && op == "+") {
            return std::get<std::string>(left) + std::to_string(std::get<int>(right));
        }
        // str-double
        else if (std::holds_alternative<std::string>(left) && std::holds_alternative<double>(right) && op == "+") {
            return std::get<std::string>(left) + std::to_string(std::get<double>(right));
        }
        // str-bool
        else if (std::holds_alternative<std::string>(left) && std::holds_alternative<bool>(right) && op == "+") {
            return std::get<std::string>(left) + (std::get<bool>(right) ? "true" : "false");
        }
        // int-str
        else if (std::holds_alternative<int>(left) && std::holds_alternative<std::string>(right) && op == "+") {
            return std::to_string(std::get<int>(left)) + std::get<std::string>(right);
        }
        // double-str
        else if (std::holds_alternative<double>(left) && std::holds_alternative<std::string>(right) && op == "+") {
            return std::to_string(std::get<double>(left)) + std::get<std::string>(right);
        }
        // bool-str
        else if (std::holds_alternative<bool>(left) && std::holds_alternative<std::string>(right) && op == "+") {
            return (std::get<bool>(left) ? "true" : "false") + std::get<std::string>(right);
        }
    }
    throw std::runtime_error("Invalid arithmetic expression: " + node->toJSON());
}
