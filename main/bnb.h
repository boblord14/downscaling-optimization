//
// Created by Ethan on 8/18/2025.
//

#ifndef BNB_H
#define BNB_H
#include <queue>

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

inline std::vector<double> Region::intersect() {
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

inline std::vector<Region> Region::subdivide(){
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

inline void Region::backtracking(const std::vector<std::vector<std::pair<int,int>>>& bounds, std::vector<Region>& regions, std::vector<int>& upper, std::vector<int>& lower, const std::vector<Weapon*>& weapons, int total_stats, int count) {
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

inline volatile double best;
inline std::vector<int> best_vector(5,0);
inline std::priority_queue<Region, std::vector<Region>, Comparar> queue;
inline std::mutex mut;
inline int total_stats;
inline volatile double prev_best = -1;
inline volatile bool first_run = true;
inline std::ofstream out("DownscalingResults.txt");

inline void thread(){
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


inline std::pair<std::vector<double>, std::vector<std::vector<int>>> branch_bound(std::vector<Weapon*> weapons,
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

inline std::vector<int> computeWeaponUpgradeInterest(std::vector<Weapon*> weapons, const std::vector<int>& defs) {
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

#endif //BNB_H
