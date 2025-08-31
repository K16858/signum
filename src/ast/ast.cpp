// SigNum AST

#include "ast.hpp"

// ノード型を文字列に変換する関数
std::string nodeType2String(NodeType type) {
    switch (type) {
        case NodeType::Program: return "ProgramRoot";
        case NodeType::Function: return "Function";
        case NodeType::FunctionCall: return "FunctionCall";
        case NodeType::Statement: return "Statement";
        case NodeType::ArithmeticExpression: return "AritmeticExpression";
        case NodeType::LogicalExpression: return "LogicalExpression";
        case NodeType::Factor: return "Factor";
        case NodeType::MemoryRef: return "MemoryRef";
        case NodeType::Number: return "Number";
        case NodeType::String: return "String";
        case NodeType::Symbol: return "Symbol";
        case NodeType::Operator: return "Operator";
        case NodeType::Comparison: return "Comparison";
        case NodeType::Condition: return "Condition";
        case NodeType::Cast: return "Cast";
        case NodeType::IfStatement: return "IfStatement";
        case NodeType::LoopStatement: return "LoopStatement";
        case NodeType::Assignment: return "Assignment";
        case NodeType::InputStatement: return "InputStatement";
        case NodeType::OutputStatement: return "OutputStatement";
        case NodeType::FileInputStatement: return "FileInputStatement";
        case NodeType::FileOutputStatement: return "FileOutputStatement";
        case NodeType::StackOperation: return "StackOperation";
        case NodeType::MemoryMapRef: return "MemoryMapRef";
        case NodeType::MapWindowSlide: return "MapWindowSlide";
        case NodeType::Error: return "Error";
        default: return "Unknown";
    }
}

// コンストラクタ
ASTNode::ASTNode(NodeType type, const std::string& value)
    : type(type), value(value) {}

// デバッグ用表示メソッド
void ASTNode::print(int indent) const {
    for (int i = 0; i < indent; i++) std::cout << "  ";
    std::cout << "Node: " << nodeType2String(type) << ", Value: " << value << std::endl;
    
    for (const auto& child : children) {
        child->print(indent + 1);
    }
}

// JSON変換メソッド
std::string ASTNode::toJSON(int indent) const {
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
std::string ASTNode::escapeJSON(const std::string& input) {
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
bool ASTNode::saveToJSONFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error file cannot open: " << filename << std::endl;
        return false;
    }
    
    file << toJSON();
    file.close();
    return true;
}
