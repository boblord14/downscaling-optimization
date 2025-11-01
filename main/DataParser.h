//
// Created by false on 6/7/2025.
//

#ifndef DOWNSCALING_OPTIMIZATION_DATAPARSER_H
#define DOWNSCALING_OPTIMIZATION_DATAPARSER_H
#include <filesystem>
#include <unordered_map>
#include "DataParser.h"
#include "csv.hpp"

using namespace csv;

static std::unordered_map<int, std::unordered_map<std::string, std::string>> equipParamWeapon;
static std::unordered_map<int, std::unordered_map<std::string, std::string>> reinforceParamWeapon;
static std::unordered_map<int, std::unordered_map<std::string, std::string>> attackElementCorrectParam;
static std::unordered_map<int, std::unordered_map<std::string, std::string>> equipParamProtector;
static std::unordered_map<int, std::unordered_map<std::string, std::string>> swordArtsParam;
static std::unordered_map<int, std::unordered_map<std::string, std::string>> magic;
static std::unordered_map<int, std::vector<float>> calcCorrectGraph;

static std::vector<int> mind;
static std::unordered_map<std::string, float> best_ehp_90;
static std::vector<float> vig_scale;
static std::vector<float> equip_load_scale;
static std::vector<float> poise_bp;

static std::vector<float> logistic_head;
static std::vector<float> logistic_chest;
static std::vector<float> logistic_arm;
static std::vector<float> logistic_leg;

static std::vector<float> poise_head;
static std::vector<float> poise_chest;
static std::vector<float> poise_arm;
static std::vector<float> poise_leg;

class DataParser{
public:
    static void generateDefs();
    static std::unordered_map<std::string, std::string> retrieveWeapon(int id);
    static std::unordered_map<std::string, std::string> retrieveUpgrade(int id);
    static std::unordered_map<std::string, std::string> retrieveElementCorrection(int id);

    static std::unordered_map<std::string, std::string> retrieveArmor(int id);
    static std::unordered_map<int, std::unordered_map<std::string, std::string>> retrieveAllArmor();
    static std::unordered_map<std::string, std::string>* retrieveArmorByName(std::string name);

    static std::unordered_map<std::string, std::string> retrieveMagic(int id);
    static std::unordered_map<int, std::unordered_map<std::string, std::string>> retrieveAllMagic();
    static std::unordered_map<std::string, std::string>* retrieveMagicByName(std::string name);

    static std::unordered_map<std::string, std::string> retrieveSwordArt(int id);
    static std::unordered_map<int, std::unordered_map<std::string, std::string>> retrieveAllSwordArt();
    static std::unordered_map<std::string, std::string>* retrieveSwordArtByName(std::string name);

    static std::vector<float> retrieveCcg(int id);
    static std::vector<int> getWeaponIds();

    static int fetchFp(int mindLevel);
    static std::vector<int> getMind();

    static float fetchEHP90(std::string startingClass);
    static float fetchHp(int index);
    static float fetchEq(int index);

    static float fetchPoise(int index);
    static int getPoiseSize();

    static std::vector<std::vector<float>> fetchLogistics();
    static std::vector<std::vector<float>> fetchArmorPoise();

    static std::vector<std::vector<int>> loadSpecificWeaponData(int weaponId, int infusionId);

private:
    static void defGenerator(const std::string& csvPath, std::unordered_map<int, std::unordered_map<std::string, std::string>> *target);
    static void defGenerator(const std::string& csvPath, std::unordered_map<int, std::vector<float>> *target);
    static void loadMind();
    static void loadEhp90();
    static void loadVigScale();
    static void loadEquipLoadScale();
    static void loadPoiseScale();
    static void loadDatafit();
};

#endif //DOWNSCALING_OPTIMIZATION_DATAPARSER_H
