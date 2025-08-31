import sys
sys.path.insert(0,'../')
import numpy as np
import math
import attackAndDefense as ad
import scipy as sci
import json
import pandas as pd
import copy
from os import listdir
from os.path import isfile, join
from tensorflow.keras.saving import load_model
from queue import PriorityQueue
import argparse
import time

magic = pd.read_csv('csv-conversions/Magic.csv')
swordArtsParam = pd.read_csv('csv-conversions/SwordArtsParam.csv')
equipParamProtect = pd.read_csv('csv-conversions/equipParamProtector.csv')
equipParamProtect['Name'] = equipParamProtect['Name'].astype('str')

mind = []
with open("Mind.txt") as f:
  for line in f:
    temp = line.split()
    mind.append(int(temp[1]))
best_ehp_90 = {}
with open('bestehp90.txt' , 'r') as f:
  for line in f:
    temp = line.split()
    best_ehp_90[temp[0]] = float(temp[1])
vig_scale = []
poise_bp = []
equip_load_scale = []
with open("vigor.txt") as f:
  for line in f:
    vig_scale = line.split()
with open("equipload.txt") as f:
  for line in f:
    equip_load_scale = line.split()
with open("poisebreakpoints.txt") as f:
  for line in f:
    poise_bp = line.split()

poise_bp =list(set(poise_bp))
for i in range(0, len(poise_bp)):
  poise_bp[i] = float(poise_bp[i])
poise_bp.sort()

for i in range(0, len(vig_scale)):
  vig_scale[i] = float(vig_scale[i])

for i in range(0, len(equip_load_scale)):
  equip_load_scale[i] = float(equip_load_scale[i])
    
logisitic_head = []
logistic_chest = []
logistic_arm = []
logistic_leg = []
poise_head = []
poise_chest = []
poise_arm = []
poise_leg = []
with open('datafit.txt' , 'r') as file:
    lines = file.readlines()
    values = lines[0].split()
    for i in range(0,len(values)):
        values[i] = float(values[i])
    logistic_head = values
    values = lines[1].split()
    for i in range(0,len(values)):
        values[i] = float(values[i])
    logistic_chest = values
    values = lines[2].split()
    for i in range(0,len(values)):
        values[i] = float(values[i])
    logistic_arm = values
    values = lines[3].split()
    for i in range(0,len(values)):
        values[i] = float(values[i])
    logistic_leg = values
    values = lines[4].split()
    for i in range(0,len(values)):
        values[i] = float(values[i])
    poise_head = values
    values = lines[5].split()
    for i in range(0,len(values)):
        values[i] = float(values[i])
    poise_chest = values
    values = lines[6].split()
    for i in range(0,len(values)):
        values[i] = float(values[i])
    poise_arm = values
    values = lines[7].split()
    for i in range(0,len(values)):
        values[i] = float(values[i])
    poise_leg = values

starting_classes = {
  "Hero": [14,9,12,16,9,7,9,11],
  "Bandit": [10,11,10,9,13,9,8,14],
  "Astrologer" : [9, 15, 9,8,12,16,7,9],
  "Warrior" : [11,12,11,10,16,10,8,9],
  "Prisoner" : [11,12,11,11,14,14,6,9],
  "Confessor" : [10,13,10,12,12,9,14,9],
  "Wretch" : [10,10,10,10,10,10,10,10],
  "Vagabond" : [15,10,11,14,13,9,9,7],
  "Prophet" : [10,14,8,11,10,7,16,10],
  "Samurai" : [12,11,13,12,15,9,8,8]
}


starting_classes_negations = {
}

starting_classes_negations_greatjar = {
}


starting_class_index = {
  "Hero": 0,
  "Bandit": 1,
  "Astrologer" : 2,
  "Warrior" : 3,
  "Prisoner" : 4,
  "Confessor" : 5,
  "Wretch" : 6,
  "Vagabond" : 7,
  "Prophet" : 8,
  "Samurai" : 9
}


