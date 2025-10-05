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

#include "character.h"

/// Helper function to retrieve the equip weight for a given armor piece by name
/// @param name Official name for the armor piece(community names from params)
/// @return Equip weight for input armor piece
double retrieveEquipWeight(const std::string& name){
    auto armor = DataParser::retrieveArmorByName(name);
    if (armor == nullptr) return -1;
    return stod(armor->at("weight"));
}

/// Helper function to retrieve the poise for a given armor piece by name
/// @param name Official name for the armor piece(community names from params)
/// @return Poise for input armor piece
double retrievePoise(const std::string& name){
    auto armor = DataParser::retrieveArmorByName(name);
    if (armor == nullptr) return -1;
    return 1000 * stod(armor->at("toughnessCorrectRate"));
}

/// Calculates the maximum possible FP value if a given number of stat points were added to mind on top of the base
/// starting class's mind value
/// @param total_stats Stat points to add to the base starting class's mind value
/// @param starting_class Starting class for the character
/// @return Max possible FP value
int loadCharacter::retrieveMaxFp(int total_stats, const std::string& starting_class) {
    int index = starting_classes.at(starting_class)[1] + total_stats;
    if (index > 99) index = 99;
    return DataParser::fetchFp(index);
}

/// Calculates the maximum possible poise value a character can have if they wear the highest poise armor pieces in the game.
/// @return the max possible poise value
double loadCharacter::retrieveMaxPoise() {
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

    return static_cast<double>(std::round(1000 * (max_head + max_chest + max_arm + max_leg) / .75));
}

/// Code representation of the standard logistic function(s-shaped curve) equation:
/// f(x) = L / (1 + e^(k (x - x0)))
/// @param L Carrying capacity
/// @param k Logistic growth rate
/// @param x0 X value of the function's midpoint
/// @param x Input argument for x
/// @return Calculated logistic value f(x)
float LogisticFunction(float L, float k, float x0, float x) {
    return L / (1 + std::exp(-k * (x - x0)));
}

/// Runs the Logistic Function with a given armor piece's formula and a custom value for x
/// @param logisticPiece Vector containing Logistic function values for L, k, and x0 in that order
/// @param x Input argument for x
/// @return Calculated logistic value f(x) for the given armor piece equation and x
float Logistic(const std::vector<float>& logisticPiece, float x) {
    return LogisticFunction(logisticPiece[0], logisticPiece[1], logisticPiece[2], x);
}

/// Calculates the ideal negations and poise for armor based on how much % of the equip load used for armor is dedicated
/// towards a given armor piece. Uses values loaded from datafit.txt for the negation and poise equations.
/// @param equipLoad Total equip load that the player can have
/// @param armorFraction Vector containing the fraction of total equip load for each armor piece's weight
/// @param armorPercent What percentage of total equip load is dedicated to armor
/// @param hasBullgoat Is the player using bullgoat talisman(poise matters more)
/// @return A pair of the ideal overall negation and poise values
std::pair<float, float> negationsPoise(float equipLoad, const std::vector<double>& armorFraction, float armorPercent, boolean hasBullgoat) {
    float armor = equipLoad * armorPercent;
    std::vector<std::vector<float>> poiseArmorPieces = DataParser::fetchArmorPoise();

    //x is the % of current equip load dedicated to armor reserved for the given armor piece
    float negationsHead = (1 - Logistic(logisticArmorPieces[0], armor * armorFraction[0]));
    float negationsChest = (1 - Logistic(logisticArmorPieces[1], armor * armorFraction[1]));
    float negationsArm = (1 - Logistic(logisticArmorPieces[2], armor * armorFraction[2]));
    float negationsLeg = (1 - Logistic(logisticArmorPieces[3], armor * armorFraction[3]));

    float negations = 1 - negationsHead * negationsChest * negationsArm * negationsLeg;

    float truePoiseHead = (poiseArmorPieces[0][0] * armor * armorFraction[0] + poiseArmorPieces[0][1]);
    if (truePoiseHead<=0) truePoiseHead = 0;

    float truePoiseChest = (poiseArmorPieces[1][0] * armor * armorFraction[0] + poiseArmorPieces[1][1]);
    if (truePoiseChest<=0) truePoiseChest = 0;

    float truePoiseArm = (poiseArmorPieces[2][0] * armor * armorFraction[0] + poiseArmorPieces[2][1]);
    if (truePoiseArm<=0) truePoiseArm = 0;

    float truePoiseLeg = (poiseArmorPieces[3][0] * armor * armorFraction[0] + poiseArmorPieces[3][1]);
    if (truePoiseLeg<=0) truePoiseLeg = 0;

    float fullPoise = truePoiseHead + truePoiseChest + truePoiseArm + truePoiseLeg;

    if (hasBullgoat) fullPoise = fullPoise / 0.75;

    if (fullPoise > 133) fullPoise = 133;

    return std::make_pair(negations, fullPoise);
}

