#pragma once

#include <utility>
#include <string>

#include "ai.h"

enum class TleState {
  Green,
  Yellow,
  Red,
};

struct SubmarineAI : public AI {
  TleState ShouldExitLoop(long long unit_start_time) const;
  std::pair<int, std::string> Step(const Game& game, const State& initial_state, int loop_count);
  std::string Run(const Game& game);

private:
  long long game_start_time = 0;
  long long time_limit = 0;
  long long time_limit_per_unit = 0;
};