def equipWeight(name):
  weight = -1
  for i in range(0, equipParamProtect['Name'].size):
    params = equipParamProtect.iloc[[i]]
    search = params['Name'].str.cat()
    if name in search:
      weight =  float(params['weight'].iloc[0])
      break
  return weight

def equipPoise(name):
  poise = -1
  for i in range(0, equipParamProtect['Name'].size):
    params = equipParamProtect.iloc[[i]]
    search = params['Name'].str.cat()
    if name in search:
      poise = 1000 * float(params['toughnessCorrectRate'].iloc[0])
      break
  return poise

def MaxFp(total_stats, starting_class):
  index = starting_classes[starting_class][1] + total_stats
  if index > 99:
    index = 99;
  return mind[index - 1]

def maxPoise():
  max_head = -1
  max_chest = -1
  max_arm = -1
  max_leg = -1
  for i in range(0, equipParamProtect['Name'].size):
    params = equipParamProtect.iloc[[i]]
    name = params['Name'].str.cat()
    if int(params['headEquip'].iloc[0]) == 1:
      temp = float(params['toughnessCorrectRate'].iloc[0])
      if temp > max_head:
        max_head = temp
    if int(params['bodyEquip'].iloc[0]) == 1:
      temp = float(params['toughnessCorrectRate'].iloc[0])
      if temp > max_chest:
        max_chest = temp
    if int(params['armEquip'].iloc[0]) == 1:
      temp = float(params['toughnessCorrectRate'].iloc[0])
      if temp > max_arm:
        max_arm = temp
    if int(params['legEquip'].iloc[0]) == 1:
      temp = float(params['toughnessCorrectRate'].iloc[0])
      if temp > max_leg:
        max_leg = temp
  return round(1000*(max_head + max_chest + max_arm + max_leg)/.75)

def LogisticFunction(L,k,x0,x):
    return L / (1 + np.exp(-k*(x - x0)))

def Logistic(var, x):
    return LogisticFunction(var[0], var[1], var[2],x)
def negationsPoise(el, armor_frac, armor_perc, bullgoat):
  armor = el * armor_perc
  neg_head = (1 - Logistic(logistic_head, armor * armor_frac[0]))
  neg_chest = (1 - Logistic(logistic_chest, armor * armor_frac[1]))
  neg_arm = (1 - Logistic(logistic_arm, armor * armor_frac[2]))
  neg_leg = (1 - Logistic(logistic_leg, armor * armor_frac[3]))
  neg = 1 - neg_head * neg_chest * neg_arm * neg_leg
  ph =(poise_head[0] * armor * armor_frac[0] + poise_head[1])
  if ph <= 0:
    ph = 0
  pc = (poise_chest[0] * armor * armor_frac[1] + poise_chest[1])
  if pc <= 0:
    pc = 0;
  pa = (poise_arm[0] * armor * armor_frac[2] + poise_arm[1])
  if pa <= 0:
    pa = 0
  pl = (poise_leg[0] * armor * armor_frac[3] + poise_leg[1])
  if pl <= 0:
    pl = 0;
  p = ph + pc + pa + pl
  if bullgoat == True:
    p = p / .75
  if p > 133:
    p = 133
  return [neg, p]

