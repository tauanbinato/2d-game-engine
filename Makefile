##############################################################################
# Declare Makefile variables
# ###########################################################################
CC = g++
LAND_STD = -std=c++17
COMPILER_FLAGS = -Wall -Wfatal-errors
INCLUDE_PATH = -I"./libs/"
SRC_FILES = ./src/*.cpp \
	    ./src/Game/*.cpp \
	    ./src/Logger/*.cpp \
	    ./src/ECS/*.cpp \
	    ./src/AssetStore/*.cpp \
	    ./libs/imgui/*.cpp
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -llua5.3
OBJ_NAME = gameengine

##############################################################################
# Declare Makefile rules
# ############################################################################

build:
	$(CC) $(COMPILER_FLAGS) $(LAND_STD) $(INCLUDE_PATH) $(SRC_FILES) $(LINKER_FLAGS) -o $(OBJ_NAME);

run:
	make build && ./$(OBJ_NAME)

debug: 
	$(CC) $(COMPILER_FLAGS) $(LAND_STD) $(INCLUDE_PATH) $(SRC_FILES) $(LINKER_FLAGS) -o $(OBJ_NAME) -g;

clean:
	rm $(OBJ_NAME)
