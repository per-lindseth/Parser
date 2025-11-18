#pragma once

#include <exception>

/**
 * Eva interpreter.
 */
namespace Eva
{
    template <typename Opr, typename Left, typename Right>
    struct Node
    {
        Opr opr;
        Left left;
        Right right;
    };

    template <typename Exp>
    Exp eval(Exp exp)
    {
        if constexpr (std::is_arithmetic_v<Exp>)
        {
            return exp;
        }

        throw std::exception("Not implementet");
    }
    std::string eval(std::string exp)
    {
        if (exp.size() >= 2 && exp[0] == '"' && exp[exp.size() - 1] == '"')
        {
            return exp.substr(1, exp.size() - 2);
        }
        return exp;
    }

    template <typename Opr, typename Left, typename Right>
    void eval(Node<Opr, Left, Right> node)
    {
        throw std::exception("Not implementet");
    }

    template <typename Left, typename Right>
    Left eval(Node<char, Left, Right> node)
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
    //template <typename Exp>
    //static void save(Exp exp)
    //{
    //    if constexpr (std::is_arithmetic_v<T>) {

    //        if constexpr (std::is_integral_v<T>) {
    //            if constexpr (std::is_signed_v<T>) {
    //                std::cout << "Signed integral\n";
    //            }
    //            else {
    //                std::cout << "Unsigned integral\n";
    //            }
    //        }

    //        else if constexpr (std::is_floating_point_v<T>) {
    //            std::cout << "Floating point\n";
    //        }

    //    }
    //    else {
    //        std::cout << "Not a number\n";
    //    }


    //    throw std::expression("Not implementet");
    //}

};