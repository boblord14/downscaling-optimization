//
// Created by Ethan on 8/29/2025.
//

#include "loadCharacter.h"

#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <nlopt.hpp>
#include <numeric>

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

    std::sort(poise_bp.begin(), poise_bp.end());
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

double retrieveEquipWeight(std::string name){
    auto armor = DataParser::retrieveArmorByName(name);
    if (armor == nullptr) return -1;
    return stod(armor->at("weight"));
}

double retrievePoise(std::string name){
    auto armor = DataParser::retrieveArmorByName(name);
    if (armor == nullptr) return -1;
    return 1000 * stod(armor->at("toughnessCorrectRate"));
}

int retrieveMaxFp(int total_stats, std::string starting_class) {
    int index = starting_classes.at(starting_class)[1] + total_stats;
    if (index > 99) index = 99;
    return mind[index-1];
}

float maxPoise() {
    float max_head = -1;
    float max_chest = -1;
    float max_arm = -1;
    float max_leg = -1;

    for (const auto& pair : DataParser::retrieveAllArmor()) {
        if (stoi(pair.second.at("headEquip")) == 1) {
            if (float temp = stof(pair.second.at("toughnessCorrectRate")); temp > max_head) max_head = temp;
        }
        if (stoi(pair.second.at("bodyEquip")) == 1) {
            if (float temp = stof(pair.second.at("toughnessCorrectRate")); temp > max_chest) max_chest = temp;
        }
        if (stoi(pair.second.at("armEquip")) == 1) {
            if (float temp = stof(pair.second.at("toughnessCorrectRate")); temp > max_arm) max_arm = temp;
        }
        if (stoi(pair.second.at("legEquip")) == 1) {
            if (float temp = stof(pair.second.at("toughnessCorrectRate")); temp > max_leg) max_leg = temp;
        }
    }

    return static_cast<float>(std::round(1000 * (max_head + max_chest + max_arm + max_leg) / .75));
}

float LogisticFunction(float L, float k, float x0, float x) {
    return L / (1 + std::exp(-k * (x - x0)));
}

float Logistic(std::vector<float> logisticPiece, float x) {
    return LogisticFunction(logisticPiece[0], logisticPiece[1], logisticPiece[2], x);
}

std::pair<float, float> negationsPoise(float equipLoad, std::vector<float> armorFraction, float armorPercent, boolean hasBullgoat) {
    float armor = equipLoad * armorPercent;

    float negationsHead = (1 - Logistic(logistic_head, armor * armorFraction[0]));
    float negationsChest = (1 - Logistic(logistic_chest, armor * armorFraction[1]));
    float negationsArm = (1 - Logistic(logistic_arm, armor * armorFraction[2]));
    float negationsLeg = (1 - Logistic(logistic_leg, armor * armorFraction[3]));

    float negations = 1 - negationsHead * negationsChest * negationsArm * negationsLeg;

    float truePoiseHead = (poise_head[0] * armor * armorFraction[0] + poise_head[1]);
    if (truePoiseHead<=0) truePoiseHead = 0;

    float truePoiseChest = (poise_chest[0] * armor * armorFraction[0] + poise_chest[1]);
    if (truePoiseChest<=0) truePoiseChest = 0;

    float truePoiseArm = (poise_arm[0] * armor * armorFraction[0] + poise_arm[1]);
    if (truePoiseArm<=0) truePoiseArm = 0;

    float truePoiseLeg = (poise_leg[0] * armor * armorFraction[0] + poise_leg[1]);
    if (truePoiseLeg<=0) truePoiseLeg = 0;

    float fullPoise = truePoiseHead + truePoiseChest + truePoiseArm + truePoiseLeg;

    if (hasBullgoat) fullPoise = fullPoise / 0.75;

    if (fullPoise > 133) fullPoise = 133;

    return std::make_pair(negations, fullPoise);
}

