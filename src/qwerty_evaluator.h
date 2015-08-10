#pragma once

#include "evaluator.h"

struct QwertyEvaluator : public Evaluator {
  virtual int evaluate(
      const Game& game,
      const State& initial_state,
      const SmallState& sstate,
      const SmallState& next_sstate);
};
