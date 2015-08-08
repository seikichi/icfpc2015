#pragma once

#include "game.h"
#include <string>
#include <memory>

struct AI {
  void Init();
  virtual std::string Run(const Game& game) = 0;
  virtual ~AI() {};

  static std::shared_ptr<AI> CreateAI();
};
