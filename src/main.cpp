#include <print>
#include <iostream>

int main() {
    int* p = nullptr;
    if (p && *p == 5) { std::cout << "yes"; }

    return 0;
}
