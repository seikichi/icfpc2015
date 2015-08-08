#include "ai.h"
#include "game.h"
#include "visualizer.h"
#include "kichiai.h"
#include "replay.h"

#include <cstdio>
#include <unistd.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

#include "picojson/picojson.h"
using namespace std;

void EventLoop(const Game& game, const std::string& commands) {
  SDL_Event event;
  double next_frame = SDL_GetTicks();
  double wait = 1000.0 / 60;
  int prev_z = 0;
  int prev_x = 0;
  int prev_c = 0;

  Replay replay;
  replay.Init(game, commands);
  Visualizer visualizer;
  visualizer.Init(game);
  for (;;) {
    /* すべてのイベントを処理する */
    while (SDL_PollEvent(&event)) {
      /* QUIT イベントが発生するか、ESC キーが押されたら終了する */
      if ((event.type == SDL_QUIT) ||
          (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) {
        goto end;
      }
      // if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_r) {
      //   game_state = GameState(field_filename);
      //   ais = GameAI(game_state, lambda_man_ai_filename, ghost_ai_filenames, is_debug);
      // }
    }
    /* 1秒間に60回Updateされるようにする */
    // cout << game_state << endl;
    if (SDL_GetTicks() >= next_frame) {
      // if (!game_state.IsGameOver()) {
        // if (manual) {
          // routine.StepFrame(game_state, ais, wait);
        // } else {
          const Uint8* keys = SDL_GetKeyboardState(NULL);
          int z = keys[SDL_GetScancodeFromKey(SDLK_z)];
          int x = keys[SDL_GetScancodeFromKey(SDLK_x)];
          int c = keys[SDL_GetScancodeFromKey(SDLK_c)];
          // int step = 0;
          if ((z && !prev_z) || x) {
            // step = game_state.lambda_man[0].NextStepTime() - game_state.utc;
          } else if (c && !prev_c) {
            // step = 10000;
          }
          if (!replay.KeyInput(game)) { goto end; }
          // routine.StepFrame(game_state, ais, step);
          prev_z = z;
          prev_x = x;
          prev_c = c;
        // }
      // }

      visualizer.DrawGameState(game, replay.GetCurrentState());
      next_frame += wait;
      // SDL_Delay(1);
    }
    SDL_Delay(5);
  }
end:;
}

int main(int argc, char** argv) {
  // input
  vector<string> problem_files;
  int time_limit_seconds;
  int memory_limit;
  int cores;
  vector<string> phrases_of_power;
  string replay_file;

  int result;
  while ((result = getopt(argc, argv, "f:t:m:c:p:r:")) != -1) {
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
      case 'r':
        replay_file = optarg;
        break;
      default:
        break;
    }
  }

  // process
  stringstream ss;
  // ss << "[";
  bool first = true;
  for (const auto& problem_file : problem_files) {
    Game game;
    auto source_seed_idx = 0;

    ifstream ifs(problem_file.c_str());
    string problem((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());;
    while (game.Init(problem, source_seed_idx++)) {
      auto ai = make_shared<KichiAI>();
      ai->Init();

      if (!first) { ss << ","; }
      string commands = ai->Run(game);

      EventLoop(game, commands);


      // ss << "{";
      // ss << "\"problemId\": " << game.problem_id << ", ";
      // ss << "\"seed\": " << game.source_seed << ", ";
      // ss << "\"tag\": " << "\"kyoto ni modoritai\"" << ", ";
      // ss << "\"solution\": " << "\"" << solution << "\"";
      // ss << "}";
      // first = false;
      break;
    }
    break;
  }
  // ss << "]";

  // output
  cout << ss.str() << endl;
  return 0;
}
