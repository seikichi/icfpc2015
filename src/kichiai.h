#pragma once

#include "ai.h"

struct KichiAI : public AI {
  std::string Step(const Game& game, const State& state);
};
