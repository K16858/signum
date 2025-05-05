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
    if (pos >= tokens.size()) {
        reportError("Error: Unexpected end of input");
        return nullptr;
    }
    
    // まずトークンの種類でどう処理するか決める
    switch (tokens[pos].type) {
        // メモリ参照から始まるステートメント
        case TokenType::MemoryRef: {
            if (pos + 1 < tokens.size()) {
                TokenType nextType = tokens[pos+1].type;
                
                // 代入系演算子
                if (nextType == TokenType::Assign || 
                    nextType == TokenType::PlusEqual || 
                    nextType == TokenType::MinusEqual ||
                    nextType == TokenType::MultiplyEqual ||
                    nextType == TokenType::DivideEqual ||
                    nextType == TokenType::ModulusEqual) {
                    return parseAssignment();
                }
                // ファイル出力
                else if (nextType == TokenType::DoubleLAngleBracket) {
                    return parseFileOutputStatement();
                }
                // ファイル入力
                else if (nextType == TokenType::DoubleRAngleBracket) {
                    return parseFileInputStatement();
                }
                // 四則演算子や比較演算子の場合は式として扱う
                else {
                    auto expr = parseExpression();
                    if (pos < tokens.size() && tokens[pos].type == TokenType::Semicolon) {
                        advance(); // セミコロンスキップ
                    }
                    return expr;
                }
            }
            // 単独メモリ参照
            auto ref = parseMemoryRef();
            if (pos < tokens.size() && tokens[pos].type == TokenType::Semicolon) {
                advance();
            }
            return ref;
        }

        // ファイル入出力
        case TokenType::String: {
            if (pos + 1 < tokens.size()) {
                TokenType nextType = tokens[pos+1].type;
                // ファイル出力
                if (nextType == TokenType::DoubleLAngleBracket) {
                    return parseFileOutputStatement();
                }
                // ファイル入力
                else if (nextType == TokenType::DoubleRAngleBracket) {
                    return parseFileInputStatement();
                }
            }
            else {
                reportError("Error: Unexpected end of input after string token");
                return nullptr;
            }
        }

        // 関数呼び出し
        case TokenType::FunctionCall:
            return parseFunctionCall();
        
        // 関数定義
        case TokenType::Function:
            return parseFunction();

        // 条件式関連
        case TokenType::Not:
        case TokenType::And:
        case TokenType::Or:
            return parseCondition();
            
        // 条件分岐
        case TokenType::If:
        case TokenType::ElseIf:
        case TokenType::Else:
            return parseIfStatement();
            
        // ループ
        case TokenType::Loop:
            return parseLoopStatement();

        //  出力文
        case TokenType::LAngleBracket:
            return parseOutputStatement();
        
        // 入力文
        case TokenType::RAngleBracket:
            return parseInputStatement();

        // ファイル出力文
        case TokenType::DoubleLAngleBracket:
            return parseFileOutputStatement();

        // ファイル入力文
        case TokenType::DoubleRAngleBracket:
            return parseFileInputStatement();
        
        case TokenType::IntCast:
        case TokenType::FloatCast:
        case TokenType::StrCast:
        case TokenType::BoolCast:
            return parseCast(); // 型変換の解析
            
        // その他の開始トークン（数値や文字列など）
        default:
            auto expr = parseExpression();
            if (pos < tokens.size() && tokens[pos].type == TokenType::Semicolon) {
                advance();
            }
            return expr;
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

    while (pos < tokens.size() && (tokens[pos].type == TokenType::Multiply || tokens[pos].type == TokenType::Divide || tokens[pos].type == TokenType::Modulus)) {
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

    if (tokens[pos].type == TokenType::Integer || tokens[pos].type == TokenType::Float) {
        std::cout << "数値を解析中..." << std::endl;
        auto node = std::make_unique<ASTNode>(NodeType::Number, tokens[pos].value);
        advance();
        return node;
    } 
    else if (tokens[pos].type == TokenType::String) {
        std::cout << "文字列を解析中..." << std::endl;
        auto node = std::make_unique<ASTNode>(NodeType::String, tokens[pos].value);
        advance();
        return node;
    } 
    else if (tokens[pos].type == TokenType::MemoryRef) {
        auto node = parseMemoryRef();
        return node;
    } 
    else if (tokens[pos].type == TokenType::LParen) {
        std::cout << "括弧を解析中..." << std::endl;
        advance(); // "("
        auto node = parseExpression(); // 括弧内の式を解析
        if (tokens[pos].type != TokenType::RParen) {
            reportError("Expected ')' after expression in factor");
            return nullptr;
        }
        advance(); // ")"
        return node;
    } 
    else if (tokens[pos].type == TokenType::IntCast ||
        tokens[pos].type == TokenType::FloatCast ||
        tokens[pos].type == TokenType::StrCast ||
        tokens[pos].type == TokenType::BoolCast) {
        std::cout << "型変換を解析中..." << std::endl;
        return parseCast();
    }
    else {
        reportError("Error: Expected factor");
        return nullptr;
    }

}

// 代入文と複合代入の解析
std::unique_ptr<ASTNode> Parser::parseAssignment() {
    std::cout << "代入文を解析中..." << std::endl;
    
    auto left = parseMemoryRef(); // 左辺のメモリ参照
    if (!left) {
        reportError("Error: Expected memory reference on left side of assignment");
        return nullptr;
    }

    // 演算子タイプを確認
    TokenType opType = tokens[pos].type;
    std::string opValue = tokens[pos].value;
    advance(); // 演算子をスキップ
    
    std::string leftValue = left->value;

    auto node = std::make_unique<ASTNode>(NodeType::Assignment, opValue);
    node->children.push_back(std::move(left));

    // 通常の代入
    if (opType == TokenType::Assign) {
        node->children.push_back(parseExpression());
    } 
    // 複合代入（+=, -=, *=, /=, %=）
    else {
        // 左辺のコピーを作成
        auto leftCopy = std::make_unique<ASTNode>(NodeType::MemoryRef, leftValue);

        // 演算子抽出
        std::string actualOp;
        if (opType == TokenType::PlusEqual) {
            actualOp = "+";
        }
        else if (opType == TokenType::MinusEqual) {
            actualOp = "-";
        }
        else if (opType == TokenType::MultiplyEqual) {
            actualOp = "*";
        }
        else if (opType == TokenType::DivideEqual) {
            actualOp = "/";
        }
        else if (opType == TokenType::ModulusEqual) {
            actualOp = "%";
        }
        else {
            reportError("Error: Unknown compound assignment operator");
            return nullptr;
        }

        // 右辺の式を構築
        auto right = std::make_unique<ASTNode>(NodeType::ArithmeticExpression, actualOp);
        right->children.push_back(std::move(leftCopy));
        right->children.push_back(parseExpression());
        
        node->children.push_back(std::move(right));
    }
        
    if (pos < tokens.size() && tokens[pos].type == TokenType::Semicolon) {
        advance();
    }

    return node;
}

// 比較演算の解析
std::unique_ptr<ASTNode> Parser::parseComparison() {
    std::cout << "比較演算を解析中..." << std::endl;
    auto left = parseExpression(); // 左辺の式

    if (pos < tokens.size() && (
        tokens[pos].type == TokenType::EqualTo ||
        tokens[pos].type == TokenType::NotEqualTo ||
        tokens[pos].type == TokenType::LAngleBracket ||
        tokens[pos].type == TokenType::RAngleBracket ||
        tokens[pos].type == TokenType::LessThanOrEqual ||
        tokens[pos].type == TokenType::GreaterThanOrEqual)) {
        
        // 比較ノードを作成
        auto node = std::make_unique<ASTNode>(NodeType::Comparison, tokens[pos].value);
        node->children.push_back(std::move(left));
        
        advance(); //　比較演算子
        
        node->children.push_back(parseExpression()); // 右辺
        
        return node;
    }

    return left;
}

// 条件式の解析
std::unique_ptr<ASTNode> Parser::parseCondition() {
    std::cout << "条件式を解析中..." << std::endl;
    
    // NOTの処理
    if (tokens[pos].type == TokenType::Not) {
        auto node = std::make_unique<ASTNode>(NodeType::LogicalExpression, "!");
        advance(); // "!" をスキップ
        node->children.push_back(parseCondition()); // NOTの後の式
        return node;
    }
    
    // 最初の比較式
    auto left = parseComparison();
    
    // AND/ORが続く場合
    while (pos < tokens.size() && (tokens[pos].type == TokenType::And || tokens[pos].type == TokenType::Or)) {
        auto op = tokens[pos].value; // "&&"  "||"
        advance();
        
        auto right = parseComparison(); // 右辺
        auto node = std::make_unique<ASTNode>(NodeType::LogicalExpression, op);
        node->children.push_back(std::move(left));
        node->children.push_back(std::move(right));
        left = std::move(node);
    }
    
    return left;
}

// 条件分岐の解析
std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    std::cout << "条件分岐を解析中..." << std::endl;
    auto node = std::make_unique<ASTNode>(NodeType::IfStatement);
    advance(); // "if"

    if (tokens[pos].type == TokenType::LParen) {
        advance(); // "("
        node->children.push_back(parseCondition()); // 条件式

        if (tokens[pos].type != TokenType::RParen) {
            reportError("Expected ')' after condition in if statement");
            return nullptr;
        } 
        advance(); // ")"
    } 
    else {
        reportError("Expected '(' after 'if'");
        return nullptr;
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

        while (pos < tokens.size() && tokens[pos].type == TokenType::Else) {
            advance(); // "else"
            
            // else if の場合
            if (pos < tokens.size() && tokens[pos].type == TokenType::If) {
                // else if部分を再帰的に解析して、子ノードとして追加
                node->children.push_back(parseIfStatement());
                // parseIfStatementが戻ったらループを抜ける
                break;
            }
            // else の場合
            else if (tokens[pos].type == TokenType::LBrace) {
                advance(); // "{"
                auto elseNode = std::make_unique<ASTNode>(NodeType::Statement);
                while (pos < tokens.size() && tokens[pos].type != TokenType::RBrace) {
                    elseNode->children.push_back(parseStatement());
                }

                if (tokens[pos].type != TokenType::RBrace) {
                    reportError("Expected '}' after else block in if statement");
                    return nullptr;
                }
                advance(); // "}"
                
                node->children.push_back(std::move(elseNode)); // elseを追加
            } 
            else {
                reportError("Expected '{' or 'if' after 'else' in if statement");
                return nullptr;
            }
        }
    }
    else {
        reportError("Expected '{' after condition in if statement");
        return nullptr;
    }

    return node;
}

