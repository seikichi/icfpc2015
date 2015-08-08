#pragma once

#include "game.h"
#include <string>
#include <memory>

struct AI {
  void Init(int time_limit_seconds);
  virtual std::string Run(const Game& game) = 0;
  virtual ~AI() {};

  static std::shared_ptr<AI> CreateAI();

  int time_limit_seconds;
};
