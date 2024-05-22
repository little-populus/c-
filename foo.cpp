#include <template.hpp>

template <> void foo<int>(int)
{
    std::cout << "foo<int>\n";
    b b1;
    b b2;
    std::cout << b1 + b2 << '\n';
    std::cout << "return from foo<int>\n";
}