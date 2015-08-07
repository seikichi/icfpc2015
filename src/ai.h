#pragma once

#include "game.h"
#include <string>

struct AI {
  void Init();
  std::string Run(const Game& game);
  char Step(const Game& game, const State& state);
};