// ループの解析
std::unique_ptr<ASTNode> Parser::parseLoopStatement() {
    std::cout << "ループを解析中..." << std::endl;
    auto node = std::make_unique<ASTNode>(NodeType::LoopStatement);
    advance(); // "&"をスキップ

    // ループ条件の解析
    if (tokens[pos].type == TokenType::LParen) {
        advance(); // "("
        node->children.push_back(parseCondition()); // 条件式

        if (tokens[pos].type != TokenType::RParen) {
            reportError("Expected ')' after condition in loop statement");
            return nullptr;
        } 
        advance(); // ")"
    } 
    else {
        reportError("Expected '(' after 'loop'");
        return nullptr;
    }
    
    // ループブロックの解析
    if (tokens[pos].type == TokenType::LBrace) {
        advance(); // "{"
        auto loopNode = std::make_unique<ASTNode>(NodeType::Statement);
        while (pos < tokens.size() && tokens[pos].type != TokenType::RBrace) {
            loopNode->children.push_back(parseStatement());
        }

        if (tokens[pos].type != TokenType::RBrace) {
            reportError("Expected '}' after loop block");
            return nullptr;
        }
        advance(); // "}"
        
        node->children.push_back(std::move(loopNode)); // ループ本体を追加
    } 
    else {
        reportError("Expected '{' after loop condition");
        return nullptr;
    }

    return node;
}

