//
// Created by Ethan on 9/27/2025.
//

#ifndef CHARACTER_H
#define CHARACTER_H

#include "Weapon.h"
#include "loadCharacter.h"

static std::unordered_map<std::string, std::vector<int>> starting_classes = {
    {"Hero", {14,9,12,16,9,7,9,11}},
    {"Bandit", {10,11,10,9,13,9,8,14}},
    {"Astrologer", {9, 15, 9,8,12,16,7,9}},
    {"Warrior", {11,12,11,10,16,10,8,9}},
    {"Prisoner", {11,12,11,11,14,14,6,9}},
    {"Confessor", {10,13,10,12,12,9,14,9}},
    {"Wretch", {10,10,10,10,10,10,10,10}},
    {"Vagabond", {15,10,11,14,13,9,9,7}},
    {"Prophet", {10,14,8,11,10,7,16,10}},
    {"Samurai", {12,11,13,12,15,9,8,8}}
};

static std::unordered_map<std::string, int> starting_classes_index = {
    {"Hero", 0},
    {"Bandit", 1},
    {"Astrologer", 2},
    {"Warrior", 3},
    {"Prisoner", 4},
    {"Confessor", 5},
    {"Wretch", 6},
    {"Vagabond", 7},
    {"Prophet", 8},
    {"Samurai", 9}
};

constexpr int MAX_CHARACTER_SPELL_SLOTS = 13;

class character {
public:
    explicit character(const std::string& jsonPath);

    std::string getName();
    int getLevel() const;
    int getUpgrade() const;
    std::string getStartingClass() const;
    int getEndurance() const;
    int getVigor() const;
    int getMind() const;
    bool getHasGreatjar() const;
    bool getHasBullgoat() const;
    std::vector<std::string> getArmor() const;
    std::vector<int> getDamageStats() const;
    double getEffectiveHealth() const;
    double getEffectiveHpRatio() const;
    int getBaseVigor() const;
    int getBaseEndurance() const;
    int getBaseMind() const;

    void setEffectiveHpRatio(double calculatedEffectiveHp);
    void setEffectiveHpVigorRatio(int setVigor);
    void setEffectiveHpEnduranceRatio(int setEndurance);
    void setPoiseRatio(int newPoise);
    void setFpRatio(int setMind);
    void setDamageStatNum(int newDamageStats);
    void setScore(double newScore);

    std::vector<double> generateMlString();

private:

    //general character values
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

    //ML related values
    int baseVigor;
    int baseEndurance;
    int baseMind;
    double effectiveHpRatio;
    int machineLearningScore;
    double effectiveHpVigorRatio;
    double effectiveHpEnduranceRatio;
    double fpRatio;
    double poiseRatio;
    double damageStatCount;
    double score;
    double bestEffectiveHpValue;
    double bestPoiseValue;
    double maxFpValue;


};



#endif //CHARACTER_H