/// Calculates Effective Hp breakpoints, which is the true HP value after negations and other related factors are accounted for.
/// Takes armor ratios and weight into account.
/// @param baseVigor The character's starting class's base vigor value
/// @param baseEndurance The character's starting class's base endurance value
/// @param armorPercent What percentage of total equip load is dedicated to armor
/// @param armorFraction Vector containing the fraction of total equip load for each armor piece's weight
/// @param allocatedStatPoints Number of stat points to allocate
/// @param hasBullgoat Is the player using bullgoat talisman(poise matters more)
/// @param hasGreatjar Is the player using the great jar talisman(poise/neg matters slightly more, significant equip load change)
/// @return A vector of the best effective HP breakpoints, containing effective HP, poise value, stat points allocated to vigor, and stat points allocated to endurance
std::vector<std::vector<float>> effectiveHealth(int baseVigor, int baseEndurance, float armorPercent, const std::vector<double>& armorFraction, int allocatedStatPoints, bool hasBullgoat, bool hasGreatjar) {
    std::vector<float> characterEquipLoadScale;
    std::vector<float> characterHealthScale;
    std::vector<float> negations;
    std::vector<float> poise;
    std::vector<float> effectiveHp;
    std::vector<std::vector<float>> bestBreakPoints(DataParser::getPoiseSize(), std::vector<float>(4, -1));

    int i = 0;

    if (logisticArmorPieces.empty()) logisticArmorPieces = DataParser::fetchLogistics();

    while (i <= allocatedStatPoints) {
        int hpIndex = i + baseVigor - 1;
        if (hpIndex >= 99) hpIndex = 98;
        float hp = DataParser::fetchHp(hpIndex);
        characterHealthScale.push_back(hp);

        int enduranceIndex = allocatedStatPoints - i + baseEndurance - 1;
        if (enduranceIndex >= 99) enduranceIndex = 98;
        float equipLoad = DataParser::fetchEq(enduranceIndex);
        if (hasGreatjar) equipLoad = equipLoad * 1.19;

        auto values = negationsPoise(equipLoad, armorFraction, armorPercent, hasBullgoat);
        float neg = values.first;
        float poiseVal = values.second;

        characterEquipLoadScale.push_back(equipLoad);
        poise.push_back(poiseVal);
        negations.push_back(neg);
        float computedEffectiveHp = hp / (1 - neg);
        effectiveHp.push_back(computedEffectiveHp);

        for (int j = 0; j < DataParser::getPoiseSize(); j++) {
            if (poiseVal >= DataParser::fetchPoise(j) && computedEffectiveHp >= bestBreakPoints[j][0]) {
                bestBreakPoints[j] = {computedEffectiveHp, poiseVal, static_cast<float>(hpIndex) + 1 - baseVigor, static_cast<float>(enduranceIndex) + 1 - baseEndurance};
            }
        }
        i += 1;
    }
    while (bestBreakPoints.back()[0] == -1) bestBreakPoints.pop_back();
    return bestBreakPoints;
}

