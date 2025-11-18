#pragma once

#include <exception>
#include <type_traits> 
#include <string>

// Required for std::is_arithmetic_v

template <typename T> 
concept Arithmetic = std::is_arithmetic_v<T>;

/**
 * Eva interpreter.
 */
namespace Eva2
{

    template <typename D, typename C>
    C eval(D)
    {
        throw std::exception("Not implementet");
    }

    template <Arithmetic Exp>
    Exp eval(Exp exp)
    {
       return exp;
    }

    std::string eval(std::string exp)
    {
        if (exp.size() >= 2 && exp[0] == '"' && exp[exp.size() - 1] == '"')
        {
            return exp.substr(1, exp.size() - 2);
        }
        return exp;
    }

    template <typename Left, typename Right>
    Left eval(std::tuple<char, Left, Right> node)
    {
        const auto [opr, left, right] = node;

        switch (opr)
        {
        case '+':
            return left + right;
            break;
        }

        throw std::exception("Not implementet");
    }

};