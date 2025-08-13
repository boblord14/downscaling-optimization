//
// Created by false on 6/7/2025.
//

#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <queue>
#include <chrono>
#include <iostream>
#include "Weapon.h"
#include <utility>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <fstream>

#include "DataParser.h"
#include "Eigen/Dense"
#include "Eigen/SVD"


using namespace std::chrono;


struct Region{
  std::vector<int> lower, upper;
  std::vector<Weapon*> weapons;
  std::vector<int> defs;
  std::vector<double> scale;
  double upper_bound;
  int total_stats;

  Region(const std::vector<int>& lower, const std::vector<int> upper,
	 const std::vector<Weapon*>& weapons, const std::vector<int>& defs, const std::vector<double>& scale, int total_stats):lower(lower), upper(upper), weapons(weapons), defs(defs), scale(scale), total_stats(total_stats){
    upper_bound = evaluate(upper);
  }
  double evaluate(const std::vector<int>& stats) {
    double eval = 0;
    for (size_t i =0; i < weapons.size(); ++i) {
      eval += weapons[i]->calculateDamage(stats,
					  defs,
					  false) / scale[i];
    }
    eval /= weapons.size();
    return eval;
  }
  std::vector<double> intersect();
  std::vector<Region> subdivide();
  void backtracking(const std::vector<std::vector<std::pair<int,int>>>& bounds, std::vector<Region>& regions, std::vector<int>& upper, std::vector<int>& lower, const std::vector<Weapon*>& weapons, int total_stats, int count);
  
};

struct Comparar{
  bool operator()(const Region& a, const Region& b) {
    return a.upper_bound < b.upper_bound;
  }
};

std::vector<double> Region::intersect() {
  std::vector<double> line(5,0);
  for (int i =0; i < 5; ++i) {
    line[i] = upper[i] - lower[i];
  }    
  std::vector<double> plane(5,1);
  double offset = 0;
  double intersect = 0;
  for (int i = 0; i < 5; ++i) {
    offset += lower[i] * plane[i];
    intersect += line[i] * plane[i];
  }
  double alpha = (total_stats - offset) / intersect;
  std::vector<double> point(5,0);
  for (int i = 0; i < 5; ++i) {
    point[i] = lower[i] + alpha * line[i];
  }
  return point;
}
std::vector<Region> Region::subdivide(){
  auto point = intersect();
  std::vector<std::vector<std::pair<int,int>>> bounds(5);
  for (int i = 0; i < 5; ++i) {
    int f = std::floor(point[i]);
    int c = std::ceil(point[i]);
    if (f == c) {
      c += 1;
    }
    bounds[i].emplace_back(lower[i], f);
    bounds[i].emplace_back(c, upper[i]);
  }
  /*
  for (int i = 0; i < 5; ++i) {
    std::cout << "lower" << std::endl;
    std::cout << bounds[i][0].first << " " << bounds[i][0].second << std::endl;
    std::cout << "upper" << std::endl;
    std::cout << bounds[i][1].first << " " << bounds[i][1].second << std::endl;

  }
  */
  std::vector<Region> regions;
  std::vector<int> temp_lower(5,0);
  std::vector<int> temp_upper(5,0);
  backtracking(bounds, regions, temp_upper, temp_lower, weapons, total_stats, 0);
  return regions;
}
void Region::backtracking(const std::vector<std::vector<std::pair<int,int>>>& bounds, std::vector<Region>& regions, std::vector<int>& upper, std::vector<int>& lower, const std::vector<Weapon*>& weapons, int total_stats, int count) {
  if (count == 5) {
    for (int i = 0; i < 5; ++i) {
      if (lower[i] > this->upper[i]){
	return;
      }
    }
    regions.emplace_back(lower, upper,
			 weapons, defs, scale, total_stats);
    return;
  }
  for(int i = 0; i < 2; ++i) {
    lower[count] = bounds[count][i].first;
    upper[count] = bounds[count][i].second;
    backtracking(bounds, regions, upper, lower, weapons, total_stats, count +1);
  }
}

volatile double best;
std::vector<int> best_vector(5,0);
std::priority_queue<Region, std::vector<Region>, Comparar> queue;
std::mutex mut;
int total_stats;
volatile double prev_best = -1;
volatile bool first_run = true;
std::ofstream out("DownscalingResults.txt");

