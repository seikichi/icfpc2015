#include "ai.h"

#include <iostream>
#include <memory>
#include <sstream>

#include "game.h"
#include "chickai.h"
#include "kichiai.h"

using namespace std;

void AI::Init() {}

string AI::Run(const Game& game) {
  State state;
  state.Init(game);

  stringstream solution;
  while (true) {
    string next = Step(game, state);
    if (next == "") { break; }
    for (const char c : next) {
      CommandResult result = state.Command(game, c);
      if (result == ERROR || result == GAMEOVER) {
        return solution.str();
      }
      if (result == CLEAR) {
        solution << c;
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
  } else if (name == "kichiai") {
    return make_shared<KichiAI>();
  }
  cerr << "AHOKA: Invalid AI (plese set AI env. variable)" << endl;
  exit(1);
  return shared_ptr<AI>(NULL);
}