def effectiveHealth(base_vig,base_end, armor_perc, armor_frac, stat_alloc, bullgoat, greatjar):
  els = []
  hps = []
  negations = []
  poise = []
  effective_hp = []
  best_break_points = [[-1,-1, -1,-1]]*len(poise_bp)
  poise_threshold = 100/.75
  i = 0
  while i <= stat_alloc:
    hp_index = i + base_vig - 1
    if hp_index >= 99:
      hp_index = 98
    hp = vig_scale[hp_index]
    hps.append(hp)
    end_index = stat_alloc - i + base_end - 1
    if end_index >= 99:
      end_index = 98
    el = equip_load_scale[end_index]
    if greatjar == True:
      el = el * 1.19
    vals = negationsPoise(el, armor_frac, armor_perc, bullgoat)
    p = vals[1]
    neg = vals[0]
    els.append(el)
    poise.append(p)
    negations.append(neg)
    ehp = hp / (1 - neg)
    effective_hp.append(ehp)
    for j in range(0,len(poise_bp)):
      if p >= poise_bp[j] and ehp >= best_break_points[j][0]:
        best_break_points[j] = [ehp, p, hp_index + 1 - base_vig, end_index + 1-base_end]
    i += 1
  while (best_break_points[-1][0] ==-1):
    best_break_points.pop()
  return best_break_points

def fpToMind(fp):
  for i in range(0,len(mind)):
    if mind[i] == fp:
      return i+1
  return -1
def bestNegations(equipload):
  eq = .69 * equipload
  def obj(x):
    neg_head = (1 - Logistic(logistic_head, x[0]))
    neg_chest = (1 - Logistic(logistic_chest, x[1]))
    neg_arm = (1 - Logistic(logistic_arm, x[2]))
    neg_leg = (1 - Logistic(logistic_leg, x[3]))
    neg = 1 - neg_head * neg_chest * neg_arm * neg_leg
    return -neg
  linear = sci.optimize.LinearConstraint([1,1,1,1], [eq], [eq])
  bounds = sci.optimize.Bounds([0], [np.inf])
  x0 = np.array([eq/4] * 4)
  res = sci.optimize.minimize(obj, x0, method='trust-constr', constraints=[linear], bounds=bounds)
  return [res.x, -obj(res.x)]

def bestEhp(stat_alloc, starting_class, greatjar):
  base_vig = starting_classes[starting_class][0]
  base_end = starting_classes[starting_class][2]
  els = []
  hps = []
  negations = []
  poise = []
  effective_hp = []
  larger = False
  for i in range(0,stat_alloc):
    hp_index = i + base_vig - 1
    if hp_index >= 99:
      hp_index = 98
    hp = vig_scale[hp_index]
    hps.append(hp)
    end_index = stat_alloc - i + base_end - 1
    if end_index >= 99:
      end_index = 98
    el = equip_load_scale[end_index]
    if greatjar == True:
      el = el * 1.19
    els.append(el)
    neg = -1
    precompute = False
    if greatjar == False and starting_class in starting_classes_negations:
      if i < len(starting_classes_negations[starting_class]):
        neg = starting_classes_negations[starting_class][i]
        precompute = True
    elif greatjar == True and starting_class in starting_classes_negations_greatjar:
      if i < len(starting_classes_negations_greatjar[starting_class]):
        neg = starting_classes_negations_greatjar[starting_class][i]
        precompute = True    
    if precompute == False:
      larger = True
      neg = bestNegations(el)[1]
    negations.append(neg)
    effective_hp.append(hp / (1-neg))
  if (greatjar == False and starting_class not in starting_classes_negations) or larger == True:
    starting_classes_negations[starting_class] = negations
  if (greatjar == True and starting_class not in starting_classes_negations_greatjar) or larger == True:
    starting_classes_negations_greatjar[starting_class] = negations
  return max(effective_hp)

def Project(x, total_stats):
    plane = np.array([1.0]* len(x))
    np_x = np.array(x)
    plane_dot = np.dot(plane,plane)
    plane /= np.sqrt(plane_dot)
    x_sum = np.dot(np_x, plane)
    y = np.array(x)
    y = y - x_sum * plane
    alpha = total_stats / plane_dot
    y = y + alpha * np.sqrt(plane_dot) * plane
    pos = True
    for i in range(0,len(x)):
      if y[i] <= 1 and np.abs(y[i]- 1.0) > 1e-4:
        y[i] = 1
        pos = False
        break
    if pos == True:
      return y
    return Project(y, total_stats)


