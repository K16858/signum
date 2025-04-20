#include "parser.hpp"

std::unique_ptr<ASTNode> Parser::parseProgram() {
    auto node = std::make_unique<ASTNode>(NodeType::Program);

    while (pos < tokens.size()) {
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
}