// 出力文の解析
std::unique_ptr<ASTNode> Parser::parseOutputStatement() {
    std::cout << "出力文を解析中..." << std::endl;
    auto node = std::make_unique<ASTNode>(NodeType::OutputStatement);
    advance(); // "<" をスキップ
    
    // 出力する式を解析
    if (tokens[pos].type == TokenType::MemoryRef) {
        auto ref = parseMemoryRef();
        if (!ref) {
            reportError("Expected memory reference in output statement");
            return nullptr;
        }
        node->children.push_back(std::move(ref));
    } 
    else if (tokens[pos].type == TokenType::String) {
        auto strNode = std::make_unique<ASTNode>(NodeType::String, tokens[pos].value);
        advance(); // 文字列をスキップ
        node->children.push_back(std::move(strNode));
    } 
    else {
        auto expr = parseExpression();
        if (!expr) {
            reportError("Expected expression in output statement");
            return nullptr;
        }
        node->children.push_back(std::move(expr));
    }
    
    if (tokens[pos].type != TokenType::Semicolon) {
        reportError("Expected ';' after output statement");
        return nullptr;
    }
    advance(); // セミコロンをスキップ

    return node;
}

// 入力文の解析
std::unique_ptr<ASTNode> Parser::parseInputStatement() {
    std::cout << "入力文を解析中..." << std::endl;
    auto node = std::make_unique<ASTNode>(NodeType::InputStatement);
    advance(); // ">" をスキップ
    
    // 入力するメモリ参照を解析
    if (tokens[pos].type == TokenType::MemoryRef) {
        auto ref = parseMemoryRef();
        if (!ref) {
            reportError("Expected memory reference in input statement");
            return nullptr;
        }
        node->children.push_back(std::move(ref));
    } 
    else {
        reportError("Expected memory reference in input statement");
        return nullptr;
    }
    
    if (tokens[pos].type != TokenType::Semicolon) {
        reportError("Expected ';' after input statement");
        return nullptr;
    }
    advance(); // セミコロンをスキップ

    return node;
}

