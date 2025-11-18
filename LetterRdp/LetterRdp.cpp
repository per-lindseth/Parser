// LetterRdp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Parser.h"

int main()
{
    Parser::parse("42");
    std::cout << Parser::parse("42") << std::endl;
}