def RescaleClasses():
  for name, stats in starting_classes.items():
    s_float = Project(stats, 79)
    s_int = []
    sum = 0
    for s in s_float:
      i = int(s)
      sum += i
      s_int.append(i)
    if sum != 79:
      remainder = 79 - sum
      while remainder > 0:
        max = -1
        used = []
        index = -1
        for i in range(0, len(s_int)):
          re = s_float[i]-s_int[i];
          if re > max and i not in used:
            max = re
            index = i
        used.append(index)
        s_int[index]+=1
        remainder -=1
    starting_classes[name] = s_int

RescaleClasses()

  
def getMaxFPAshOfWar():
    fp = []
    for i in range(0, swordArtsParam['Name'].size):
        params = swordArtsParam.iloc[[i]]
        l1 = int(params['useMagicPoint_L1'].iloc[0])
        if l1 != -1 and l1 != 0:
            fp.append(l1)
        l2 = int(params['useMagicPoint_L2'].iloc[0])
        if  l2 != -1 and l2 != 0:
            fp.append(l2)
        r1 = int(params['useMagicPoint_R1'].iloc[0])
        if  r1 != -1 and r1 != 0:
            fp.append(r1)
        r2 = int(params['useMagicPoint_R2'].iloc[0])
        if  r2 != -1 and r2 != 0:
            fp.append(r2)
    return max(fp)

def getMaxFpSpell():
    fp = []
    for i in range(0, magic['Name'].size):
        params = magic.iloc[[i]]
        fp.append(int(params['mp'].iloc[0]))
        
    return max(fp)

def getFPAshOfWar(name):
    fp = []
    for i in range(0, swordArtsParam['Name'].size):
        if swordArtsParam['Name'][i] == name:
            params = swordArtsParam.iloc[[i]]
            l1 = int(params['useMagicPoint_L1'].iloc[0])
            if l1 != -1 and l1 != 0:
                fp.append(l1)
            l2 = int(params['useMagicPoint_L2'].iloc[0])
            if  l2 != -1 and l2 != 0:
                fp.append(l2)
            r1 = int(params['useMagicPoint_R1'].iloc[0])
            if  r1 != -1 and r1 != 0:
                fp.append(r1)
            r2 = int(params['useMagicPoint_R2'].iloc[0])
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
            fp = int(params['mp'].iloc[0])
            break
    return fp

class Character():
    weapons: list[ad.Weapon]
    ashes: list[int]
    armor: list[str]
    spells: list[int]
    starting_class: str
    upgrade_level: int
    effective_health : float
    level : int
    vigor : int
    end : int
    mind : int
    damage_stats: list[int]
    bullgoat : bool
    greatjar: bool
    poise : float
    
    def __init__(self,level, vigor, end, mind, damage_stats, weapons, armor, ashes, spells, starting_class, upgrade_level, effective_health, poise, bullgoat, greatjar):
        self.level = level
        self.vigor = vigor
        self.end = end
        self.mind = mind
        self.damage_stats = damage_stats
        self.weapons = weapons
        self.armor = armor
        self.ashes = ashes
        self.spells = spells
        self.starting_class = starting_class
        self.upgrade_level = upgrade_level
        self.effective_health = effective_health
        self.poise = poise
        self.bullgoat = bullgoat
        self.greatjar = greatjar

