// SigNum AST

#include "ast.hpp"

// ノード型を文字列に変換する関数
std::string nodeType2String(NodeType type) {
    switch (type) {
        case NodeType::Program: return "ルート";
        case NodeType::Function: return "関数定義";
        case NodeType::FunctionCall: return "関数呼び出し";
        case NodeType::Statement: return "ステートメント";
        case NodeType::ArithmeticExpression: return "四則演算式";
        case NodeType::LogicalExpression: return "論理式";
        case NodeType::Factor: return "因子";
        case NodeType::MemoryRef: return "メモリ参照";
        case NodeType::Number: return "数値";
        case NodeType::String: return "文字列";
        case NodeType::Symbol: return "シンボル";
        case NodeType::Operator: return "演算子";
        case NodeType::Comparison: return "比較演算式";
        case NodeType::Condition: return "条件式";
        case NodeType::Cast: return "型変換";
        case NodeType::IfStatement: return "条件分岐";
        case NodeType::LoopStatement: return "ループ";
        case NodeType::Assignment: return "代入";
        case NodeType::InputStatement: return "入力文";
        case NodeType::OutputStatement: return "出力文";
        case NodeType::FileInputStatement: return "ファイル入力文";
        case NodeType::FileOutputStatement: return "ファイル出力文";
        default: return "不明";
    }
}

// コンストラクタ
ASTNode::ASTNode(NodeType type, const std::string& value)
    : type(type), value(value) {}

// デバッグ用表示メソッド
void ASTNode::print(int indent) const {
    for (int i = 0; i < indent; i++) std::cout << "  ";
    std::cout << "ノード: " << nodeType2String(type) << ", 値: " << value << std::endl;
    
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
