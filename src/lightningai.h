#pragma once

#include "ai.h"

struct LightningAI : public AI {
  std::string Run(const Game& game);
};
