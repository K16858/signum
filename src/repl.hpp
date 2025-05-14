#pragma once
#include "interpreter/interpreter.hpp"
#include "semantic/semantic.hpp"

class REPL {
private:
    Interpreter interpreter;
    SemanticAnalyzer analyzer;
    bool running = true;
    
    void processCommand(const std::string& command);
    void printHelp();
    void executeCode(const std::string& code);

public:
    REPL() = default;
    void start();
    void stop();
};
