// SigNum Parser
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "lexer.hpp"

// ASTノード
struct ASTNode {
    std::string value;
    std::vector<ASTNode> children;
};

