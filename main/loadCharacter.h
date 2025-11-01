//
// Created by Ethan on 8/29/2025.
//

#ifndef LOADCHARACTER_H
#define LOADCHARACTER_H

#include "Weapon.h"
#include <Eigen/Core>

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

constexpr int DAMAGE_STAT_COUNT = 5;

class loadCharacter {
public:
    static void loadData();
    static double retrieveMaxPoise();
    static double bestEffectiveHP(int statPoints, const std::string& startingClass, boolean hasGreatjar);
    static int retrieveMaxFp(int total_stats, const std::string& starting_class);
    static int getMaxFpSpell();
    static int getMaxFPAshOfWar();
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
                        "vvd[%ld] size (%ld) does not match vvd[0] size (%ld)",
                        i, vvd.at(i).size(), n_cols);
            std::string err_mesg(buffer);
            throw std::invalid_argument(err_mesg);
        }

        result.row(i) = Eigen::VectorXd::Map(&vvd[i][0], n_cols);
    }

    return result;
}

#endif //LOADCHARACTER_H
