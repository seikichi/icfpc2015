#include "visualizer.h"
#include "ai.h"
#include "game.h"

#include <cstdio>
#include <unistd.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "picojson/picojson.h"

using namespace std;

bool Visualizer::CreateSDL() {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
    std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
    exit(1);
  }

  win = SDL_CreateWindow("Honeycomb Tetris", 100, 100, 640, 480, SDL_WINDOW_SHOWN);

  if (win == nullptr){
    std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return false;
    // exit(1);
  }
  ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (ren == nullptr){
    SDL_DestroyWindow(win);
    std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return false;
    // exit(1);
  }

  std::string imagePath = "resources/tiles.png";
  tex = LoadTexture(imagePath, ren);
  if (tex == nullptr){
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    std::cout << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return false;
    // exit(1);
  }
  std::string letterPath = "resources/letters.png";
  letters_image = LoadTexture(letterPath, ren);
  if (tex == nullptr){
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    std::cout << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return false;
    // exit(1);
  }
  SDL_RenderClear(ren);
  SDL_RenderPresent(ren);
  return true;
}

SDL_Texture* Visualizer::LoadTexture(const std::string &file, SDL_Renderer *ren){
  SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str());
  if (texture == nullptr)     
    std::cout << "LoadTexture Error:" << SDL_GetError() << std::endl;
  return texture;
}

void Visualizer::DestroySDL() {
  SDL_DestroyTexture(tex);
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
}

/* (x, y)にテキスト(ただし、数字、アルファベット、空白のみ)を表示する。
 * printf と同じような書き方ができる。
 */
void Visualizer::DrawText(int x, int y, const char* format, ...)
{
  int i;
  va_list ap;
  char buffer[256];

  va_start(ap, format);
  vsnprintf(buffer, 255, format, ap);
  va_end(ap);
  buffer[255] = '\0';
  for (i = 0; buffer[i] != '\0'; ++i) {
    SDL_Rect srcrect;
    SDL_Rect destrect = { x + i * 10, y, 10, 20 };
    if (isdigit(buffer[i])) {           /* 数字 */
      srcrect.x = (buffer[i] - '0') * 10;
      srcrect.y = 20;
    } else if (isalpha(buffer[i])) {    /* アルファベット */
      srcrect.x = (toupper(buffer[i]) - 'A') * 10;
      srcrect.y = 0;
    } else {                            /* それ以外は空白とみなす */
      continue;
    }
    srcrect.w = 10;
    srcrect.h = 20;
    destrect.w = 10;
    destrect.h = 20;
    SDL_RenderCopy(ren, letters_image, &srcrect, &destrect);
  }
}

void Visualizer::Draw(int x, int y, Image image) {
  SDL_Rect srcrect, destrect;
  srcrect.x = (int)image * TILE_SIZE;
  srcrect.y = 0;
  srcrect.w = TILE_SIZE;
  srcrect.h = TILE_SIZE;
  destrect.x = x;
  destrect.y = y;
  destrect.w = TILE_SIZE;
  destrect.h = TILE_SIZE;
  SDL_RenderCopy(ren, tex, &srcrect, &destrect);
}
int Visualizer::CellX(int x, int y) {
  const int dx = TILE_SIZE;
  const int offset = TILE_SIZE / 2;
  return dx * x + (y % 2 * offset);
}
int Visualizer::CellY(int x, int y) {
  const int dy = TILE_SIZE - 4;
  return y * dy;
}
void Visualizer::DrawGameState(const Game &game) {
  SDL_RenderClear(ren);
  for (int y = 0; y < game.h; y++) {
    for (int x = 0; x < game.w; x++) {
      if (game.initial[y * game.w + x]) {
        Draw(CellX(x, y), CellY(x, y), Image::FILLED);
      } else if (false) {
        Draw(CellX(x, y), CellY(x, y), Image::UNIT);
        // TODO draw units
      } else {
        Draw(CellX(x, y), CellY(x, y), Image::EMPTY);
      }
    }
  }
  Draw(CellX(0, 0), CellY(0, 0), Image::PIVOT);
  SDL_RenderPresent(ren);
  // TODO draw pivot
}
