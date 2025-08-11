import numpy as np
import numpy.matlib
import json
import matplotlib.pyplot as plt
from sklearn import linear_model
from timeit import default_timer as timer

f = open('../../json-data/EldenRingArmor.json')
data = json.load(f)

armor_map = {'head' : 0, 'chest' : 1, 'hands' : 2, 'legs' : 3}
abs_map = {"physical" : 0,
            "strike" : 1,
            "slash" : 2,
            "thrust" : 3,
            "magic" : 4,
            "fire" : 5,
            "lightning" : 6,
            "holy" : 7}
armor = [[],[],[],[]]


for entry in data:
  if entry['poise'] == None:
    continue
  armor[armor_map[entry['slot']]].append(entry)

num_head = len(armor[0])
num_chest = len(armor[1])
num_arm = len(armor[2])
num_leg = len(armor[3])

num_armor = num_head + num_chest + num_arm + num_leg

name_matrix = []
for i in range(0, num_armor):
  name_matrix.append("None")
armor_matrix = np.matlib.zeros((8, num_armor))
poise_matrix = np.matlib.zeros((1, num_armor))
weight_matrix = np.matlib.zeros((1, num_armor))
immunity_matrix = np.matlib.zeros((1, num_armor))
robustness_matrix = np.matlib.zeros((1, num_armor))
focus_matrix = np.matlib.zeros((1, num_armor))
vitality_matrix = np.matlib.zeros((1, num_armor))

head_idx = 0
chest_idx = num_head
arm_idx = chest_idx + num_chest
leg_idx = arm_idx + num_arm

idx_map = {'head' : head_idx, 'chest' : chest_idx, 'hands' : arm_idx, 'legs' : leg_idx}

for entry in data:
  idx = idx_map[entry['slot']]
  for abs in entry['abs']:
    val = entry['abs'][abs]
    armor_matrix.A[abs_map[abs]][idx] = val
  name_matrix[idx] = entry['name']
  poise_matrix.A[0][idx] = entry['poise']
  weight_matrix.A[0][idx] = entry['weight']
  immunity_matrix.A[0][idx] = entry['res']['immunity']
  robustness_matrix.A[0][idx] = entry['res']['robustness']
  focus_matrix.A[0][idx] = entry['res']['focus']
  vitality_matrix.A[0][idx] = entry['res']['vitality']

  idx_map[entry['slot']] += 1

vit_scale = []
poise_bp = []
equip_load_scale = []
with open("vigor.txt") as f:
  for line in f:
    vit_scale = line.split()
with open("equipload.txt") as f:
  for line in f:
    equip_load_scale = line.split()
with open("poisebreakpoints.txt") as f:
  for line in f:
    poise_bp = line.split()
print(vit_scale)
print(equip_load_scale)
print(poise_bp)
    
poise_bp =list(set(poise_bp))
for i in range(0, len(poise_bp)):
  poise_bp[i] = float(poise_bp[i])

for i in range(0, len(vit_scale)):
  vit_scale[i] = float(vit_scale[i])

for i in range(0, len(equip_load_scale)):
  equip_load_scale[i] = float(equip_load_scale[i])

def LogisticFit(x,y):
  w = np.array(y)
  eps = .00001
  normalize = np.max(w) + eps
  w_norm = w / normalize
  # y = 1 / exp(-k(x - x0))
  # (1 + exp(-k(x - x0))) = 1 / y
  # exp(-k(x - x0)) = 1 / y - 1 =
  # -k(x - x0) = log ( 1 /y - 1)
  # x = x0 + -(log(1 - w) - log(w) ) / k
  # x = x0 - (1/k)log((1 - y)/y)
  # x = b + m log((1 - y)/y)
  loggy = np.log(1/w_norm - 1).reshape(-1,1)
  ransac = linear_model.RANSACRegressor(min_samples = 10)
  ransac.fit(loggy, x)
  m, b = float(ransac.estimator_.coef_), float(ransac.estimator_.intercept_)
  k = -1 / m
  x0 = b
  return [normalize, k, x0]

def LogisticFunction(L,k,x0,x):
  return L / (1 + np.exp(-k*(x - x0)))

avg_negation_head = []
poise_head = []
weight_head = []
immunity_head = []
robustness_head = []
focus_head = []
vitality_head = []

