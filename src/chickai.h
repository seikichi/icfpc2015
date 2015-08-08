#pragma once

#include "ai.h"

struct ChickAI : public AI {
  std::string Step(const Game& game,
                   const State& state,
                   const Unit& unit);
};
