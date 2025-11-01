// SigNum Parser
#include "parser.hpp"

// プログラム全体を解析
std::shared_ptr<ASTNode> Parser::parseProgram() {
    auto node = std::make_shared<ASTNode>(NodeType::Program);

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
std::shared_ptr<ASTNode> Parser::parseStatement() {
    if (pos >= tokens.size()) {
        return recoverFromError("Error: Unexpected end of input");
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
                else {
                    auto expr = parseExpression();
                    if (pos < tokens.size() && tokens[pos].type == TokenType::Semicolon) {
                        advance();
                    }
                    return expr;
                }
            }
            else {
                return recoverFromError("Error: Unexpected end of input after string token");
            }
        }

        // メモリマップ参照
        case TokenType::MemoryMapRef: {
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
                // ファイル出力（マップ→ファイル）
                else if (nextType == TokenType::DoubleLAngleBracket) {
                    return parseFileOutputStatement();
                }
                // スライド操作
                else if (nextType == TokenType::MapWindowSlide) {
                    return parseMapWindowSlide();
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
            // 単独メモリマップ参照
            auto ref = parseMemoryMapRef();
            if (pos < tokens.size() && tokens[pos].type == TokenType::Semicolon) {
                advance();
            }
            return ref;
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

        // スライド操作
        case TokenType::Integer:
        case TokenType::Float:
            if (pos + 1 < tokens.size() && tokens[pos+1].type == TokenType::MapWindowSlide) {
                return parseMapWindowSlide();
            }
            // それ以外は通常の式として処理
            else {
                auto expr = parseExpression();
                if (pos < tokens.size() && tokens[pos].type == TokenType::Semicolon) {
                    advance();
                }
                return expr;
            }
            
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
std::shared_ptr<ASTNode> Parser::parseMemoryRef() {
    if (tokens[pos].type == TokenType::MemoryRef) {
        auto node = std::make_shared<ASTNode>(NodeType::MemoryRef, tokens[pos].value);
        advance();
        return node;
    }

    return recoverFromError("Error: Expected memory reference");
}

// メモリマップ参照の解析
std::shared_ptr<ASTNode> Parser::parseMemoryMapRef() {
    if (tokens[pos].type == TokenType::MemoryMapRef) {
        auto node = std::make_shared<ASTNode>(NodeType::MemoryMapRef, tokens[pos].value);
        advance();
        return node;
    }

    return recoverFromError("Error: Expected memory map reference");
}

// 加減算式の解析
std::shared_ptr<ASTNode> Parser::parseExpression() {
    debugLog("加減算式を解析中...");
    auto left = parseTerm(); // 乗除算を先に処理

    while (pos < tokens.size() && (tokens[pos].type == TokenType::Plus || tokens[pos].type == TokenType::Minus)) {
        auto op = tokens[pos].value;
        advance();
        
        auto right = parseTerm();
        auto node = std::make_shared<ASTNode>(NodeType::ArithmeticExpression, op);
        node->children.push_back(std::move(left));
        node->children.push_back(std::move(right));
        left = std::move(node);
    }
    
    return left;
}

// 乗除算式の解析
std::shared_ptr<ASTNode> Parser::parseTerm() {
    debugLog("乗除算式を解析中...");
    auto left = parseFactor(); // 左辺の因子

    while (pos < tokens.size() && (tokens[pos].type == TokenType::Multiply || tokens[pos].type == TokenType::Divide || tokens[pos].type == TokenType::Modulus)) {
        auto op = tokens[pos].value;
        advance(); // 演算子をスキップ
        
        auto right = parseFactor(); // 右辺の因子
        auto node = std::make_shared<ASTNode>(NodeType::ArithmeticExpression, op);
        node->children.push_back(std::move(left));
        node->children.push_back(std::move(right)); // 右辺の因子
        left = std::move(node); // 左辺を更新
    }

    return left; // 演算子がなければ左辺だけを返す
}

// 因子の解析
std::shared_ptr<ASTNode> Parser::parseFactor() {
    debugLog("因子を解析中...");

    std::shared_ptr<ASTNode> node;

    if (tokens[pos].type == TokenType::Integer || tokens[pos].type == TokenType::Float) {
        debugLog("数値を解析中...");
        node = std::make_shared<ASTNode>(NodeType::Number, tokens[pos].value);
        advance();
    } 
    else if (tokens[pos].type == TokenType::String) {
        debugLog("文字列を解析中...");
        node = std::make_shared<ASTNode>(NodeType::String, tokens[pos].value);
        advance();
    } 
    else if (tokens[pos].type == TokenType::MemoryRef) {
        node = parseMemoryRef();
    } 
    else if (tokens[pos].type == TokenType::MemoryMapRef) {
        node = parseMemoryMapRef();
    } 
    else if (tokens[pos].type == TokenType::LParen) {
        debugLog("括弧を解析中...");
        advance(); // "("
        node = parseExpression(); // 括弧内の式を解析
        if (tokens[pos].type != TokenType::RParen) {
            return recoverFromError("Expected ')' after expression in factor");
        }
        advance(); // ")"
    } 
    else if (tokens[pos].type == TokenType::IntCast ||
        tokens[pos].type == TokenType::FloatCast ||
        tokens[pos].type == TokenType::StrCast ||
        tokens[pos].type == TokenType::BoolCast) {
        debugLog("型変換を解析中...");
        node = parseCast();
    }
    else {
        return recoverFromError("Error: Expected factor");
    }

    // スタック操作解析
    if (pos < tokens.size() && (
        tokens[pos].type == TokenType::IntegerStackPush ||
        tokens[pos].type == TokenType::IntegerStackPop ||
        tokens[pos].type == TokenType::FloatStackPush ||
        tokens[pos].type == TokenType::FloatStackPop ||
        tokens[pos].type == TokenType::StringStackPush ||
        tokens[pos].type == TokenType::StringStackPop ||
        tokens[pos].type == TokenType::BooleanStackPush ||
        tokens[pos].type == TokenType::BooleanStackPop
    )) {
        debugLog("スタック操作を解析中...");
        std::string operation;
        if (tokens[pos].type == TokenType::IntegerStackPush) operation = "IntegerStackPush";
        else if (tokens[pos].type == TokenType::IntegerStackPop) operation = "IntegerStackPop";
        else if (tokens[pos].type == TokenType::FloatStackPush) operation = "FloatStackPush";
        else if (tokens[pos].type == TokenType::FloatStackPop) operation = "FloatStackPop";
        else if (tokens[pos].type == TokenType::StringStackPush) operation = "StringStackPush";
        else if (tokens[pos].type == TokenType::StringStackPop) operation = "StringStackPop";
        else if (tokens[pos].type == TokenType::BooleanStackPush) operation = "BooleanStackPush";
        else if (tokens[pos].type == TokenType::BooleanStackPop) operation = "BooleanStackPop";

        advance(); // スタック操作をスキップ

        auto stackNode = std::make_shared<ASTNode>(NodeType::StackOperation, operation);
        stackNode->children.push_back(node);
        node = stackNode;
    }

    return node;
}

// 代入文と複合代入の解析
std::shared_ptr<ASTNode> Parser::parseAssignment() {
    debugLog("代入文を解析中...");
    
    std::shared_ptr<ASTNode> left;
    
    // メモリ参照かメモリマップ参照かを判定
    if (tokens[pos].type == TokenType::MemoryRef) {
        left = parseMemoryRef();
    } 
    else if (tokens[pos].type == TokenType::MemoryMapRef) {
        left = parseMemoryMapRef();
    } 
    else {
        return recoverFromError("Error: Expected memory reference or memory map reference on left side of assignment");
    }
    
    if (!left) {
        return recoverFromError("Error: Expected memory reference or memory map reference on left side of assignment");
    }

    // 演算子タイプを確認
    TokenType opType = tokens[pos].type;
    std::string opValue = tokens[pos].value;
    advance(); // 演算子をスキップ
    
    std::string leftValue = left->value;

    auto node = std::make_shared<ASTNode>(NodeType::Assignment, opValue);
    node->children.push_back(std::move(left));

    // 通常の代入
    if (opType == TokenType::Assign) {
        node->children.push_back(parseExpression());
    } 
    // 複合代入（+=, -=, *=, /=, %=）
    else {
        // 左辺のコピーを作成
        auto leftCopy = std::make_shared<ASTNode>(NodeType::MemoryRef, leftValue);

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
            return recoverFromError("Error: Unknown compound assignment operator");
        }

        // 右辺の式を構築
        auto right = std::make_shared<ASTNode>(NodeType::ArithmeticExpression, actualOp);
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
std::shared_ptr<ASTNode> Parser::parseComparison() {
    debugLog("比較演算を解析中...");
    auto left = parseExpression(); // 左辺の式

    if (pos < tokens.size() && (
        tokens[pos].type == TokenType::EqualTo ||
        tokens[pos].type == TokenType::NotEqualTo ||
        tokens[pos].type == TokenType::LAngleBracket ||
        tokens[pos].type == TokenType::RAngleBracket ||
        tokens[pos].type == TokenType::LessThanOrEqual ||
        tokens[pos].type == TokenType::GreaterThanOrEqual)) {
        
        // 比較ノードを作成
        auto node = std::make_shared<ASTNode>(NodeType::Comparison, tokens[pos].value);
        node->children.push_back(std::move(left));
        
        advance(); //　比較演算子
        
        node->children.push_back(parseExpression()); // 右辺
        
        return node;
    }

    return left;
}

// 条件式の解析
std::shared_ptr<ASTNode> Parser::parseCondition() {
    debugLog("条件式を解析中...");
    
    // NOTの処理
    if (tokens[pos].type == TokenType::Not) {
        auto node = std::make_shared<ASTNode>(NodeType::LogicalExpression, "!");
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
        auto node = std::make_shared<ASTNode>(NodeType::LogicalExpression, op);
        node->children.push_back(std::move(left));
        node->children.push_back(std::move(right));
        left = std::move(node);
    }
    
    return left;
}

// 条件分岐の解析
std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    debugLog("条件分岐を解析中...");
    auto node = std::make_shared<ASTNode>(NodeType::IfStatement);
    advance(); // "if"

    if (tokens[pos].type == TokenType::LParen) {
        advance(); // "("
        node->children.push_back(parseCondition()); // 条件式

        if (tokens[pos].type != TokenType::RParen) {
            return recoverFromError("Expected ')' after condition in if statement");
        } 
        advance(); // ")"
    } 
    else {
        return recoverFromError("Expected '(' after 'if'");
    }

    // thenの解析
    if (tokens[pos].type == TokenType::LBrace) {
        advance(); // "{"
        auto thenNode = std::make_shared<ASTNode>(NodeType::Statement);
        while (pos < tokens.size() && tokens[pos].type != TokenType::RBrace) {
            thenNode->children.push_back(parseStatement());
        }

        if (tokens[pos].type != TokenType::RBrace) {
            return recoverFromError("Expected '}' after then block in if statement");
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
                auto elseNode = std::make_shared<ASTNode>(NodeType::Statement);
                while (pos < tokens.size() && tokens[pos].type != TokenType::RBrace) {
                    elseNode->children.push_back(parseStatement());
                }

                if (tokens[pos].type != TokenType::RBrace) {
                    return recoverFromError("Expected '}' after else block in if statement");
                }
                advance(); // "}"
                
                node->children.push_back(std::move(elseNode)); // elseを追加
            } 
            else {
                return recoverFromError("Expected '{' or 'if' after 'else' in if statement");
            }
        }
    }
    else {
        return recoverFromError("Expected '{' after condition in if statement");
    }

    return node;
}

// ループの解析
std::shared_ptr<ASTNode> Parser::parseLoopStatement() {
    debugLog("ループを解析中...");
    auto node = std::make_shared<ASTNode>(NodeType::LoopStatement);
    advance(); // "&"をスキップ

    // ループ条件の解析
    if (tokens[pos].type == TokenType::LParen) {
        advance(); // "("
        node->children.push_back(parseCondition()); // 条件式

        if (tokens[pos].type != TokenType::RParen) {
            return recoverFromError("Expected ')' after condition in loop statement");
        } 
        advance(); // ")"
    } 
    else {
        return recoverFromError("Expected '(' after 'loop'");
    }
    
    // ループブロックの解析
    if (tokens[pos].type == TokenType::LBrace) {
        advance(); // "{"
        auto loopNode = std::make_shared<ASTNode>(NodeType::Statement);
        while (pos < tokens.size() && tokens[pos].type != TokenType::RBrace) {
            loopNode->children.push_back(parseStatement());
        }

        if (tokens[pos].type != TokenType::RBrace) {
            return recoverFromError("Expected '}' after loop block");
        }
        advance(); // "}"
        
        node->children.push_back(std::move(loopNode)); // ループ本体を追加
    } 
    else {
        return recoverFromError("Expected '{' after loop condition");
    }

    return node;
}

// 出力文の解析
std::shared_ptr<ASTNode> Parser::parseOutputStatement() {
    debugLog("出力文を解析中...");
    auto node = std::make_shared<ASTNode>(NodeType::OutputStatement);
    advance(); // "<" をスキップ
    
    // 出力する式を解析
    auto expr = parseExpression();
    if (!expr) {
        return recoverFromError("Expected expression in output statement");
    }
    node->children.push_back(std::move(expr));
    
    if (tokens[pos].type != TokenType::Semicolon) {
        return recoverFromError("Expected ';' after output statement");
    }
    advance(); // セミコロンをスキップ

    return node;
}

// 入力文の解析
std::shared_ptr<ASTNode> Parser::parseInputStatement() {
    debugLog("入力文を解析中...");
    auto node = std::make_shared<ASTNode>(NodeType::InputStatement);
    advance(); // ">" をスキップ
    
    // 入力するメモリ参照を解析
    if (tokens[pos].type == TokenType::MemoryRef) {
        auto ref = parseMemoryRef();
        if (!ref) {
            return recoverFromError("Expected memory reference in input statement");
        }
        node->children.push_back(std::move(ref));
    } 
    else {
        return recoverFromError("Expected memory reference in input statement");
    }
    
    if (tokens[pos].type != TokenType::Semicolon) {
        return recoverFromError("Expected ';' after input statement");
    }
    advance(); // セミコロンをスキップ

    return node;
}

// ファイル出力文の解析
std::shared_ptr<ASTNode> Parser::parseFileOutputStatement() {
    debugLog("ファイル出力文を解析中...");
    auto node = std::make_shared<ASTNode>(NodeType::FileOutputStatement);

    // ファイル名の解析（文字列かメモリ参照）
    if (tokens[pos].type == TokenType::String) {
        auto fileNode = std::make_shared<ASTNode>(NodeType::String, tokens[pos].value);
        advance(); // 文字列をスキップ
        node->children.push_back(std::move(fileNode));
    } 
    else if (tokens[pos].type == TokenType::MemoryRef) {
        auto fileNode = parseMemoryRef();
        if (!fileNode) {
            return recoverFromError("Expected memory reference for file name");
        }
        node->children.push_back(std::move(fileNode));
    } 
    else {
        return recoverFromError("Expected string or memory reference for file name");
    }
    
    // "<<"" をチェック
    if (tokens[pos].type != TokenType::DoubleLAngleBracket) {
        return recoverFromError("Expected '<<' after file name in file output statement");
    }
    advance(); // "<<"
    
    // 出力する式を解析
    if (tokens[pos].type == TokenType::MemoryRef) {
        auto ref = parseMemoryRef();
        if (!ref) {
            return recoverFromError("Expected memory reference in file output statement");
        }
        node->children.push_back(std::move(ref));
    } 
    else if (tokens[pos].type == TokenType::MemoryMapRef) {
        auto ref = parseMemoryMapRef();
        if (!ref) {
            return recoverFromError("Expected memory map reference in file output statement");
        }
        node->children.push_back(std::move(ref));
    } 
    else if (tokens[pos].type == TokenType::String) {
        auto strNode = std::make_shared<ASTNode>(NodeType::String, tokens[pos].value);
        advance(); // 文字列をスキップ
        node->children.push_back(std::move(strNode));
    } 
    else {
        auto expr = parseExpression();
        if (!expr) {
            return recoverFromError("Expected expression in file output statement");
        }
        node->children.push_back(std::move(expr));
    }
    
    if (tokens[pos].type != TokenType::Semicolon) {
        return recoverFromError("Expected ';' after file output statement");
    }
    advance(); // セミコロンをスキップ

    return node;
}

// ファイル入力文の解析
std::shared_ptr<ASTNode> Parser::parseFileInputStatement() {
    debugLog("ファイル入力文を解析中...");
    auto node = std::make_shared<ASTNode>(NodeType::FileInputStatement);

    // ファイル名の解析（文字列かメモリ参照）
    if (tokens[pos].type == TokenType::String) {
        auto fileNode = std::make_shared<ASTNode>(NodeType::String, tokens[pos].value);
        advance(); // 文字列をスキップ
        node->children.push_back(std::move(fileNode));
    } 
    else if (tokens[pos].type == TokenType::MemoryRef) {
        auto fileNode = parseMemoryRef();
        if (!fileNode) {
            return recoverFromError("Expected memory reference for file name");
        }
        node->children.push_back(std::move(fileNode));
    } 
    else {
        return recoverFromError("Expected string or memory reference for file name");
    }
    
    // ">>" をチェック
    if (tokens[pos].type != TokenType::DoubleRAngleBracket) {
        return recoverFromError("Expected '>>' after file name in file input statement");
    }
    advance(); // ">>"
    
    // 入力するメモリ参照を解析
    if (tokens[pos].type == TokenType::MemoryRef) {
        auto ref = parseMemoryRef();
        if (!ref) {
            return recoverFromError("Expected memory reference in file input statement");
        }
        node->children.push_back(std::move(ref));
    } 
    else if (tokens[pos].type == TokenType::MemoryMapRef) {
        auto ref = parseMemoryMapRef();
        if (!ref) {
            return recoverFromError("Expected memory map reference in file input statement");
        }
        node->children.push_back(std::move(ref));
    } 
    else {
        return recoverFromError("Expected memory reference or memory map reference in file input statement");
    }
    
    if (tokens[pos].type != TokenType::Semicolon) {
        return recoverFromError("Expected ';' after file input statement");
    }
    advance(); // セミコロンをスキップ

    return node;
}

// 関数の解析
std::shared_ptr<ASTNode> Parser::parseFunction() {
    debugLog("関数を解析中...");
    // 関数番号の取得
    std::string tokenValue = tokens[pos].value;
    std::string functionNumber;
    
    if (tokenValue.size() >= 3 && tokenValue.substr(0, 1) == "_") {
        functionNumber = tokenValue.substr(1);
    } 
    else {
        return recoverFromError("Invalid function call format. Expected $_XXX where X is a digit.");
    }
    
    // 関数番号の検証
    if (functionNumber.size() != 3 || 
        !std::isdigit(functionNumber[0]) || 
        !std::isdigit(functionNumber[1]) || 
        !std::isdigit(functionNumber[2])) {
        return recoverFromError("Invalid function number format. Expected $_XXX where X is a digit.");
    }
    
    auto node = std::make_shared<ASTNode>(NodeType::Function, functionNumber);
    advance(); // 関数定義トークンをスキップ

    if (tokens[pos].type != TokenType::LBrace) {
        return recoverFromError("Expected '{' after function definition");
    }
    advance(); // "{"
    
    while (pos < tokens.size() && tokens[pos].type != TokenType::RBrace) {
        node->children.push_back(parseStatement());
    }

    if (tokens[pos].type != TokenType::RBrace) {
        return recoverFromError("Expected '}' after function body");
    }
    advance(); // "}"

    return node;
}

// 関数呼び出しの解析
std::shared_ptr<ASTNode> Parser::parseFunctionCall() {
    debugLog("関数呼び出しを解析中...");
    
    // 関数番号の取得
    std::string tokenValue = tokens[pos].value;
    std::string functionNumber;
    
    if (tokenValue.size() >= 3 && tokenValue.substr(0, 2) == "$_") {
        functionNumber = tokenValue.substr(2);
    } 
    else {
        return recoverFromError("Invalid function call format. Expected $_XXX where X is a digit.");
    }
    
    // 関数番号の検証
    if (functionNumber.size() != 3 || 
        !std::isdigit(functionNumber[0]) || 
        !std::isdigit(functionNumber[1]) || 
        !std::isdigit(functionNumber[2])) {
        return recoverFromError("Invalid function number format. Expected $_XXX where X is a digit.");
    }
    
    auto node = std::make_shared<ASTNode>(NodeType::FunctionCall, functionNumber);
    advance(); // 関数呼び出しトークンをスキップ
    
    // セミコロンチェック
    if (tokens[pos].type != TokenType::Semicolon) {
        return recoverFromError("Expected ';' after function call");
    }
    advance(); // セミコロンをスキップ
    
    return node;
}

// 型変換の解析
std::shared_ptr<ASTNode> Parser::parseCast() {
    debugLog("型変換を解析中...");
    
    // キャスト種類を保存
    std::string castType;
    if (tokens[pos].type == TokenType::IntCast) castType = "int";
    else if (tokens[pos].type == TokenType::FloatCast) castType = "float";
    else if (tokens[pos].type == TokenType::StrCast) castType = "string";
    else if (tokens[pos].type == TokenType::BoolCast) castType = "bool";
    else {
        return recoverFromError("Expected cast type (int, float, string, bool)");
    }
    
    auto node = std::make_shared<ASTNode>(NodeType::Cast, castType);
    advance(); // キャストトークンをスキップ
    
    // キャストする対象の式
    auto expr = parseExpression();
    if (!expr) {
        return recoverFromError("Expected expression for cast");
    }
    node->children.push_back(std::move(expr));
    
    // セミコロンチェック
    if (tokens[pos].type != TokenType::Semicolon) {
        return recoverFromError("Expected ';' after cast expression");
    }
    advance(); // セミコロンをスキップ

    return node;
}

std::shared_ptr<ASTNode> Parser::parseStackOperation() {
    debugLog("スタック操作を解析中...");

    std::string operation;
    if (tokens[pos].type == TokenType::IntegerStackPush) operation = "IntegerStackPush";
    else if (tokens[pos].type == TokenType::IntegerStackPop) operation = "IntegerStackPop";
    else if (tokens[pos].type == TokenType::FloatStackPush) operation = "FloatStackPush";
    else if (tokens[pos].type == TokenType::FloatStackPop) operation = "FloatStackPop";
    else if (tokens[pos].type == TokenType::StringStackPush) operation = "StringStackPush";
    else if (tokens[pos].type == TokenType::StringStackPop) operation = "StringStackPop";
    else if (tokens[pos].type == TokenType::BooleanStackPush) operation = "BooleanStackPush";
    else if (tokens[pos].type == TokenType::BooleanStackPop) operation = "BooleanStackPop";
    else {
        return recoverFromError("Expected stack operation");
    }

    advance(); // スタック操作トークンをスキップ

    auto node = std::make_shared<ASTNode>(NodeType::StackOperation, operation);
    node->children.push_back(parseExpression());

    // セミコロンチェック
    if (tokens[pos].type != TokenType::Semicolon) {
        return recoverFromError("Expected ';' after stack operation");
    }
    advance(); // セミコロンをスキップ

    return node;
}

// メモリマップウィンドウスライドの解析
std::shared_ptr<ASTNode> Parser::parseMapWindowSlide() {
    debugLog("メモリマップウィンドウスライドステートメントを解析中...");

    // スライド量を解析
    auto slideAmount = parseExpression();
    if (!slideAmount) {
        return recoverFromError("Expected slide amount expression");
    }
    
    // スライド演算子をチェック
    if (tokens[pos].type != TokenType::MapWindowSlide) {
        return recoverFromError("Expected '+>' slide operator");
    }
    advance();
    
    // メモリマップ参照を解析
    auto mapRef = parseMemoryMapRef();
    if (!mapRef) {
        return recoverFromError("Expected memory map reference in slide statement");
    }
    
    auto node = std::make_shared<ASTNode>(NodeType::MapWindowSlide, "+>");
    node->children.push_back(std::move(slideAmount));
    node->children.push_back(std::move(mapRef));
    
    // セミコロンチェック
    if (tokens[pos].type != TokenType::Semicolon) {
        return recoverFromError("Expected ';' after map slide statement");
    }
    advance(); // セミコロンをスキップ
    
    return node;
}

std::shared_ptr<ASTNode> Parser::recoverFromError(const std::string& message) {
    reportError(message);
    synchronize(); // 次のポイントまでスキップ

    return std::make_shared<ASTNode>(NodeType::Error, message); // エラーを示すノードを返す
}

// エラー回復用
void Parser::synchronize() {
    // ファイル終端
    if (pos >= tokens.size()) return;
    
    advance(); // エラーのあるトークンをスキップ
    
    // 安全なポイントに達するまでスキップ
    while (pos < tokens.size()) {
        // セミコロンを見つけたらそこで同期完了
        if (tokens[pos-1].type == TokenType::Semicolon) {
            return;
        }
        
        // ブロックの終わりも同期ポイント
        if (tokens[pos].type == TokenType::RBrace) {
            return;
        }
        switch (tokens[pos].type) {
            case TokenType::Function:
            case TokenType::If:
            case TokenType::Else:
            case TokenType::Loop:
            case TokenType::LAngleBracket:
            case TokenType::RAngleBracket:
                return;
            default:
                advance();
        }
    }
}
