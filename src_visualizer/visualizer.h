#include "game.h"

#include <SDL.h>
#include <SDL_image.h>

struct Visualizer {
  enum class Image {
    EMPTY,
    FILLED,
    UNIT,
    PIVOT,
  };
  SDL_Window *win = nullptr;
  SDL_Renderer *ren = nullptr;
  SDL_Texture *tex = nullptr;
  SDL_Texture *letters_image = nullptr;
  int tile_size = 32;
  const int TEXT_TILE_SIZE = 20;

  bool Init(const Game& game) {
    if (!CreateSDL(game)) { return false; }
    return true;
  }
  virtual ~Visualizer() { DestroySDL(); }
  bool CreateSDL(const Game& game);
  SDL_Texture* LoadTexture(const std::string &file, SDL_Renderer *ren);
  void DestroySDL();

  void DrawText(int x, int y, const char* format, ...);
  void Draw(int x, int y, Image image);
  int CellX(const Cell& cell);
  int CellY(const Cell& cell);
  void BeginDraw();
  void DrawGameState(const Game& game, const State& state);
  void DrawNext(const Game& game, const State& state);
  void DrawCommandResult(const Game& game, const CommandResult result);
  void EndDraw();
  int GetBoardWidth(const Game& game) {
    return game.w * tile_size;
  }
  int GetBoardHeight(const Game& game) {
    return game.h * tile_size * 7 / 8;
  }
};
