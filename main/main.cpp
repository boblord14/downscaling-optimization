//
// Created by false on 6/7/2025.
//

#include <iostream>
#include "Weapon.h"


int main() {
    Weapon::generateDefs();
    Weapon weapon(67520000, BASE, 2);
    double calculatedAR = weapon.calcAR(32, 67, 40, 99, 10, true);
    std::cout << "Weapon AR test: " << calculatedAR << std::endl;

    return 0;
}
