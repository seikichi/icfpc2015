#pragma once

#include "game.h"

#include <map>
#include <vector>

struct ManualPlayer {
  std::vector<State> states;
  bool Init(const Game& game) {
    states.clear();
    State state;
    state.Init(game);
    states.push_back(state);
    return true;
  }
  State GetCurrentState() const { return states.back(); }
  CommandResult Move(const Game& game, char c) {
    std::map<char, char> mp{
      {'a', 'p'},
      {'d', 'b'},
      {'z', 'a'},
      {'x', 'l'},
      {'j', 'd'},
      {'k', 'k'},
    };
    State state = GetCurrentState();
    CommandResult ret = MOVE;
    if (c == ' ') {
      Rollback();
    } else {
      ret = state.Command(game, mp[c]);
      states.push_back(state);
    }
    return ret;
  }
  void Rollback() {
    if ((int)states.size() == 1) { return; }
    states.pop_back();
  }
};
