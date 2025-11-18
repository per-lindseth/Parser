#pragma once

#include <stdexcept>
#include <vector>

#include "AST_nodes.h"
#include "Instruction.h"

class Compiler {
public:
    Compiler() = default;

    std::vector<Instruction> compile(const std::vector<Stmt*>& statements) {
        byteCode.clear();
        for (Stmt* s : statements) compileStmt(s);
        byteCode.push_back({ OP_HALT, 0 });
        return byteCode;
    }

    [[nodiscard]] const std::vector<std::string>& getConstants() const { return constants; }
    [[nodiscard]] const std::vector<std::string>& getVarNames() const { return varNames; }

private:
    std::vector<Instruction> byteCode;
    std::vector<std::string> constants; // not heavily used here, but placeholder
    std::vector<std::string> varNames;

    int findVarIndex(const std::string& name) {
        for (size_t i = 0; i < varNames.size(); ++i) if (varNames[i] == name) return static_cast<int>(i);
        varNames.push_back(name);
        return (int)(varNames.size() - 1);
    }

    void compileStmt(Stmt* s) {
        if (const auto* ls = dynamic_cast<LetStmt*>(s)) {
            compileExpr(ls->expr);
            const int idx = findVarIndex(ls->name);
            byteCode.push_back({ OP_STORE_VAR, idx });
        }
        else if (const auto* ps = dynamic_cast<PrintStmt*>(s)) {
            compileExpr(ps->expr);
            byteCode.push_back({ OP_PRINT, 0 });
            byteCode.push_back({ OP_POP, 0 });
        }
        else if (const auto* es = dynamic_cast<ExprStmt*>(s)) {
            compileExpr(es->expr);
            byteCode.push_back({ OP_POP, 0 });
        }
    }

    void compileExpr(Expr* e) {
        if (auto* n = dynamic_cast<NumberExpr*>(e)) {
            byteCode.push_back({ OP_CONST_INT, n->value });
            return;
        }
        if (const auto* v = dynamic_cast<VariableExpr*>(e)) {
            const int idx = findVarIndex(v->name);
            byteCode.push_back({ OP_LOAD_VAR, idx });
            return;
        }
        if (auto* b = dynamic_cast<BinaryExpr*>(e)) {
            compileExpr(b->left);
            compileExpr(b->right);
            switch (b->op) {
            case '+': byteCode.push_back({ OP_ADD, 0 }); break;
            case '-': byteCode.push_back({ OP_SUB, 0 }); break;
            case '*': byteCode.push_back({ OP_MUL, 0 }); break;
            case '/': byteCode.push_back({ OP_DIV, 0 }); break;
            default: ;
            }
            return;
        }
        throw std::runtime_error("Unknown expression in compiler");
    }
};
