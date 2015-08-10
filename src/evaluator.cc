#include "evaluator.h"

#include "qwerty_evaluator.h"

using namespace std;

shared_ptr<Evaluator> Evaluator::CreateEvaluator() {
  string name = "";
  if (getenv("EVALUATOR") != NULL) { name = getenv("EVALUATOR"); }

  shared_ptr<Evaluator> result;
  if (name == "qwerty" || name == "") {
    result = make_shared<QwertyEvaluator>();
  } else if (name == "TODO") {
    // TODO
  } else {
    cerr << "Invalid evaluator name: " << name << endl;
    exit(1);
  }

  return result;
}