avg_negation_chest = []
poise_chest = []
weight_chest = []
immunity_chest = []
robustness_chest = []
focus_chest = []
vitality_chest = []

avg_negation_arm = []
poise_arm = []
weight_arm = []
immunity_arm = []
robustness_arm = []
focus_arm = []
vitality_arm = []

avg_negation_leg = []
poise_leg = []
weight_leg = []
immunity_leg = []
robustness_leg = []
focus_leg = []
vitality_leg = []

for a in range(head_idx, chest_idx):
  d = np.sum(armor_matrix.A[:,a]) / 8
  avg_negation_head.append(d)
  poise_head.append(poise_matrix.A[0][a])
  weight_head.append(weight_matrix.A[0][a])
  immunity_head.append(immunity_matrix.A[0][a])
  robustness_head.append(robustness_matrix.A[0][a])
  focus_head.append(focus_matrix.A[0][a])
  vitality_head.append(vitality_matrix.A[0][a])
weight_head = np.array(weight_head)
poise_head = np.array(poise_head)

for a in range(chest_idx, arm_idx):
  d = np.sum(armor_matrix.A[:,a]) / 8
  avg_negation_chest.append(d)
  poise_chest.append(poise_matrix.A[0][a])
  weight_chest.append(weight_matrix.A[0][a])
  immunity_chest.append(immunity_matrix.A[0][a])
  robustness_chest.append(robustness_matrix.A[0][a])
  focus_chest.append(focus_matrix.A[0][a])
  vitality_chest.append(vitality_matrix.A[0][a])
weight_chest = np.array(weight_chest)
poise_chest = np.array(poise_chest)

for a in range(arm_idx, leg_idx):
  d = np.sum(armor_matrix.A[:,a]) / 8
  avg_negation_arm.append(d)
  poise_arm.append(poise_matrix.A[0][a])
  weight_arm.append(weight_matrix.A[0][a])
  immunity_arm.append(immunity_matrix.A[0][a])
  robustness_arm.append(robustness_matrix.A[0][a])
  focus_arm.append(focus_matrix.A[0][a])
  vitality_arm.append(vitality_matrix.A[0][a])
weight_arm = np.array(weight_arm)
poise_arm = np.array(poise_arm)

for a in range(leg_idx, num_armor):
  d = np.sum(armor_matrix.A[:,a]) / 8
  avg_negation_leg.append(d)
  weight_leg.append(weight_matrix.A[0][a])
  poise_leg.append(poise_matrix.A[0][a])
  immunity_leg.append(immunity_matrix.A[0][a])
  robustness_leg.append(robustness_matrix.A[0][a])
  focus_leg.append(focus_matrix.A[0][a])
  vitality_leg.append(vitality_matrix.A[0][a])
weight_leg = np.array(weight_leg)
poise_leg = np.array(poise_leg)



logistic_head = LogisticFit(weight_head, avg_negation_head)
logistic_chest = LogisticFit(weight_chest, avg_negation_chest)
logistic_arm = LogisticFit(weight_arm, avg_negation_arm)
logistic_leg = LogisticFit(weight_leg, avg_negation_leg)

weight_head = weight_head.reshape(-1,1)
weight_chest = weight_chest.reshape(-1,1)
weight_arm = weight_arm.reshape(-1,1)
weight_leg = weight_leg.reshape(-1,1)


ransac = linear_model.RANSACRegressor(min_samples = 10)
ransac.fit(weight_head, poise_head)
m_head, b_head = float(ransac.estimator_.coef_), float(ransac.estimator_.intercept_)
ransac.fit(weight_chest, poise_chest)
m_chest, b_chest = float(ransac.estimator_.coef_), float(ransac.estimator_.intercept_)
ransac.fit(weight_arm, poise_arm)
m_arm, b_arm = float(ransac.estimator_.coef_), float(ransac.estimator_.intercept_)
ransac.fit(weight_leg, poise_leg)
m_leg, b_leg = float(ransac.estimator_.coef_), float(ransac.estimator_.intercept_)


