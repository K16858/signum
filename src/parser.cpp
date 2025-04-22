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
    // 比較演算の解析
    else if (tokens[pos+1].type == TokenType::EqualTo||
             tokens[pos+1].type == TokenType::NotEqualTo ||
             tokens[pos+1].type == TokenType::LAngleBracket ||
             tokens[pos+1].type == TokenType::RAngleBracket ||
             tokens[pos+1].type == TokenType::LessThanOrEqual ||
             tokens[pos+1].type == TokenType::GreaterThanOrEqual) {
        return parseComparison();
    }
    // 演算の解析
    else if (tokens[pos+1].type == TokenType::Plus ||
             tokens[pos+1].type == TokenType::Minus ||
             tokens[pos+1].type == TokenType::Multiply ||
             tokens[pos+1].type == TokenType::Divide) {
        return parseExpression();
    }
    // 条件式の解析
    else if (tokens[pos].type == TokenType::Not ||
             tokens[pos].type == TokenType::And ||
             tokens[pos].type == TokenType::Or) {
        return parseCondition();
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

// 加減算式の解析
std::unique_ptr<ASTNode> Parser::parseExpression() {
    std::cout << "加減算式を解析中..." << std::endl;
    auto left = parseTerm(); // 乗除算を先に処理

    while (pos < tokens.size() && (tokens[pos].type == TokenType::Plus || tokens[pos].type == TokenType::Minus)) {
        auto op = tokens[pos].value;
        advance();
        
        auto right = parseTerm();
        auto node = std::make_unique<ASTNode>(NodeType::ArithmeticExpression, op);
        node->children.push_back(std::move(left));
        node->children.push_back(std::move(right));
        left = std::move(node);
    }
    
    return left;
}

// 乗除算式の解析
std::unique_ptr<ASTNode> Parser::parseTerm() {
    std::cout << "乗除算式を解析中..." << std::endl;
    auto left = parseFactor(); // 左辺の因子

    if (pos < tokens.size() && (tokens[pos].type == TokenType::Multiply || tokens[pos].type == TokenType::Divide)) {
        auto op = tokens[pos].value;
        advance(); // 演算子をスキップ
        
        auto right = parseFactor(); // 右辺の因子
        auto node = std::make_unique<ASTNode>(NodeType::ArithmeticExpression, op);
        node->children.push_back(std::move(left));
        node->children.push_back(std::move(right)); // 右辺の因子
        left = std::move(node); // 左辺を更新
    }

    return left; // 演算子がなければ左辺だけを返す
}

// 因子の解析
std::unique_ptr<ASTNode> Parser::parseFactor() {
    std::cout << "因子を解析中..." << std::endl;
    auto node = std::make_unique<ASTNode>(NodeType::Factor);

    if (tokens[pos].type == TokenType::Integer || tokens[pos].type == TokenType::Float) {
        node->children.push_back(std::make_unique<ASTNode>(NodeType::Number, tokens[pos].value));
        advance();
    } 
    else if (tokens[pos].type == TokenType::String) {
        node->children.push_back(std::make_unique<ASTNode>(NodeType::String, tokens[pos].value));
        advance();
    } 
    else if (tokens[pos].type == TokenType::MemoryRef) {
        node->children.push_back(parseMemoryRef());
    } 
    else if (tokens[pos].type == TokenType::LParen) {
        advance(); // "("
        node->children.push_back(parseExpression()); // 式を解析
        if (tokens[pos].type != TokenType::RParen) {
            reportError("Expected ')' after expression in factor");
            return nullptr;
        }
        advance(); // ")"
    } 
    else {
        reportError("Error: Expected factor");
    }

    return node;
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

std::unique_ptr<ASTNode> Parser::parseComparison() {
    std::cout << "比較演算を解析中..." << std::endl;
    auto node = std::make_unique<ASTNode>(NodeType::Comparison, tokens[pos+1].value);
    node->children.push_back(parseExpression()); // 左辺の式
    node->value = tokens[pos].value; // 演算子の値を保存
    advance(); // 比較演算子をスキップ

    node->children.push_back(parseExpression()); // 右辺の式

    return node;
}

std::unique_ptr<ASTNode> Parser::parseCondition() {
    std::cout << "条件式を解析中..." << std::endl;
    auto node = std::make_unique<ASTNode>(NodeType::Condition);
    node->children.push_back(parseExpression());
    
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
