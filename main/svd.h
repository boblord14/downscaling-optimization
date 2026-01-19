//
// Created by Ethan on 8/18/2025.
//

#ifndef SVD_H
#define SVD_H

 inline void svd(std::vector<Weapon>& weapons, std::vector<int> base_stats, int start_stats, std::vector<int> stats, Eigen::MatrixXd mat, std::vector<std::pair<std::vector<double>, std::vector<std::vector<int>>>> opts, std::vector<int> lower, std::vector<int> upper, std::vector<int> defs, bool use_cache = false) {
   std::vector<Weapon*> weapon_ptr;

   for (size_t i = 0; i < weapons.size();++i) {
     weapon_ptr.push_back(&weapons[i]);
   }
   int index_start = 0;
   if (use_cache) {
     index_start = (start_stats / 5) - 1;
   }
   out << "index start is " << index_start << std::endl;
   out << "starting stats is " << stats[0] << std::endl;
   int size = static_cast<int>(stats.size());
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
       svd_stats[k] = static_cast<int>(std::min(approx[k] * alpha + base_stats[k], 99.0));
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
 }
#endif //SVD_H
