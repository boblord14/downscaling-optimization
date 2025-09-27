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
    defGenerator(R"(..\..\csv-conversions\equipParamWeapon.csv)", &equipParamWeapon);
    defGenerator(R"(..\..\csv-conversions\reinforceParamWeapon.csv)", &reinforceParamWeapon);
    defGenerator(R"(..\..\csv-conversions\AttackElementCorrectParam.csv)", &attackElementCorrectParam);
    defGenerator(R"(..\..\csv-conversions\CalcCorrectGraphEZPreCalc.csv)", &calcCorrectGraph);
    defGenerator(R"(..\..\csv-conversions\equipParamProtector.csv)", &equipParamProtector);
    defGenerator(R"(..\..\csv-conversions\Magic.csv)", &magic);
    defGenerator(R"(..\..\csv-conversions\SwordArtsParam.csv)", &swordArtsParam);

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
        if (nameConv == originalName) {
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
