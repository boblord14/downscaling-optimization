//
// Created by Ethan on 8/29/2025.
//

#ifndef LOADCHARACTER_H
#define LOADCHARACTER_H

#include "Weapon.h"

extern std::unordered_map<std::string, std::vector<int>> starting_classes;

static std::unordered_map<std::string, std::vector<double>> starting_classes_negations;
static std::unordered_map<std::string, std::vector<double>> starting_classes_negations_greatjar;

static std::vector<std::vector<float>> logisticArmorPieces = {};

constexpr int STARTING_CLASS_STAT_POINTS = 79; //number of stat points to rescale the classes to
constexpr int SCALING_LEVEL_TARGET = 90;
constexpr int NUM_ARMOR_PIECES = 4; //how many armor pieces the player can equip at once
constexpr double GREATJAR_MULTIPLIER = 1.19; //multiplier to equip load from great jar talisman
constexpr int DAGGER_POISE_THRESHOLD = 58; //the amount of poise required to not get easily staggered by daggers

constexpr int CLASS_DAMAGE_STAT_INDEX = 3; //index position where the damage stats start in the starting classes list
constexpr int CLASS_MIND_STAT_INDEX = 1; //index position where the mind stat is in the starting classes list

class loadCharacter {
public:
    static void loadData();
    static double retrieveMaxPoise();
    static double bestEffectiveHP(int statPoints, const std::string& startingClass, boolean hasGreatjar);
    static int retrieveMaxFp(int total_stats, const std::string& starting_class);
    static int getMaxFpSpell();
    static int getMaxFPAshOfWar();
};

#endif //LOADCHARACTER_H
