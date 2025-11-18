#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "Instruction.h"

class VM {
public:
    VM(std::vector<Instruction> code, const std::vector<std::string>& varNames)
        : code(std::move(code)), pc(0), vars(varNames.size(), 0) {
    }

    void run() {
        while (pc < static_cast<int>(code.size())) {
            switch (const auto& [op, operand] = code[pc++]; op)
            {
            case OP_CONST_INT:
                push(operand);
                break;
            case OP_LOAD_VAR:
                push(vars.at(operand));
                break;
            case OP_STORE_VAR: {
                const int val = pop();
                if (operand >= static_cast<int>(vars.size())) vars.resize(operand + 1);
                vars[operand] = val;
            } break;
            case OP_ADD: {
                const int b = pop();
                const int a = pop();
                push(a + b);
            } break;
            case OP_SUB: {
                const int b = pop();
                const int a = pop();
                push(a - b);
            } break;
            case OP_MUL: {
                const int b = pop();
                const int a = pop();
                push(a * b);
            } break;
            case OP_DIV: {
                const int b = pop();
                const int a = pop();
                push(a / b);
            } break;
            case OP_PRINT: {
                const int v = pop();
                std::cout << v << std::endl;
            } break;
            case OP_POP: pop(); break;
            case OP_HALT: return;
            }
        }
    }

private:
    std::vector<Instruction> code;
    int pc;
    std::vector<int> stack;
    std::vector<int> vars;

    int pop() 
    {
        if (stack.empty()) throw std::runtime_error("stack underflow");
        const int v = stack.back();
        stack.pop_back();
        return v;
    }
    void push(const int var)
    {
        stack.push_back(var);
        
    }
};

