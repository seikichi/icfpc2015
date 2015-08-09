#pragma once

#include "game.h"
#include <iostream>
#include <string>

struct Replay {
  int turn = 0;
  std::string commands_;
  State state;
  bool Init(const Game& game, const std::string& commands) {
    turn = 0;
    commands_ = commands;
    state.Init(game);
    return true;
  }
  State GetCurrentState() const { return state; }
  bool OneCommandStep(const Game& game) {
    if (turn == (int)commands_.size()) { return false; }
    char c = commands_[turn++];
    auto result = state.Command(game, c);
    if (result == ERROR) {
      std::cerr << "AHOKA: The given command caused error." << std::endl;
      exit(1);
    }
    return true;
  }
};
