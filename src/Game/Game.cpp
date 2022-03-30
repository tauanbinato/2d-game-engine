#include "Game.h"
#include "../ECS/ECS.h"
#include "../Logger/Logger.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"
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
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>
#include <fstream>


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

void Game::LoadLevel(int level){

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

    // Adding assets to the asset store
    m_assetStore->AddTexture(m_ptrRenderer, "tank-image", "./assets/images/tank-panther-right.png");
    m_assetStore->AddTexture(m_ptrRenderer, "chopper-image", "./assets/images/chopper-spritesheet.png");
    m_assetStore->AddTexture(m_ptrRenderer, "radar-image", "./assets/images/radar.png");
    m_assetStore->AddTexture(m_ptrRenderer, "truck-image", "./assets/images/truck-ford-right.png");
    m_assetStore->AddTexture(m_ptrRenderer, "tilemap-image", "./assets/tilemaps/jungle.png");
    m_assetStore->AddTexture(m_ptrRenderer, "bullet-image", "./assets/images/bullet.png");
   

    // Load the tilemap
    int tileSize = 32;
    double tileScale = 4;
    int mapNumCols = 25;
    int mapNumRows = 20;

    std::fstream mapFile;
    mapFile.open("./assets/tilemaps/jungle.map");

    for (int y = 0; y < mapNumRows; y++) {
        for (int x = 0; x < mapNumCols; x++) {
            char ch;
            mapFile.get(ch);
            int srcRectY = std::atoi(&ch) * tileSize;
            mapFile.get(ch);
            int srcRectX = std::atoi(&ch) * tileSize;
            mapFile.ignore();

            Entity tile = m_registry->CreateEntity();
            tile.AddComponent<TransformComponent>(glm::vec2(x * (tileScale * tileSize), y * (tileScale * tileSize)), glm::vec2(tileScale, tileScale), 0.0);
            tile.AddComponent<SpriteComponent>("tilemap-image", tileSize, tileSize, srcRectX, srcRectY, 0, false);
        }
    }
    mapFile.close();
    m_mapWidth = mapNumCols * tileSize * tileScale;
    m_mapHeight = mapNumRows * tileSize * tileScale;

    // We need to load the tilemap from ./assets/tilemaps/jungle.png
    // We need to load the file ./assets/tilemaps/jungle.map

    // Create some entities
    Entity chopper = m_registry->CreateEntity();
    chopper.AddComponent<TransformComponent>(glm::vec2(20.0, 20.0), glm::vec2(2.0, 2.0), 0.0);
    chopper.AddComponent<RigidBodyComponent>(glm::vec2(120, 0.0));
    chopper.AddComponent<SpriteComponent>("chopper-image", 32, 32, 0, 0, 2, false);
    chopper.AddComponent<AnimationComponent>(2, 15, true);
    chopper.AddComponent<KeyboardControlledComponent>(glm::vec2(0.0, -120.0), glm::vec2(120.0, 00.0), glm::vec2(00.0, 120.0), glm::vec2(-120.0, 00.0));
    chopper.AddComponent<CameraFollowComponent>();
    chopper.AddComponent<ProjectileEmitterComponent>(glm::vec2(500.0, 500.0), 300, 5000, 0, true);
    chopper.AddComponent<HealthComponent>(100);

    Entity radar = m_registry->CreateEntity();
    radar.AddComponent<TransformComponent>(glm::vec2(m_windowWidth - 75, 10.0), glm::vec2(1.0, 1.0), 0.0);
    radar.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    radar.AddComponent<SpriteComponent>("radar-image", 64, 64, 0, 0, 3, true);
    radar.AddComponent<AnimationComponent>(8, 6, true);


    Entity tank = m_registry->CreateEntity();
    tank.AddComponent<TransformComponent>(glm::vec2(100.0, 10.0), glm::vec2(1.0, 1.0), 0.0);
    tank.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    tank.AddComponent<SpriteComponent>("tank-image", 32, 32, 0, 0, 2);
    tank.AddComponent<BoxColliderComponent>(32, 32);
    tank.AddComponent<ProjectileEmitterComponent>(glm::vec2(300.0, 0.0), 1000, 5000, 0, false);
    tank.AddComponent<HealthComponent>(100);

    Entity truck = m_registry->CreateEntity();
    truck.AddComponent<TransformComponent>(glm::vec2(1000.0, 10.0), glm::vec2(1.0, 1.0), 0.0);
    truck.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    truck.AddComponent<SpriteComponent>("truck-image", 32, 32, 0, 0, 1);
    truck.AddComponent<BoxColliderComponent>(32, 32);
    truck.AddComponent<ProjectileEmitterComponent>(glm::vec2(300.0, 0.0), 1000, 5000, 0, false);
    truck.AddComponent<HealthComponent>(100);

};

void Game::Setup(){
  LoadLevel(1);
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
    m_registry->GetSystem<KeyboardControlSystem>().SubscribeToKeyPressedEvents(m_eventBus);
    m_registry->GetSystem<ProjectileEmitSystem>().SubscribeToSpaceBarEvent(m_eventBus);

    // Updating our systems
    m_registry->GetSystem<MovementSystem>().Update(deltaTime);
    m_registry->GetSystem<AnimationSystem>().Update();
    m_registry->GetSystem<CollisionSystem>().Update(true, m_eventBus);
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
    if(m_isDebug) {
        m_registry->GetSystem<DebugCollisionSystem>().Update(m_ptrRenderer, m_camera);
    }
    

    // Swap back buffer with front buffer.
    SDL_RenderPresent(m_ptrRenderer);
};


// Create an SDL Window and Renderer.
void Game::Initialize(){
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        Logger::Error("Error initializing SDL Init.");
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

    m_isRunning = true;
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
        switch (sdlEvent.type)
        {
        case SDL_QUIT:
            m_isRunning = false;
            break;
        
        case SDL_KEYDOWN:
            if(sdlEvent.key.keysym.sym == SDLK_ESCAPE) {
                m_isRunning = false;
            }
            if(sdlEvent.key.keysym.sym == SDLK_p) {
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
    SDL_DestroyRenderer(m_ptrRenderer);
    SDL_DestroyWindow(m_ptrWindow);
    SDL_Quit();
};
