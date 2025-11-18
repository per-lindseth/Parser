#pragma once

#include <string>

#include "TokenType.h"

struct Token {
    TokenType type{ TokenType::Unknown };
    std::string text;
};