// SigNum Lexer
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "token.hpp"

// メモリ参照を解析
std::string parseMemoryRef(const std::string& src, size_t& pos);

// 字句解析関数
std::vector<Token> tokenize(const std::string& src);

// トークン表示関数
void printTokens(const std::vector<Token>& tokens);
