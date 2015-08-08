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
  vector<char> commands = {'p', 'b', 'a', 'l', 'd', 'k'};
  queue<SearchState> que;
  string step_result = "";
  double max_score = -1;
  vector<bool> visited(game.h * game.w * 6, false);
  que.push(SearchState(state, ""));

  while (!que.empty()) {
    SearchState search_state = que.front(); que.pop();
    const State& state = search_state.state;

    int visited_index = game.w * 6 * state.pivot.y +
      6 * state.pivot.x +
      (state.rot % game.CurrentPeriod(state.source_idx));

    if (visited[visited_index]) { continue; }
    visited[visited_index] = true;

    for (auto c : commands) {
      State next_state = search_state.state;
      CommandResult result = next_state.Command(game, c);
      string next_commands = search_state.commands + string(1, c);
      if (result == ERROR || result == GAMEOVER) { continue; }
      if (result == LOCK || result == CLEAR) {
        double score = state.pivot.y;
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

// pivot.{y,x} , rot % period (max 0~5)
// 
