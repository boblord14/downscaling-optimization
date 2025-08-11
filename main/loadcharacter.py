import sys
sys.path.insert(0,'../')
import attackAndDefense as ad
import json
import pandas as pd
from dataclasses import dataclass
from pandas import Series

magic = pd.read_csv('../../csv-conversions/magic.csv')
swordArtsParam = pd.read_csv('../../csv-conversions/SwordArtsParam.csv')

def getFPAshOfWar(name):
    fp = []
    for i in range(0, swordArtsParam['Name'].size):
        if swordArtsParam['Name'][i] == name:
            params = swordArtsParam.iloc[[i]]
            l1 = int(params['useMagicPoint_L1'])
            if l1 != -1 and l1 != 0:
                fp.append(l1)
            l2 = int(params['useMagicPoint_L2'])
            if  l2 != -1 and l2 != 0:
                fp.append(l2)
            r1 = int(params['useMagicPoint_R1'])
            if  r1 != -1 and r1 != 0:
                fp.append(r1)
            r2 = int(params['useMagicPoint_R2'])
            if  r2 != -1 and r2 != 0:
                fp.append(r2)
            break
    return fp

def getFPSpell(name):
    fp = -1
    for i in range(0, magic['Name'].size):
        magic_name = magic['Name'][i].lower() 
        if name.lower() in magic_name:
            params = magic.iloc[[i]]
            fp = int(params['mp'])
            #print(magic_name)
            #print(fp)
            #print(int(params['mp_charge']))
            break
    return fp

class Character():
    weapons: list[ad.Weapon]
    ashes: list[int]
    armor: list[int]
    spells: list[int]
    starting_class: str
    upgrade_level: int
    
    def __init__(self, weapons, armor, ashes, spells, starting_class, upgrade_level):
        self.weapons = weapons
        self.armor = armor
        self.ashes = ashes
        self.spells = spells
        self.starting_class = starting_class
        self.upgrade_level = upgrade_level

def loadCharacter(f):
    with open(f) as data_file:
        data = json.load(data_file)
        starting_class = data['characterClass']
        weapons = []
        ashes = []
        spells = []
        armor =[]
        for w in data['inventory']['slots']:
            id = int(w['weapon_hex_id'],16)
            print(id)
            infusion = int(w['affinity_hex_id'],16)%1000
            print(infusion)
            upgrade_level = int(w['upgrade_hex_id'],16)
            temp = ad.Weapon(id, infusion, upgrade_level)
            ad.load_weapon(temp)
            weapons.append(temp)
            ash_name = w['weaponArt']
            print(ash_name)
            fp = getFPAshOfWar(ash_name)
            for f in fp:
                ashes.append(f)
        for s in data['spells']['slots']:
            spell_name = s['name']
            print(spell_name)
            spells.append(getFPSpell(spell_name))
        for a in data['protectors']['head']['slots']:
            if a['order'] == 0:
                armor.append(int(a['full_hex_id'],16))
        for a in data['protectors']['body']['slots']:
            if a['order'] == 0:
                armor.append(int(a['full_hex_id'],16))
        for a in data['protectors']['arms']['slots']:
            if a['order'] == 0:
                armor.append(int(a['full_hex_id'],16))
        for a in data['protectors']['legs']['slots']:
            if a['order'] == 0:
                armor.append(int(a['full_hex_id'],16))
        
        return Character(weapons, armor, ashes, spells, starting_class, int(data['weaponUpgrade']))


ad.load_data()
c = loadCharacter('Noj.json')
print(c.weapons)
print(c.armor)
print(c.ashes)
print(c.spells)
        
