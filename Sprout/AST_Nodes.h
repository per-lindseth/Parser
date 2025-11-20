#pragma once

#include <format>
#include <iostream>
#include <string>

// --- AST Nodes ---

struct Expr {
    virtual ~Expr() {}
    virtual void print(std::ostream& os) = 0;
};

struct NumberExpr : Expr {
    int value;
    NumberExpr(int v) : value(v) {}
    void print(std::ostream& os) override
    {
        os << std::format("NumberExpr(value: {})", value);
    }
};

struct VariableExpr : Expr {
    std::string name;
    VariableExpr(const std::string& n) : name(n) {}
    void print(std::ostream& os) override
    {
        os << std::format("VariableExpr(name: {})", name);
    }
};

struct BinaryExpr : Expr {
    ~BinaryExpr() { delete left; left = nullptr; delete right; right = nullptr; }
    char op;
    Expr* left;
    Expr* right;
    BinaryExpr(char op, Expr* l, Expr* r) : op(op), left(l), right(r) {}
    void print(std::ostream& os) override
    {
        os << "BinaryExpr(";
        os << std::format("BinaryExpr(op: {}, ", op);
        if (left) left->print(os);
        os << ", ";
        if (right) right->print(os);
        os << ")";
    }
};

// --- Statements ---
struct Stmt {
    virtual ~Stmt() {}
    virtual void print(std::ostream& os)=0;
};

struct LetStmt : Stmt {
    ~LetStmt() { delete expr; expr = nullptr; }
    std::string name;
    Expr* expr;
    LetStmt(const std::string& n, Expr* e) : name(n), expr(e) {}
    void print(std::ostream& os) override 
    { 
        os << std::format("LetStmt(name: {}, ", name); 
        if (expr) expr->print(os);
        os << ")\n";
    }
};

struct PrintStmt : Stmt {
    ~PrintStmt() { delete expr; expr = nullptr; }
    Expr* expr;
    PrintStmt(Expr* e) : expr(e) {}
    void print(std::ostream& os) override
    {
        os << "PrintStmt(";
        if (expr) expr->print(os);
        os << ")\n";
    }
};

struct ExprStmt : Stmt {
    ~ExprStmt() { delete expr; expr = nullptr; }
    Expr* expr;
    ExprStmt(Expr* e) : expr(e) {}
    void print(std::ostream& os) override
    {
        os << "ExprStmt(";
        if (expr) expr->print(os);
        os << ")\n";
    }
};
