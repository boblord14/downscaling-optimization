//
// Created by Ethan on 9/27/2025.
//

#include "character.h"

character::character(std::vector<Weapon> weapons, std::vector<int> ashes, std::vector<std::string> armor,
    std::vector<int> spells, std::string startingClass, int upgradeLevel, double effectiveHealth, int level, int vigor,
    int endurance, int mind, std::vector<int> damageStats, boolean hasBullgoat, boolean hasGreatjar, double poise)
{
    this->weapons = weapons;
    this->ashes = ashes;
    this->armor = armor;
    this->spells = spells;
    this->startingClass = startingClass;
    this->upgradeLevel = upgradeLevel;
    this->effectiveHealth = effectiveHealth;
    this->level = level;
    this->vigor = vigor;
    this->endurance = endurance;
    this->mind = mind;
    this->damageStats = damageStats;
    this->hasBullgoat = hasBullgoat;
    this->hasGreatjar = hasGreatjar;
    this->poise = poise;
}
