#pragma once

#include "ai.h"

struct KichiAI : public AI {
  std::string Run(const Game& game);
};