void thread(){
  while(true) {
    mut.lock();
    if (queue.empty()){
      mut.unlock();
      return;
    }
    Region r = queue.top();
    //std::cout << queue.size() << std::endl;
    queue.pop();
    mut.unlock();
    int sum_lower = 0;
    int sum_upper = 0;
    for (int i = 0; i < 5; ++i) {
      sum_lower += r.lower[i];
      sum_upper += r.upper[i];
    }      
    if (sum_upper < total_stats || sum_lower > total_stats ) {
      continue;
    }
    double val  = 0;
    if(sum_lower == total_stats) {
      val = r.evaluate(r.lower);
      mut.lock();
      if (val > best) {
	best = val;
	best_vector = r.lower;
	/*
	out << best << std::endl;
	out << "Best Vector " << std::endl;
	for (int i = 0; i < 5; ++i) {
	  out << best_vector[i] << " ";
	}
	out << std::endl;
	*/
      }
      mut.unlock();
      continue;
    }    
    if (r.upper_bound <= best) {
      continue;
    }
      /*
	std::cout << "Upper bound ";
	for (int i = 0; i < 5; ++i) {
	std::cout << r.upper[i] << " ";
	}
	std::cout << std::endl;
      
	std::cout << "Lower bound ";
	for (int i = 0; i < 5; ++i) {
	std::cout << r.lower[i] << " ";
	}
	std::cout << std::endl;
      */
    auto point = r.intersect();
    std::vector<int>  point_int(5,0);

    
    mut.lock();
    std::cout << "Eval Point ";
    for (int i = 0; i < 5; ++i) {
      point_int[i] = point[i];
      std::cout << point_int[i] << " ";
    }
    std::cout << std::endl;
    mut.unlock();
    
    val = r.evaluate(point_int);
    mut.lock();
    if (val > best) {
      best = val;
      best_vector = point_int;
      /*
      out << best << std::endl;
      out << "Best Vector " << std::endl;
      for (int i = 0; i < 5; ++i) {
	out << best_vector[i] << " ";
      }
      out << std::endl;
      */
    }
    mut.unlock();
    auto regions = r.subdivide();
    mut.lock();
    for (const auto & reg : regions){
      queue.push(reg);
    }
    mut.unlock();
  }
}


std::pair<std::vector<double>, std::vector<std::vector<int>>> branch_bound(std::vector<Weapon*> weapons,
									   std::vector<int> defs,
									   std::vector<double> scale,
									   std::vector<int> lower,
									   std::vector<int> upper,
									   int start_stats,
									   int max_stats) {
  total_stats = start_stats;
  best = -1;
  std::vector<double> output;
  std::vector<double> times;
  std::vector<std::vector<int>> stats;
  auto start = high_resolution_clock::now();
  while (total_stats <= max_stats) {
    for(int i = 0; i < 5; ++i) {
      upper[i] = std::min(total_stats, 99);
    }
    if (best != -1) {
      double sum = 0;
      for (int i = 0; i < 5; ++i) {
	sum += best_vector[i];
      }
      double alpha = total_stats / sum;
      out << "Best guess vector ";
      for (int i = 0; i < 5; ++i) {
	best_vector[i] = std::min(alpha * best_vector[i], 99.0);
	out << best_vector[i] << " ";
      }
      out << std::endl;
      double possible_best = 0;
      for (size_t i = 0; i < weapons.size(); ++i) {
	possible_best += weapons[i]->calculateDamage(best_vector,
						     defs,
						     false);
      }
      best = possible_best;
      out << "Best Guess " << best << std::endl;
      
      
    }
    std::cout << "test" << std::endl;
    for(auto & s : scale) {
      std::cout << s << std::endl;
    }
    std::cout << weapons.size() << std::endl;
    Region region(lower, upper, weapons, defs, scale, total_stats);
    std::cout << region.evaluate(upper) << std::endl;
   queue.push(region);
    auto s = high_resolution_clock::now();
    std::thread t0{thread};
    std::thread t1{thread};
    std::thread t2{thread};
    std::thread t3{thread};
    t0.join();
    t1.join();
    t2.join();
    t3.join();
    auto ss = high_resolution_clock::now();
    out << std::endl << std::endl;
    out << "Total Stats " << total_stats << std::endl;
    out << "best damage " << best << std::endl;
    out << "best stats ";
    for (int i = 0; i < 5;++i) {
      out << best_vector[i] << " ";
    }
    out << std::endl;
    double best_result = best;
    output.push_back(best_result);
    stats.push_back(best_vector);
    auto duration = duration_cast<microseconds>(ss - s);
    out << "Time taken by function: "
	<< duration.count()/1.0e6 << " seconds" << std::endl;
    times.push_back(duration.count() / 1.0e6);
    first_run = false;
    prev_best = best;
    total_stats += 5;
  }

  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(stop - start);

  out << "Time taken by function: "
      << duration.count()/1.0e6 << " seconds" << std::endl;
  double count = start_stats;
  for (auto t: times) {
    out << count << " " << t << std::endl;
    count += 5;
  }
  return std::make_pair(output,stats);
}

