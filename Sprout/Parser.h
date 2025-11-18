#pragma once

#include <vector>

#include "AST_Nodes.h"
#include "Lexer.h"

// --- Parser Implementation ---

class Parser {
public:
    Parser(Lexer& lexer) : lexer(lexer) { advance(); }

    std::vector<Stmt*> parseProgram() {
        std::vector<Stmt*> stmts;
        while (current.type != TokenType::EndOfFile) {
            stmts.push_back(parseStatement());
        }
        return stmts;
    }

private:
    Lexer& lexer;
    Token current;

    void advance() { current = lexer.next(); }
    void expect(TokenType t) {
        if (current.type != t) throw std::runtime_error("syntax error");
        advance();
    }

    Stmt* parseStatement() {
        if (current.type == TokenType::Let) return parseLet();
        if (current.type == TokenType::Print) return parsePrint();
        Expr* e = parseExpr();
        expect(TokenType::Semicolon);
        return new ExprStmt(e);
    }

    Stmt* parseLet() {
        expect(TokenType::Let);
        std::string name = current.text;
        expect(TokenType::Identifier);
        expect(TokenType::Equals);
        Expr* e = parseExpr();
        expect(TokenType::Semicolon);
        return new LetStmt(name, e);
    }

    Stmt* parsePrint() {
        expect(TokenType::Print);
        Expr* e = parseExpr();
        expect(TokenType::Semicolon);
        return new PrintStmt(e);
    }

    Expr* parseExpr() {
        Expr* left = parseTerm();
        while (current.type == TokenType::Plus || current.type == TokenType::Minus) {
            char op = current.text[0];
            advance();
            Expr* right = parseTerm();
            left = new BinaryExpr(op, left, right);
        }
        return left;
    }

    Expr* parseTerm() {
        Expr* left = parseFactor();
        while (current.type == TokenType::Star || current.type == TokenType::Slash) {
            char op = current.text[0];
            advance();
            Expr* right = parseFactor();
            left = new BinaryExpr(op, left, right);
        }
        return left;
    }

    Expr* parseFactor() {
        if (current.type == TokenType::Number) {
            int v = std::stoi(current.text);
            advance();
            return new NumberExpr(v);
        }
        if (current.type == TokenType::Identifier) {
            std::string name = current.text;
            advance();
            return new VariableExpr(name);
        }
        if (current.type == TokenType::LParen) {
            advance();
            Expr* e = parseExpr();
            expect(TokenType::RParen);
            return e;
        }
        throw std::runtime_error("syntax error in factor");
    }
};
