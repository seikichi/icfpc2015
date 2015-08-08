#include "manual_player.h"

#include <map>

CommandResult ManualPlayer::Move(const Game& game, char c) {
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
    if (ret == MOVE || ret == LOCK) {
      commands += mp[c];
      states.push_back(state);
    }
  }
  return ret;
}
