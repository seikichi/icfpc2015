#pragma once

#include "game.h"

#include <vector>

struct ManualPlayer {
  std::string commands;
  std::vector<State> states;
  bool Init(const Game& game) {
    states.clear();
    State state;
    state.Init(game);
    states.push_back(state);
    return true;
  }
  State GetCurrentState() const { return states.back(); }
  std::string GetCommands() const { return commands; }
  CommandResult Move(const Game& game, char c);
  void Rollback() {
    if ((int)states.size() == 1) { return; }
    states.pop_back();
    commands.pop_back();
  }
};
