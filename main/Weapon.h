//
// Created by false on 6/7/2025.
//

#ifndef DOWNSCALING_OPTIMIZATION_WEAPON_H
#define DOWNSCALING_OPTIMIZATION_WEAPON_H
#include <array>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <cmath>
#include "DataParser.h"

enum Infusion {
    BASE = 0,
    HEAVY = 100,
    KEEN = 200,
    QUALITY = 300,
    FIRE = 400,
    FLAME = 500,
    LIGHTNING = 600,
    SACRED = 700,
    MAGIC = 800,
    COLD = 900,
    POISON = 1000,
    BLOOD = 1100,
    OCCULT = 1200
};


class Weapon{
public:
  explicit Weapon(int id, Infusion infusion, int upgrade);
  std::vector<double> calcAR(int strength, int dexterity, int intelligence, int faith, int arcane, bool isTwoHanded);
  double calculateDefenseReduction(double ratio);
  double calculateDamage(const std::vector<int>& stats, const std::vector<int>& defs, bool two_handed);
  int getId() const{return id;}
  Infusion getInfusion() const {return infusion;}
  int getUpgrade() const{return upgrade;}
private:
    int id;
    std::string name;
    Infusion infusion;
    int upgrade;
    std::array<int, 5> attackBaseElement;
    std::array<double, 5> elementAttackRate;
    int attackElementCorrectId;
    bool isDualBlade;
    std::array<int, 5> correctTypeElement;
    std::array<std::vector<float>, 5> saturationIndex;
    std::array<double, 5> correctStat;
    std::array<double, 5> correctStatRate;
    std::array<std::array<bool, 5>, 5> isStatCorrectByElement;
    bool isSomber;
};

#endif //DOWNSCALING_OPTIMIZATION_WEAPON_H