def loadCharacter(f):
    with open(f) as data_file:
        data = json.load(data_file)
        level = int(data['stats']['rl'])
        vigor = int(data['stats']['vig'])
        mind = int(data['stats']['mnd'])
        end = int(data['stats']['vit'])
        damage_stats = []
        damage_stats.append(int(data['stats']['str']))
        damage_stats.append(int(data['stats']['dex']))
        damage_stats.append(int(data['stats']['int']))
        damage_stats.append(int(data['stats']['fth']))
        damage_stats.append(int(data['stats']['arc']))
        starting_class = data['characterClass']
        weapons = []
        ashes = []
        spells = []
        armor = [None] * 4
        poise = -1
        if 'poise' not in data['computed']:
          poise = 0
        elif 'altered' in data['computed']['poise']:
          poise = float(data['computed']['poise']['altered'])
        else:
          poise = float(data['computed']['poise']['original'])
        for w in data['inventory']['slots']:
            id = int(w['weapon_hex_id'],16)
            infusion = int(w['affinity_hex_id'],16)%1000
            upgrade_level = int(w['upgrade_hex_id'],16)
            temp = ad.Weapon(id, infusion, upgrade_level)
            ad.load_weapon(temp)
            weapons.append(temp)
            if 'weaponArt' in w:
              ash_name = w['weaponArt']
              fp = getFPAshOfWar(ash_name)
              for f in fp:
                ashes.append(f)
        for s in data['spells']['slots']:
            spell_name = s['name']
            spells.append(getFPSpell(spell_name))
        for a in data['protectors']['head']['slots']:
          if 'equipIndex' in a:
            armor[0] = a['name']
            break
        for a in data['protectors']['body']['slots']:
          if 'equipIndex' in a:
            armor[1] = a['name']
            break
        for a in data['protectors']['arms']['slots']:
          if 'equipIndex' in a:
            armor[2] = a['name']
            break
        for a in data['protectors']['legs']['slots']:
          if 'equipIndex' in a:
            armor[3] = a['name']
            break
        bullgoat = False
        greatjar = False
        for t in data['talismans']['slots']:
          if 'equipIndex' in t:
            if 'Bull-Goat' in t['name']:
              bullgoat = True
            if 'Great-Jar' in t['name']:
              greatjar = True
        effective_health  = vig_scale[vigor - 1]
        neg = 0
        neg += float(data['computed']['absorption']['physical'])
        neg += float(data['computed']['absorption']['slash'])
        neg += float(data['computed']['absorption']['pierce'])
        neg += float(data['computed']['absorption']['strike'])
        neg += float(data['computed']['absorption']['magic'])
        neg += float(data['computed']['absorption']['fire'])
        neg += float(data['computed']['absorption']['lightning'])
        neg += float(data['computed']['absorption']['holy'])
        neg /= 800
        effective_health /= (1 - neg)
        return Character(level, vigor, end, mind, damage_stats, weapons, armor, ashes, spells, starting_class, upgrade_level, effective_health, poise, bullgoat, greatjar)

def grid(level, delta):
  count = 0;
  n = int(level/delta) + 1
  output = []
  for i in range(0,n):
    if i * delta <= level:
      x = i * delta
      for j in range(0,n):
        if x + j * delta <= level:
          y = j * delta
          count = x + y
          z = level - count
          output.append([x,y,z])
  return output
      
      
