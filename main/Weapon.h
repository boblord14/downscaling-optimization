//
// Created by false on 6/7/2025.
//

#ifndef DOWNSCALING_OPTIMIZATION_WEAPON_H
#define DOWNSCALING_OPTIMIZATION_WEAPON_H
#include <unordered_map>
#include <iostream>
#include <vector>
#include <cmath>
#include "csv.hpp"


using namespace csv;

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
  static void generateDefs();
  int getId(){return id;}
  Infusion getInfusion(){return infusion;}
  int getUpgrade(){return upgrade;}
private:
    int id;
    Infusion infusion;
    int upgrade;
    static void defGenerator(const std::string& csvPath, const std::string& key, int target);
    static std::vector<std::string> GetAttackElementCorrect(int attackElementCorrectId, const std::string& element);
};

static std::unordered_map<int, std::unordered_map<std::string, std::string>> weaponData = {};
static std::unordered_map<int, std::unordered_map<std::string, std::string>> reinforceParamWeapon = {};
static std::unordered_map<int, std::unordered_map<std::string, std::string>> attackElementCorrectParam = {};
static std::unordered_map<int, std::vector<float>> calcCorrectGraph = {};

#endif //DOWNSCALING_OPTIMIZATION_WEAPON_H