std::vector<std::vector<float>> effectiveHealth(int baseVigor, int baseEndurance, float armorPercent, std::vector<float> armorFraction, int allocatedStatPoints, bool hasBullgoat, bool hasGreatjar) {
    std::vector<float> characterEquipLoadScale;
    std::vector<float> characterHealthScale;
    std::vector<float> negations;
    std::vector<float> poise;
    std::vector<float> effectiveHp;
    std::vector<std::vector<float>> bestBreakPoints(poise_bp.size(), std::vector<float>(4, -1));

    int i = 0;

    while (i <= allocatedStatPoints) {
        int hpIndex = i + baseVigor - 1;
        if (hpIndex >= 99) hpIndex = 98;
        float hp = vig_scale[hpIndex];
        characterHealthScale.push_back(hp);

        int enduranceIndex = allocatedStatPoints - i + baseEndurance - 1;
        if (enduranceIndex >= 99) enduranceIndex = 98;
        float equipLoad = equip_load_scale[enduranceIndex];
        if (hasGreatjar) equipLoad = equipLoad * 1.19;

        auto values = negationsPoise(equipLoad, armorFraction, armorPercent, hasBullgoat);
        float neg = values.first;
        float poiseVal = values.second;

        characterEquipLoadScale.push_back(equipLoad);
        poise.push_back(poiseVal);
        negations.push_back(neg);
        float computedEffectiveHp = hp / (1 - neg);
        effectiveHp.push_back(computedEffectiveHp);

        for (int j = 0; j < poise_bp.size(); j++) {
            if (poiseVal >= poise_bp[j] && computedEffectiveHp >= bestBreakPoints[j][0]) {
                bestBreakPoints[j] = {computedEffectiveHp, poiseVal, static_cast<float>(hpIndex) + 1 - baseVigor, static_cast<float>(enduranceIndex) + 1 - baseEndurance};
            }
        }
        i += 1;
    }
    while (bestBreakPoints.back()[0] == -1) bestBreakPoints.pop_back();
    return bestBreakPoints;
}

int fpToMind(int fp) {
    for (int i = 0; i < mind.size(); i++) {
        if (mind[i] == fp) return i+1;
    }
    return -1;
}

double obj(const std::vector<double> &x, std::vector<double> &grad, void* f_data) //grad and f_data are unused
{
    double negationHead = (1- Logistic(logistic_head, x[0]));
    double negationChest = (1- Logistic(logistic_chest, x[1]));
    double negationArm = (1- Logistic(logistic_arm, x[2]));
    double negationLeg = (1- Logistic(logistic_leg, x[3]));

    double neg = 1 - (negationHead * negationChest * negationArm * negationLeg);
    return -neg;
}

double equality(const std::vector<double> &x, std::vector<double> &grad, void* f_data) //grad is derivative and f_data is our equip load
{
    double equipLoad = *reinterpret_cast<double*>(f_data);

    return (x[0] + x[1] + x[2] + x[3]) - (equipLoad); //satisfied when the sum of our armor's equip load equals our max equip load
}

std::pair<std::vector<double>, double> bestNegations(float equipLoad) {
    double eq = 0.69 * equipLoad; //medium roll 69% threshold

    nlopt::opt opt(nlopt::AUGLAG, 4); //four optimization parameters, auglag for constraint support
    nlopt::opt inner(nlopt::LN_COBYLA, 4); //gradient free algo
    opt.set_local_optimizer(inner);

    opt.set_lower_bounds({0, 0, 0, 0}); //x>=0
    opt.set_min_objective(obj, nullptr); //derivative free function

    float tolerance = 1e-4 * std::max(1.0, std::abs(eq));
    opt.add_equality_constraint(equality, &eq, tolerance); //satisfied when equip load equals max acceptable equip load

    opt.set_xtol_rel(1e-6); //relative tolerances
    opt.set_ftol_rel(1e-6);
    opt.set_maxeval(2000); //limit to prevent it from taking forever or breaking

    //both of these initial guesses seem to work, but just dividing by 4 seems to be "good enough" for the optimizer most of the time
    //results in the best x0 values being the ones you start with, the 1234 one does not. both have a pretty similar best_neg so idm too much

    //std::vector<double> x0 = {1, 2, 3, 4}; //initial guess alt
    std::vector<double> x0(4, eq/4); // initial guess

    double fmin;
    nlopt::result result;
    try {
        result = opt.optimize(x0, fmin);
    }
    catch (std::exception& e) {}

    return std::make_pair(x0, -fmin); //first is all the piece equip loads, second is the negation
}

