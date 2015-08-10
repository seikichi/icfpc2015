#pragma once

#include "game.h"
#include "small_state.h"

#include <memory>

struct Evaluator {
  virtual int evaluate(
      const Game& game,
      const State& initial_state,
      const SmallState& sstate,
      const SmallState& next_sstate) = 0;

  static std::shared_ptr<Evaluator> CreateEvaluator();
};
