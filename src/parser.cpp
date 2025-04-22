// SigNum Parser
#include "parser.hpp"

// プログラム全体を解析
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

// ステートメントの解析
std::unique_ptr<ASTNode> Parser::parseStatement() {
    // 代入文の解析
    if (pos + 2 < tokens.size() && tokens[pos+1].type == TokenType::Assign) {
        return parseAssignment();
    }
    // 条件分岐の解析
    else if (tokens[pos].type == TokenType::If) {
        return parseIfStatement();
    }
    else if (tokens[pos].type == TokenType::Loop) {
        return parseLoopStatement();
    }
    else {
        reportError("Unsupported statement type");
        advance();
        return nullptr;
    }
}

// メモリ参照の解析
std::unique_ptr<ASTNode> Parser::parseMemoryRef() {
    if (tokens[pos].type == TokenType::MemoryRef) {
        auto node = std::make_unique<ASTNode>(NodeType::MemoryRef, tokens[pos].value);
        advance();
        return node;
    }

    reportError("Error: Expected memory reference");
    return nullptr;
}

// 式の解析
std::unique_ptr<ASTNode> Parser::parseExpression() {
    std::cout << "式を解析中..." << std::endl;
    if (tokens[pos].type == TokenType::Integer || tokens[pos].type == TokenType::Float) {
        auto node = std::make_unique<ASTNode>(NodeType::Number, tokens[pos].value);
        advance();
        return node;
    } 
    else if (tokens[pos].type == TokenType::String) {
        auto node = std::make_unique<ASTNode>(NodeType::String, tokens[pos].value);
        advance();
        return node;
    }
    else if (tokens[pos].type == TokenType::MemoryRef) {
        return parseMemoryRef(); // メモリ参照も式
    }

    reportError("Error: Expected expression");
    return nullptr;
}

// 代入文の解析
std::unique_ptr<ASTNode> Parser::parseAssignment() {
    std::cout << "代入文を解析中..." << std::endl;
    auto node = std::make_unique<ASTNode>(NodeType::Assignment);

    node->children.push_back(parseMemoryRef()); // 左辺のメモリ参照
    advance(); // "="
    node->children.push_back(parseExpression()); // 右辺の式

    if (pos < tokens.size() && tokens[pos].type == TokenType::Semicolon) {
        advance();
    }

    return node;
}

// 条件分岐の解析
std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    std::cout << "条件分岐を解析中..." << std::endl;
    auto node = std::make_unique<ASTNode>(NodeType::IfStatement);
    advance(); // "if"
    if (tokens[pos].type == TokenType::LParen) {
        advance(); // "("
        node->children.push_back(parseExpression()); // 条件式

        if (tokens[pos].type != TokenType::RParen) {
            reportError("Expected ')' after condition in if statement");
            return nullptr;
        } 
        advance(); // ")"
    } 
    else {
        reportError("Expected '(' after 'if'");
    }

    // thenの解析
    if (tokens[pos].type == TokenType::LBrace) {
        advance(); // "{"
        auto thenNode = std::make_unique<ASTNode>(NodeType::Statement);
        while (pos < tokens.size() && tokens[pos].type != TokenType::RBrace) {
            thenNode->children.push_back(parseStatement());
        }

        if (tokens[pos].type != TokenType::RBrace) {
            reportError("Expected '}' after then block in if statement");
            return nullptr;
        }
        advance(); // "}"

        node->children.push_back(std::move(thenNode)); // thenを追加
    }
    else {
        reportError("Expected '{' after condition in if statement");
        return nullptr;
    }

    return node;
}

// ループの解析
std::unique_ptr<ASTNode> Parser::parseLoopStatement() {

}

 // 入出力文の解析
std::unique_ptr<ASTNode> Parser::parseIOStatement() {

}

// 型変換の解析
std::unique_ptr<ASTNode> Parser::parseCast() {

}
