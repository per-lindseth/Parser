#pragma once

#include <cctype>
#include <iostream>
#include <string>

#include "Token.h"

class Lexer 
{
public:
    explicit Lexer(std::string src) : src(std::move(src)), pos(0) {}

    Token next() {
        skipWhitespace();
        if (pos >= src.size()) return make(TokenType::EndOfFile, "");

        const char c = src[pos];

        // Identifiers or keywords
        if (std::isalpha(c)) {
            const size_t start = pos;
            while (pos < src.size() && std::isalnum(src[pos])) pos++;
            const std::string text = src.substr(start, pos - start);
            if (text == "let") return make(TokenType::Let, text);
            if (text == "print") return make(TokenType::Print, text);
            return make(TokenType::Identifier, text);
        }

        // Numbers
        if (std::isdigit(c)) {
            const size_t start = pos;
            while (pos < src.size() && std::isdigit(src[pos])) pos++;
            return make(TokenType::Number, src.substr(start, pos - start));
        }

        // Single‑char tokens
        pos++;
        switch (c) {
        case '=': return make(TokenType::Equals, "=");
        case '+': return make(TokenType::Plus, "+");
        case '-': return make(TokenType::Minus, "-");
        case '*': return make(TokenType::Star, "*");
        case '/': return make(TokenType::Slash, "/");
        case ';': return make(TokenType::Semicolon, ";");
        case '(': return make(TokenType::LParen, "(");
        case ')': return make(TokenType::RParen, ")");
        default: ;
        }

        return make(TokenType::Unknown, std::string(1, c));
    }

private:
    std::string src;
    size_t pos;

    void skipWhitespace() {
        while (pos < src.size() && std::isspace(src[pos])) pos++;
    }

    static Token make(const TokenType type, const std::string& text) {
        return Token{ type, text };
    }
};
