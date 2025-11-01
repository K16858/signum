#include <iostream>
#include <string>
#include <fstream>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic/semantic.hpp"
#include "interpreter/interpreter.hpp"
#include "repl.hpp"
#include "version.hpp"

struct DebugConfig {
    bool debugMode = false;
};

void showhelp() {
    std::cout << "Usage: signum [options] [file]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help    Show this help message" << std::endl;
    std::cout << "  -v, --version Show version information" << std::endl;
    std::cout << "  -d, --debug   Enable debug mode" << std::endl;
}

int main(int argc, char* argv[]) {
    DebugConfig config;
    std::string filename;
    // 引数がない場合はREPLを起動
    if (argc == 1) {
        REPL repl;
        repl.start();
        return 0;
    }

    std::string arg = argv[1];

    // ヘルプメッセージを表示
    if (arg == "-h" || arg == "--help") {
        showhelp();
        return 0;
    }
    // バージョン情報を表示
    else if (arg == "-v" || arg == "--version") {
        std::cout << SigNum::getVersionString() << std::endl;
        return 0;
    }
    // デバッグモードを有効にする
    else if (arg == "-d" || arg == "--debug") {
        config.debugMode = true;
        std::cout << "Debug mode enabled." << std::endl;
        if (argc > 2) {
            filename = argv[2];
        }
        else {
            std::cerr << "Error: No file specified for debug mode." << std::endl;
            return 1;
        }
    }
    else {
        // ファイル名を取得
        filename = arg;
    }

    if (filename.empty()) {
        std::cerr << "Error: No input file specified" << std::endl;
        showhelp();
        return 1;
    }

    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos || 
        (filename.substr(dotPos) != ".sgnm" && filename.substr(dotPos) != ".sg")) {
        std::cerr << "Error: Invalid file extension. Expected .sgnm or .sg file" << std::endl;
        return 1;
    }

    // ファイルが指定されている場合
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }

    try {
        std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        Lexer lexer(code);
        auto tokens = lexer.tokenize();
        if (lexer.hasErrors()) {
            std::cerr << "Lexical Analysis Failed!" << std::endl;
            lexer.printErrors();
            return 1;
        }
        if (config.debugMode) {
            std::cout << "=== Tokens ===" << std::endl;
            printTokens(tokens);
        }
        Parser parser(tokens);
        auto ast = parser.parseProgram();

        if (ast) {
            if (config.debugMode) {
                std::cout << "\n=== AST ===" << std::endl;
                ast->print();
                std::cout << "\n=== JSON Output ===" << std::endl;
                if (ast->saveToJSONFile("ast_output.json")) {
                    std::cout << "Save : ast_output.json" << std::endl << std::endl;
                }
            }
            SemanticAnalyzer semanticAnalyzer;
            if (semanticAnalyzer.analyze(ast)) {
                Interpreter interpreter;
                interpreter.interpret(ast);
            } 
            else {
                std::cerr << "Semantic analysis failed!" << std::endl;
                return 1;
            }
        } 
        else {
            std::cerr << "Parsing Failed!" << std::endl;
            if (parser.hasErrors()) {
                parser.printErrors();
            }
            return 1;
        }
    } 
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
