//
// Created by false on 6/7/2025.
//

#include "DataParser.h"


void DataParser::generateDefs() {
    defGenerator(R"(..\..\csv-conversions\equipParamWeapon.csv)", &equipParamWeapon);
    defGenerator(R"(..\..\csv-conversions\reinforceParamWeapon.csv)", &reinforceParamWeapon);
    defGenerator(R"(..\..\csv-conversions\AttackElementCorrectParam.csv)", &attackElementCorrectParam);
    defGenerator(R"(..\..\csv-conversions\CalcCorrectGraphEZPreCalc.csv)", &calcCorrectGraph);
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

    CSVReader reader(csvPath);

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
