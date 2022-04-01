#include "Game.h"
#include "../ECS/ECS.h"
#include "../Logger/Logger.h"
#include "./LevelLoader.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/DebugCollisionSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/KeyboardControlSystem.h"
#include "../Systems/CameraMovementSystem.h"
#include "../Systems/ProjectileEmitSystem.h"
#include "../Systems/ProjectileLifeCycleSystem.h"
#include "../Systems/RenderTextSystem.h"
#include "../Systems/RenderHealthBarSystem.h"
#include "../Systems/RenderGUISystem.h"
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include <imgui/imgui_impl_sdl.h>

int Game::m_windowHeight;
int Game::m_windowWidth;
int Game::m_mapHeight;
int Game::m_mapWidth;


// Constructor
Game::Game(){
    m_isRunning = false;
    m_isDebug = false;

    // Creating our Registry Manager of ECS
    m_registry = std::make_unique<Registry>();
    m_assetStore = std::make_unique<AssetStore>();
    m_eventBus = std::make_unique<EventBus>();
    Logger::Log("Game constructor called");
};

// Destructor
Game::~Game(){
    Logger::Log("Game destructor called");
    Game::Destroy();
};

// Create an SDL Window and Renderer.
void Game::Initialize(){
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        Logger::Error("Error initializing SDL Init.");
        return;
    }

    if (TTF_Init() != 0) {
        Logger::Error("Error initializing SDL TTF.");
        return;
    }

    // Getting window sizes
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    m_windowWidth = 2560;
    m_windowHeight = 1080;

    m_ptrWindow = SDL_CreateWindow(
        NULL, 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        m_windowWidth,
        m_windowHeight,
        SDL_WINDOW_BORDERLESS
    );

    if (!m_ptrWindow) {
        Logger::Error("Error creating SDL Window.");
        return;
    }

    m_ptrRenderer = SDL_CreateRenderer(m_ptrWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!m_ptrRenderer) {
        Logger::Error("Error creating SDL Renderer.");
        return;
    }

    SDL_SetWindowFullscreen(m_ptrWindow, SDL_WINDOW_FULLSCREEN);

    //Initialize the camera view with the entire screen area
    m_camera = { 0, 0, m_windowWidth, m_windowHeight };

    // Initialize the ImGui with SDL2 context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiSDL::Initialize(m_ptrRenderer, m_windowWidth, m_windowHeight);

    m_isRunning = true;
};

void Game::Setup(){

    // Add systems that need to be processed in our game
    m_registry->AddSystem<MovementSystem>();
    m_registry->AddSystem<RenderSystem>();
    m_registry->AddSystem<AnimationSystem>();
    m_registry->AddSystem<CollisionSystem>();
    m_registry->AddSystem<DebugCollisionSystem>();
    m_registry->AddSystem<DamageSystem>();
    m_registry->AddSystem<KeyboardControlSystem>();
    m_registry->AddSystem<CameraMovementSystem>();
    m_registry->AddSystem<ProjectileEmitSystem>();
    m_registry->AddSystem<ProjectileLifeCycleSystem>();
    m_registry->AddSystem<RenderTextSystem>();
    m_registry->AddSystem<RenderHealthBarSystem>();
    m_registry->AddSystem<RenderGUISystem>();
    
    //load the first level
    LevelLoader loader;
    m_lua.open_libraries(sol::lib::base, sol::lib::math);
    loader.LoadLevel(m_lua, m_registry, m_assetStore, m_ptrRenderer, 1);
}

void Game::Update(){
    
    //Time to wait
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - m_millisecsPreviousFrame);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME) {
        // Clamp to the target time each frame should take based on our target FPS.
        SDL_Delay(timeToWait);
    }

    // The diff in ticks since the last frame, converted to seconds.
    double deltaTime = (SDL_GetTicks() - m_millisecsPreviousFrame)/ 1000.0;

    // How many millisecs have passed?
    m_millisecsPreviousFrame = SDL_GetTicks();  

    //Reset all event handlers
    m_eventBus->ClearSubscribers();

    // Perform the subscribtion of the events for all systems
    m_registry->GetSystem<DamageSystem>().SubscribeToCollisionEvent(m_eventBus);
    m_registry->GetSystem<MovementSystem>().SubscribeToCollisionEvent(m_eventBus);
    m_registry->GetSystem<KeyboardControlSystem>().SubscribeToKeyPressedEvents(m_eventBus);
    m_registry->GetSystem<ProjectileEmitSystem>().SubscribeToSpaceBarEvent(m_eventBus);

    // Updating our systems
    m_registry->GetSystem<MovementSystem>().Update(deltaTime);
    m_registry->GetSystem<AnimationSystem>().Update();
    m_registry->GetSystem<CollisionSystem>().Update(false, m_eventBus);
    m_registry->GetSystem<CameraMovementSystem>().Update(m_camera);
    m_registry->GetSystem<ProjectileEmitSystem>().Update(m_registry);
    m_registry->GetSystem<ProjectileLifeCycleSystem>().Update();
    

    // Update the registry to process the entities that are in the buffer
    m_registry->Update();
};

void Game::Render(){
    // Grey background
    SDL_SetRenderDrawColor(m_ptrRenderer, 21, 21, 21, 255);
    SDL_RenderClear(m_ptrRenderer);

    // Rendering our systems
    m_registry->GetSystem<RenderSystem>().Update(m_ptrRenderer, m_assetStore, m_camera);
    m_registry->GetSystem<RenderTextSystem>().Update(m_ptrRenderer, m_assetStore, m_camera);
    m_registry->GetSystem<RenderHealthBarSystem>().Update(m_ptrRenderer, m_assetStore, m_camera);

    // Debug Mode
    if(m_isDebug) {
        m_registry->GetSystem<DebugCollisionSystem>().Update(m_ptrRenderer, m_camera);

        m_registry->GetSystem<RenderGUISystem>().Update(m_registry, m_camera);
    }


    // Swap back buffer with front buffer.
    SDL_RenderPresent(m_ptrRenderer);
};


// Engine loop
void Game::Run(){

    Game::Setup();

    while(m_isRunning) {
        Game::ProcessInput();
        Game::Update();
        Game::Render();
    }
};

void Game::ProcessInput(){
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        //Handling Imgui SDL
        ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
        ImGuiIO& io = ImGui::GetIO();

        int mouseX, mouseY;
        const int buttons = SDL_GetMouseState(&mouseX, &mouseY);

        io.MousePos = ImVec2(mouseX, mouseY);
        io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
        io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

        // Handling core sdl event
        switch (sdlEvent.type)
        {
        case SDL_QUIT:
            m_isRunning = false;
            break;
        
        case SDL_KEYDOWN:
            if(sdlEvent.key.keysym.sym == SDLK_ESCAPE) {
                m_isRunning = false;
            }
            if(sdlEvent.key.keysym.sym == SDLK_F1) {
                m_isDebug = !m_isDebug;
            }
            if (sdlEvent.key.keysym.sym == SDLK_SPACE) {
                m_eventBus->EmitEvent<ShootProjectileEvent>(m_registry);
            }

            // Emiting KeyDown Events
            m_eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.keysym.sym);
            break;
        default:
            break;
        }
    }
};

void Game::Destroy(){
    ImGuiSDL::Deinitialize();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(m_ptrRenderer);
    SDL_DestroyWindow(m_ptrWindow);
    SDL_Quit();
};
