#pragma once
#include "interpreter/interpreter.hpp"

class REPL {
private:
    Interpreter interpreter;
    bool running = true;
    
    void processCommand(const std::string& command);
    void printHelp();
    void executeCode(const std::string& code);

public:
    REPL() = default;
    void start();
    void stop();
};