with open("datafit.txt", "a") as f:
  for x in logistic_head:
    f.write(str(x))
    f.write(' ')
  f.write('\n')
  for x in logistic_chest:
    f.write(str(x))
    f.write(' ')
  f.write('\n')
  for x in logistic_arm:
    f.write(str(x))
    f.write(' ')
  f.write('\n')
  for x in logistic_leg:
    f.write(str(x))
    f.write(' ')
  f.write('\n')
  f.write(str(m_head))
  f.write(' ')
  f.write(str(b_head))
  f.write('\n')
  f.write(str(m_chest))
  f.write(' ')
  f.write(str(b_chest))
  f.write('\n')
  f.write(str(m_arm))
  f.write(' ')
  f.write(str(b_arm))
  f.write('\n')
  f.write(str(m_leg))
  f.write(' ')
  f.write(str(b_leg))
  f.write('\n')


line_head_weight = np.arange(weight_head.min(), weight_head.max(),.01)[:, np.newaxis]
line_head_fit = []
poise_head_fit = []
for x in line_head_weight:
  line_head_fit.append(LogisticFunction(logistic_head[0],logistic_head[1],logistic_head[2], x))
  poise_head_fit.append(m_head * x + b_head)

line_chest_weight = np.arange(weight_chest.min(), weight_chest.max(),.01)[:, np.newaxis]
line_chest_fit = []
poise_chest_fit = []
for x in line_chest_weight:
  line_chest_fit.append(LogisticFunction(logistic_chest[0],logistic_chest[1],logistic_chest[2], x))
  poise_chest_fit.append(m_chest * x + b_chest)

line_arm_weight = np.arange(weight_arm.min(), weight_arm.max(),.01)[:, np.newaxis]
line_arm_fit = []
poise_arm_fit = []
for x in line_arm_weight:
  line_arm_fit.append(LogisticFunction(logistic_arm[0],logistic_arm[1],logistic_arm[2], x))
  poise_arm_fit.append(m_arm * x + b_arm)

line_leg_weight = np.arange(weight_leg.min(), weight_leg.max(),.01)[:, np.newaxis]
line_leg_fit = []
poise_leg_fit = []
for x in line_leg_weight:
  line_leg_fit.append(LogisticFunction(logistic_leg[0],logistic_leg[1],logistic_leg[2], x))
  poise_leg_fit.append(m_leg * x + b_leg)

plt.figure(figsize=(20, 20), facecolor=(1,1,1))
plt.subplot(411)
plt.scatter(weight_head, avg_negation_head)
plt.plot(line_head_weight, line_head_fit)
plt.title('Logistic Curve Fit Head')
plt.xlabel('Weight')
plt.ylabel('Avg Negations')
plt.subplot(412)
plt.scatter(weight_chest, avg_negation_chest)
plt.plot(line_chest_weight, line_chest_fit)
plt.title('Logistic Curve Fit Chest')
plt.xlabel('Weight')
plt.ylabel('Avg Negations')
plt.subplot(413)
plt.scatter(weight_arm, avg_negation_arm)
plt.plot(line_arm_weight, line_arm_fit)
plt.title('Logistic Curve Fit Arm')
plt.xlabel('Weight')
plt.ylabel('Avg Negations')
plt.subplot(414)
plt.scatter(weight_leg, avg_negation_leg)
plt.plot(line_leg_weight, line_leg_fit)
plt.title('Logistic Curve Fit Leg')
plt.xlabel('Weight')
plt.ylabel('Avg Negations')
plt.savefig('Negations.png')

plt.figure(figsize=(20, 20), facecolor=(1,1,1))
plt.subplot(411)
plt.scatter(weight_head, poise_head)
plt.plot(line_head_weight, poise_head_fit)
plt.title('Linear Curve Fit Head')
plt.xlabel('Weight')
plt.ylabel('Poise')
plt.subplot(412)
plt.scatter(weight_chest, poise_chest)
plt.plot(line_chest_weight, poise_chest_fit)
plt.title('Linear Curve Fit Chest')
plt.xlabel('Weight')
plt.ylabel('Poise')
plt.subplot(413)
plt.scatter(weight_arm, poise_arm)
plt.plot(line_arm_weight, poise_arm_fit)
plt.title('Linear Curve Fit Arm')
plt.xlabel('Weight')
plt.ylabel('Poise')
plt.subplot(414)
plt.scatter(weight_leg, poise_leg)
plt.plot(line_leg_weight, poise_leg_fit)
plt.title('Linear Curve Fit Leg')
plt.xlabel('Weight')
plt.ylabel('Poise')
plt.savefig('Poise.png')

