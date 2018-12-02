#include <iostream>
#include <set>
int main()
{
    std::set<int> arrSet;
    arrSet.insert(1);
    arrSet.insert(2);
    arrSet.insert(3);
    for (auto iterSet : arrSet)
        std::cout << iterSet << std::endl;
    system("pause");
    return 0;
}