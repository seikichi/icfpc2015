#pragma once

#include "game.h"
#include "time_keeper.h"
#include <string>
#include <memory>

struct AI {
  void Init(const TimeKeeper& time_keeper);
  virtual std::string Run(const Game& game) = 0;
  virtual ~AI() {};

  static std::shared_ptr<AI> CreateAI();

  const TimeKeeper* time_keeper;
};