def exponentialDecay(character, vec, stride):
  base_vig = starting_classes[character.starting_class][0]
  base_mind = starting_classes[character.starting_class][1]
  base_end = starting_classes[character.starting_class][2]
  max_fp = MaxFp(character.level, character.starting_class)
  eq = equip_load_scale[character.end - 1]
  max_poise = maxPoise()
  if character.greatjar == True:
    eq = eq * 1.19
  armor_frac = []
  armor_perc = 0
  weights = []
  for i in range(0,4):
    weight = 0
    if character.armor[i] != None:
      weight = equipWeight(character.armor[i])
    weights.append(weight)
    armor_perc += weight
  for i in range(0,4):
    if armor_perc != 0:
      armor_frac.append(weights[i] / armor_perc)
    else:
      armor_frac.append(0)
  armor_perc /= eq
  vals = negationsPoise(eq, armor_frac, armor_perc, character.bullgoat)
  neg = vals[0]
  p = vals[1]
  best_ehp = bestEhp(character.level, character.starting_class, True)
  starting_ehp = character.effective_health / best_ehp
  ehp_stats = character.vigor - base_vig + character.end - base_end
  temp_ehps = effectiveHealth(base_vig, base_end, armor_perc, armor_frac, ehp_stats, character.bullgoat, character.greatjar)
  vec[3] = temp_ehps[0][0] / best_ehp
  vec[len(vec) - 2] = temp_ehps[0][2] / character.level
  vec[len(vec) - 1] = temp_ehps[0][3] / character.level
  damage_stats = 0
  for i in range(3,8):
    index = i - 3;
    damage_stats += character.damage_stats[index] - starting_classes[character.starting_class][i]
  m = character.mind - starting_classes[character.starting_class][1]
  level = character.level
  delta = stride
  mesh = grid(level, delta)
  output = []
  #enough poise not to be bullied by daggers
  vec[3] = 58 / max_poise
  output.append(vec)
  for g in mesh:
    out = copy.deepcopy(vec)
    #fp
    mind_index = starting_classes[character.starting_class][1] + g[2] - 1
    if mind_index >= 99:
      mind_index = 98
    out[4] = mind[mind_index] / max_fp
    #number of damage stats
    out[9] = int(g[0]) / character.level
    temp_ehps = effectiveHealth(base_vig, base_end, armor_perc, armor_frac, int(g[1]), character.bullgoat, character.greatjar)
    dist = np.abs(g[0] - damage_stats) + np.abs(g[1] - ehp_stats) + np.abs(g[2] - m)
    score = np.exp(-dist / 100)
    if score < .5:
      continue
    out[2] = temp_ehps[0][0] / best_ehp
    #out[3] = temp_ehps[i][1] / max_poise
    out[len(out) - 2] = temp_ehps[0][2] / character.level
    out[len(out) - 1] = temp_ehps[0][3] / character.level
    out[0] = score
    output.append(copy.deepcopy(out))
  return output

# [score, character_level, ehp, poise, fp, average ash fp cost, average spell cost, max spell cost, number of spells, number of damage stats, starting class one hot encoding, greatjar, vigor, end]
def baseVec(character):
  base_vig = starting_classes[character.starting_class][0]
  base_end = starting_classes[character.starting_class][2]
  ehp_stats = character.vigor - base_vig + character.end - base_end
  best_ehp = bestEhp(character.level, character.starting_class, True)
  norm_ehp = character.effective_health / best_ehp
  output = [1, character.level /90, norm_ehp]
  max_poise = maxPoise()
  output.append(character.poise / max_poise)
  output.append(mind[character.mind - 1] / MaxFp(character.level, character.starting_class))
  fp = 0
  for i in range(0, len(character.ashes)):
    fp += character.ashes[i]
  if len(character.ashes) > 0:
    fp /= len(character.ashes)
  else:
    fp = 0
  output.append(fp / getMaxFPAshOfWar())
  fp = 0
  for i in range(0, len(character.spells)):
    fp += character.spells[i]
  max_fp = 0  
  if len(character.spells) > 0:
    max_fp = max(character.spells)
    fp /= len(character.spells)
  max_fp_spell = getMaxFpSpell()
  output.append(fp / max_fp_spell)
  output.append(max_fp / max_fp_spell)
  output.append(len(character.spells) / 13)
  damage_stats = 0
  for i in range(3,8):
    index = i - 3;
    damage_stats += character.damage_stats[index] - starting_classes[character.starting_class][i]
  output.append(damage_stats / character.level)
  #damage = -1
  #output.append(damage)
  index = starting_class_index[character.starting_class]
  for i in range(0,len(starting_class_index)):
    if i == index:
      output.append(1)
    else:
      output.append(0)
  #if character.bullgoat == True:
  #  output.append(1)
  #else:
  #output.append(0)
  if character.greatjar == True:
    output.append(1)
  else:
    output.append(0)
  output.append(character.vigor / character.level)
  output.append(character.end / character.level)
  
  return output

def PrepareData(character, stride):
  if character.starting_class == None:
    return [[]]
  output = baseVec(character)
  labeled_data = exponentialDecay(character, output, stride)  
  return labeled_data

