#include "ai.h"

#include <memory>
#include <sstream>

#include "chickai.h"

using namespace std;

void AI::Init() {}

string AI::Run(const Game& game) {
  State state;
  state.Init(game);

  stringstream solution;
  while (true) {
    string next = Step(game, state, game.CurrentUnit(state.source_idx));

    for (const char c : next) {
      if (!state.Command(game, c)) {
        return solution.str();
      }
      solution << c;
    }
  }
  return solution.str();
}

shared_ptr<AI> AI::CreateAI(const std::string& name) {
  if (name == "chickai") {
    return make_shared<ChickAI>();
  }
  return shared_ptr<AI>(NULL);
}
