// SigNum Parser
#include "parser.hpp"

std::unique_ptr<ASTNode> Parser::parseProgram() {
    auto node = std::make_unique<ASTNode>(NodeType::Program);

    while (pos < tokens.size()) {
        if (tokens[pos].type == TokenType::End) {
            break; // 終端トークンに到達
        }
        if (tokens[pos].type == TokenType::Semicolon) {
            advance(); // セミコロンをスキップ
            continue;
        }
        node->children.push_back(parseStatement());
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    // 代入文の解析
    if (pos + 2 < tokens.size() && tokens[pos+1].type == TokenType::Assign) {
        auto node = std::make_unique<ASTNode>(NodeType::Assignment);

        node->children.push_back(parseMemoryRef()); // 左辺のメモリ参照
        advance(); // '='
        node->children.push_back(parseExpression()); // 右辺の式

        if (pos < tokens.size() && tokens[pos].type == TokenType::Semicolon) {
            advance();
        }

        return node;
    }



    reportError("Unsupported statement type");
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseMemoryRef() {
    if (tokens[pos].type == TokenType::MemoryRef) {
        auto node = std::make_unique<ASTNode>(NodeType::MemoryRef, tokens[pos].value);
        advance();
        return node;
    }

    reportError("Error: Expected memory reference");
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    if (tokens[pos].type == TokenType::Number) {
        auto node = std::make_unique<ASTNode>(NodeType::Number, tokens[pos].value);
        advance();
        return node;
    } 
    else if (tokens[pos].type == TokenType::String) {
        auto node = std::make_unique<ASTNode>(NodeType::String, tokens[pos].value);
        advance();
        return node;
    }

    reportError("Error: Expected expression");
    return nullptr;
}
