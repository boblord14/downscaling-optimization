//
// Created by false on 6/7/2025.
//

#include "Weapon.h"

/** Poor man's string to bool helper function for the exact string format used in the param database */
bool stob(std::string str) {
    if (str == "FALSE") return FALSE;
    if (str == "TRUE") return TRUE;
    return NULL;
}

Weapon::Weapon(int id, Infusion infusion, int upgrade) {
    this->id = id;
    this->infusion = infusion;
    this->upgrade = upgrade;

    const int equipParamWeaponId = id + infusion;
    auto equipParamWeaponEntry = DataParser::retrieveWeapon(equipParamWeaponId);

    this->name = equipParamWeaponEntry.at("Name");
    this->isSomber = (stoi(equipParamWeaponEntry.at("materialSetId")) == 2200);

    int reinforceTypeId = stoi(equipParamWeaponEntry.at("reinforceTypeId"));

    if (reinforceTypeId == 3000) this->upgrade = 0;
    else if (this->isSomber) this->upgrade = std::round(this->upgrade/2.5); //weapon upgrade level edge cases

    this->attackBaseElement = {stoi(equipParamWeaponEntry.at("attackBasePhysics")), stoi(equipParamWeaponEntry.at("attackBaseMagic")), stoi(equipParamWeaponEntry.at("attackBaseFire")), stoi(equipParamWeaponEntry.at("attackBaseThunder")), stoi(equipParamWeaponEntry.at("attackBaseDark"))};

    auto reinforceParamEntry = DataParser::retrieveUpgrade(reinforceTypeId + this->upgrade);

    this->elementAttackRate = {stod(reinforceParamEntry.at("physicsAtkRate")), stod(reinforceParamEntry.at("magicAtkRate")), stod(reinforceParamEntry.at("fireAtkRate")), stod(reinforceParamEntry.at("thunderAtkRate")), stod(reinforceParamEntry.at("darkAtkRate"))};

    this->attackElementCorrectId = stoi(equipParamWeaponEntry.at("attackElementCorrectId"));

    this->isDualBlade = stob(equipParamWeaponEntry.at("isDualBlade"));

    this->correctTypeElement = {stoi(equipParamWeaponEntry.at("correctType_Physics")), stoi(equipParamWeaponEntry.at("correctType_Magic")), stoi(equipParamWeaponEntry.at("correctType_Fire")), stoi(equipParamWeaponEntry.at("correctType_Thunder")), stoi(equipParamWeaponEntry.at("correctType_Dark"))};

    this->saturationIndex = {DataParser::retrieveCcg(this->correctTypeElement[0]), DataParser::retrieveCcg(this->correctTypeElement[1]), DataParser::retrieveCcg(this->correctTypeElement[2]), DataParser::retrieveCcg(this->correctTypeElement[3]), DataParser::retrieveCcg(this->correctTypeElement[4])};

    this->correctStatRate = {stod(reinforceParamEntry.at("correctStrengthRate")), stod(reinforceParamEntry.at("correctAgilityRate")), stod(reinforceParamEntry.at("correctMagicRate")), stod(reinforceParamEntry.at("correctFaithRate")), stod(reinforceParamEntry.at("correctLuckRate"))};

    auto attackElementParamEntry = DataParser::retrieveElementCorrection(this->attackElementCorrectId);

    this->isStatCorrectByElement = {{
        {stob(attackElementParamEntry.at("isStrengthCorrect_byPhysics")), stob(attackElementParamEntry.at("isStrengthCorrect_byMagic")), stob(attackElementParamEntry.at("isStrengthCorrect_byFire")), stob(attackElementParamEntry.at("isStrengthCorrect_byThunder")), stob(attackElementParamEntry.at("isStrengthCorrect_byDark"))},
        {stob(attackElementParamEntry.at("isDexterityCorrect_byPhysics")), stob(attackElementParamEntry.at("isDexterityCorrect_byMagic")), stob(attackElementParamEntry.at("isDexterityCorrect_byFire")), stob(attackElementParamEntry.at("isDexterityCorrect_byThunder")), stob(attackElementParamEntry.at("isDexterityCorrect_byDark"))},
        {stob(attackElementParamEntry.at("isMagicCorrect_byPhysics")), stob(attackElementParamEntry.at("isMagicCorrect_byMagic")), stob(attackElementParamEntry.at("isMagicCorrect_byFire")), stob(attackElementParamEntry.at("isMagicCorrect_byThunder")), stob(attackElementParamEntry.at("isMagicCorrect_byDark"))},
        {stob(attackElementParamEntry.at("isFaithCorrect_byPhysics")), stob(attackElementParamEntry.at("isFaithCorrect_byMagic")), stob(attackElementParamEntry.at("isFaithCorrect_byFire")), stob(attackElementParamEntry.at("isFaithCorrect_byThunder")), stob(attackElementParamEntry.at("isFaithCorrect_byDark"))},
        {stob(attackElementParamEntry.at("isLuckCorrect_byPhysics")), stob(attackElementParamEntry.at("isLuckCorrect_byMagic")), stob(attackElementParamEntry.at("isLuckCorrect_byFire")), stob(attackElementParamEntry.at("isLuckCorrect_byThunder")), stob(attackElementParamEntry.at("isLuckCorrect_byDark"))},
    }};

    this->correctStat = {stod(equipParamWeaponEntry.at("correctStrength")), stod(equipParamWeaponEntry.at("correctAgility")), stod(equipParamWeaponEntry.at("correctMagic")), stod(equipParamWeaponEntry.at("correctFaith")), stod(equipParamWeaponEntry.at("correctLuck"))};
}

//The fact that I cant do "int int" means I have to explicitly write the names of all the stats and I hate it
std::vector<double> Weapon::calcAR(int strength, int dexterity, int intelligence, int faith, int arcane, bool isTwoHanded) {
    std::vector<double> arCalcs = {0, 0, 0, 0, 0};

    for (int i=0; i<5; i++) {
        int damage = attackBaseElement[i] * elementAttackRate[i];
        arCalcs[i] += damage;
        if (damage == 0) continue;

        int stats[5] = {strength, dexterity, intelligence, faith, arcane};
        for (int j=0; j<5; j++) {
            if (isStatCorrectByElement[j][i]) {
                int stat = stats[j];
                if (j == 0 && !isDualBlade && isTwoHanded) stat = static_cast<int>(stat * 1.5);

                float curveValue = static_cast<float>(saturationIndex[i][stat - 1])/100;
                double scalingCoef = correctStat[j];
                double reinforceStatMod = correctStatRate[j]/100;

                arCalcs[i] += damage * scalingCoef * curveValue * reinforceStatMod;
            }
        }
    }
    return arCalcs;
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