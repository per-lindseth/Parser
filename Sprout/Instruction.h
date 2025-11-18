#pragma once

#include "OpCode.h"

struct Instruction {
    OpCode op;
    int operand; // used for const value or variable index
};