double bestEffectiveHP(int statPoints, std::string startingClass, boolean hasGreatJar)
{
    int baseVigor = starting_classes[startingClass][0];
    int baseEndurance = starting_classes[startingClass][2];

    std::vector<double> equipLoads = {};
    std::vector<double> healthPoints = {};
    std::vector<double> negations = {};
    std::vector<double> poise = {};
    std::vector<double> effectiveHP = {};
    boolean larger = false;

    for (int i=0; i<statPoints; i++)
    {
        int hpIndex = i + baseVigor - 1;
        if (hpIndex>=99)
        {
            hpIndex = 98;
        }
        double hp = vig_scale[hpIndex];
        healthPoints.push_back(hp);

        int endIndex = statPoints - i + baseEndurance - 1;
        if (endIndex>=99)
        {
            endIndex = 98;
        }
        double el = equip_load_scale[endIndex];
        if (hasGreatJar)
        {
            el *= 1.19;
        }
        equipLoads.push_back(el);

        double negation = -1;
        boolean precompute = false;

        if (!hasGreatJar && starting_classes_negations.count(startingClass)) //no great jar and starting class negations computed already
        {
            if (i < starting_classes_negations[startingClass].size())
            {
                negation = starting_classes_negations[startingClass][i];
                precompute = true;
            }
        } else if (hasGreatJar && starting_classes_negations_greatjar.count(startingClass))
        {
            if (i < starting_classes_negations_greatjar[startingClass].size())
            {
                negation = starting_classes_negations_greatjar[startingClass][i];
                precompute = true;
            }
        }

        if (!precompute)
        {
            larger = true;
            negation = bestNegations(el).second;
        }

        negations.push_back(negation);
        effectiveHP.push_back(hp / (1-negation));

    }

    if ((!hasGreatJar && !starting_classes_negations.count(startingClass)) || larger)
    {
        starting_classes_negations[startingClass] = negations;
    }
    if ((hasGreatJar && !starting_classes_negations.count(startingClass)) || larger)
    {
        starting_classes_negations_greatjar[startingClass] = negations;
    }

    return *std::max_element(effectiveHP.begin(), effectiveHP.end());  //biggest effective hp value we've found

}

std::vector<double> project(std::vector<double> originalX, int totalStats)
{
    std::vector<double> plane(originalX.size(), 1.0);

    double planeDotProduct = std::inner_product(plane.begin(), plane.end(), plane.begin(), 0.0);

    double sqrtDot = std::sqrt(planeDotProduct);

    for (int i=0; i<plane.size(); i++)
    {
        plane[i] /= sqrtDot;
    }

    double xSum = std::inner_product(originalX.begin(), originalX.end(), plane.begin(), 0.0);

    std::vector<double> newY(originalX.begin(), originalX.end());

    for (int i=0; i<newY.size(); i++)
    {
        newY[i] = newY[i] - xSum * plane[i];
    }

    double alpha = totalStats / planeDotProduct;

    for (int i=0; i<newY.size(); i++)
    {
        newY[i] = newY[i] + alpha * sqrtDot * plane[i];
    }

    boolean pos = true;

    for (int i=0; i<originalX.size(); i++)
    {
        if (newY[i] <= 1 && std::abs(newY[i] - 1.0) > 1e-4)
        {
            newY[i] = 1;
            pos = false;
            break;
        }
    }

    if (pos) return newY;
    return project(newY, totalStats);
}