def damageStatAllocation(character, damage_stats):
  ds = damage_stats
  #for i in range(3,8):
    #ds += starting_classes[character.starting_class][i]
  a,ds = math.modf(ds / 5)
  if a > 0:
    ds = 5*(ds+1)
  index = int(ds / 5 - 1)
  scaling = []
  opts = []
  for w in character.weapons:
    file = join('Database', str(w.id + w.infusion)+'.txt')
    ar = []
    with open(file) as f:
      for line in f:
        temp = line.split()
        for i in range(0,len(temp)):
          temp[i] = float(temp[i])
        ar.append(temp)
    array = np.array(ar)
    scaling.append(array[index,0])
    opts.append(array[index,1:len(ar[0])])
  opts = np.array(opts)
  opts = np.transpose(opts)
  U, S, Vh = np.linalg.svd(opts)
  dir = np.abs(U[:,0])
  u = copy.deepcopy(dir)
  sum = 0
  for i in range(0,len(u)):
    if u[i] < .1:
      u[i] = 0
    sum += u[i]
  alpha = damage_stats / sum
  whole = []
  frac = []
  sum = 0
  for i in range(0,len(u)):
    f,w = math.modf(alpha * u[i])
    sum += w
    whole.append(w)
    frac.append(f)
  remainder = damage_stats - sum
  while remainder > 0:
    index = -1
    biggest = -1
    for i in range(0, len(frac)):
      if frac[i] > biggest:
        biggest = frac[i]
        index = i
    whole[index] += 1
    frac[index] = 0
    remainder -= 1
  stats = [None]*5
  for i in range(0, len(stats)):
    stats[i] = starting_classes[character.starting_class][i + 3] + whole[i]
    stats[i] = int(stats[i])
  sum = 0
  for i in range(0,len(character.weapons)):
    w = ad.Weapon(character.weapons[i].id, character.weapons[i].infusion, 17)
    ad.load_weapon(w)
    d = ad.calculate_damage(w, stats, [140] * 5, False)
    o = list(opts[:,i])
    for j in range(0,len(o)):
      o[j] = int(o[j])
    #s = ad.calculate_damage(w, o, [140]*5, False)
    #d /= s
    #sum += d
    ars = ad.calc_ar(w, stats, False)
    print('%r with infusion %r:  ar is [Physical %r, Magic %r, Fire %r, Lightning %r, Holy %r]' % (w.name, w.infusion, ars[0],ars[1],ars[2],ars[3],ars[4]))
    #print('Weapon %r is at %r efficiency'%(character.weapons[i].name, d))
  #print('Average efficiency is %r' % (sum / len(character.weapons)))
  return stats
    
def rank(builds, model, level, character, num_builds):
  pq = PriorityQueue()
  data = np.array(builds)
  data = data[:,1:len(data[0,:])]
  scores = model.predict(data)
  for i in range(0, len(scores)):
    if pq.qsize() < num_builds:
      pq.put((scores[i], data[i,:]))
    else:
      top = pq.get()
      if top[0] > scores[i]:
        pq.put(top)
      else:
        pq.put((scores[i], data[i,:]))
  best_ehp = bestEhp(level, character.starting_class, True)
  max_fp = MaxFp(level, character.starting_class)
  while not pq.empty():
    result = pq.get()
    print(result[0])
    vec = result[1]
    n = len(vec)
    m = fpToMind(round(vec[3] * max_fp)) - starting_classes[character.starting_class][1]
    vig = round(vec[n-2] * level)
    end = round(vec[n-1] * level)
    damage_stats = round(vec[8] * level)
    print('EHP is %r. vigor invested is %r. endurance invested is %r. mind is %r. Number of Damage Stats is %r.' % (vec[1] * best_ehp, vig, end, m, damage_stats))
    stats = damageStatAllocation(character, damage_stats)
    print('vigor %r, mind %r, end %r, str %r, dex %r, int %r, faith %r, arc %r'%(starting_classes[character.starting_class][0] + vig, starting_classes[character.starting_class][1] + m, starting_classes[character.starting_class][2] + end, stats[0],stats[1],stats[2],stats[3],stats[4]))                      

