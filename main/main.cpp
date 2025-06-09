//
// Created by false on 6/7/2025.
//

#include <iostream>
#include "Weapon.h"


int main() {
    Weapon::generateDefs();
    Weapon broadsword(2020000, FIRE, 3);
    double calculatedAR = broadsword.calcAR(10, 10, 10, 10, 10, false);
    std::cout << "Broadsword AR test: " << calculatedAR << std::endl;

    return 0;
}
