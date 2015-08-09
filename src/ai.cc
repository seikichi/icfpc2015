#include "ai.h"

#include <iostream>
#include <memory>
#include <sstream>

#include "game.h"
#include "chickai.h"
#include "lightningai.h"
#include "kichiai.h"
#include "echo_ai.h"

using namespace std;

void AI::Init(int time_limit_seconds_) {
    time_limit_seconds = time_limit_seconds_;
}

shared_ptr<AI> AI::CreateAI() {
  string name = "";
  if (getenv("AI") != NULL) { name = getenv("AI"); }

  shared_ptr<AI> result = make_shared<LightningAI>();
  if (name == "kichiai") {
    result = make_shared<KichiAI>();
  } else if (name == "lightningai") {
    result = make_shared<LightningAI>();
  } else if (name == "chickai") {
    result = make_shared<ChickAI>();
  } else if (name == "echoai") {
    result = make_shared<EchoAI>();
  }

  return result;
}
