#include "kichiai.h"
#include <iostream>
#include <queue>
using namespace std;

// MOVE, LOCK, ERROR

namespace {

struct SearchState {
  State state;
  string commands;
  SearchState(State state, string commands)
    : state(state), commands(commands) {}
};
};

string KichiAI::Step(const Game& game, const State& state) {
  // game.CurrentUnit(state.source_idx);
  // vector<char> commands = {'p', 'b', 'a', 'l', 'd', 'k'};
  vector<char> commands = {'a'};
  queue<SearchState> que;
  string step_result = "";
  double max_score = -1;

  que.push(SearchState(state, ""));
  while (!que.empty()) {
    SearchState search_state = que.front(); que.pop();
    for (auto c : commands) {
      State next_state = search_state.state;
      CommandResult result = next_state.Command(game, c);
      string next_commands = search_state.commands + string(1, c);
      if (result == ERROR || result == GAMEOVER) { continue; }
      if (result == LOCK || result == CLEAR) {
        double score = 1;
        if (score > max_score) {
          max_score = score;
          step_result = next_commands;
        }
        continue;
      }

      que.push(SearchState(next_state, next_commands));
    }
  }

  return step_result;
}
