//
// Created by false on 6/7/2025.
//

#include "Weapon.h"


Weapon::Weapon(int id, Infusion infusion, int upgrade) {
    this->id = id;
    this->infusion = infusion;
    this->upgrade = upgrade;

}


std::vector<double> coefficients = {(-0.8/121), (-1.6/121), (-0.4/3), (-0.8/3), (19.2/49), (38.4/49)};

double Weapon::calculateDefenseReduction(double ratio) {
  double damage = 1;
  if (8 >= ratio && 2.5 < ratio) {
    double val = ratio - 8;
    int index = 0;
    double square = (coefficients[2 * index] * std::pow(val, 2) + 0.9);
    damage = square;
  }
  if ( 2.5 >= ratio && 1 < ratio) {
    double val = ratio - 2.5;
    int index = 1;
    double square = (coefficients[2 * index] * std::pow(val, 2) + 0.7);
    damage = square;
  }
  if (1 >= ratio && .125 < ratio) {
    double val = ratio - 0.125;
    int index = 2;
    double square = (coefficients[2 * index] * std::pow(val, 2) + 0.1);
    damage = square;
  }
  if (.125 >= ratio) {
    damage = .1;
  }
  if (8 < ratio) {
    damage = .9;
  }
  return damage;
}

double Weapon::calculateDamage(const std::vector<int>& stats, const std::vector<int>& defs, bool two_handed) {
  auto ars = calcAR(stats[0],
		    stats[1],
		    stats[2],
		    stats[3],
		    stats[4],
		    two_handed);
  double damage = 0;
  for (int i = 0; i < 5; ++i) {
    damage += calculateDefenseReduction(ars[i]/defs[i]) * ars[i];
  }
  return int(damage);
}

//The fact that I cant do "int int" means I have to explicitly write the names of all the stats and I hate it
std::vector<double> Weapon::calcAR(int strength, int dexterity, int intelligence, int faith, int arcane, bool isTwoHanded) {
    std::unordered_map<std::string, int> characterStats =
            {{"Strength", strength}, {"Agility", dexterity}, {"Magic", intelligence}, {"Faith", faith}, {"Luck", arcane}};

    if(weaponData.empty()){
      return std::vector<double>(5,-1);
    }

    int equipParamWeaponID = id + infusion;
    std::unordered_map<std::string, std::string> weaponLookup;
    try { //verify weapon + infusion combo actually exists
        weaponLookup = weaponData.at(equipParamWeaponID);
    } catch (const std::out_of_range& e) {
      return std::vector<double>(5,-1);
    }

    std::string damageTypes[10] = {"Physics", "Magic", "Fire", "Thunder", "Dark",
                                   "physics", "magic", "fire", "thunder", "dark"}; //yes, we're still using dark in the big 2025
    std::vector<double> arCalcs(5,0);

    for(int i=0; i<5; i++){
        double baseDmg = stod(weaponLookup.at("attackBase" + damageTypes[i]));

        int reinforceParamId = stoi(weaponLookup.at("reinforceTypeId")) + upgrade; //reinforceparamid is scaling id + upgrade level


        baseDmg *= stod(reinforceParamWeapon.at(reinforceParamId).at(damageTypes[i+5] + "AtkRate")); //query AR multiplier for reinforcement

        arCalcs[i] = baseDmg;

        if(baseDmg == 0) continue;

        std::vector<std::string> scalingStats = GetAttackElementCorrect(stoi(weaponLookup.at("attackElementCorrectId")), damageTypes[i]);

        for(const std::string& statName : scalingStats){

            int stat = characterStats.at(statName); //find character level of scaling stat

            if(statName == "Strength" && isTwoHanded && (weaponLookup.at("isDualBlade")) == "FALSE") stat *= 1.5; //2h mult

            int saturationIndex = stoi(weaponLookup.at("correctType_" + damageTypes[i])); //calcCorrectGraph id to use

            std::vector<float> satIndexValues = calcCorrectGraph.at(saturationIndex); //get calcCorrectGraph scaling graph

            double curveValue = satIndexValues[stat-1] / 100; //find value of scaling in calcCorrectGraph graph

            double scalingCoef = stod(weaponLookup.at("correct" + statName));

            double reinforceStatMod = stod(reinforceParamWeapon.at(reinforceParamId).at("correct" + statName + "Rate"))/100;

            arCalcs[i] += baseDmg * scalingCoef * curveValue * reinforceStatMod;

        }
    }

    return arCalcs;
}

std::vector<std::string> Weapon::GetAttackElementCorrect(int attackElementCorrectId, const std::string& element){
    std::string statNames[5] = {"Strength", "Dexterity", "Magic", "Faith", "Luck"};
    std::vector<std::string> activeScalingStats;

    std::unordered_map<std::string, std::string> elementData = attackElementCorrectParam.at(attackElementCorrectId);


    for(int i=0; i<5; i++){
        std::string scalingEnabled = elementData.at("is" + statNames[i] + "Correct_by" + element);
        if(scalingEnabled == "TRUE"){
            if(i==1){//dexterity is called agility is 75% of cases
                activeScalingStats.emplace_back("Agility");
            }else{
                activeScalingStats.emplace_back(statNames[i]);
            }
        }
    }

    return activeScalingStats;
}

void Weapon::generateDefs() {
    defGenerator(R"(..\..\csv-conversions\equipParamWeapon.csv)", "ID", 1);
    defGenerator(R"(..\..\csv-conversions\reinforceParamWeapon.csv)", "ID", 2);
    defGenerator(R"(..\..\csv-conversions\CalcCorrectGraphEZPreCalc.csv)", "ID", 3);
    defGenerator(R"(..\..\csv-conversions\AttackElementCorrectParam.csv)", "ID", 4);
}

void Weapon::defGenerator(const std::string& csvPath, const std::string& key, int target){
    CSVReader reader(csvPath);

    std::vector<std::string> columnNames = reader.get_col_names();

    if(target == 3){ //separate generator for calcCorrectGraph because it's significantly simplified
        int index = 0;
        for (CSVRow& row: reader) {
            std::vector<float> saturationIndex = {};

            int i = 0;
            for (const std::string& saturationValue : row.operator std::vector<std::string>()) {
                saturationIndex.push_back(stof(saturationValue));
                i++;
            }
            calcCorrectGraph.insert({index, saturationIndex});
            index++;
        }
        return;
    }

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
        }else if(target == 4){
            attackElementCorrectParam.insert({id, rowInfo});
        }
    }
}





