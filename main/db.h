//
// Created by Ethan on 8/18/2025.
//

#ifndef DB_H
#define DB_H
#include "Weapon.h"

/*Manages the file output results of computing the weapon statistics.*/

/*Loads saved scaling stat values for a given weapon, used in cached values.
 */
inline std::pair<std::vector<double>,std::vector<std::vector<int>>> loadScale(const Weapon& w) {
    std::string file = "Scaling\\" + std::to_string(w.getId() + w.getInfusion());
    std::ifstream in(file);
    auto opt = std::make_pair(std::vector<double>(), std::vector<std::vector<int>>());
    if (in.good() == false) {
        std::cout << "Bad File" << std::endl;
        return opt;
    }
    std::cout << "Loading weapon " << std::endl;
    std::vector<int> stats(5,0);
    double damage;
    while(in >> damage) {
        in >> stats[0];
        in >> stats[1];
        in >> stats[2];
        in >> stats[3];
        in >> stats[4];
        opt.first.push_back(damage);
        opt.second.push_back(stats);
    }
    return opt;
}

/* Saves scaling output values to file, with AR post defenses first followed by stats in str dex int fth arc order
 */
inline void saveScaling(const Weapon& w, const std::pair<std::vector<double>,std::vector<std::vector<int>>>& opt) {
    std::string file = "Scaling\\" + std::to_string(w.getId() + w.getInfusion()) + ".txt";
    std::ofstream out(file);
    for (size_t i = 0; i < opt.first.size(); ++i) {
        out << opt.first[i] << " ";
        for (size_t j = 0; j < 5; ++j) {
            out << opt.second[i][j] << " ";
        }
        out << std::endl;
    }

}

/* Quick check using a magic number(24) to check if a given file is properly filled out, and deleting it if it isn't
 */
inline bool isCompleted(const Weapon& w) {
    std::string file = "Scaling\\" + std::to_string(w.getId() + w.getInfusion()) + ".txt";

    if (!std::filesystem::exists(file)) return false; //file doesn't exist

    int number_of_lines = 0;
    std::string line;
    std::ifstream inputFile;
    inputFile.open(file);

    while (std::getline(inputFile, line)) ++number_of_lines;

    if (number_of_lines == 24) return true; //file exists with correct # of lines

    inputFile.close();
    std::filesystem::remove(file);
    std::cout << "File for weapon " << w.getId() << " broken, recomputing..." << std::endl;
    return false; //file exists, is old/incomplete, so delete it and re-compute it fresh
}
#endif //DB_H
