#pragma once

#include "game.h"
#include <string>

struct AI {
  void Init();
  std::string Run(const Game& game);
  virtual std::string Step(const Game& game,
                           const State& state,
                           const Unit& unit) = 0;

  static std::shared_ptr<AI> CreateAI(const std::string& name);
};
