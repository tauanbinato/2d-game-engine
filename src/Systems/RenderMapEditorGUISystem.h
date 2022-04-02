#ifndef RENDERMAPEDITORGUISYSTEM_H
#define RENDERMAPEDITORGUISYSTEM_H

#include "../ECS/ECS.h"
#include <SDL2/SDL.h>
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include <imgui/imgui_impl_sdl.h>
#include <glm/glm.hpp>
#include "../MapEditor/MapEditor.h"

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
#include "../Components/TextLabelComponent.h"
#include "../Utilities/IconsFontAwesome.h"

static void ShowExampleMenuFile()
{
    ImGui::MenuItem("(demo menu)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse.."))
            {
                ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}

    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }

    // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // In a real code-base using it would make senses to use this feature from very different code locations.
    if (ImGui::BeginMenu("Options")) // <-- Append!
    {
        static bool b = true;
        ImGui::Checkbox("SomeOption", &b);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}

class RenderMapEditorGUISystem: public System {
    public:
        RenderMapEditorGUISystem() = default;

        void Update(const std::unique_ptr<Registry>& registry, const SDL_Rect& camera, std::unique_ptr<AssetStore>& assetStore){
            // Start the Dear ImGui frame
            ImGui::NewFrame();

            if (ImGui::BeginMainMenuBar())
	        {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Add Tileset"))
                    {
                        
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
            
            if (ImGui::Begin("Texture"))
	        {   
                ImGuiIO& io = ImGui::GetIO();

                int imageWidth = 0;
                int imageHeight = 0;

                // We need to query the texture to get the image width/height. This is used for setting the src_Rect positions
                if (SDL_QueryTexture(assetStore->GetTexture("tilemap"), NULL, NULL, &imageWidth, &imageHeight) != 0)
                {
                    const char* errMsg = SDL_GetError();
                    Logger::Error("__FUNC: Load Tileset: " + std::string(errMsg));
                    //mImageLoaded = false;
                }
                else
                {
                    //mImageLoaded = true;
                    // Both of these will be used for saving the project file and 
                    // for access of the assetID via ImGui combo-box
                    //mLoadedTilesets.push_back(mAssetID);
                    //mTilesetLocations.push_back(mImageName);
                }

                ImGui::Image(assetStore->GetTexture("tilemap"), ImVec2(imageWidth, imageHeight));

                int mousePosX = static_cast<int>(ImGui::GetMousePos().x - ImGui::GetWindowPos().x);
                int mousePosY = static_cast<int>(ImGui::GetMousePos().y - ImGui::GetWindowPos().y - 30);

                int rows = imageHeight / (100 * 2);
                int cols = imageWidth / (200 * 2);

                for (int i = 0; i < cols; i++)
                {
                    for (int j = 0; j < rows; j++)
                    {
                        auto drawList = ImGui::GetWindowDrawList();

                        // Check to see if we are in the area of the desired 2D tile
                        if ((mousePosX >= (imageWidth / cols) * i && mousePosX <= (imageWidth / cols) + ((imageWidth / cols) * i))
                            && (mousePosY >= (imageHeight / rows) * j && mousePosY <= (imageHeight / rows) + ((imageHeight / rows) * j)))
                        {
                            if (ImGui::IsItemHovered())
                            {

                                if (ImGui::IsMouseClicked(0))
                                {
                                    //mSrcRectX = i * mouseRect.x;
                                    //mSrcRectY = j * mouseRect.y;
                                }
                            }
                        }
                    }
                }
            }
            ImGui::End();

          

            ImGui::Render();
            ImGuiSDL::Render(ImGui::GetDrawData());
        }
};

#endif