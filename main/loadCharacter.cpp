//
// Created by Ethan on 8/29/2025.
//

#include "loadCharacter.h"

#include <string>
#include <sstream>
#include <vector>
#include <fstream>


void loadMind() {
    std::ifstream mindFile;
    mindFile.open (R"(..\..\csv-conversions\non csv data\Mind.txt)");

    std::string line;
    while (std::getline(mindFile, line))
    {
        std::istringstream iss(line);
        int level, value;
        if (!(iss >> level >> value)) { break; } // error

        mind.push_back(value);
    }
    mindFile.close();
}

void loadEhp90() {
    std::ifstream ehp90File;
    ehp90File.open (R"(..\..\csv-conversions\non csv data\bestehp90.txt)");

    std::string line;
    while (std::getline(ehp90File, line))
    {
        std::istringstream iss(line);
        std::string startingClass;
        float ehp;
        if (!(iss >> startingClass >> ehp)) { break; } // error

        best_ehp_90.emplace(startingClass, ehp);
    }
    ehp90File.close();
}

void loadVigScale() {
    std::ifstream vigFile;
    vigFile.open (R"(..\..\csv-conversions\non csv data\vigor.txt)");

    std::string line;
    std::getline(vigFile, line);

    std::istringstream iss(line);
    float vigor;

    while (iss >> vigor) {
        vig_scale.push_back(vigor);
    }
    vigFile.close();
}

void loadEquipLoadScale() {
    std::ifstream elFile;
    elFile.open (R"(..\..\csv-conversions\non csv data\equipload.txt)");

    std::string line;
    std::getline(elFile, line);

    std::istringstream iss(line);
    float equipLoad;

    while (iss >> equipLoad) {
        equip_load_scale.push_back(equipLoad);
    }
    elFile.close();
}

void loadPoiseScale() {
    std::ifstream poiseFile;
    poiseFile.open (R"(..\..\csv-conversions\non csv data\poisebreakpoints.txt)");

    std::string line;
    std::getline(poiseFile, line);

    std::istringstream iss(line);
    float poise;

    while (iss >> poise) {
        poise_bp.push_back(poise);
    }
    poiseFile.close();
}

//I REALLY need a better way to load this.
void loadDatafit() {
    std::ifstream datafitFile;
    datafitFile.open (R"(..\..\csv-conversions\non csv data\datafit.txt)");

    std::string line;
    std::istringstream iss;

    float fit1;
    float fit2;
    float fit3_logisticOnly;

    std::getline(datafitFile, line);
    iss.str(line);
    iss >> fit1 >> fit2 >> fit3_logisticOnly;
    logistic_head.insert(std::end(logistic_head), {fit1, fit2, fit3_logisticOnly});
    iss.clear();

    std::getline(datafitFile, line);
    iss.str(line);
    iss >> fit1 >> fit2 >> fit3_logisticOnly;
    logistic_chest.insert(std::end(logistic_chest), {fit1, fit2, fit3_logisticOnly});
    iss.clear();

    std::getline(datafitFile, line);
    iss.str(line);
    iss >> fit1 >> fit2 >> fit3_logisticOnly;
    logistic_arm.insert(std::end(logistic_arm), {fit1, fit2, fit3_logisticOnly});
    iss.clear();

    std::getline(datafitFile, line);
    iss.str(line);
    iss >> fit1 >> fit2 >> fit3_logisticOnly;
    logistic_leg.insert(std::end(logistic_leg), {fit1, fit2, fit3_logisticOnly});
    iss.clear();

    std::getline(datafitFile, line);
    iss.str(line);
    iss >> fit1 >> fit2 >> fit3_logisticOnly;
    poise_head.insert(std::end(poise_head), {fit1, fit2});
    iss.clear();

    std::getline(datafitFile, line);
    iss.str(line);
    iss >> fit1 >> fit2 >> fit3_logisticOnly;
    poise_chest.insert(std::end(poise_chest), {fit1, fit2});
    iss.clear();

    std::getline(datafitFile, line);
    iss.str(line);
    iss >> fit1 >> fit2;
    poise_arm.insert(std::end(poise_arm), {fit1, fit2});
    iss.clear();

    std::getline(datafitFile, line);
    iss.str(line);
    iss >> fit1 >> fit2;
    poise_leg.insert(std::end(poise_leg), {fit1, fit2});
    iss.clear();

    datafitFile.close();
}

void loadCharacter::loadData() {
    loadMind();
    loadEhp90();
    loadVigScale();
    loadEquipLoadScale();
    loadPoiseScale();
    loadDatafit();
}
