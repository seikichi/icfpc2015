#pragma once

#include "game.h"
#include <string>
#include <memory>

struct AI {
  void Init();
  std::string Run(const Game& game);
  virtual std::string Step(const Game& game,
                           const State& state) = 0;
  virtual ~AI() {};

  static std::shared_ptr<AI> CreateAI(const std::string& name);
};
