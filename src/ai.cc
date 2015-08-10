#include "ai.h"

#include <iostream>
#include <memory>
#include <sstream>

#include "game.h"
#include "time_keeper.h"
#include "chickai.h"
#include "kichiai.h"
#include "echo_ai.h"
#include "submarineai.h"

using namespace std;

void AI::Init(const TimeKeeper& tk) {
  time_keeper = &tk;
}

shared_ptr<AI> AI::CreateAI() {
  string name = "";
  if (getenv("AI") != NULL) { name = getenv("AI"); }

  shared_ptr<AI> result;
  if (name == "kichiai") {
    result = make_shared<KichiAI>();
  } else if (name == "chickai") {
    result = make_shared<ChickAI>();
  } else if (name == "echoai") {
    result = make_shared<EchoAI>();
  } else if (name == "submarineai" || name == "") {
    result = make_shared<SubmarineAI>();
  } else {
    cerr << "Invalid AI name: " << name << endl;
    exit(1);
  }

  return result;
}