/// Given an FP total, determine what value of mind is required to reach it
/// @param fp Fp total
/// @return Mind value needed to reach Fp total
int fpToMind(int fp) {
    auto mindData = DataParser::getMind();

    for (int i = 0; i < mindData.size(); i++) {
        if (mindData[i] == fp) return i+1;
    }
    return -1;
}

/// Nlopt's obj function, takes in guesses from the algorithm and returns the calculated output value
/// @param x Nlopt guessed arguments for each armor piece's x in the Logicstic Function
/// @param grad Required for nlopt, unused. Would be the gradient values
/// @param f_data Required for nlopt, unused. Additional arguments that would be passed in would go here
/// @return Overall negation value as a negative for nlopt to work off of
double obj(const std::vector<double> &x, std::vector<double> &grad, void* f_data)
{
    double negationHead = (1- Logistic(logisticArmorPieces[0], x[0]));
    double negationChest = (1- Logistic(logisticArmorPieces[1], x[1]));
    double negationArm = (1- Logistic(logisticArmorPieces[2], x[2]));
    double negationLeg = (1- Logistic(logisticArmorPieces[3], x[3]));

    double neg = 1 - (negationHead * negationChest * negationArm * negationLeg);
    return -neg;
}

/// Equality constraint for nlopt, satisfied when the sum of our armor's equip load equals our max equip load within
/// a given tolerance of "close enough"
/// @param x Vector of armor equip load values passed in
/// @param grad Required for nlopt, unused. Would be the gradient values
/// @param f_data Passes in our equip load value
/// @return Difference between armor equip load sum and total equip load
double equality(const std::vector<double> &x, std::vector<double> &grad, void* f_data)
{
    double equipLoad = *static_cast<double*>(f_data);

    return (x[0] + x[1] + x[2] + x[3]) - (equipLoad);
}

/// Uses nlopt's local derivative free variant of the COBYA algorithm wrapped in the general purpose AUGLAG algorithm
/// to generate an optimal set of armor equip loads per piece to obtain the best possible negations for a total equip
/// load value
/// @param equipLoad Total equip load the function has to work with
/// @return A paid first containing the 4 ideal armor equip load values, followed by the total negation value obtained
std::pair<std::vector<double>, double> bestNegations(float equipLoad) {
    if (logisticArmorPieces.empty()) logisticArmorPieces = DataParser::fetchLogistics();

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
    try {
        opt.optimize(x0, fmin);
    }
    catch (std::exception& e)
    {
        //std::cout << "Optimizer error thrown: " << e.what() << std::endl;
        //bunch of roundoff limited errors thrown, gotta look into sometime
    }

    return std::make_pair(x0, -fmin); //first is all the piece equip loads, second is the negation
}

