#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include <sol/sol.hpp>
#include <SDL2/SDL.h>
#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../EventBus/EventBus.h"


const int MAP_EDITOR_FPS = 60;
const int MAP_EDITOR_MILLISECS_PER_FRAME = 1000 / MAP_EDITOR_FPS;

class MapEditor {
    private:
        sol::state m_lua;
        SDL_Window* m_ptrWindow;
        SDL_Renderer* m_ptrRenderer;
        bool m_isRunning = false;
        int m_millisecsPreviousFrame = 0;
        SDL_Rect m_camera;
        float mZoom;
        bool m_demoWindow = false;

        const int m_WINDOW_BAR = 25;
	    const int m_WINDOW_HEIGHT = 1080 - m_WINDOW_BAR;
	    const int m_WINDOW_WIDTH = 1920;
	    const int m_DEFAULT_ZOOM = 1;
	    const int m_DEFAULT_CAM_X = -360;
	    const int m_DEFAULT_CAM_Y = -176;
	    const int m_CAM_SPEED = 10;

        std::unique_ptr<Registry> m_registry; // Registry* m_registry;
        std::unique_ptr<AssetStore> m_assetStore;
        std::unique_ptr<EventBus> m_eventBus;

        void InitializeImGUIConfigs();

    public:
        MapEditor();
        ~MapEditor();
        void Initialize();
        void Destroy();
        void Run();
        void ProcessInput();
        void Update();
        void Render();
        void Setup();
    
};

#endif