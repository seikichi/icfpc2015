#include "kichiai.h"
#include <iostream>
#include <sstream>
#include <queue>
#include <cassert>
using namespace std;

namespace {

struct SearchState {
  State state;
  string commands;
  SearchState(State state, string commands)
    : state(state), commands(commands) {}
};

};

string KichiAI::Run(const Game& game) {
  const vector<char> commands = {'p', 'b', 'a', 'l', 'd', 'k'};
  State state;
  state.Init(game);

  const int visited_w = 3 * game.w;
  const int visited_offset_x = game.w;
  const int visited_h = 3 * game.h;
  const int visited_offset_y = game.h;

  stringstream solution;

  while (true) {
    // 1ユニットごとにループ
    State initial_state = state;

    vector<bool> visited(visited_h * visited_w * 6, false);

    queue<SearchState> que;
    string step_result = "";
    double max_score = -1;
    que.push(SearchState(initial_state, ""));

    while (!que.empty()) {
      const SearchState search_state = que.front();
      que.pop();
      const State& state = search_state.state;

      assert(0 <= state.pivot.x + visited_offset_x);
      assert(state.pivot.x + visited_offset_x < visited_w);
      assert(0 <= state.pivot.y + visited_offset_y);
      assert(state.pivot.y + visited_offset_y < visited_h);
      const int visited_index =
        visited_w * 6 * (state.pivot.y + visited_offset_y) +
        6 * (state.pivot.x + visited_offset_x) + state.rot;

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

    if (step_result == "") { return solution.str(); }
    for (const char c : step_result) {
      CommandResult result = state.Command(game, c);
      if (result == ERROR || result == GAMEOVER) {
        return solution.str();
      }
      if (result == CLEAR) {
        // solution << c;
        return solution.str();
      }
      solution << c;
    }
  }

  return "";
}
