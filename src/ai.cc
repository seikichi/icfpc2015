#include "ai.h"

#include <iostream>
#include <memory>
#include <sstream>

#include "game.h"
#include "chickai.h"
#include "lightningai.h"
#include "kichiai.h"

using namespace std;

void AI::Init() {}

shared_ptr<AI> AI::CreateAI() {
  string name = "";
  if (getenv("AI") != NULL) { name = getenv("AI"); }

  shared_ptr<AI> result = make_shared<KichiAI>();
  if (name == "kichiai") {
    result = make_shared<KichiAI>();
  } else if (name == "lightningai") {
    result = make_shared<LightningAI>();
  } else if (name == "chickai") {
    result = make_shared<ChickAI>();
  }

  return result;
}