ad.load_data()



def WriteTrainData():
  train_path = 'test90//test90'
  train_files = [f for f in listdir(train_path) if isfile(join(train_path, f))]
  output = []
  iter = 0
  with open("labeled_data.txt", "w") as file:
    for f in train_files:
      print(f)
      char = loadCharacter(join(train_path,f))
      o = PrepareData(char, 1)
      for build in o:
        n = len(build)
        for x in range(0,n):
          file.write(str(build[x]))
          if x == (n-1):
            file.write('\n')
          else:
            file.write(',')

def createBuilds(character, level, stride):
  vec = baseVec(character)
  vec[1] = 1
  base_vig = starting_classes[character.starting_class][0]
  base_end = starting_classes[character.starting_class][2]
  armor_frac = []
  armor_perc = 0
  eq = equip_load_scale[character.end - 1]
  max_poise = maxPoise()
  if character.greatjar == True:
    eq = eq * 1.19
  weights = []
  
  for i in range(0,4):
    weight = 0
    if character.armor[i] != None:
      weight = equipWeight(character.armor[i])
    weights.append(weight)
    armor_perc += weight
  for i in range(0,4):
    if armor_perc != 0:
      armor_frac.append(weights[i] / armor_perc)
    else:
      armor_frac.append(0)
  armor_perc /= eq
  max_poise = maxPoise()
  vec[3] = 58 / max_poise
  max_fp = MaxFp(level, character.starting_class)
  max_poise = maxPoise()
  best_ehp = best_ehp_90[character.starting_class]
  mesh = grid(level, stride)
  output = []
  ehp_map = {}
  for g in mesh:
    out = copy.deepcopy(vec)
    #fp
    mind_index = starting_classes[character.starting_class][1] + g[2] - 1
    if mind_index >= 99:
      mind_index = 98
    out[4] = mind[mind_index] / max_fp
    #number of damage stats
    out[9] = int(g[0]) / level
    if int(g[1]) in ehp_map:
      temp_ehps = ehp_map[int(g[1])]
    else:
      temp_ehps = effectiveHealth(base_vig, base_end, armor_perc, armor_frac, int(g[1]), character.bullgoat, character.greatjar)
      ehp_map[int(g[1])] = temp_ehps
    out[2] = temp_ehps[0][0] / best_ehp
    #out[3] = temp_ehps[i][1] / max_poise
    out[len(out) - 2] = temp_ehps[0][2] / level
    out[len(out) - 1] = temp_ehps[0][3] / level
    out[0] = 1
    output.append(copy.deepcopy(out))
  return output

            
def Test(json_file, num_builds):
  model = load_model("model.keras")
  char = loadCharacter(json_file)
  tic = time.perf_counter()
  o = createBuilds(char, 90, 5)
  toc = time.perf_counter()
  print(f"Grid Search of Builds in {toc - tic:0.4f} seconds")
  tic = time.perf_counter()
  rank(o, model, 90, char, num_builds)
  toc = time.perf_counter()
  print(f"Found top two builds in {toc - tic:0.4f} seconds")
#WriteTrainData()  

#with open('bestehp90.txt' , 'w') as file:
#  for key,value in starting_classes.items():
#    best_ehp = bestEhp(90, key, True) 
#    file.write(key)
#    file.write(' ')
#    file.write(str(best_ehp))
#    file.write('\n')

if __name__=="__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument('--json_file', required=True, help='Json file of build to convert to level 90')
  parser.add_argument('--top_k', default=2, help='Number of protential builds to output, default is 2')
  args = parser.parse_args()
  Test(args.json_file, args.top_k)
