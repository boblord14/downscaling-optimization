//
// Created by false on 6/7/2025.
//

#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include "Weapon.h"
#include <utility>
#include <fstream>
#include "loadCharacter.h"

#include "DataParser.h"
#include <Eigen/Dense>
#include "bnb.h"
#include "svd.h"
#include "db.h"

void fullPipeline(std::vector<Weapon>& weapons, std::vector<int> base_stats, int start_stats, bool ignore_completed, bool useSVD, bool use_cache=false){
  std::vector<double> scale;
  std::vector<double> singleton(1, 1);
  std::vector<int> defs(5, 140);
  std::vector<Weapon*> weapon_ptr;
  std::vector<int> lower(5,0);// 
  std::vector<int> upper(5, 99);
  int max_stats = 120;
  Eigen::MatrixXd mat(5, weapons.size());
  std::vector<std::pair<std::vector<double>, std::vector<std::vector<int>>>> opts;
  std::vector<int> stats;
  start_stats = (start_stats / 5) * 5;
  int counter = start_stats;
  while (counter <= max_stats) {
    stats.push_back(counter);
    counter += 5;
  }
  out << "Computing for the range [" << stats[0] << "," << stats[stats.size() - 1] << "] for a total of " << stats.size() << " allocations" << std::endl; 
  for (size_t i = 0;  i < weapons.size();++i) {
    
    auto opt = std::make_pair(std::vector<double>(), std::vector<std::vector<int>>());

    if (ignore_completed && isCompleted(weapons[i])) {
      std::cout << "Weapon " << weapons[i].getId() << " already computed" << std::endl;
      continue;
    }

    if (use_cache == true) {
      opt = loadScale(weapons[i]);
    }
    if (opt.first.size() == 0){
      std::cout << "Not Using Cache" << std::endl;
      weapon_ptr.clear();
      weapon_ptr.push_back(&weapons[i]);
      opt = branch_bound(weapon_ptr,
			 defs,
			 singleton,
			 lower,
			 upper,
			 start_stats,
			 max_stats);
    }
    opts.push_back(opt);
    if (start_stats == 5) {
      saveScaling(weapons[i], opt);
    }
  }
  weapon_ptr.clear();

  if (useSVD) svd(weapons, base_stats, start_stats, stats, mat, opts, lower, upper, defs);

}

void pinco() {
  std::vector<Weapon> weapons;
  // Ripple Crescent Halberd
  weapons.emplace_back(18060000, BASE, 17);
  // Albinauric Staff
  weapons.emplace_back(33190000, BASE, 17);
  // Dragon Communion Seal
  weapons.emplace_back(34080000, BASE, 7);
  // Venomous Fang
  weapons.emplace_back(22010000, POISON, 17);
  // Ripple Blade
  weapons.emplace_back(14050000, BASE, 17);
  // Lazuli Glintstone Blade
  weapons.emplace_back(2250000, BASE, 7);
  // Celebrant's Skull
  weapons.emplace_back(12130000, MAGIC, 17);
  // Mantis Blade
  weapons.emplace_back(7120000, OCCULT, 17);
  std::vector<int> base_stats= {0,4,0,0,0};
  int base = 0;
  for (int i = 0; i < 5; ++i) {
    base += base_stats[i];
  }
  int start_stats = ((base/5) + 1) * 5;
  fullPipeline(weapons, base_stats, start_stats, true, false);
}

