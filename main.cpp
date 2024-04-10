#include <iostream>
struct base
{
    int a;
    int b;
};
struct d : base
{
};
int main()
{
    d obj;
    obj.a = 10;
    obj.b = 20;
    auto [x, y] = obj;
    return 0;
}
