#include "Lexer_Test.h"

#include "format"
#include <iostream>
#include <string>
#include <vector>
#include <cctype>

#include "Lexer.h"

using namespace std;

void Lexer_Test() 
{
    std::string code = "let x = 10; print x;";
    Lexer lexer(code);

    Token t;
    while ((t = lexer.next()).type != TokenType::EndOfFile)
    {
        cout << format("{} : {}\n", static_cast<int>(t.type), t.text);
    }
}