std::vector<int> computeWeaponUpgradeInterest(std::vector<Weapon*> weapons, const std::vector<int>& defs) {
  std::vector<int> ranges;
  std::vector<int> stats(5,99);
  for (int i = 0; i < 25; ++i) {
    std::vector<double> ratios;
    for (size_t j = 0; j < weapons.size(); ++j) {
      Weapon w(weapons[j]->getId(), weapons[j]->getInfusion(), i);
      auto ars = w.calcAR(stats[0],
			  stats[1],
			  stats[2],
			  stats[3],
			  stats[4],
			  false);
      for (size_t k = 0; k < ars.size(); ++k) {
	if (ars[k] == 0) {
	  continue;
	}
	double ratio = ars[k] / defs[k] ;
	ratios.push_back(ratio);
      }
    }
    std::sort(ratios.begin(), ratios.end());
    for(auto r : ratios) {
      std::cout << r <<  " ";
    }
    std::cout << std::endl;
    double median = -1;
    if (ratios.size() %2 == 0) {
      int index = ratios.size() / 2;
      median = (ratios[index + 1] + ratios[index]) / 2;
    }
    else {
      median = ratios[ratios.size() / 2];
    }
    double ratio = median;
    if (8 >= ratio && 2.5 < ratio) {
      ranges.push_back(0);
    }
    if ( 2.5 >= ratio && 1 < ratio) {
      ranges.push_back(1);
    }
    if (1 >= ratio && .125 < ratio) {
      ranges.push_back(2);
    }
    if (.125 >= ratio) {
      ranges.push_back(3);
    }
    if (8 < ratio) {
      ranges.push_back(4);
    }
  }
  return ranges;
}

std::pair<std::vector<double>,std::vector<std::vector<int>>> loadScale(const Weapon& w) {
  std::string file = "Scaling\\" + std::to_string(w.getId() + w.getInfusion());
  std::ifstream in(file);
  auto opt = std::make_pair(std::vector<double>(), std::vector<std::vector<int>>());
  if (in.good() == false) {
    std::cout << "Bad File" << std::endl;
    return opt;
  }
  std::cout << "Loading weapon " << std::endl;
  std::vector<int> stats(5,0);
  double damage;
  while(in >> damage) {
    in >> stats[0];
    in >> stats[1];
    in >> stats[2];
    in >> stats[3];
    in >> stats[4];
    opt.first.push_back(damage);
    opt.second.push_back(stats);
  }
  return opt;
}

void saveScaling(const Weapon& w, const std::pair<std::vector<double>,std::vector<std::vector<int>>>& opt) {
  std::string file = "Scaling\\" + std::to_string(w.getId() + w.getInfusion()) + ".txt";
  std::ofstream out(file);
  for (size_t i = 0; i < opt.first.size(); ++i) {
    out << opt.first[i] << " ";
    for (size_t j = 0; j < 5; ++j) {
      out << opt.second[i][j] << " ";
    }
    out << std::endl;
  }

}


