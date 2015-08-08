#include "ai.h"

#include <iostream>
#include <memory>
#include <sstream>

#include "game.h"
#include "chickai.h"
#include "kichiai.h"

using namespace std;

void AI::Init() {}

shared_ptr<AI> AI::CreateAI(const std::string& name) {
  if (name == "chickai") {
    return make_shared<ChickAI>();
  } else if (name == "kichiai") {
    return make_shared<KichiAI>();
  }

  return make_shared<KichiAI>();
}
