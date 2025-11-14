import json
import os

if __name__=="__main__":
    f = "soulsplanner-build-archive\\build-archive-1.json"
    with open(f, encoding='utf-8') as data_file:
        data = json.load(data_file)
        for build in data:
            print("testing build " + build['id'])

            # clean up weird problems
            if "rl" not in build['stats']:
                continue

            if build['stats']['rl'] == 0:
                continue

            if build['id'] == '':
                continue

            # capture rl125 +/- 10 levels
            if 115 <= build['stats']['rl'] <= 135:
                filepath = 'soulsplanner-build-archive\\rl125ish\\' + build['id'] + '.json'
                if not os.path.exists(filepath):
                    with open(filepath, "w") as file:
                        json.dump(build, file, indent=4)
                    continue

            #capture rl90 +/- 5 levels
            if 85 <= build['stats']['rl'] <= 95:
                filepath = 'soulsplanner-build-archive\\rl90ish\\' + build['id'] + '.json'
                if not os.path.exists(filepath):
                    with open(filepath, "w") as file:
                        json.dump(build, file, indent=4)
                    continue

            #capture rl150 +/- 15 levels
            if 135 <= build['stats']['rl'] <= 165:
                filepath = 'soulsplanner-build-archive\\rl150ish\\' + build['id'] + '.json'
                if not os.path.exists(filepath):
                    with open(filepath, "w") as file:
                        json.dump(build, file, indent=4)
                    continue