// ファイル出力文の解析
std::unique_ptr<ASTNode> Parser::parseFileOutputStatement() {
    std::cout << "ファイル出力文を解析中..." << std::endl;
    auto node = std::make_unique<ASTNode>(NodeType::FileOutputStatement);

    // ファイル名の解析（文字列かメモリ参照）
    if (tokens[pos].type == TokenType::String) {
        auto fileNode = std::make_unique<ASTNode>(NodeType::String, tokens[pos].value);
        advance(); // 文字列をスキップ
        node->children.push_back(std::move(fileNode));
    } else if (tokens[pos].type == TokenType::MemoryRef) {
        auto fileNode = parseMemoryRef();
        if (!fileNode) {
            reportError("Expected memory reference for file name");
            return nullptr;
        }
        node->children.push_back(std::move(fileNode));
    } else {
        reportError("Expected string or memory reference for file name");
        return nullptr;
    }
    
    // "<<"" をチェック
    if (tokens[pos].type != TokenType::DoubleLAngleBracket) {
        reportError("Expected '<<' after file name in file output statement");
        return nullptr;
    }
    advance(); // "<<"
    
    // 出力する式を解析
    if (tokens[pos].type == TokenType::MemoryRef) {
        auto ref = parseMemoryRef();
        if (!ref) {
            reportError("Expected memory reference in file output statement");
            return nullptr;
        }
        node->children.push_back(std::move(ref));
    } 
    else if (tokens[pos].type == TokenType::String) {
        auto strNode = std::make_unique<ASTNode>(NodeType::String, tokens[pos].value);
        advance(); // 文字列をスキップ
        node->children.push_back(std::move(strNode));
    } 
    else {
        auto expr = parseExpression();
        if (!expr) {
            reportError("Expected expression in file output statement");
            return nullptr;
        }
        node->children.push_back(std::move(expr));
    }
    
    if (tokens[pos].type != TokenType::Semicolon) {
        reportError("Expected ';' after file output statement");
        return nullptr;
    }
    advance(); // セミコロンをスキップ

    return node;
}

// ファイル入力文の解析
std::unique_ptr<ASTNode> Parser::parseFileInputStatement() {
    std::cout << "ファイル入力文を解析中..." << std::endl;
    auto node = std::make_unique<ASTNode>(NodeType::FileInputStatement);

    // ファイル名の解析（文字列かメモリ参照）
    if (tokens[pos].type == TokenType::String) {
        auto fileNode = std::make_unique<ASTNode>(NodeType::String, tokens[pos].value);
        advance(); // 文字列をスキップ
        node->children.push_back(std::move(fileNode));
    } else if (tokens[pos].type == TokenType::MemoryRef) {
        auto fileNode = parseMemoryRef();
        if (!fileNode) {
            reportError("Expected memory reference for file name");
            return nullptr;
        }
        node->children.push_back(std::move(fileNode));
    } else {
        reportError("Expected string or memory reference for file name");
        return nullptr;
    }
    
    // ">>" をチェック
    if (tokens[pos].type != TokenType::DoubleRAngleBracket) {
        reportError("Expected '>>' after file name in file input statement");
        return nullptr;
    }
    advance(); // ">>"
    
    // 入力するメモリ参照を解析
    if (tokens[pos].type == TokenType::MemoryRef) {
        auto ref = parseMemoryRef();
        if (!ref) {
            reportError("Expected memory reference in file input statement");
            return nullptr;
        }
        node->children.push_back(std::move(ref));
    } 
    else {
        reportError("Expected memory reference in file input statement");
        return nullptr;
    }
    
    if (tokens[pos].type != TokenType::Semicolon) {
        reportError("Expected ';' after file input statement");
        return nullptr;
    }
    advance(); // セミコロンをスキップ

    return node;
}