void rescaleClasses()
{
    const int STARTING_CLASS_STAT_POINTS = 79;

    for (const auto & [ name, stats ] : starting_classes)
    {
        std::vector<double> imperfectStats = project(std::vector<double>(stats.begin(), stats.end()), STARTING_CLASS_STAT_POINTS);
        std::vector<int> adjustedStats = {};
        int sum = 0;
        for (double stat : imperfectStats)
        {
            int castStat = static_cast<int>(stat);
            sum += castStat;
            adjustedStats.push_back(castStat);
        }
        //sometimes we can have less than 79 allocated stats due to decimals in the projection
        //if so, find the stat closest to rounding up and move it up one, repeat until we actually allocate 79 stats
        if (sum != 79)
        {
            int remainder = 79 - sum;
            while (remainder > 0)
            {
                double max = -1;
                std::vector<int> used = {};
                int index = -1;
                for (int i=0; i<adjustedStats.size(); i++)
                {
                    double difference = imperfectStats[i] - adjustedStats[i];

                    //the second condition is an annoyingly complicated way to check if the value of "i" is not in the array "used"
                    if (difference > max && std::find(std::begin(used), std::end(used), i) == std::end(used))
                    {
                        max = difference;
                        index = i;
                    }
                }
                used.push_back(index);
                adjustedStats[index] += 1;
                remainder -= 1;
            }
        }
        starting_classes[name] = adjustedStats;
    }
}

int getMaxFPAshOfWar()
{
    std::vector<int> fp = {};
    for (const auto & [ id, data ] : DataParser::retrieveAllSwordArt())
    {
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
    }
    return *std::max_element(fp.begin(), fp.end());
}

int getMaxFpSpell()
{
    std::vector<int> fp = {};
    for (const auto & [ id, data ] : DataParser::retrieveAllMagic())
    {
        fp.push_back(stoi(data.at("mp")));
    }
    return *std::max_element(fp.begin(), fp.end());
}

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

void loadCharacter::loadData() {
    loadMind();
    loadEhp90();
    loadVigScale();
    loadEquipLoadScale();
    loadPoiseScale();
    loadDatafit();
    std::cout << "equip weight: " << retrieveEquipWeight("Blue Cloth Vest") << std::endl;
    std::cout << "poise: " << retrievePoise("Blue Cloth Vest") << std::endl;
    std::cout << "mind value test: " << retrieveMaxFp(20, "Bandit") << std::endl;
    std::cout << "max poise: " << maxPoise() << std::endl;

    auto result = bestNegations(190.4);
    std::cout << "bestNeg test: ";
    std::cout << "x0 = [ ";
    for (double v : result.first) {
        std::cout << v << " ";
    }
    std::cout << "] ";
    std::cout << "best_neg = " << result.second << std::endl;
    std::vector<double> grad;
    std::cout << "direct obj eval with x0: " << obj(result.first, grad, nullptr) << std::endl;

    std::cout << "max EHP: " << bestEffectiveHP(90, "Hero", true) << std::endl;

    auto projResult = project(std::vector<double>(starting_classes["Hero"].begin(), starting_classes["Hero"].end()), 79);
    std::cout << "projection test: ";
    std::cout << "newY = [ ";
    for (double y : projResult) {
        std::cout << y << " ";
    }
    std::cout << "] " << std::endl;

    rescaleClasses();

    std::cout << "class rescale test: ";
    std::cout << "new hero stats(should be same or close to projection) = [ ";
    for (int stat : starting_classes["Hero"]) {
        std::cout << stat << " ";
    }
    std::cout << "] " << std::endl;

    std::cout << "highest fp AoW test: " << getMaxFPAshOfWar() << std::endl;
    std::cout << "highest fp spell test: " << getMaxFpSpell() << std::endl;

    std::cout << "fp cost fetch of square off AoW: ";
    auto fpFetch = getFPAshOfWar("Square Off");
    std::cout << "fp costs  = [ ";
    for (double fp : fpFetch) {
        std::cout << fp << " ";
    }
    std::cout << "] " << std::endl;

    std::cout << "fp cost fetch of bloodflame talons spell: " << getFpSpell("Bloodflame Talons") << std::endl;
}
