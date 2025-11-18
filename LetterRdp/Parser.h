#pragma once

#include <string>
#include <format>
/**
 * Letter parser: recursive decent implementation.   
 */

class Parser
{
    std::string m_ProgramText;

    Parser(std::string programText)
        : m_ProgramText(programText)
    { }

public:
    /**
     * Parses a string into an AST
     */
    static std::string parse(std::string programText)
    {
        Parser parser(programText);
        return parser.Program();
    }

private:
    /**
     * NumericLiteral
     * : NUMBER
     * ;
     */
    std::string ParseNumericLiteral()
    {
        //return std::format("\{\\n type: 'NumericLiteral',\\n value: Number({}),\\n\}",
        return std::format("{{\n type: 'NumericLiteral',\n value: Number({})\n}}",
            m_ProgramText);
    }

    /**
     * Main entry point
     *
     * Program
     *   : NumericLiteral
     *   ;
     */
    std::string Program()
    {
        return ParseNumericLiteral();
    }
};

