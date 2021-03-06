#pragma once

#include "util.h"
#include "small_state.h"
#include <utility>
#include <string>

#include "ai.h"

enum class TleState {
  Green,
  Yellow,
  Red,
};

struct SubmarineAI : public AI {
  TleState ShouldExitLoop(long long unit_start_time, long long time_limit_per_unit) const;
  std::pair<int, std::string> Step(const Game& game, const State& initial_state, int loop_count);
  std::string Run(const Game& game);
  std::string Annealing(const Game &game, const State& initial_state, std::string& initial_answer, int loop_count) const;
  std::string ChangePath(const Game &game, const State& initial_state, const std::string& commands, int loop_count, double temperature, long long start_time) const;
  std::string ChangeNode(const Game &game, const State& initial_state, const std::string& commands, int loop_count, int start_pos, int mid_point, int end_pos, long long start_time) const;
  std::string FindPath(const Game &game, const State& initial_state, const SmallState& initial_sstate, int loop_count, Cell mid_point, Cell end_point, int end_rot, long long start_time) const;

  double CalcEnergy(const Game& game, const SmallState& last_state) const;
  double CalcProbability(double energy, double next_energy, double temperature) const;

  SmallState CalcLastState(const Game& game, const State& initial_state, const SmallState& sstate, const std::string& commands) const;

private:
  mutable util::Random random;
  long long time_limit_per_unit_for_annealing;
  long long time_limit_per_unit_for_initial_search;
};
