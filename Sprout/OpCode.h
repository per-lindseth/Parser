#pragma once

#include <cstdint>

// --- Bytecode Compiler + VM ---

enum OpCode : uint8_t {
    OP_CONST_INT,
    OP_LOAD_VAR,
    OP_STORE_VAR,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_PRINT,
    OP_POP,
    OP_HALT
};