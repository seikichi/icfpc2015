#include "ai.h"
#include "game.h"
#include "visualizer.h"
#include "kichiai.h"
#include "manual_player.h"
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

void EventLoopAI(const Game& game, const std::string& commands) {
  SDL_Event event;
  double next_frame = SDL_GetTicks();
  double wait = 1000.0 / 60;
  int prev_z = 0;
  int prev_x = 0;
  int prev_c = 0;

  CommandResult command_result = MOVE;
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

      visualizer.BeginDraw();
      visualizer.DrawGameState(game, replay.GetCurrentState());
      visualizer.DrawCommandResult(game, command_result);
      visualizer.EndDraw();
      next_frame += wait;
      // SDL_Delay(1);
    }
    SDL_Delay(5);
  }
end:;
}

bool Pushed(int now, int prev) {
  return now && !prev;
}

void EventLoopManual(const Game& game) {
  SDL_Event event;
  double next_frame = SDL_GetTicks();
  double wait = 1000.0 / 60;
  int prev_keys[256];
  memset(prev_keys, 0, sizeof(prev_keys));
  vector<pair<char, int> > target_keys = {
    { 'a', SDLK_a },
    { 'z', SDLK_z },
    { 'x', SDLK_x },
    { 'd', SDLK_d },
    { 'j', SDLK_j },
    { 'k', SDLK_k },
    { ' ', SDLK_SPACE },
  };

  CommandResult command_result = MOVE;

  ManualPlayer player;
  player.Init(game);

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
    }
    /* 1秒間に60回Updateされるようにする */
    if (SDL_GetTicks() >= next_frame) {
      const Uint8* keys = SDL_GetKeyboardState(NULL);
      int now_keys[256];
      memset(now_keys, 0, sizeof(now_keys));
      char c = 0;
      for (auto key : target_keys) {
        now_keys[(int)key.first] = keys[SDL_GetScancodeFromKey(key.second)];
        if (Pushed(now_keys[(int)key.first], prev_keys[(int)key.second])) {
          c = key.first;
        }
      }
      if (c != 0) {
        command_result = player.Move(game, c);
      }
      memcpy(prev_keys, now_keys, sizeof(prev_keys));

      visualizer.BeginDraw();
      visualizer.DrawGameState(game, player.GetCurrentState());
      visualizer.DrawCommandResult(game, command_result);
      visualizer.EndDraw();
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
  bool manual_play = false;

  int result;
  while ((result = getopt(argc, argv, "f:t:m:c:p:r:i")) != -1) {
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
      case 'i':
        manual_play = true;
        break;
      default:
        break;
    }
  }

  // process
  for (const auto& problem_file : problem_files) {
    Game game;
    auto source_seed_idx = 0;

    ifstream ifs(problem_file.c_str());
    string problem((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());;
    while (game.Init(problem, source_seed_idx++)) {
      if (!manual_play) {
        auto ai = AI::CreateAI();
        ai->Init();
        string commands = ai->Run(game);
        EventLoopAI(game, commands);
      } else {
        EventLoopManual(game);
      }


      break;
    }
    break;
  }
  return 0;
}
