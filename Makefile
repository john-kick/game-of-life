CXX=g++
CXXFLAGS=-std=c++17 -Iinclude `sdl2-config --cflags`
LDFLAGS=`sdl2-config --libs`
SRC=src/main.cpp
OUT=build/game_of_life

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)