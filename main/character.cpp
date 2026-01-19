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

    if (int leftHandLightAttackFP = stoi(data.at("useMagicPoint_L1")); leftHandLightAttackFP != -1 && leftHandLightAttackFP != 0)
    {
        fp.push_back(leftHandLightAttackFP);
    }

    if (int leftHandHeavyAttackFP = stoi(data.at("useMagicPoint_L2")); leftHandHeavyAttackFP != -1 && leftHandHeavyAttackFP != 0)
    {
        fp.push_back(leftHandHeavyAttackFP);
    }

    if (int rightHandLightAttackFP = stoi(data.at("useMagicPoint_R1")); rightHandLightAttackFP != -1 && rightHandLightAttackFP != 0)
    {
        fp.push_back(rightHandLightAttackFP);
    }

    if (int rightHandHeavyAttackFP = stoi(data.at("useMagicPoint_R2")); rightHandHeavyAttackFP != -1 && rightHandHeavyAttackFP != 0)
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

int Character::getBaseVigor() const
{
    return baseVigor;
}

int Character::getBaseEndurance() const
{
    return baseEndurance;
}

int Character::getBaseMind() const
{
    return baseMind;
}

double Character::getPoise() const
{
    return poise;
}

std::vector<Weapon> Character::getWeapons() const
{
    return weapons;
}

Character::Character(const std::string& jsonPath)
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
    this->upgradeLevel = data["weaponUpgrade"];
    this->machineLearningScore = 1;
    this->baseVigor = starting_classes[this->startingClass][0];
    this->baseMind = starting_classes[this->startingClass][1];
    this->baseEndurance = starting_classes[this->startingClass][2];
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
        int weaponId = DataParser::retrieveWeaponIdByName(serializedWeapon["name"]);

        //convert infusion name to infusion enum(and then int version)
        int infusion = serializedWeapon["infusion"].is_null() ? BASE : infusion_name_mapping[serializedWeapon["infusion"]];

        int upgrade = 0;
        if (!serializedWeapon["upgrade"].is_null())
        {
            upgrade = serializedWeapon["upgrade"].get<int>();
        } else
        {
            upgrade = this->upgradeLevel;
        }

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
    if (!data["computed"]["absorption"].is_null())
    {
        negation += static_cast<double>(data["computed"]["absorption"]["physical"]);
        negation += static_cast<double>(data["computed"]["absorption"]["slash"]);
        negation += static_cast<double>(data["computed"]["absorption"]["pierce"]);
        negation += static_cast<double>(data["computed"]["absorption"]["strike"]);
        negation += static_cast<double>(data["computed"]["absorption"]["magic"]);
        negation += static_cast<double>(data["computed"]["absorption"]["fire"]);
        negation += static_cast<double>(data["computed"]["absorption"]["lightning"]);
        negation += static_cast<double>(data["computed"]["absorption"]["holy"]);
    }else
    {
        negation += 300; //generic value, usually recomputed when it matters + only an edge case doesnt have it
    }

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
    this->damageStatCount = static_cast<double>(allocatedDamageStats) / level;
    this->effectiveHpVigorRatio = static_cast<double>(this->vigor) / level;
    this->effectiveHpEnduranceRatio = static_cast<double>(this->endurance) / level;

    f.close();
}

std::string Character::getName()
{
    return name;
}

int Character::getLevel() const
{
    return level;
}

int Character::getUpgrade() const
{
    return upgradeLevel;
}

std::string Character::getStartingClass() const
{
    return startingClass;
}

std::vector<int> Character::getStartingClassStats() const
{
    return starting_classes.at(startingClass);
}

int Character::getEndurance() const
{
    return endurance;
}

int Character::getVigor() const
{
    return vigor;
}

int Character::getMind() const
{
    return mind;
}

bool Character::getHasGreatjar() const
{
    return hasGreatjar;
}

bool Character::getHasBullgoat() const
{
    return hasBullgoat;
}

std::vector<std::string> Character::getArmor() const
{
    return armor;
}

std::vector<int> Character::getDamageStats() const
{
    return damageStats;
}

double Character::getEffectiveHealth() const
{
    return effectiveHealth;
}

double Character::getEffectiveHpRatio() const
{
    return effectiveHpRatio;
}

void Character::setEffectiveHpRatio(const double calculatedEffectiveHp)
{
    effectiveHpRatio = calculatedEffectiveHp / bestEffectiveHpValue;
}

void Character::setEffectiveHpVigorRatio(const int setVigor)
{
    effectiveHpVigorRatio = static_cast<double>(setVigor) / level;
}

void Character::setEffectiveHpEnduranceRatio(const int setEndurance)
{
    effectiveHpEnduranceRatio = static_cast<double>(setEndurance) / level;
}

void Character::setPoiseRatio(int newPoise)
{
    poiseRatio = static_cast<double>(newPoise) / loadCharacter::retrieveMaxPoise();
}

void Character::setFpRatio(int setMind)
{
    fpRatio = DataParser::fetchFp(setMind) / maxFpValue;
}

void Character::setDamageStatNum(int newDamageStats)
{
    damageStatCount = static_cast<double>(newDamageStats) / level;
}

void Character::setScore(double newScore)
{
    score = newScore;
}

// [score, character_level, ehp, poise, fp, average ash fp cost, average spell cost, max spell cost, number of spells,
// number of damage stats, starting class one hot encoding, greatjar, bullgoat, vigor, end]
std::vector<double> Character::generateMlString()
{
    double ashFpRatio = 0;
    for (int ashe : ashes)
    {
        ashFpRatio += ashe;
    }
    if (!ashes.empty())
    {
        ashFpRatio /= ashes.size();
    }else
    {
        ashFpRatio = 0;
    }
    ashFpRatio /= loadCharacter::getMaxFPAshOfWar();


    double spellFpRatio = 0;
    for (int spell : spells)
    {
        spellFpRatio += spell;
    }
    double maxSpellFp = 0;
    if (!spells.empty())
    {
        maxSpellFp = *std::max_element(spells.begin(), spells.end());
        spellFpRatio /= spells.size();
    }
    int highestFpSpell = loadCharacter::getMaxFpSpell();
    spellFpRatio /= highestFpSpell;
    maxSpellFp /= highestFpSpell;

    double spellSlotRatio = static_cast<double>(spells.size()) / MAX_CHARACTER_SPELL_SLOTS;

    std::vector<double> finalMlString = {score, static_cast<double>(level)/SCALING_LEVEL_TARGET, effectiveHpRatio, /*poiseRatio,*/ fpRatio,
    ashFpRatio, spellFpRatio, maxSpellFp, spellSlotRatio, damageStatCount};

    std::vector<double> startingClassHotEncode(starting_classes_index.size(), 0);
    int classIndex = starting_classes_index[startingClass];
    startingClassHotEncode[classIndex] = 1;
    finalMlString.insert(finalMlString.end(), startingClassHotEncode.begin(), startingClassHotEncode.end());

    if (hasBullgoat) finalMlString.push_back(1); else finalMlString.push_back(0);
    if (hasGreatjar) finalMlString.push_back(1); else finalMlString.push_back(0);
    finalMlString.push_back(effectiveHpVigorRatio);
    finalMlString.push_back(effectiveHpEnduranceRatio);

    return finalMlString;
}


