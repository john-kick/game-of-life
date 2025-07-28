# John Kick's Game of Life

This is my implementation of [Conway's Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life). I created this project to learn and practice some C++ with basic GUI.

Currently it only features just three example objects: A Block ("still life"), a blinker (oscillator) and a glider (spaceship). I plan to create some basic controls and an editor to play around with different start configurations.

---

## Setup

### Prerequisites

- Linux system (tested on Ubuntu)
- C++17 compatible compiler (e.g., g++)
- [SDL2 library](https://www.libsdl.org/download-2.0.php) (development headers)

On Ubuntu, you can install SDL2 with:

```sh
sudo apt-get install libsdl2-dev
```

### Build

Clone the repository and build using the provided Makefile:

```sh
git clone <repo-url>
cd game-of-life
make
```

### Run

After building, run the game with:

```sh
./build/game_of_life
```

The window will open and start the simulation with the example
