//
// Created by false on 6/7/2025.
//

#include "Weapon.h"

Weapon::Weapon(int id, Infusion infusion, int upgrade) {
    this->id = id;
    this->infusion = infusion;
    this->upgrade = upgrade;

}

//The fact that I cant do "int int" means I have to explicitly write the names of all the stats and I hate it
double Weapon::calcAR(int strength, int dexterity, int intelligence, int faith, int arcane, bool isTwoHanded) {
    std::unordered_map<std::string, int> characterStats =
            {{"str", strength}, {"dex", dexterity}, {"int", intelligence}, {"fth", faith}, {"arc", arcane}};

    if(weaponData.empty()){
        return -1;
    }
    std::unordered_map<std::string, std::string> weaponLookup = weaponData.at(id);
    std::string infusionPath = "infusions/" + std::to_string(infusion);
    if(weaponLookup.at(infusionPath + "/infusion").empty()){
        return -1;
    }

    std::string damageTypes[5] = {"physical", "magic", "fire", "lightning", "holy"};
    double arCalcs[5] = {0,0,0,0,0};
    for(int i=0; i<5; i++){
        double baseDmg = stod(weaponLookup.at(infusionPath + "/baseDmg/" + damageTypes[i] + "/0"));

        int reinforceParamId = stoi(weaponLookup.at(infusionPath + "/scalingId")) + upgrade; //reinforceparamid is scaling id + upgrade level

        baseDmg *= stod(reinforceParamWeapon.at(reinforceParamId).at(damageTypes[i] + "AtkRate")); //query AR mult from scaling

        arCalcs[i] = baseDmg;

        if(baseDmg == 0) continue;

        int physicalEdgeCase = 1;
        if(i == 0) physicalEdgeCase = 2;

        for(int j=0; j<physicalEdgeCase; j++){
            std::string statName = weaponLookup.at(infusionPath + "/scalingOrigin/" + damageTypes[i] + "/" + std::to_string(j)); //get primary scaling stat for damage type

            int stat = characterStats.at(statName); //find character level of scaling stat

            if(statName == "str" && !isTwoHanded && weaponLookup.at("applyTwoHandMod") == "FALSE") stat *= 1.5; //2h mult

            double curveValue = graphScaling(stoi(weaponLookup.at(infusionPath + "/saturationIndex/" + damageTypes[i] + "/0")), stat);

        }
    }

    double AR = std::stod(weaponLookup.at("weaponId"));
    return AR;
}

double Weapon::graphScaling(int saturationIndex, int statVal){
    std::array<float, 19> curve = calcCorrectGraph.at(saturationIndex);


    return 0.0;
}

void Weapon::generateDefs() {
    defGenerator(R"(..\..\csv-conversions\weaponData.csv)", "weaponId", 1);
    defGenerator(R"(..\..\csv-conversions\reinforceParamWeapon.csv)", "_key", 2);
}

void Weapon::defGenerator(const std::string& csvPath, const std::string& key, int target){
    CSVReader reader(csvPath);

    std::vector<std::string> columnNames = reader.get_col_names();

    for (CSVRow& row: reader) {
        int id = row[key].get<int>();
        std::unordered_map<std::string, std::string> rowInfo = {};

        int i = 0;
        for (const std::string& value : row.operator std::vector<std::string>()) {
            rowInfo.insert({columnNames[i], value});
            i++;
        }
        if(target == 1){
            weaponData.insert({id,rowInfo});
        }else if(target == 2){
            reinforceParamWeapon.insert({id,rowInfo});
        }
    }
}





