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
