#include <iostream>
#include "./Game/Game.h"
#include "./MapEditor/MapEditor.h"

int main(int argc, char* argv[]) {

    //Game game;
    //game.Initialize();
    //game.Run();
    //game.Destroy();
    MapEditor mapEditor;
    mapEditor.Initialize();
    mapEditor.Run();
    mapEditor.Destroy();

    return 0;
}
