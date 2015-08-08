#include "game.h"

#include <SDL.h>
#include <SDL_image.h>
//#include <SDL_ttf.h>

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
  int TILE_SIZE = 32;
  const int TEXT_TILE_SIZE = 20;

  bool Init() {
    if (!CreateSDL()) { return false; }
    return true;
  }
  bool CreateSDL();
  SDL_Texture* LoadTexture(const std::string &file, SDL_Renderer *ren);
  void DestroySDL();

  void DrawText(int x, int y, const char* format, ...);
  void Draw(int x, int y, Image image);
  int CellX(int x, int y);
  int CellY(int x, int y);
  void DrawGameState(const Game &game);
};
