import csv
from operator import truediv

import numpy
import pandas as pd
from dataclasses import dataclass

from pandas import Series


@dataclass
class Weapon():
    id: int
    name: str
    infusion: int
    upgrade: int
    attackBaseElement: list[int]
    elementAttackRate: list[int]
    attackElementCorrectId: int
    isDualBlade: bool
    correctTypeElement: int
    saturationIndex = list[list[float]]
    correctStat = list[float]
    correctStatRate = list[float]
    isStatCorrectByElement = list[list[bool]]
    isSomber = bool

    def __init__(self, id, infusion, upgrade):
        self.id = id
        self.infusion = infusion
        self.upgrade = upgrade

equipParamWeapon = pd.read_csv('../../csv-conversions/equipParamWeapon.csv')
attackElementCorrectParam = pd.read_csv('../../csv-conversions/AttackElementCorrectParam.csv')
calcCorrectGraph = list(csv.reader(open('../../csv-conversions/CalcCorrectGraphEZPreCalc.csv')))
reinforceParamWeapon = pd.read_csv('../../csv-conversions/reinforceParamWeapon.csv')

def load_data():
    equipParamWeapon.set_index(equipParamWeapon.columns[0], inplace=True)
    attackElementCorrectParam.set_index(attackElementCorrectParam.columns[0], inplace=True)
    reinforceParamWeapon.set_index(reinforceParamWeapon.columns[0], inplace=True)

def load_weapon(weapon):
    real_weapon_id = weapon.id + weapon.infusion
    paramEntry = equipParamWeapon.loc[real_weapon_id]
    weapon.isSomber = True if paramEntry.materialSetId == 2200 else False
    upgrade = weapon.upgrade
    if weapon.isSomber == True:
        upgrade = int((upgrade/2.5) + .5)
    weapon.name = paramEntry.Name
    weapon.attackBaseElement = [paramEntry.attackBasePhysics, paramEntry.attackBaseMagic, paramEntry.attackBaseFire, paramEntry.attackBaseThunder, paramEntry.attackBaseDark]
    reinforceParamEntry = reinforceParamWeapon.loc[paramEntry.reinforceTypeId + upgrade]
    weapon.elementAttackRate = [reinforceParamEntry.physicsAtkRate, reinforceParamEntry.magicAtkRate, reinforceParamEntry.fireAtkRate, reinforceParamEntry.thunderAtkRate, reinforceParamEntry.darkAtkRate]
    weapon.attackElementCorrectId = paramEntry.attackElementCorrectId
    weapon.isDualBlade = paramEntry.isDualBlade
    weapon.correctTypeElement = [paramEntry.correctType_Physics, paramEntry.correctType_Magic, paramEntry.correctType_Fire, paramEntry.correctType_Thunder, paramEntry.correctType_Dark]
    weapon.saturationIndex = [calcCorrectGraph[weapon.correctTypeElement[0]+1], calcCorrectGraph[weapon.correctTypeElement[1]+1], calcCorrectGraph[weapon.correctTypeElement[2]+1], calcCorrectGraph[weapon.correctTypeElement[3]+1], calcCorrectGraph[weapon.correctTypeElement[4]+1]]
    weapon.correctStatRate = [reinforceParamEntry.correctStrengthRate, reinforceParamEntry.correctAgilityRate, reinforceParamEntry.correctMagicRate, reinforceParamEntry.correctFaithRate, reinforceParamEntry.correctLuckRate]
    attackElementParamEntry = attackElementCorrectParam.loc[weapon.attackElementCorrectId]
    weapon.isStatCorrectByElement = [[attackElementParamEntry.isStrengthCorrect_byPhysics, attackElementParamEntry.isStrengthCorrect_byMagic, attackElementParamEntry.isStrengthCorrect_byFire, attackElementParamEntry.isStrengthCorrect_byThunder, attackElementParamEntry.isStrengthCorrect_byDark],
    [attackElementParamEntry.isDexterityCorrect_byPhysics, attackElementParamEntry.isDexterityCorrect_byMagic, attackElementParamEntry.isDexterityCorrect_byFire, attackElementParamEntry.isDexterityCorrect_byThunder, attackElementParamEntry.isDexterityCorrect_byDark],
    [attackElementParamEntry.isMagicCorrect_byPhysics, attackElementParamEntry.isMagicCorrect_byMagic, attackElementParamEntry.isMagicCorrect_byFire, attackElementParamEntry.isMagicCorrect_byThunder, attackElementParamEntry.isMagicCorrect_byDark],
    [attackElementParamEntry.isFaithCorrect_byPhysics, attackElementParamEntry.isFaithCorrect_byMagic, attackElementParamEntry.isFaithCorrect_byFire, attackElementParamEntry.isFaithCorrect_byThunder, attackElementParamEntry.isFaithCorrect_byDark],
    [attackElementParamEntry.isLuckCorrect_byPhysics, attackElementParamEntry.isLuckCorrect_byMagic, attackElementParamEntry.isLuckCorrect_byFire, attackElementParamEntry.isLuckCorrect_byThunder, attackElementParamEntry.isLuckCorrect_byDark]]
    weapon.correctStat = [paramEntry.correctStrength, paramEntry.correctAgility, paramEntry.correctMagic, paramEntry.correctFaith, paramEntry.correctLuck]



