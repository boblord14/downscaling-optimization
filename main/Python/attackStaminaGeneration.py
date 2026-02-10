import csv
import matplotlib.pyplot
from matplotlib import pyplot
import math


def createDict(reader):
    stamData = {}
    for row in reader:
        id = int(row["behaviorJudgeId"])
        category = int(row["category"])
        if id == 0 and category == 1:
            stamData[int(row["variationId"])] = row["stamina"]
    return stamData
def dataConversion():
    stamTable = []
    with open('../../csv-conversions/BehaviorParam_PC.csv', mode='r', newline='') as behaviorParam:
        with open('../../csv-conversions/equipParamWeapon.csv', mode='r', newline='') as equipParamWeapon:
            equipParamWeaponReader = csv.DictReader(equipParamWeapon)
            behaviorParamReader = csv.DictReader(behaviorParam)
            stamBehaviorTable = createDict(behaviorParamReader)
            for row in equipParamWeaponReader:
                id = int(row["behaviorVariationId"])
                r1Stam = stamBehaviorTable.get(id)

                if r1Stam is None: #potential case catcher for special weapons who use the base IDs per class
                    altId = (int(row["behaviorVariationId"]) // 100) * 100
                    r1Stam = stamBehaviorTable.get(altId)

                if r1Stam is None:
                    print(row["ID"] + ": " + row["Name"])
                    realStam = -1
                else:
                    realStam = int(r1Stam) * float(row["staminaConsumptionRate"])

                stamTable.append((row["ID"], realStam))
    return stamTable

def writeDataToCsv(dataArray):
    with open("../../csv-conversions/weaponStaminaData.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["id", "stamina"])  # header
        writer.writerows(dataArray)              # rows


def generateHistogram(stamData):
    staminaValues = [stamina for id, stamina in stamData]

    min_bin = math.floor(min(staminaValues) / 5) * 5
    max_bin = math.ceil(max(staminaValues) / 5) * 5

    bins = list(range(int(min_bin), int(max_bin) + 5, 5))

    pyplot.hist(staminaValues, bins=bins, edgecolor="black")
    pyplot.xlabel("stamina")
    pyplot.ylabel("#")
    pyplot.show()

if __name__=="__main__":
    print("\n")
    stamData = dataConversion()
    writeDataToCsv(stamData)
    generateHistogram(stamData)
