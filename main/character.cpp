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

int getFpSpell(std::string name)
{
    auto data = *DataParser::retrieveMagicByName(name);
    return stoi(data.at("mp"));
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

    if (data["computed"].count("poise"))
    {
        if (data["computed"]["poise"].count("altered"))
        {
            this->poise = data["computed"]["poise"]["altered"];
        }else
        {
            this->poise = data["computed"]["poise"]["original"];
        }
    }else this->poise = 0;

    for (auto serializedWeapon : data["inventory"]["slots"])
    {
        int weaponId = stoi(static_cast<std::string>(serializedWeapon["weapon_hex_id"]), nullptr, 16); //convert hex string to integer
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
            if (static_cast<std::string>(serializedTalisman["name"]).find("Bull-Goat") != std::string::npos) this->hasBullgoat = true;
            if (static_cast<std::string>(serializedTalisman["name"]).find("Great-Jar") != std::string::npos) this->hasBullgoat = true;
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
}

//unused constructor in the odd case we need to read in from some way other than json formatting
character::character(std::vector<Weapon> weapons, std::vector<int> ashes, std::vector<std::string> armor,
                     std::vector<int> spells, std::string startingClass, int upgradeLevel, double effectiveHealth, int level, int vigor,
                     int endurance, int mind, std::vector<int> damageStats, bool hasBullgoat, bool hasGreatjar, double poise, std::string name)
{
    this->weapons = weapons;
    this->ashes = ashes;
    this->armor = armor;
    this->spells = spells;
    this->startingClass = startingClass;
    this->upgradeLevel = upgradeLevel;
    this->effectiveHealth = effectiveHealth;
    this->level = level;
    this->vigor = vigor;
    this->endurance = endurance;
    this->mind = mind;
    this->damageStats = damageStats;
    this->hasBullgoat = hasBullgoat;
    this->hasGreatjar = hasGreatjar;
    this->poise = poise;
    this->name = name;
}
