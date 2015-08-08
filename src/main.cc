#include "ai.h"
#include "chickai.h"
#include "game.h"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

#include "picojson/picojson.h"
using namespace std;

int main(int argc, char** argv) {
  // input
  vector<string> problem_files;
  int time_limit_seconds;
  int memory_limit;
  int cores;
  vector<string> phrases_of_power;

  int result;
  while ((result = getopt(argc, argv, "f:t:m:c:p:")) != -1) {
    switch (result) {
    case 'f':
      problem_files.push_back(optarg);
      break;
    case 't':
      time_limit_seconds = stoi(optarg);
      break;
    case 'm':
      memory_limit = stoi(optarg);
      break;
    case 'c':
      cores = stoi(optarg);
    case 'p':
      phrases_of_power.push_back(optarg);
      break;
    default:
      break;
    }
  }

  // process
  stringstream ss;
  ss << "[";
  bool first = true;
  for (const auto& problem_file : problem_files) {
    Game game;
    auto source_seed_idx = 0;

    ifstream ifs(problem_file.c_str());
    string problem((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());;

    while (game.Init(problem, source_seed_idx++)) {
      auto ai = AI::CreateAI(getenv("AI"));
      ai->Init();

      if (!first) { ss << ","; }
      auto solution = ai->Run(game);
      ss << "{";
      ss << "\"problemId\": " << game.problem_id << ", ";
      ss << "\"seed\": " << game.source_seed << ", ";
      ss << "\"tag\": " << "\"kyoto ni modoritai\"" << ", ";
      ss << "\"solution\": " << "\"" << solution << "\"";
      ss << "}";
      first = false;
    }
  }
  ss << "]";

  // output
  cout << ss.str() << endl;
  return 0;
}