plt.figure(figsize=(20, 20), facecolor=(1,1,1))
plt.subplot(411)
plt.scatter(weight_head, robustness_head)
#plt.plot(line_head_weight, poise_head_fit)
plt.title('Linear Curve Fit Head')
plt.xlabel('Weight')
plt.ylabel('Robustness')
plt.subplot(412)
plt.scatter(weight_chest, robustness_chest)
#plt.plot(line_chest_weight, poise_chest_fit)
plt.title('Linear Curve Fit Chest')
plt.xlabel('Weight')
plt.ylabel('Robustness')
plt.subplot(413)
plt.scatter(weight_arm, robustness_arm)
#plt.plot(line_arm_weight, poise_arm_fit)
plt.title('Linear Curve Fit Arm')
plt.xlabel('Weight')
plt.ylabel('Robustness')
plt.subplot(414)
plt.scatter(weight_leg, robustness_leg)
#plt.plot(line_leg_weight, poise_leg_fit)
plt.title('Linear Curve Fit Leg')
plt.xlabel('Weight')
plt.ylabel('Robustness')
plt.savefig('Robustness.png')


plt.figure(figsize=(20, 20), facecolor=(1,1,1))
plt.subplot(411)
plt.scatter(weight_head, focus_head)
#plt.plot(line_head_weight, poise_head_fit)
plt.title('Linear Curve Fit Head')
plt.xlabel('Weight')
plt.ylabel('Focus')
plt.subplot(412)
plt.scatter(weight_chest, focus_chest)
#plt.plot(line_chest_weight, poise_chest_fit)
plt.title('Linear Curve Fit Chest')
plt.xlabel('Weight')
plt.ylabel('Focus')
plt.subplot(413)
plt.scatter(weight_arm, focus_arm)
#plt.plot(line_arm_weight, poise_arm_fit)
plt.title('Linear Curve Fit Arm')
plt.xlabel('Weight')
plt.ylabel('Focus')
plt.subplot(414)
plt.scatter(weight_leg, focus_leg)
#plt.plot(line_leg_weight, poise_leg_fit)
plt.title('Linear Curve Fit Leg')
plt.xlabel('Weight')
plt.ylabel('Focus')
plt.savefig('Focus.png')

plt.figure(figsize=(20, 20), facecolor=(1,1,1))
plt.subplot(411)
plt.scatter(weight_head, vitality_head)
#plt.plot(line_head_weight, poise_head_fit)
plt.title('Linear Curve Fit Head')
plt.xlabel('Weight')
plt.ylabel('Vitality')
plt.subplot(412)
plt.scatter(weight_chest, vitality_chest)
#plt.plot(line_chest_weight, poise_chest_fit)
plt.title('Linear Curve Fit Chest')
plt.xlabel('Weight')
plt.ylabel('Vitality')
plt.subplot(413)
plt.scatter(weight_arm, vitality_arm)
#plt.plot(line_arm_weight, poise_arm_fit)
plt.title('Linear Curve Fit Arm')
plt.xlabel('Weight')
plt.ylabel('Vitality')
plt.subplot(414)
plt.scatter(weight_leg, vitality_leg)
#plt.plot(line_leg_weight, poise_leg_fit)
plt.title('Linear Curve Fit Leg')
plt.xlabel('Weight')
plt.ylabel('Vitality')
plt.savefig('Vitality.png')

plt.figure(figsize=(20, 20), facecolor=(1,1,1))
plt.subplot(411)
plt.scatter(weight_head, immunity_head)
#plt.plot(line_head_weight, poise_head_fit)
plt.title('Linear Curve Fit Head')
plt.xlabel('Weight')
plt.ylabel('Immunity')
plt.subplot(412)
plt.scatter(weight_chest, immunity_chest)
#plt.plot(line_chest_weight, poise_chest_fit)
plt.title('Linear Curve Fit Chest')
plt.xlabel('Weight')
plt.ylabel('Immunity')
plt.subplot(413)
plt.scatter(weight_arm, immunity_arm)
#plt.plot(line_arm_weight, poise_arm_fit)
plt.title('Linear Curve Fit Arm')
plt.xlabel('Weight')
plt.ylabel('Immunity')
plt.subplot(414)
plt.scatter(weight_leg, immunity_leg)
#plt.plot(line_leg_weight, poise_leg_fit)
plt.title('Linear Curve Fit Leg')
plt.xlabel('Weight')
plt.ylabel('Immunity')
plt.savefig('Immunity.png')
