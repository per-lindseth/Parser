#pragma once

#include <string>

// --- AST Nodes ---

struct Expr {
    virtual ~Expr() {}
};

struct NumberExpr : Expr {
    int value;
    NumberExpr(int v) : value(v) {}
};

struct VariableExpr : Expr {
    std::string name;
    VariableExpr(const std::string& n) : name(n) {}
};

struct BinaryExpr : Expr {
    char op;
    Expr* left;
    Expr* right;
    BinaryExpr(char op, Expr* l, Expr* r) : op(op), left(l), right(r) {}
};

// --- Statements ---
struct Stmt {
    virtual ~Stmt() {}
};

struct LetStmt : Stmt {
    std::string name;
    Expr* expr;
    LetStmt(const std::string& n, Expr* e) : name(n), expr(e) {}
};

struct PrintStmt : Stmt {
    Expr* expr;
    PrintStmt(Expr* e) : expr(e) {}
};

struct ExprStmt : Stmt {
    Expr* expr;
    ExprStmt(Expr* e) : expr(e) {}
};