/// Calculates the best effective HP value by testing every possible combination of vigor and endurance values. Uses
/// the armor negation optimizer to generate values for optimal negation at an equip load to modify the base HP from the
/// vigor value. Ignores preset armor ratios.
/// @param statPoints Stat points to allocate
/// @param startingClass Starting class name as a string
/// @param hasGreatjar Is the player using the great jar talisman(poise/neg matters slightly more, significant equip load change)
/// @return The largest effective HP value calculated
double loadCharacter::bestEffectiveHP(int statPoints, const std::string& startingClass, boolean hasGreatjar)
{
    int baseVigor = starting_classes[startingClass][0];
    int baseEndurance = starting_classes[startingClass][2];

    std::vector<double> negations = {};
    std::vector<double> poise = {}; //unused, poise currently not taken into account
    std::vector<double> effectiveHP = {};
    boolean larger = false;

    for (int i=0; i<statPoints; i++)
    {
        int hpIndex = i + baseVigor - 1;
        if (hpIndex>=99)
        {
            hpIndex = 98;
        }
        double hp = DataParser::fetchHp(hpIndex);

        int endIndex = statPoints - i + baseEndurance - 1;
        if (endIndex>=99)
        {
            endIndex = 98;
        }
        double el = DataParser::fetchEq(endIndex);

        if (hasGreatjar)
        {
            el *= 1.19;
        }

        double negation = -1;
        boolean precompute = false;

        if (!hasGreatjar && starting_classes_negations.count(startingClass)) //no great jar and starting class negations computed already
        {
            if (i < starting_classes_negations[startingClass].size())
            {
                negation = starting_classes_negations[startingClass][i];
                precompute = true;
            }
        } else if (hasGreatjar && starting_classes_negations_greatjar.count(startingClass))
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

    if ((!hasGreatjar && !starting_classes_negations.count(startingClass)) || larger)
    {
        starting_classes_negations[startingClass] = negations;
    }
    if ((hasGreatjar && !starting_classes_negations.count(startingClass)) || larger)
    {
        starting_classes_negations_greatjar[startingClass] = negations;
    }

    return *std::max_element(effectiveHP.begin(), effectiveHP.end());  //biggest effective hp value we've found

}

/// A projection function that modifies a starting class to only have a certain number of total stat points allocated.
/// For example, the hero class has the starting stats {14,9,12,16,9,7,9,11} which are a total of 87 stat points.
/// This function can keep the weighting/game design identity of the hero class while making it only have 50 stat points.
/// @param originalX Vector containing the starting class's stat allocations
/// @param totalStats Total stat points we want the starting class to have
/// @return A modified version of the starting class's stat values adjusted the number of stat points we want it to have
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

    bool pos = true;

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

///Rescales all starting classes to allocate the same number of stat points as in the STARTING_CLASS_STAT_POINTS constant.
///Functionally makes all starting classes have the same base soul level.
void rescaleClasses()
{
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
        //sometimes we can have less than STARTING_CLASS_STAT_POINTS allocated stats due to decimals in the projection
        //if so, find the stat closest to rounding up and move it up one, repeat until we actually allocate STARTING_CLASS_STAT_POINTS stats
        if (sum != STARTING_CLASS_STAT_POINTS)
        {
            int remainder = STARTING_CLASS_STAT_POINTS - sum;
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

/// Gets the maximum possible amount of FP that an ash of war could use from every ash of war in the game. Handles
/// cases where some AoWs use different FP values for their light and heavy variants and uses the highest amounts.
/// @return The highest amount of FP an ash of war can possibly consume
int loadCharacter::getMaxFPAshOfWar()
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

/// Gets the maximum possible amount of FP that a spell could use from every spell in the game.
/// @return The highest amount of FP a spell can possibly consume
int loadCharacter::getMaxFpSpell()
{
    std::vector<int> fp = {};
    for (const auto & [ id, data ] : DataParser::retrieveAllMagic())
    {
        fp.push_back(stoi(data.at("mp")));
    }
    return *std::max_element(fp.begin(), fp.end());
}

/// Generates a grid of every possible [Damage, Spell Slots, Effective Hp] combination to test, with a number of stat points
/// to put towards each of the three categories. Adjusts each stat in preset increments to save time and computing power.
/// @param soulLevel The total number of stat points we have to allocate
/// @param delta The amount we increment each category's stat point allocation by
/// @return Every possible [Damage, Spell Slots, Effective Hp] stat combination in increments of delta
std::vector<std::vector<int>> gridGenerator(int soulLevel, int delta)
{
    const int iterations = (int)(soulLevel/delta) + 1;
    std::vector<std::vector<int>> outputGrid;
    for(int i=0; i<iterations; i++){
        int xVal = i * delta;
        if(xVal <= soulLevel){
            for(int j=0; j<iterations; j++){
                int yVal = j * delta;
                if(xVal + yVal <= soulLevel){
                    int zVal = soulLevel - (xVal + yVal);
                    outputGrid.push_back({xVal, yVal, zVal});
                }
            }
        }
    }
    return outputGrid;
}

/// Works to set up the training data to be written to the machine learning model.
/// @param characterInput Character class loaded with data
/// @param delta How coarse or fine we want to test with our stat allocations. The larger the value, the less accurate our computed answer will be.
/// @return A final vector output of characters with the updated ML data
std::vector<character> exponentialDecay(character& characterInput, int delta)
{
    std::string startingClass = characterInput.getStartingClass();

    int baseClassVigor = characterInput.getBaseVigor();
    int baseClassMind = characterInput.getBaseMind();
    int baseClassEndurance = characterInput.getBaseEndurance();

    int maxFp = loadCharacter::retrieveMaxFp(characterInput.getLevel(), startingClass);
    double equipLoad = DataParser::fetchEq(characterInput.getEndurance());

    if (characterInput.getHasGreatjar()) equipLoad *= GREATJAR_MULTIPLIER;

    std::vector<double> armorFraction(NUM_ARMOR_PIECES);
    double armorPercent = 0;
    std::vector<double> armorWeight(NUM_ARMOR_PIECES);

    for (int i=0; i<NUM_ARMOR_PIECES; i++)
    {
        std::string armorPiece = characterInput.getArmor()[i];
        double weight = armorPiece.empty() ? 0.0 : retrieveEquipWeight(armorPiece); //if no armor, weight is obviously 0
        armorWeight.push_back(weight);
        armorPercent += weight;
    }

    if (armorPercent != 0)
    {
        for (int i=0; i<NUM_ARMOR_PIECES; i++)
        {
            armorFraction[i] = armorWeight[i] / armorPercent;
        }
    } else std::fill(armorFraction.begin(), armorFraction.end(), 0.0);

    armorPercent /= equipLoad;

    auto optimizedValues = negationsPoise(equipLoad, armorFraction, armorPercent, characterInput.getHasBullgoat());
    float optimalNegations = optimizedValues.first(); //unused for later(?)
    float optimalPoise = optimizedValues.second(); //unused for later(?)

    //compute EHP with all stat points put towards EHP, armor ratios ignored
    double optimalEffectiveHp = loadCharacter::bestEffectiveHP(characterInput.getLevel(), characterInput.getStartingClass(), characterInput.getHasGreatjar());

    double startingEffectiveHp = characterInput.getEffectiveHealth() / optimalEffectiveHp; //unused for later(?)

    //original number of stat points allocated towards EHP
    int allocatedEhpStats = characterInput.getVigor() - baseClassVigor + characterInput.getEndurance() - baseClassEndurance;

    //compute EHP breakpoints with the original amount of stat points allocated towards EHP and armor taken into account
    std::vector<std::vector<float>> allocatedEhp = effectiveHealth(baseClassVigor, baseClassEndurance, armorPercent,
        armorFraction, allocatedEhpStats, characterInput.getHasBullgoat(), characterInput.getHasGreatjar());

    //how close did our allocated EHP calculated w/ armor come to the optimal EHP?
    characterInput.setEffectiveHpRatio(allocatedEhp[0][0]);

    //what amount of our total soul level does the vigor and endurance for our EHP use?
    characterInput.setEffectiveHpVigorRatio(allocatedEhp[0][2]);
    characterInput.setEffectiveHpEnduranceRatio(allocatedEhp[0][3]);

    int allocatedDamageStats = 0; //damage stats allocated not from starting class base values
    for (int i=CLASS_DAMAGE_STAT_INDEX; i<starting_classes.begin()->second.size(); i++) //"interesting" way to get the number of stat points
    {
        int charDamageStatIndex = i - CLASS_DAMAGE_STAT_INDEX; //character damage stats start at 0, not the constant
        allocatedDamageStats += characterInput.getDamageStats()[charDamageStatIndex] - starting_classes.at(characterInput.getStartingClass())[i];
    }

    int allocatedMindStats = characterInput.getMind() - starting_classes.at(characterInput.getStartingClass())[CLASS_MIND_STAT_INDEX];

    characterInput.setPoiseRatio(DAGGER_POISE_THRESHOLD);

    auto statAllocationVariants = gridGenerator(characterInput.getLevel(), delta);

    std::vector<character> outputMlCharacters;

    outputMlCharacters.push_back(characterInput);

    for (auto statAllocation : statAllocationVariants)
    {
        character newStatMlCharacter = characterInput; //deep copy

        //fp adjustment
        int mindIndex = starting_classes[characterInput.getStartingClass()][CLASS_MIND_STAT_INDEX] + statAllocation[2] - 1;
        if (mindIndex >= 99) mindIndex = 98;
        newStatMlCharacter.setFpRatio(mindIndex);

        //damage adjustment
        newStatMlCharacter.setDamageStatNum(statAllocation[0]);
        allocatedEhp = effectiveHealth(baseClassVigor, baseClassEndurance, armorPercent,
       armorFraction, statAllocation[1], characterInput.getHasBullgoat(), characterInput.getHasGreatjar());

        double distance = std::abs(statAllocation[0] - allocatedDamageStats) + std::abs(statAllocation[1] - allocatedEhpStats)
        + std::abs(statAllocation[2] - allocatedMindStats);
        double score = std::exp(-distance / 100);
        if (score <= 0.5) continue;

        newStatMlCharacter.setEffectiveHpRatio(allocatedEhp[0][0]);
        //newStatMlCharacter.setPoiseRatio(allocatedEhp[0][1]); ignoring poise for now
        newStatMlCharacter.setEffectiveHpVigorRatio(allocatedEhp[0][2]);
        newStatMlCharacter.setEffectiveHpEnduranceRatio(allocatedEhp[0][3]);

        newStatMlCharacter.setScore(score);
        outputMlCharacters.push_back(newStatMlCharacter);
    }

    return outputMlCharacters;
}

/// Prepares data for machine learning, takes in character, generates many possibilities on where to allocate stats,
/// and returns the possibilities with their values.
/// @param characterInput character to process
/// @param delta "resolution" of the product, the higher, the less precise the end result.
/// @return vector of many character variants
std::vector<character> prepareData(character characterInput, int delta)
{
    std::vector<character> labledData;
    if (characterInput.getStartingClass() == "") return labledData; //return empty vector if invalid
    labledData = exponentialDecay(characterInput, delta);
    return labledData;
}

void loadCharacter::loadData()
{
    std::cout << "equip weight: " << retrieveEquipWeight("Blue Cloth Vest") << std::endl;
    std::cout << "poise: " << retrievePoise("Blue Cloth Vest") << std::endl;
    std::cout << "mind value test: " << retrieveMaxFp(20, "Bandit") << std::endl;
    std::cout << "max poise: " << retrieveMaxPoise() << std::endl;

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

    std::cout << "load bloodsage character: ";
    character bloodsage(R"(..\..\csv-conversions\non csv data\BloodsageNadine.json)");
    std::cout << bloodsage.getName() << " RL " << bloodsage.getLevel() << " +" << bloodsage.getUpgrade() << std::endl;

    std::cout << "grid build test: ";
    auto grid = gridGenerator(90, 5);
    std::cout << "debug this, too long to print" << std::endl;

    std::cout << "character hp, fp, and equip load fetch test: " << std::endl;
    std::cout << "equip load: " << DataParser::fetchEq(bloodsage.getEndurance()) << std::endl;
    std::cout << "hp: " << DataParser::fetchHp(bloodsage.getVigor()) << std::endl;
    std::cout << "mind: " << DataParser::fetchFp(bloodsage.getMind()) << std::endl;

}
