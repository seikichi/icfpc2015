#include "time_keeper.h"
#include "game.h"
#include <iostream>
#include <fstream>
#include <sys/time.h>
using namespace std;

namespace {

long long getTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (long long)tv.tv_sec * 1000000 + (long long)tv.tv_usec;
}

} // namespace

void TimeKeeper::Init(const vector<string>& problems, long long time_limit_usec) {
  num_problems = problems.size();
  remaining_seeds.resize(num_problems);
  for (int i = 0; i < (int)problems.size(); ++i) {
    ifstream ifs(problems[i]);
    string json(istreambuf_iterator<char>{ifs}, istreambuf_iterator<char>{});
    Game game;
    bool ok = game.Init(json, 0, {});
    if (!ok)
      continue;
    remaining_seeds[i] = game.num_source_seeds;
  }

  for (int i = num_problems - 2; i >= 0; --i) {
    remaining_seeds[i] += remaining_seeds[i + 1];
  }

  execution_start_time = getTime();
  execution_end_time = execution_start_time + time_limit_usec * 5 / 6;
}

void TimeKeeper::StartNewProblem(int problem) {
  current_problem = problem;
  current_seed_index = 0;
}

void TimeKeeper::StartNewSeed(int seed_idx) {
  current_seed_index = seed_idx;
  seed_start_time = getTime();
  seed_end_time = seed_start_time + TimeLimitForTheSeed();
}

long long TimeKeeper::TimeLimitForTheSeed() const {
  long long remaining_time = execution_end_time - seed_start_time;
  long long remaining_seed = remaining_seeds[current_problem] - current_seed_index;
  return remaining_time / remaining_seed;
}

long long TimeKeeper::RemainingTimeForTheSeed() const {
  return seed_end_time - getTime();
}
