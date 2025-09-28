//
// Created by false on 6/7/2025.
//

#include "DataParser.h"

char asciitolower(char in) {
    if (in <= 'Z' && in >= 'A')
        return in - ('Z' - 'z');
    return in;
}

//quick and dirty way to make toLower work
std::string toLower(std::string data)
{
    std::string convString = data;
    std::transform(convString.begin(), convString.end(), convString.begin(), asciitolower);
    return convString;
}

void DataParser::generateDefs() {
    //load params
    defGenerator(R"(..\..\csv-conversions\equipParamWeapon.csv)", &equipParamWeapon);
    defGenerator(R"(..\..\csv-conversions\reinforceParamWeapon.csv)", &reinforceParamWeapon);
    defGenerator(R"(..\..\csv-conversions\AttackElementCorrectParam.csv)", &attackElementCorrectParam);
    defGenerator(R"(..\..\csv-conversions\CalcCorrectGraphEZPreCalc.csv)", &calcCorrectGraph);
    defGenerator(R"(..\..\csv-conversions\equipParamProtector.csv)", &equipParamProtector);
    defGenerator(R"(..\..\csv-conversions\Magic.csv)", &magic);
    defGenerator(R"(..\..\csv-conversions\SwordArtsParam.csv)", &swordArtsParam);

    //load additional data and precomputed info
    loadMind();
    loadEhp90();
    loadVigScale();
    loadEquipLoadScale();
    loadPoiseScale();
    loadDatafit();

}

std::unordered_map<std::string, std::string> DataParser::retrieveWeapon(int id) {
    return equipParamWeapon.at(id);
}

std::unordered_map<std::string, std::string> DataParser::retrieveUpgrade(int id) {
    return reinforceParamWeapon.at(id);
}

std::unordered_map<std::string, std::string> DataParser::retrieveElementCorrection(int id) {
    return attackElementCorrectParam.at(id);
}

std::unordered_map<std::string, std::string> DataParser::retrieveArmor(int id) {
    return equipParamProtector.at(id);
}

std::unordered_map<int, std::unordered_map<std::string, std::string>> DataParser::retrieveAllArmor() {
    return equipParamProtector;
}

std::unordered_map<std::string, std::string>* DataParser::retrieveArmorByName(std::string name) {
    for (const auto& pair : equipParamProtector) {
        if (toLower(pair.second.at("Name")) == toLower((name))) {
            return &equipParamProtector.at(pair.first);
        }
    }
    return nullptr;
}

std::unordered_map<std::string, std::string> DataParser::retrieveMagic(int id) {
    return magic.at(id);
}

std::unordered_map<int, std::unordered_map<std::string, std::string>> DataParser::retrieveAllMagic()
{
    return magic;
}

std::unordered_map<std::string, std::string>* DataParser::retrieveMagicByName(std::string name) {
    std::string originalName = toLower(name);
    for (const auto& pair : magic) {
        std::string nameConv = toLower(pair.second.at("Name"));

        //check if name is a substring of the actual param name and that it's not the npc version
        if (nameConv.find(originalName) != std::string::npos && nameConv.find("npc") == std::string::npos) {
            return &magic.at(pair.first);
        }
    }
    return nullptr;
}

std::unordered_map<std::string, std::string> DataParser::retrieveSwordArt(int id) {
    return swordArtsParam.at(id);
}

std::unordered_map<int, std::unordered_map<std::string, std::string>> DataParser::retrieveAllSwordArt()
{
    return swordArtsParam;
}

std::unordered_map<std::string, std::string>* DataParser::retrieveSwordArtByName(std::string name) {
    for (const auto& pair : swordArtsParam) {
        if (toLower(pair.second.at("Name")) == toLower(name)) {
            return &swordArtsParam.at(pair.first);
        }
    }
    return nullptr;
}

std::vector<float> DataParser::retrieveCcg(int id) {
    return calcCorrectGraph.at(id);
}

std::vector<int> DataParser::getWeaponIds() {
    std::vector<int> keys;
    for (const auto& pair : equipParamWeapon) {
        keys.push_back(pair.first);
    }
    return keys;
}

int DataParser::fetchFp(int mindLevel)
{
    return mind[mindLevel-1];
}

std::vector<int> DataParser::getMind()
{
    return mind;
}

float DataParser::fetchEHP90(std::string startingClass)
{
    return best_ehp_90[startingClass];
}

float DataParser::fetchHp(int index)
{
    return vig_scale[index];
}

float DataParser::fetchEq(int index)
{
    return equip_load_scale[index];
}

float DataParser::fetchPoise(int index)
{
    return poise_bp[index];
}

int DataParser::getPoiseSize()
{
    return poise_bp.size();
}

std::vector<std::vector<float>> DataParser::fetchLogistics()
{
    return {logistic_head, logistic_chest, logistic_arm, logistic_leg};
}

std::vector<std::vector<float>> DataParser::fetchArmorPoise()
{
    return {poise_head, poise_chest, poise_arm, poise_leg};
}


void DataParser::defGenerator(const std::string& csvPath, std::unordered_map<int, std::unordered_map<std::string, std::string>> *target){

    CSVFormat format;
    format.delimiter(',');
    CSVReader reader(csvPath, format);

    std::vector<std::string> columnNames = reader.get_col_names();

    for (CSVRow& row: reader) {
        int id = row["ID"].get<int>();
        std::unordered_map<std::string, std::string> rowInfo = {};

        int i = 0;
        for (const std::string& value : row.operator std::vector<std::string>()) {
            rowInfo.insert({columnNames[i], value});
            i++;
        }
        target->insert({id,rowInfo});
    }
}

void DataParser::defGenerator(const std::string& csvPath, std::unordered_map<int, std::vector<float>> *target) {
    CSVReader reader(csvPath);

    std::vector<std::string> columnNames = reader.get_col_names();
    int index = 0;
    for (CSVRow& row: reader) {
        std::vector<float> saturationIndex = {};

        int i = 0;
        for (const std::string& saturationValue : row.operator std::vector<std::string>()) {
            saturationIndex.push_back(stof(saturationValue));
            i++;
        }
        target->insert({index, saturationIndex});
        index++;
    }
}

void DataParser::loadMind() {
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

void DataParser::loadEhp90() {
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

void DataParser::loadVigScale() {
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

void DataParser::loadEquipLoadScale() {
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

void DataParser::loadPoiseScale() {
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

    std::sort(poise_bp.begin(), poise_bp.end());
}

//I REALLY need a better way to load this.
void DataParser::loadDatafit() {
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
    iss >> fit1 >> fit2;
    poise_head.insert(std::end(poise_head), {fit1, fit2});
    iss.clear();

    std::getline(datafitFile, line);
    iss.str(line);
    iss >> fit1 >> fit2;
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
