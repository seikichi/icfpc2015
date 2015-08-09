#pragma once

#include "ai.h"

struct SubmarineAI : public AI {
  std::string Run(const Game& game);
};
