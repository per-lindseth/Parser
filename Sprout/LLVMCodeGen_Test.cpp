#include "LLVMCodeGen_Test.h"

#include <string>
//#include "Lexer.h"
//#include "Parser.h"
//#include "Compiler.h"
//#include "LLVMCodeGen.h"

// --- Demo main: parse, codegen, print LLVM IR ---
void LLVMCodeGen_Test() {
    std::string code = "let x = 10; let y = x * 2 + 3; print y;";
    //Lexer lexer(code);
    //Parser parser(lexer);
    //auto stmts = parser.parseProgram();

    //LLVMCodeGen cg;
    //cg.generate(stmts);
}