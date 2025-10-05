//
// Created by Ethan on 9/27/2025.
//

#include "character.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::vector<int> getFPAshOfWar(const std::string& name)
{
    std::vector<int> fp = {};

    auto data = *DataParser::retrieveSwordArtByName(name);

    if (int leftHandLightAttackFP = stoi(data.at("useMagicPoint_L1")); leftHandLightAttackFP != -1 and leftHandLightAttackFP != 0)
    {
        fp.push_back(leftHandLightAttackFP);
    }

    if (int leftHandHeavyAttackFP = stoi(data.at("useMagicPoint_L2")); leftHandHeavyAttackFP != -1 and leftHandHeavyAttackFP != 0)
    {
        fp.push_back(leftHandHeavyAttackFP);
    }

    if (int rightHandLightAttackFP = stoi(data.at("useMagicPoint_R1")); rightHandLightAttackFP != -1 and rightHandLightAttackFP != 0)
    {
        fp.push_back(rightHandLightAttackFP);
    }

    if (int rightHandHeavyAttackFP = stoi(data.at("useMagicPoint_R2")); rightHandHeavyAttackFP != -1 and rightHandHeavyAttackFP != 0)
    {
        fp.push_back(rightHandHeavyAttackFP);
    }

    return fp;
}

int getFpSpell(const std::string& name)
{
    auto data = *DataParser::retrieveMagicByName(name);
    return stoi(data.at("mp"));
}

int character::getBaseVigor() const
{
    return baseVigor;
}

int character::getBaseEndurance() const
{
    return baseEndurance;
}

int character::getBaseMind() const
{
    return baseMind;
}

character::character(const std::string& jsonPath)
{
    std::ifstream f(jsonPath);
    json data = json::parse(f);

    this->name = data["name"];
    this->level = data["stats"]["rl"];
    this->vigor = data["stats"]["vig"];
    this->mind = data["stats"]["mnd"];
    this->endurance = data["stats"]["vit"];
    this->damageStats.push_back(data["stats"]["str"]);
    this->damageStats.push_back(data["stats"]["dex"]);
    this->damageStats.push_back(data["stats"]["int"]);
    this->damageStats.push_back(data["stats"]["fth"]);
    this->damageStats.push_back(data["stats"]["arc"]);
    this->startingClass = data["characterClass"];
    this->armor = std::vector<std::string>(4, "");
    this->hasBullgoat = false;
    this->hasGreatjar = false;
    this->upgradeLevel = -1;
    this->machineLearningScore = 1;
    this->baseVigor = starting_classes[this->startingClass][0];
    this->baseMind = starting_classes[this->startingClass][1];
    this->baseEndurance = starting_classes[this->startingClass][2];
    this->effectiveHpVigorRatio = 0;
    this->effectiveHpEnduranceRatio = 0;
    this->damageStatCount = 0;
    this->score = 1;

    if (data["computed"].count("poise"))
    {
        if (data["computed"]["poise"].count("altered"))
        {
            this->poise = data["computed"]["poise"]["altered"];
        }
        else
        {
            this->poise = data["computed"]["poise"]["original"];
        }
    }
    else this->poise = 0;

    for (auto serializedWeapon : data["inventory"]["slots"])
    {
        int weaponId = stoi(static_cast<std::string>(serializedWeapon["weapon_hex_id"]), nullptr, 16);
        //convert hex string to integer
        int infusion = stoi(static_cast<std::string>(serializedWeapon["affinity_hex_id"]), nullptr, 16) % 1000;
        int upgrade = stoi(static_cast<std::string>(serializedWeapon["upgrade_hex_id"]), nullptr, 16);

        if (upgrade > this->upgradeLevel) this->upgradeLevel = upgrade;

        this->weapons.emplace_back(weaponId, static_cast<Infusion>(infusion), upgrade);

        if (serializedWeapon.count("weaponArt"))
        {
            std::string AoW = (serializedWeapon["weaponArt"].is_null()) ? "" : serializedWeapon["weaponArt"];
            std::vector<int> AoWFp = getFPAshOfWar(AoW);
            this->ashes.insert(this->ashes.end(), AoWFp.begin(), AoWFp.end());
        }
    }

    for (auto serializedSpell : data["spells"]["slots"])
    {
        this->spells.emplace_back(getFpSpell(serializedSpell["name"]));
    }

    for (auto serializedHead : data["protectors"]["head"]["slots"])
    {
        if (serializedHead.count("equipIndex"))
        {
            this->armor[0] = serializedHead["name"];
            break;
        }
    }

    for (auto serializedBody : data["protectors"]["body"]["slots"])
    {
        if (serializedBody.count("equipIndex"))
        {
            this->armor[1] = serializedBody["name"];
            break;
        }
    }

    for (auto serializedArms : data["protectors"]["arms"]["slots"])
    {
        if (serializedArms.count("equipIndex"))
        {
            this->armor[2] = serializedArms["name"];
            break;
        }
    }

    for (auto serializedLegs : data["protectors"]["legs"]["slots"])
    {
        if (serializedLegs.count("equipIndex"))
        {
            this->armor[3] = serializedLegs["name"];
            break;
        }
    }

    for (auto serializedTalisman : data["talismans"]["slots"])
    {
        if (serializedTalisman.count("equipIndex"))
        {
            //checks if substring is in the main string
            if (static_cast<std::string>(serializedTalisman["name"]).find("Bull-Goat") != std::string::npos)
                this->
                    hasBullgoat = true;
            if (static_cast<std::string>(serializedTalisman["name"]).find("Great-Jar") != std::string::npos)
                this->
                    hasBullgoat = true;
        }
    }

    this->effectiveHealth = DataParser::fetchHp(this->vigor - 1);

    double negation = 0;
    negation += static_cast<double>(data["computed"]["absorption"]["physical"]);
    negation += static_cast<double>(data["computed"]["absorption"]["slash"]);
    negation += static_cast<double>(data["computed"]["absorption"]["pierce"]);
    negation += static_cast<double>(data["computed"]["absorption"]["strike"]);
    negation += static_cast<double>(data["computed"]["absorption"]["magic"]);
    negation += static_cast<double>(data["computed"]["absorption"]["fire"]);
    negation += static_cast<double>(data["computed"]["absorption"]["lightning"]);
    negation += static_cast<double>(data["computed"]["absorption"]["holy"]);
    negation /= 800;

    this->effectiveHealth /= (1 - negation);
    this->bestEffectiveHpValue = loadCharacter::bestEffectiveHP(this->level, this->startingClass, this->hasGreatjar);
    this->effectiveHpRatio = this->effectiveHealth / this->bestEffectiveHpValue;

    this->bestPoiseValue = loadCharacter::retrieveMaxPoise();
    this->poiseRatio = this->poise / this->bestPoiseValue;

    this->maxFpValue = loadCharacter::retrieveMaxFp(this->level, this->startingClass);
    this->fpRatio = DataParser::fetchFp(this->mind) / maxFpValue;

    int allocatedDamageStats = 0; //damage stats allocated not from starting class base values
    for (int i=CLASS_DAMAGE_STAT_INDEX; i<starting_classes.begin()->second.size(); i++) //"interesting" way to get the number of stat points
    {
        int charDamageStatIndex = i - CLASS_DAMAGE_STAT_INDEX; //character damage stats start at 0, not the constant
        allocatedDamageStats += this->damageStats[charDamageStatIndex] - starting_classes.at(startingClass)[i];
    }
    this->damageStatCount = allocatedDamageStats / level;
}