def calc_ar(weapon_input, char_stats, is_two_handed):
    ar_calcs = [0, 0, 0, 0, 0]
    for i in range(5):
        damage = weapon_input.attackBaseElement[i] * weapon_input.elementAttackRate[i]
        ar_calcs[i] += damage
        if damage == 0: continue

        for j in range(5):
            if weapon_input.isStatCorrectByElement[j][i]: #j is stat, i is element(in order)
                stat = char_stats[j]
                if j == 0 and not weapon_input.isDualBlade and is_two_handed: stat = int(stat*1.5)
                curve_value = float(weapon_input.saturationIndex[i][stat-1])/100
                scaling_coef = weapon_input.correctStat[j]
                reinforce_stat_mod = weapon_input.correctStatRate[j]/100

                ar_calcs[i] += damage * scaling_coef * curve_value * reinforce_stat_mod

    return ar_calcs

coefficients = [(-0.8/121), (-1.6/121), (-0.4/3), (-0.8/3), (19.2/49), (38.4/49)]

def calculate_defense_reduction(ratio):
    damage = 1
    if 8 >= ratio > 2.5:
        val = ratio - 8
        index = 0
        square = (coefficients[2 * index] * pow(val, 2) + 0.9)
        damage = square
    if 2.5 >= ratio > 1:
        val = ratio - 2.5
        index = 1
        square = (coefficients[2 * index] * pow(val, 2) + 0.7)
        damage = square
    if 1 >= ratio > .125:
        val = ratio - 0.125
        index = 2
        square = (coefficients[2 * index] * pow(val, 2) + 0.1)
        damage = square
    if .125 >= ratio:
        damage = .1
    if 8 < ratio:
        damage = .9
    return damage

def calculate_damage(weapon, stats, defs, is_two_handed):
    ars = calc_ar(weapon, stats, is_two_handed)
    damage = 0
    for i in range(5):
        damage += calculate_defense_reduction(ars[i]/defs[i]) * ars[i]
    return int(damage)

#if __name__ == '__main__':
#    numpy.set_printoptions(legacy='1.25') #makes cleaner printouts
#
#    load_data()
#    broadsword = Weapon(2020000, 400, 13)
#    load_weapon(broadsword) #fire is infusion id 400
#    stats = [15, 12, 26, 33, 60] #str dex int fth arc
#    ar = calc_ar(broadsword, stats, True)
#    print(broadsword.name + " AR:", ar)

#    defs = [122, 78, 111, 152, 189]
#    damage = calculate_damage(broadsword, stats, defs, True)
#    print(broadsword.name + " Defense Test:", damage)
#    codedsword = Weapon(2110000, 000, 17)
#    load_weapon(codedsword)
    




