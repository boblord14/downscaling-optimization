//
// Created by false on 6/7/2025.
//

#ifndef DOWNSCALING_OPTIMIZATION_DATAPARSER_H
#define DOWNSCALING_OPTIMIZATION_DATAPARSER_H
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include "DataParser.h"
#include "csv.hpp"
#include "Weapon.h"

using namespace csv;

static std::unordered_map<int, std::unordered_map<std::string, std::string>> equipParamWeapon;
static std::unordered_map<int, std::unordered_map<std::string, std::string>> reinforceParamWeapon;
static std::unordered_map<int, std::unordered_map<std::string, std::string>> attackElementCorrectParam;
static std::unordered_map<int, std::vector<float>> calcCorrectGraph;

class DataParser{
public:
    static void generateDefs();
    static std::unordered_map<std::string, std::string> retrieveWeapon(int id);
    static std::unordered_map<std::string, std::string> retrieveUpgrade(int id);
    static std::unordered_map<std::string, std::string> retrieveElementCorrection(int id);
    static std::vector<float> retrieveCcg(int id);
    static std::vector<int> getWeaponIds();
private:
    static void defGenerator(const std::string& csvPath, std::unordered_map<int, std::unordered_map<std::string, std::string>> *target);
    static void defGenerator(const std::string& csvPath, std::unordered_map<int, std::vector<float>> *target);
};

#endif //DOWNSCALING_OPTIMIZATION_DATAPARSER_H