std::string character::getName()
{
    return name;
}

int character::getLevel() const
{
    return level;
}

int character::getUpgrade() const
{
    return upgradeLevel;
}

std::string character::getStartingClass() const
{
    return startingClass;
}

int character::getEndurance() const
{
    return endurance;
}

int character::getVigor() const
{
    return vigor;
}

int character::getMind() const
{
    return mind;
}

bool character::getHasGreatjar() const
{
    return hasGreatjar;
}

bool character::getHasBullgoat() const
{
    return hasBullgoat;
}

std::vector<std::string> character::getArmor() const
{
    return armor;
}

std::vector<int> character::getDamageStats() const
{
    return damageStats;
}

double character::getEffectiveHealth() const
{
    return effectiveHealth;
}

double character::getEffectiveHpRatio() const
{
    return effectiveHpRatio;
}

void character::setEffectiveHpRatio(const double calculatedEffectiveHp)
{
    effectiveHpRatio = calculatedEffectiveHp / bestEffectiveHpValue;
}

void character::setEffectiveHpVigorRatio(const int setVigor)
{
    effectiveHpVigorRatio = setVigor / level;
}

void character::setEffectiveHpEnduranceRatio(const int setEndurance)
{
    effectiveHpEnduranceRatio = setEndurance / level;
}

void character::setPoiseRatio(int newPoise)
{
    poiseRatio = newPoise / bestPoiseValue;
}

void character::setFpRatio(int setMind)
{
    fpRatio = DataParser::fetchFp(setMind) / maxFpValue;
}

void character::setDamageStatNum(int newDamageStats)
{
    damageStatCount = newDamageStats / level;
}

void character::setScore(double newScore)
{
    score = newScore;
}

// [score, character_level, ehp, poise, fp, average ash fp cost, average spell cost, max spell cost, number of spells,
// number of damage stats, starting class one hot encoding, greatjar, vigor, end]
std::vector<double> character::generateMlString()
{
    double ashFpRatio = 0;
    for (int i=0; i<ashes.size(); i++)
    {
        ashFpRatio += ashes[i];
    }
    if (ashes.size()>0)
    {
        ashFpRatio /= ashes.size();
    }else
    {
        ashFpRatio = 0;
    }
    ashFpRatio /= loadCharacter::getMaxFPAshOfWar();


    double spellFpRatio = 0;
    for (int i=0; i<spells.size(); i++)
    {
        spellFpRatio += spells[i];
    }
    double maxSpellFp = 0;
    if (spells.size()>0)
    {
        maxSpellFp = *std::max_element(spells.begin(), spells.end());
        spellFpRatio /= spells.size();
    }
    int highestFpSpell = loadCharacter::getMaxFpSpell();
    spellFpRatio /= highestFpSpell;
    maxSpellFp /= highestFpSpell;

    double spellSlotRatio = spells.size() / MAX_CHARACTER_SPELL_SLOTS;

    std::vector<double> finalMlString = {score, static_cast<double>(level/SCALING_LEVEL_TARGET), effectiveHpRatio, poiseRatio, fpRatio,
    ashFpRatio, spellFpRatio, maxSpellFp, spellSlotRatio, damageStatCount};

    std::vector<double> startingClassHotEncode(starting_classes_index.size(), 0);
    int classIndex = 0;
    for (auto classEntry : starting_classes_index)
    {
        if (classEntry.first == startingClass)
        {
            startingClassHotEncode[classIndex] = 1;
            break;
        }
        classIndex++;
    }
    finalMlString.insert(finalMlString.end(), startingClassHotEncode.begin(), startingClassHotEncode.end());

    //if (hasBullgoat) finalMlString.push_back(1); else finalMlString.push_back(0);
    if (hasGreatjar) finalMlString.push_back(1); else finalMlString.push_back(0);
    finalMlString.push_back(effectiveHpVigorRatio);
    finalMlString.push_back(effectiveHpEnduranceRatio);

    return finalMlString;
}


