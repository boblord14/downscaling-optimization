//
// Created by Ethan on 8/29/2025.
//

#ifndef LOADCHARACTER_H
#define LOADCHARACTER_H

#include "Weapon.h"

static std::unordered_map<std::string, std::vector<double>> starting_classes_negations;
static std::unordered_map<std::string, std::vector<double>> starting_classes_negations_greatjar;

static std::vector<std::vector<float>> logisticArmorPieces = {};

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

class loadCharacter {
public:
    static void loadData();
};

#endif //LOADCHARACTER_H