void fullPipeline(std::vector<Weapon>& weapons, std::vector<int> base_stats, int start_stats, bool use_cache=false){
  std::vector<double> scale;
  std::vector<double> singleton(1, 1);
  std::vector<int> defs(5, 140);
  std::vector<Weapon*> weapon_ptr;
  std::vector<int> lower(5,0);// 
  std::vector<int> upper(5, 99);
  int max_stats = 80;
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
  /*
  for (size_t i = 0; i < weapons.size();++i) {
    weapon_ptr.push_back(&weapons[i]);
  }
  int index_start = 0;
  if (use_cache) {
    index_start = (start_stats / 5) - 1;
  }
  out << "index start is " << index_start << std::endl;
  out << "starting stats is " << stats[0] << std::endl;
  int size = stats.size();
  std::vector<double> svd_dmg;
  auto start = high_resolution_clock::now();
  for (int i = 0; i < size; ++i) {
    std::vector<double> scale(weapons.size(), 0);
    for (int j = 0; j < weapons.size(); ++j) {
      scale[j] = opts[j].first[index_start + i];
      for (int k = 0; k < 5; ++k) {
	      mat(k,j) = opts[j].second[index_start + i][k];
	      out << mat(k, j) << " ";
      }
      out << std::endl;
    }

    Eigen::JacobiSVD<Eigen::MatrixXd, Eigen::ComputeThinU | Eigen::ComputeThinV> svd(mat);
    if (svd.info() != Eigen::ComputationInfo::Success) {
      out << "SVD failed" << std::endl;
    }
    std::vector<double> approx(5,0);
    out << "First Singular Value U column" << std::endl;
    double sum = 0;
    double offset = 0;
    double initial_stats = 0;
    for (int k = 0; k < 5; ++k) {
      approx[k] =  svd.matrixU()(k,0);
      if (std::abs(approx[k]) <= .1) {
	approx[k] = 0;
      }
      sum += approx[k];
      out << approx[k] << std::endl;
      initial_stats += base_stats[k];
    }
    out << "Stats to assign " << (stats[i] - initial_stats) << std::endl;
    double alpha = (stats[i] - initial_stats) / sum;
    std::vector<int> svd_stats(5,0);
    out << "SVD stats" << std::endl;
    int svd_sum = 0;
    for (int k = 0; k < 5; ++k) {
      svd_stats[k] = std::min(approx[k] * alpha + base_stats[k], 99.0);
      out << svd_stats[k] << std::endl;
      svd_sum += svd_stats[k];
    }
    out << "SVD total stats " << svd_sum << std::endl;
    for (int k = 0; k < scale.size(); ++k) {
      out << scale[k] << std::endl;
    }
    Region region(lower, upper, weapon_ptr, defs, scale, stats[i]);
    double svd_approx = region.evaluate(svd_stats);
    out << "SVD APPROX: " << svd_approx << std::endl; 
    svd_dmg.push_back(svd_approx);

  }
  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(stop - start);

  out << "Time taken by SVD function: "
      << duration.count()/1.0e6 << " seconds" << std::endl;


  std::vector<double> actual_dmg;
  for (int i = 0; i < size; ++i) {
    std::vector<double> scale(weapons.size(), 0);
    for (int j = 0; j < weapons.size(); ++j) {
      scale[j] = opts[j].first[index_start + i];
    }

    auto opt = branch_bound(weapon_ptr,
			    defs,
			    scale,
			    base_stats,
			    upper,
			    stats[i],
			    stats[i]);
    actual_dmg.push_back(opt.first[0]);
  
  }
  double error = 0;
  for (int i = 0; i < size; ++i) {
    out << "Actual Damage " << actual_dmg[i] << std::endl; // " and svd damage " << svd_dmg[i] << std::endl;
   // error += std::abs(actual_dmg[i] - svd_dmg[i]) / actual_dmg[i];
  }
  //error /= size;
 // out << "Average relative error " << error << std::endl;
  */
}

void pinco() {
  std::vector<Weapon> weapons;
  // Ripple Crescent Halberd
  //weapons.emplace_back(18060000, BASE, 17);
  // Albinauric Staff
  weapons.emplace_back(33190000, BASE, 17);
  // Dragon Communion Seal
  weapons.emplace_back(34080000, BASE, 7);
  // Venomous Fang
  weapons.emplace_back(22010000, POISON, 17);
  // Ripple Blade
  //weapons.emplace_back(14050000, BASE, 17);
  // Lazuli Glintstone Blade
  //weapons.emplace_back(2250000, BASE, 7);
  // Celebrant's Skull
  //weapons.emplace_back(12130000, MAGIC, 17);
  // Mantis Blade
  //weapons.emplace_back(7120000, OCCULT, 17);
  std::vector<int> base_stats= {0,4,0,0,0};
  int base = 0;
  for (int i = 0; i < 5; ++i) {
    base += base_stats[i];
  }
  int start_stats = ((base/5) + 1) * 5;
  fullPipeline(weapons, base_stats, start_stats, true);
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
  fullPipeline(weapons, base_stats, start_stats);
}


int main() {
    DataParser::generateDefs();
    Weapon shortsword(22010000, POISON, 17);
    auto ar = shortsword.calcAR(15, 12, 26, 33, 60, TRUE);
    pinco();
    // montage();
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
