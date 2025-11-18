#include "Demo.h"

#include <format>
#include <string>
#include "Lexer.h"
#include "Parser.h"
#include "Compiler.h"
#include "VM.h"

using namespace std;

// --- Demo main: parse, compile, run ---
void Demo()
{
    const string code = "let x = 10; let y = x * 2 + 3; print y;";
    Lexer lexer(code);
    Parser parser(lexer);
    auto statements = parser.parseProgram();

    Compiler compiler;
    auto byteCode = compiler.compile(statements);

    // For demo: print byte code
    cout << "Byte code (op, operand):\n";
    for (size_t i = 0; i < byteCode.size(); ++i) {
        cout << format("{}: {}, {}\n", i, static_cast<int>(byteCode[i].op), byteCode[i].operand);
    }

    VM vm(byteCode, compiler.getVarNames());
    vm.run();
}
