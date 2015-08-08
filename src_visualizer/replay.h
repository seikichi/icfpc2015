#pragma once

#include "game.h"
#include <unistd.h>
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
  bool KeyInput(const Game& game) {
    sleep(1);
    if (turn == (int)commands_.size()) { return false; }
    char c = commands_[turn++];
    state.Command(game, c);
    return true;
  }
};
