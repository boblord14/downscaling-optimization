//
// Created by Ethan on 8/29/2025.
//

#ifndef LOADCHARACTER_H
#define LOADCHARACTER_H

#include "Weapon.h"
#include <eigen3/Eigen/Core>
#include "character.h"
#include "onnxruntime_cxx_api.h"
#include "onnxruntime_c_api.h"

extern std::unordered_map<std::string, std::vector<int>> starting_classes;
static std::mutex negationsCacheMutex;
static std::unordered_map<std::string, std::vector<double>> starting_classes_negations;
static std::unordered_map<std::string, std::vector<double>> starting_classes_negations_greatjar;

static std::vector<std::vector<float>> logisticArmorPieces = {};

constexpr int STARTING_CLASS_STAT_POINTS = 79; //number of stat points to rescale the classes to
constexpr double SCALING_LEVEL_TARGET = 719.0;
constexpr int NUM_ARMOR_PIECES = 4; //how many armor pieces the player can equip at once
constexpr double GREATJAR_MULTIPLIER = 1.19; //multiplier to equip load from great jar talisman
constexpr int DAGGER_POISE_THRESHOLD = 58; //the amount of poise required to not get easily staggered by daggers

constexpr int CLASS_DAMAGE_STAT_INDEX = 3; //index position where the damage stats start in the starting classes list
constexpr int CLASS_MIND_STAT_INDEX = 1; //index position where the mind stat is in the starting classes list
constexpr int CLASS_VIGOR_STAT_INDEX = 0;
constexpr int CLASS_ENDURANCE_STAT_INDEX = 2;


constexpr int DAMAGE_STAT_COUNT = 5;

class loadCharacter {
public:
    static void loadData();
    static void loadData(std::string buildJsonPath, int targetLevel, int buildsToProduce);
    static void writeTrainingData(const std::string& trainingPath, const std::string& outputFilePath, int sl);
    static void functionTesting();
    static double retrieveMaxPoise();
    static double bestEffectiveHP(int statPoints, const std::string& startingClass, bool hasGreatjar);
    static int retrieveMaxFp(int total_stats, const std::string& starting_class);
    static int getMaxFpSpell();

    static double getMinStamCostWeapon();

    static int getMaxFPAshOfWar();
  static std::pair<double, double> negationsPoise(double equipLoad, const std::vector<double>& armorFraction, double armorPercent, bool hasBullgoat, const std::vector<std::vector<float>>& poiseArmorPieces);
  static double retrieveEquipWeight(const std::string& name); 
};

class Predict{
public:
    Predict(std::string modelPath, int level, int weaponUpgrade, int numBuilds, int grid): env(ORT_LOGGING_LEVEL_WARNING, "predict"),
    session(nullptr), level(level), weaponUpgrade(weaponUpgrade), numBuilds(numBuilds), grid(grid)    {
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);
        session_options.SetGraphOptimizationLevel(
            GraphOptimizationLevel::ORT_ENABLE_EXTENDED
        );

        std::wstring w_modelPath(modelPath.begin(), modelPath.end());

        session = Ort::Session(env, w_modelPath.c_str(), session_options);

        Ort::AllocatorWithDefaultOptions allocator;
        auto input_name_alloc = session.GetInputNameAllocated(0, allocator);
        auto output_name_alloc = session.GetOutputNameAllocated(0, allocator);

        input_name = input_name_alloc.get(); // needed for later things
        output_name = output_name_alloc.get();
    };
  void operator()(std::string jsonFile);
private:
    Ort::Env env;
    Ort::Session session;

    std::string input_name;
    std::string output_name;
  int weaponUpgrade;
  int numBuilds;
  int grid;
  int level;
};


// Lifted from stackoverflow for this purpose because I aint writing it myself
// Convert a 2-D vector<vector<double> > into an Eigen MatrixXd.
// Throws exception if rows do not have same length.
inline Eigen::MatrixXd convert_vvd_to_matrix(std::vector<std::vector<double> > vvd) {

    std::size_t n_rows = vvd.size();
    std::size_t n_cols = vvd.at(0).size();

    Eigen::MatrixXd result(n_rows, n_cols);
    result.row(0) = Eigen::VectorXd::Map(&vvd[0][0], n_cols);

    // Add each vector row to the MatrixXd.
    for (std::size_t i = 1; i < n_rows; i++) {

        // Make sure that every row of vvd has the same size.
        if (n_cols != vvd.at(i).size()) {
            char buffer[200];
            snprintf(buffer, 200,
                        "vvd[%zd] size (%zd) does not match vvd[0] size (%zd)",
                        i, vvd.at(i).size(), n_cols);
            std::string err_mesg(buffer);
            throw std::invalid_argument(err_mesg);
        }

        result.row(i) = Eigen::VectorXd::Map(&vvd[i][0], n_cols);
    }

    return result;
}

#endif //LOADCHARACTER_H
