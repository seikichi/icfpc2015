#pragma once

#include "ai.h"

struct RonricoAI : public AI {
  std::string Run(const Game& game);
};