// 関数の解析
std::unique_ptr<ASTNode> Parser::parseFunction() {
    std::cout << "関数を解析中..." << std::endl;
    // 関数番号の取得
    std::string tokenValue = tokens[pos].value;
    std::string functionNumber;
    
    if (tokenValue.size() >= 3 && tokenValue.substr(0, 1) == "_") {
        functionNumber = tokenValue.substr(1);
    } 
    else {
        reportError("Invalid function call format. Expected $_XXX where X is a digit.");
        return nullptr;
    }
    
    // 関数番号の検証
    if (functionNumber.size() != 3 || 
        !std::isdigit(functionNumber[0]) || 
        !std::isdigit(functionNumber[1]) || 
        !std::isdigit(functionNumber[2])) {
        reportError("Invalid function number format. Expected $_XXX where X is a digit.");
        return nullptr;
    }
    
    auto node = std::make_unique<ASTNode>(NodeType::FunctionCall, functionNumber);
    advance(); // 関数定義トークンをスキップ

    if (tokens[pos].type != TokenType::LBrace) {
        reportError("Expected '{' after function definition");
        return nullptr;
    }
    advance(); // "{"
    
    while (pos < tokens.size() && tokens[pos].type != TokenType::RBrace) {
        node->children.push_back(parseStatement());
    }

    if (tokens[pos].type != TokenType::RBrace) {
        reportError("Expected '}' after function body");
        return nullptr;
    }
    advance(); // "}"

    return node;
}

// 関数呼び出しの解析
std::unique_ptr<ASTNode> Parser::parseFunctionCall() {
    std::cout << "関数呼び出しを解析中..." << std::endl;
    
    // 関数番号の取得
    std::string tokenValue = tokens[pos].value;
    std::string functionNumber;
    
    if (tokenValue.size() >= 3 && tokenValue.substr(0, 2) == "$_") {
        functionNumber = tokenValue.substr(2);
    } 
    else {
        reportError("Invalid function call format. Expected $_XXX where X is a digit.");
        return nullptr;
    }
    
    // 関数番号の検証
    if (functionNumber.size() != 3 || 
        !std::isdigit(functionNumber[0]) || 
        !std::isdigit(functionNumber[1]) || 
        !std::isdigit(functionNumber[2])) {
        reportError("Invalid function number format. Expected $_XXX where X is a digit.");
        return nullptr;
    }
    
    auto node = std::make_unique<ASTNode>(NodeType::FunctionCall, functionNumber);
    advance(); // 関数呼び出しトークンをスキップ
    
    // セミコロンチェック
    if (tokens[pos].type != TokenType::Semicolon) {
        reportError("Expected ';' after function call");
        return nullptr;
    }
    advance(); // セミコロンをスキップ
    
    return node;
}

// 型変換の解析
std::unique_ptr<ASTNode> Parser::parseCast() {
    std::cout << "型変換を解析中..." << std::endl;
    
    // キャスト種類を保存
    std::string castType;
    if (tokens[pos].type == TokenType::IntCast) castType = "int";
    else if (tokens[pos].type == TokenType::FloatCast) castType = "float";
    else if (tokens[pos].type == TokenType::StrCast) castType = "string";
    else if (tokens[pos].type == TokenType::BoolCast) castType = "bool";
    else {
        reportError("Expected cast type (int, float, string, bool)");
        return nullptr;
    }
    
    auto node = std::make_unique<ASTNode>(NodeType::Cast, castType);
    advance(); // キャストトークンをスキップ
    
    // キャストする対象の式
    auto expr = parseExpression();
    if (!expr) {
        reportError("Expected expression for cast");
        return nullptr;
    }
    node->children.push_back(std::move(expr));
    
    // セミコロンチェック
    if (tokens[pos].type != TokenType::Semicolon) {
        reportError("Expected ';' after cast expression");
        return nullptr;
    }
    advance(); // セミコロンをスキップ

    return node;
}
