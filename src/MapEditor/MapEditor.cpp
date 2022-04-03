#include "MapEditor.h"
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include <imgui/imgui_impl_sdl.h>
#include "../Systems/RenderMapEditorGUISystem.h"

#include <memory>
#include "../Utilities/IconsFontAwesome.h"
#include "../Logger/Logger.h"


MapEditor::MapEditor(){
    m_isRunning = false;

    // Creating our Registry Manager of ECS
    m_registry = std::make_unique<Registry>();
    m_assetStore = std::make_unique<AssetStore>();
    m_eventBus = std::make_unique<EventBus>();
}

MapEditor::~MapEditor(){
    MapEditor::Destroy();
}

void MapEditor::Run(){
    MapEditor::Setup();

    while(m_isRunning) {
        MapEditor::ProcessInput();
        MapEditor::Update();
        MapEditor::Render();
    }
}

void MapEditor::Initialize(){
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        Logger::Error("Error initializing SDL Init.");
        exit(-1);
    }

    if (TTF_Init() != 0) {
        Logger::Error("Error initializing SDL TTF.");
        exit(-1);
    }

    // Getting window sizes
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(2, &displayMode);

    m_ptrWindow = SDL_CreateWindow("Tilemap Editor",
		0, // Place the window in the top left corner 
		m_WINDOW_BAR, // Subtract the window title bar from y position
		m_WINDOW_WIDTH,
		m_WINDOW_HEIGHT,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

    if (!m_ptrWindow) {
        Logger::Error("Error creating SDL Window.");
        exit(-1);
    }

    m_ptrRenderer = SDL_CreateRenderer(m_ptrWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!m_ptrRenderer) {
        Logger::Error("Error creating SDL Renderer.");
        exit(-1);
    }

    // Set the renderer to blend mode
	SDL_SetRenderDrawBlendMode(m_ptrRenderer, SDL_BLENDMODE_BLEND);

    MapEditor::InitializeImGUIConfigs();

    m_isRunning = true;
}

void MapEditor::InitializeImGUIConfigs() {
    // Initialize the ImGui with SDL2 context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
    io.KeyMap[ImGuiKey_KeyPadEnter] = 271;
    io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
    io.KeyMap[ImGuiKey_Space] = SDLK_SPACE;
    io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
    
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable some options
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	ImGuiSDL::Initialize(m_ptrRenderer, m_WINDOW_WIDTH, m_WINDOW_HEIGHT);

	// Initialize Camera --> This Centers the Grid
	m_camera = {
		m_DEFAULT_CAM_X, m_DEFAULT_CAM_Y, m_WINDOW_WIDTH, m_WINDOW_HEIGHT
	};

	// Initialize the mouse box for the tiles
	//mMouseBox.x = 0;
	//mMouseBox.y = 0;
	//mMouseBox.h = 1;
	//mMouseBox.w = 1;
}

void MapEditor::Setup(){
    m_fileDialog = std::make_unique<ImGui::FileBrowser>();
    m_fileDialog->SetTitle("Select a texture");
    m_fileDialog->SetTypeFilters({".jpg", ".png"});

    m_assetStore->ClearAssets();
    m_assetStore->AddTexture(m_ptrRenderer, "tilemap", "./assets/tilemaps/jungle.png");
    m_registry->AddSystem<RenderMapEditorGUISystem>();
}

void MapEditor::Update(){
    //Time to wait
    int timeToWait = MAP_EDITOR_MILLISECS_PER_FRAME - (SDL_GetTicks() - m_millisecsPreviousFrame);
    if (timeToWait > 0 && timeToWait <= MAP_EDITOR_MILLISECS_PER_FRAME) {
        // Clamp to the target time each frame should take based on our target FPS.
        SDL_Delay(timeToWait);
    }

    // The diff in ticks since the last frame, converted to seconds.
    // double deltaTime = (SDL_GetTicks() - m_millisecsPreviousFrame)/ 1000.0;

    // How many millisecs have passed?
    m_millisecsPreviousFrame = SDL_GetTicks();

    // Update the registry to process the entities that are in the buffer
    m_registry->Update();
}

void MapEditor::Render(){
    // Grey background
    SDL_SetRenderDrawColor(m_ptrRenderer, 21, 21, 21, 255);
    SDL_RenderClear(m_ptrRenderer);

    
    if (m_demoWindow) {
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();
        ImGuiSDL::Render(ImGui::GetDrawData());
        ImGui::EndFrame();
    } else {
        m_registry->GetSystem<RenderMapEditorGUISystem>().Update(m_registry, m_camera, m_assetStore, m_fileDialog, m_ptrRenderer);
        ImGui::EndFrame();
    }

    SDL_Rect rect = { 0, 0, 10, 10 };
	SDL_SetRenderDrawColor(m_ptrRenderer, 255, 0, 0, 0);
	SDL_RenderFillRect(m_ptrRenderer, &rect);
	SDL_RenderDrawRect(m_ptrRenderer, &rect);
    

    // Swap back buffer with front buffer.
    SDL_RenderPresent(m_ptrRenderer);
}

void MapEditor::ProcessInput(){

    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {

        //Handling Imgui SDL
        ImGuiIO& io = ImGui::GetIO();
        

        int mouseX, mouseY;
        const int buttons = SDL_GetMouseState(&mouseX, &mouseY);

        io.MousePos = ImVec2(mouseX, mouseY);
        io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
        io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

        if (sdlEvent.type == SDL_WINDOWEVENT && sdlEvent.window.event == SDL_WINDOWEVENT_CLOSE && sdlEvent.window.windowID == SDL_GetWindowID(m_ptrWindow))
            break;

        // Handling core sdl event
        switch (sdlEvent.type)
        {
            case SDL_QUIT:
                m_isRunning = false;
                break;
            
            case SDL_KEYDOWN:
                if (sdlEvent.key.keysym.sym == SDLK_F1) {
                    m_demoWindow = !m_demoWindow;
                    break;
                }
                if(sdlEvent.key.keysym.sym == SDLK_ESCAPE) {
                    m_isRunning = false;
                }

                if(sdlEvent.key.keysym.sym == SDLK_BACKSPACE) {
                    io.KeysDown[sdlEvent.key.keysym.sym] = true;
                }
                
                io.AddInputCharacter(sdlEvent.key.keysym.sym);
                break;
            
            
            case SDL_KEYUP:
                if (sdlEvent.key.keysym.sym == SDLK_BACKSPACE) {
                    io.KeysDown[sdlEvent.key.keysym.sym] = false;
                    break;
                }
                

            default:
                break;
        }
    }
}

void MapEditor::Destroy(){
    ImGui::DestroyContext();
    ImGuiSDL::Deinitialize();
    SDL_DestroyRenderer(m_ptrRenderer);
    SDL_DestroyWindow(m_ptrWindow);
    SDL_Quit();
}

