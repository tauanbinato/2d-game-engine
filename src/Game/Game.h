#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../EventBus/EventBus.h"

const int FPS = 60;
const int MILLISECS_PER_FRAME = 1000 / FPS;

class Game {
    private:
        SDL_Window* m_ptrWindow;
        SDL_Renderer* m_ptrRenderer;
        bool m_isRunning = false;
        bool m_isDebug = false;
        int m_millisecsPreviousFrame = 0;
        SDL_Rect m_camera;
        
        std::unique_ptr<Registry> m_registry; // Registry* m_registry;
        std::unique_ptr<AssetStore> m_assetStore;
        std::unique_ptr<EventBus> m_eventBus;

    public:
        Game();
        ~Game();
        void Initialize();
        void Destroy();
        void Run();
        void ProcessInput();
        void Update();
        void Render();
        void Setup();
        void LoadLevel(int level);

        static int m_windowWidth;
        static int m_windowHeight;
        static int m_mapWidth;
        static int m_mapHeight;
};

#endif