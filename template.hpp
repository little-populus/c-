#include <iostream>
template <typename T> void foo(T)
{
    std::cout << "normal realization of foo<T>\n";
}
struct b
{
    int operator+(b)
    {
        return 2;
    }
};