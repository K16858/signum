#include <iostream>
#include <string>
#include <fstream>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic/semantic.hpp"
#include "interpreter/interpreter.hpp"
#include "repl.hpp"

int main(int argc, char* argv[]) {
    // 引数がない場合はREPLを起動
    if (argc == 1) {
        REPL repl;
        repl.start();
        return 0;
    }

    std::string arg = argv[1];

    // ヘルプメッセージを表示
    if (arg == "-h" || arg == "--help") {
        std::cout << "Usage: signum [options] [file]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -h, --help    Show this help message" << std::endl;
        return 0;
    }

    size_t dotPos = arg.find_last_of('.');
    if (dotPos == std::string::npos || 
        (arg.substr(dotPos) != ".sgnm" && arg.substr(dotPos) != ".sg")) {
        std::cerr << "Error: Invalid file extension. Expected .sgnm or .sg file" << std::endl;
        return 1;
    }

    // ファイルが指定されている場合
    std::ifstream file(arg);
    if (!file) {
        std::cerr << "Error: Could not open file " << arg << std::endl;
        return 1;
    }

    try {
        std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        auto tokens = tokenize(code);
        Parser parser(tokens);
        auto ast = parser.parseProgram();

        if (ast) {
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
            std::cerr << "Parsing failed!" << std::endl;
            return 1;
        }
    } 
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