void montage(){
  std::vector<Weapon> weapons;
  // Greatsword
  weapons.emplace_back(4000000, HEAVY, 17);
  // Cleanrot
  weapons.emplace_back(5010000, HEAVY, 17);
  // Ringed Finger
  weapons.emplace_back(11130000, BASE, 7);
  // Axe of Godrick
  weapons.emplace_back(15040000, BASE, 7);
  // Banished Halberd
  weapons.emplace_back(18030000, HEAVY, 17);
  // Star Fist
  weapons.emplace_back(21080000, HEAVY, 17);
  // Lion Greatbow
  weapons.emplace_back(42000000, BASE, 7);
  // Coil Shield
  weapons.emplace_back(30200000, BASE, 7);
  // twinned knight swords
  weapons.emplace_back(10030000, COLD, 17);
  // Knight Sword
  weapons.emplace_back(3040000, HEAVY, 17);
  // Lordsword Straightsword
  weapons.emplace_back(2040000, HEAVY, 17);
  // Bloodhound Fang
  weapons.emplace_back(8030000, BASE, 7);
  // Brick Hammer
  weapons.emplace_back(12190000, HEAVY, 17);
  // Torchpole
  weapons.emplace_back(16080000, BASE, 17);
  // Venomous Fang
  weapons.emplace_back(22010000, HEAVY, 17);
  // Dragon Halberd
  weapons.emplace_back(18140000, BASE, 7);
  // Smithscript Hammer
  weapons.emplace_back(12500000, HEAVY, 17);
  // FrenzyFlame Bottle
  weapons.emplace_back(61520000, BASE, 17);
  // Smithscript Dagger
  weapons.emplace_back(63500000, COLD, 17);
  // Milady
  weapons.emplace_back(67500000, HEAVY, 17);
  // Great Katana
  weapons.emplace_back(66500000, BLOOD, 17);
  // Dane's
  weapons.emplace_back(60510000, HEAVY, 17);
  // Bone Bow
  weapons.emplace_back(40500000, BASE, 17);

  std::vector<int> base_stats= {14,13,9,9,7};
  int base = 0;
  for (int i = 0; i < 5; ++i) {
    base += base_stats[i];
  }
  int start_stats = ((base/5) + 1) * 5;
  fullPipeline(weapons, base_stats, start_stats, false, false);
}

void computeAllWeapons() {
  std::vector<Weapon> weapons;
  auto weaponData = DataParser::getWeaponIds();

  for (int equipParamWeaponId : weaponData) {
    try { //run whatever weapons in equipParamId that work and trash whatever entries throw an error when we try and build them
      Weapon newWeapon(equipParamWeaponId, BASE, 17);
      weapons.push_back(newWeapon);
    } catch (const std::exception& e) {
      std::cerr << "Caught exception: " << e.what() << std::endl;
    }
  }

  std::vector<int> base_stats= {0,1,0,0,0};
  int base = 0;
  for (int i = 0; i < 5; ++i) {
    base += base_stats[i];
  }
  int start_stats = ((base/5) + 1) * 5;
  fullPipeline(weapons, base_stats, start_stats, true, false);

}

int main() {
    DataParser::generateDefs();
    //pinco();
    //montage();
  loadCharacter::loadData();
  //computeAllWeapons();
 // Weapon shortsword(67520000, BASE, 17);
 // auto ar = shortsword.calcAR(20, 32, 12, 42, 10, false);
    return 0;
  /*
    std::vector<int> defs(5,140);
    Weapon shortsword(2010000, COLD, 0);
    Weapon broadsword(2020000, BASE, 0);
    std::vector<Weapon*> weapons;
    weapons.push_back(&shortsword);
    weapons.push_back(&broadsword);
    auto ranges = computeWeaponUpgradeInterest(weapons, defs);
    for(auto& range : ranges) {
      std::cout << range << std::endl;
    }
    std::cout << std::endl;
    int val = -1;
    int index = 0;
    std::vector<int> upgrade_levels;
    for (size_t i = 1; i < ranges.size(); ++i) {
      if (i == 1) {
	index = 0;
      }
      if (ranges[i] != ranges[i-1]) {
	upgrade_levels.push_back(index);
	index = i;
      }
    }
    upgrade_levels.push_back(index);
    for (auto u : upgrade_levels) {
      std::cout << u << std::endl;
    }
    return 0;
    */
}
