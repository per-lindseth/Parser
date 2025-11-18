#pragma once

#include <cassert>
#include "Eva.h"
#include "Eva2.h"

using namespace Eva;

struct EvaTest
{
    static void Test1()
    {
        assert(Eva::eval(1) == 1);
        auto v1 = Eva::eval(std::string(R"("hello")"));
        assert(v1 == "hello");
        assert(Eva::eval(Eva::Node{ '+',3,2 }) == 5);
        std::cout << "All assertions passed!\n";
    }
    static void Test2()
    {
        assert(Eva2::eval(1) == 1);
        auto v1 = Eva2::eval(std::string(R"("hello")"));
        auto v2 = Eva2::eval({ '+',3,2 });
        ;
        //    assert(eval(Node{ '+',3,2 }), 5);
        //    std::cout << "All assertions passed!\n";
    }
};