//
// Created by Ethan on 9/27/2025.
//

#ifndef CHARACTER_H
#define CHARACTER_H

#include "Weapon.h"

class character {
public:
    explicit character(const std::string& jsonPath);
    explicit character(std::vector<Weapon> weapons, std::vector<int> ashes, std::vector<std::string> armor, std::vector<int> spells,
std::string startingClass, int upgradeLevel, double effectiveHealth, int level, int vigor, int endurance, int mind,
std::vector<int> damageStats, bool hasBullgoat, bool hasGreatjar, double poise, std::string name);

private:

    std::vector<Weapon> weapons;
    std::vector<int> ashes;
    std::vector<std::string> armor;
    std::vector<int> spells;
    std::string startingClass;
    int upgradeLevel;
    double effectiveHealth;
    int level;
    int vigor;
    int endurance;
    int mind;
    std::vector<int> damageStats;
    bool hasBullgoat;
    bool hasGreatjar;
    double poise;
    std::string name;
};



#endif //CHARACTER_H
