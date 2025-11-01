#include "repl.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "version.hpp"
#include <iostream>
#include <string>

void REPL::start() {
    std::cout << "Welcome to the " << SigNum::getVersionString() << " REPL!" << std::endl;
    std::cout << "Type '.help' for a list of commands." << std::endl;

    running = true;
    
    while (running) {
        std::string inputBuffer;
        int bracketCount = 0;  // 複数行実行のための括弧のカウント
        bool multilineInput = false;

        std::cout << "\n>> ";

        std::string line;
        while (std::getline(std::cin, line)) {
            if (inputBuffer.empty()) {
                if (line == ".exit" || line == ".quit") {
                    running = false;
                    std::cout << "Exiting REPL..." << std::endl;
                    std::cout << "Bye!" << std::endl;
                    break;
                }
                else if (line == ".help") {
                    printHelp();
                    break;
                }
                else if (line.empty()) {
                    break;
                }
            }

            inputBuffer += line + "\n";

            for (char c : line) {
                if (c == '{') {
                    bracketCount++;
                    multilineInput = true;
                }
                else if (c == '}') {
                    bracketCount--;
                }
            }

            bool hasTerminator = !line.empty() && line.back() == ';';

            if ((multilineInput && bracketCount > 0) ||
                (!multilineInput && !hasTerminator && !line.empty())) {
                // コードが続く場合
                std::cout << ">>> ";
            } 
            else {
                if (!inputBuffer.empty()) {
                    executeCode(inputBuffer);
                }
                break;
            }
        }

        if (!running) {
            break;
        }
    }
}

void REPL::processCommand(const std::string& command) {
    if (command == ".exit" || command == ".quit") {
        running = false;
        std::cout << "Exiting REPL..." << std::endl;
        std::cout << "Bye!" << std::endl;
    } 
    else if (command == ".help") {
        printHelp();
    } 
    else {
        std::cout << "Unknown command: " << command << std::endl;
        printHelp();
    }
}

void REPL::printHelp() {
    std::cout << "Available commands:" << std::endl;
    std::cout << ".help       Show this help message" << std::endl;
    std::cout << ".exit       Exit the REPL" << std::endl;
    std::cout << ".quit       Exit the REPL" << std::endl;
}

void REPL::executeCode(const std::string& code) {
    try {
        Lexer lexer(code);
        auto tokens = lexer.tokenize();

        if (lexer.hasErrors()) {
            std::cerr << "Lexical Analysis Failed!" << std::endl;
            lexer.printErrors();
        }

        Parser parser(tokens);
        auto ast = parser.parseProgram();

        if (ast) {
            if (analyzer.analyze(ast)) {
                interpreter.interpret(ast);
            }
            else {
                std::cout << "Semantic analysis failed!" << std::endl;
            }
        }
        else {
            std::cerr << "Parsing Failed!" << std::endl;
            if (parser.hasErrors()) {
                parser.printErrors();
            }
        }
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void REPL::stop() {
    running = false;
    std::cout << "Stopping REPL..." << std::endl;
}
