#include "ai.h"

#include <sstream>

using namespace std;

void AI::Init() {}

string AI::Run(const Game& game) {
  State state;
  state.Init(game);

  stringstream solution;
  do {
    char next = Step(game, state);
    solution << next;
  } while (false);
  // } while (!state.Command(game, next));
  return solution.str();
}

char AI::Step(const Game&, const State&) {
  return 'b';
}
