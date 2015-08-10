#pragma once

#include <vector>
#include <string>

struct TimeKeeper {
  void Init(const std::vector<std::string>& problems, long long time_limit_usec);
  void StartNewProblem(int problem);
  void StartNewSeed(int seed_idx);
  long long TimeLimitForTheSeed() const;
  long long RemainingTimeForTheSeed() const;

  int num_problems = 0;
  std::vector<int> remaining_seeds;
  int current_problem = 0;
  int current_seed_index = 0;
  long long execution_start_time = 0;
  long long execution_end_time = 0;
  long long seed_start_time = 0;
  long long seed_end_time = 0;
};
