#pragma once

#include "ai.h"

struct ChickAI : public AI {
  std::string Run(const Game& game);
};
