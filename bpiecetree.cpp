#include "PieceTable.hpp"
#include <iostream>

int main() {
    PieceTable pt{"Hello world"};

    pt.insert(5, " brave");
    std::cout << pt.toString() << "\n";

    pt.erase(6, 6);
    std::cout << pt.toString() << "\n";

    pt.insert(6, "cruel ");
    std::cout << pt.toString() << "\n";
}