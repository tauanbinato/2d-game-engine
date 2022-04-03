#ifndef RENDERMAPEDITORGUISYSTEM_H
#define RENDERMAPEDITORGUISYSTEM_H

#include "../ECS/ECS.h"
#include <SDL2/SDL.h>
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include <imgui/imgui_impl_sdl.h>
#include <glm/glm.hpp>
#include "../MapEditor/MapEditor.h"
#include "../Utilities/imfilebrowser.h"

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
#include <string>
#include <map>

static bool m_show_add_texture = false;
static bool m_show_assets = false;

void ShowAddTexture(std::unique_ptr<ImGui::FileBrowser>& m_fileDialog, std::unique_ptr<AssetStore>& m_assetStore, SDL_Renderer* m_ptrRenderer) {
    if (ImGui::Begin("Add New Texture", &m_show_add_texture, ImGuiWindowFlags_MenuBar))
    {   

        ImGui::Text("Upload new Texture:");
        ImGui::BulletText("Please select a file from tour computer. This can be a tilemap, sprite, etc..");
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Spacing();
        if(ImGui::Button("Upload Texture")){
            m_fileDialog->Open();
        }
        ImGui::Spacing();

        if(m_fileDialog->HasSelected())
        {
            static char str1[128] = "";
            static char* char_arr_texture_path;
            std::string texturePath = m_fileDialog->GetSelected().string();
            char_arr_texture_path = &texturePath[0];
            ImGui::InputText("Selected Texture Path ", char_arr_texture_path, IM_ARRAYSIZE(str1), ImGuiInputTextFlags_ReadOnly);
            ImGui::Spacing();

            ImGui::BulletText("Please, create a texture id for your file.");
            static char asset_id[128] = "";
            ImGui::InputTextWithHint("Texture ID", "ex: tilemap-texture, player..", asset_id, IM_ARRAYSIZE(asset_id), ImGuiInputTextFlags_CharsNoBlank);
            ImGui::Spacing();

            if(ImGui::Button("Create Texture")){
                m_assetStore->AddTexture(m_ptrRenderer, asset_id, char_arr_texture_path);
                m_show_add_texture = false;
            }
            
        }
    }
    ImGui::End();
};

void ShowAssets(std::unique_ptr<ImGui::FileBrowser>& m_fileDialog, std::unique_ptr<AssetStore>& m_assetStore, SDL_Renderer* m_ptrRenderer) {
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Asset Store", &m_show_assets, ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Add Texture")) m_show_add_texture = true;
                if (ImGui::MenuItem("Close")) m_show_assets = false;
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Left
        static int selected = 1;
        static std::string selectedAssetId;
        {
            ImGui::BeginChild("Assets List", ImVec2(150, 0), true);
            std::map<std::string, SDL_Texture*> textures = m_assetStore->GetAllTextures();
            
            for (auto it = ++textures.begin(); it != textures.end(); ++it)
            {
                auto i = std::distance(textures.begin(), it);
                char label[128];
                sprintf(label, "%s", (it->first.c_str()));
                if (ImGui::Selectable(label, selected == i)) {
                    selected = i;
                    selectedAssetId = label;
                }
                    
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();

        // Right
        {
            ImGui::BeginGroup();
            ImGui::BeginChild("Assets View", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
            ImGui::Text("Selected: %d", selected);
            ImGui::Separator();
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Image"))
                {
                    ImGui::Image(m_assetStore->GetTexture(selectedAssetId), ImVec2(200, 200));
                    ImGui::EndTabItem();
                }

                    
                if (ImGui::BeginTabItem("Details"))
                {
                    ImGui::Text("ASSET_ID: %s", selectedAssetId.c_str());
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChild();

            if (ImGui::Button("Revert")) {}
            ImGui::SameLine();
            if (ImGui::Button("Save")) {}
            ImGui::EndGroup();
        }
    }
    ImGui::End();
}

class RenderMapEditorGUISystem: public System {
    public:
        RenderMapEditorGUISystem() = default;

        void Update(
            const std::unique_ptr<Registry>& registry, 
            const SDL_Rect& camera, 
            std::unique_ptr<AssetStore>& m_assetStore, 
            std::unique_ptr<ImGui::FileBrowser>& m_fileDialog,
            SDL_Renderer* m_ptrRenderer
        )
        {
            // Start the Dear ImGui frame
            ImGui::NewFrame();
            

            if(m_show_add_texture) { ShowAddTexture(m_fileDialog, m_assetStore, m_ptrRenderer); };
            if(m_show_assets) { ShowAssets(m_fileDialog, m_assetStore, m_ptrRenderer); };
        
            if (ImGui::BeginMainMenuBar())
	        {
                if (ImGui::BeginMenu("Menu"))
                {
                    if (ImGui::MenuItem("Open Asset Store")) m_show_assets = true;
                    
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
            m_fileDialog->Display();

            
            if (ImGui::Begin("Texture"))
	        {   

                int imageWidth = 0;
                int imageHeight = 0;

                // We need to query the texture to get the image width/height. This is used for setting the src_Rect positions
                if (SDL_QueryTexture(m_assetStore->GetTexture("tilemap"), NULL, NULL, &imageWidth, &imageHeight) != 0)
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

                ImGui::Image(m_assetStore->GetTexture("tilemap"), ImVec2(imageWidth, imageHeight));

                int mousePosX = static_cast<int>(ImGui::GetMousePos().x - ImGui::GetWindowPos().x);
                int mousePosY = static_cast<int>(ImGui::GetMousePos().y - ImGui::GetWindowPos().y - 30);

                int rows = imageHeight / (100 * 2);
                int cols = imageWidth / (200 * 2);

                for (int i = 0; i < cols; i++)
                {
                    for (int j = 0; j < rows; j++)
                    {
                        // auto drawList = ImGui::GetWindowDrawList();

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