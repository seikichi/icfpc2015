#pragma once

#include "evaluator.h"

struct QwEvaluator : public Evaluator {
  virtual int evaluate(
      const Game& game,
      const State& initial_state,
      const SmallState& sstate,
      const SmallState& next_sstate);
};
