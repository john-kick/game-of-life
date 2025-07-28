#include <SDL2/SDL.h>
#include <iostream>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define CELL_SIZE 10
#define GRID_WIDTH (WINDOW_WIDTH / CELL_SIZE)
#define GRID_HEIGHT (WINDOW_HEIGHT / CELL_SIZE)
#define FRAMES_PER_SECOND 144

bool getAlive(bool isAlive, uint8_t neighbors) {
  return isAlive ? (neighbors == 2 || neighbors == 3) : (neighbors == 3);
}

bool **updateGrid(bool **oldGrid) {
  bool **newGrid = new bool *[GRID_HEIGHT];
  for (int i = 0; i < GRID_HEIGHT; ++i)
    newGrid[i] = new bool[GRID_WIDTH]{false};

  for (int i = 0; i < GRID_HEIGHT; ++i) {
    for (int j = 0; j < GRID_WIDTH; ++j) {
      uint8_t livingNeighbors = 0;

      for (int di = -1; di <= 1; ++di) {
        for (int dj = -1; dj <= 1; ++dj) {
          if (di == 0 && dj == 0)
            continue; // Skip self
          int ni = i + di, nj = j + dj;
          if (ni >= 0 && ni < GRID_HEIGHT && nj >= 0 && nj < GRID_WIDTH)
            livingNeighbors += oldGrid[ni][nj];
        }
      }

      newGrid[i][j] = getAlive(oldGrid[i][j], livingNeighbors);
    }
  }

  for (int i = 0; i < GRID_HEIGHT; ++i)
    delete[] oldGrid[i];
  delete[] oldGrid;

  return newGrid;
}

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError()
              << '\n';
    return 1;
  }

  SDL_Window *window =
      SDL_CreateWindow("Game of Life", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (!window) {
    std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError()
              << '\n';
    SDL_Quit();
    return 1;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

  // Allocate and initialize grid
  bool **grid = new bool *[GRID_HEIGHT];
  for (int i = 0; i < GRID_HEIGHT; ++i)
    grid[i] = new bool[GRID_WIDTH]{false};

  // Block
  grid[1][1] = grid[1][2] = true;
  grid[2][1] = grid[2][2] = true;

  // Blinker
  grid[1][6] = grid[2][6] = grid[3][6] = true;

  // Glider
  grid[1][12] = grid[2][13] = true;
  grid[3][11] = grid[3][12] = grid[3][13] = true;

  bool running = true;
  SDL_Event event;

  while (running) {
    while (SDL_PollEvent(&event))
      if (event.type == SDL_QUIT)
        running = false;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < GRID_HEIGHT; ++i) {
      for (int j = 0; j < GRID_WIDTH; ++j) {
        SDL_SetRenderDrawColor(renderer, grid[i][j] ? 255 : 0,
                               grid[i][j] ? 255 : 0, grid[i][j] ? 255 : 0, 255);
        SDL_Rect cell = {j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE};
        SDL_RenderFillRect(renderer, &cell);
      }
    }

    SDL_RenderPresent(renderer);
    grid = updateGrid(grid);
    SDL_Delay(1000 / FRAMES_PER_SECOND);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
