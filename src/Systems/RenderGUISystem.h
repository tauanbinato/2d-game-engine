#ifndef RENDERGUISYSTEM_H
#define RENDERGUISYSTEM_H

#include "../ECS/ECS.h"
#include <SDL2/SDL.h>
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include <imgui/imgui_impl_sdl.h>
#include <glm/glm.hpp>

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

class RenderGUISystem: public System {
    public:
        RenderGUISystem() = default;

        void Update(const std::unique_ptr<Registry>& registry){
            ImGui::NewFrame();

            if(ImGui::Begin("Enemy Spawner")){
                ImGui::TextWrapped("[Transform Component]");
                // Transform Component ------------
                static int enemyXPos = 500;
                static int enemyYPos = 500;
                ImGui::InputInt("Enemy X Position", &enemyXPos);
                ImGui::InputInt("Enemy Y Position", &enemyYPos);

                static float enemyXScale = 1.0;
                static float enemyYScale = 1.0;
                ImGui::InputFloat("Enemy X Scale", &enemyXScale);
                ImGui::InputFloat("Enemy Y Scale", &enemyYScale);

                static float enemyRotation = 0.0;
                ImGui::InputFloat("Enemy Rotation (Angle)", &enemyRotation);

                // Rigidbody Component -------------
                ImGui::TextWrapped("[RigidBody Component]");
                static float enemyVelocityX = 0.0;
                static float enemyVelocityY = 0.0;
                ImGui::InputFloat("Enemy Velocity X", &enemyVelocityX);
                ImGui::InputFloat("Enemy Velocity Y", &enemyVelocityY);

                // Sprite Component -------------
                ImGui::TextWrapped("[Sprite Component]");
                const char* assets_ids[] = { "tank-image","chopper-image","radar-image","truck-image" };
                static int assetId = 1;
                ImGui::Combo("Assets in Store", &assetId, assets_ids, IM_ARRAYSIZE(assets_ids));
                static int enemySpriteWidth = 32;
                static int enemySpriteHeight = 32;
                ImGui::InputInt("Enemy Sprite Width", &enemySpriteWidth);
                ImGui::InputInt("Enemy Sprite Height", &enemySpriteHeight);
                static int enemySpriteSourceRectX = 0;
                static int enemySpriteSourceRectY = 0;
                ImGui::InputInt("Enemy Sprite Source Rect X", &enemySpriteSourceRectX);
                ImGui::InputInt("Enemy Sprite Source Rect Y", &enemySpriteSourceRectY);
                static int enemySpriteZIndex = 2;
                ImGui::InputInt("Enemy Sprite Z Index", &enemySpriteZIndex);

                // BoxColliderComponent ----------------
                ImGui::TextWrapped("[Box Collider Component]");
                static int enemyBoxColliderX = 32;
                static int enemyBoxColliderY = 32;
                ImGui::InputInt("Enemy X Position", &enemyBoxColliderX);
                ImGui::InputInt("Enemy X Position", &enemyBoxColliderY);
                

                // ProjectileEmitterComponent -----------
                ImGui::TextWrapped("[Projectile Emitter Component]");
                static float enemyEmitterVelocityX = 0.0;
                static float enemyEmitterVelocityY = 0.0;
                ImGui::InputFloat("Enemy Projectile Velocity X", &enemyEmitterVelocityX);
                ImGui::InputFloat("Enemy Projectile Velocity Y", &enemyEmitterVelocityY);
                static float enemyEmitterRateOfFire = 0.0;
                ImGui::InputFloat("Enemy Projectile Emitter Rate Of Fire (ms)", &enemyEmitterRateOfFire);
                static float enemyEmitterDuration = 0.0;
                ImGui::InputFloat("Enemy Projectile Duration (ms)", &enemyEmitterDuration);
                static int enemyEmitterDamagePercentage = 0.0;
                ImGui::InputInt("Enemy Projectile Damage Percentage (%)", &enemyEmitterDamagePercentage);
                static bool enemyEmitterIsFriendly = false;
                ImGui::Checkbox("Is enemy emitter friendly?", &enemyEmitterIsFriendly);

                // HealthComponent -----------
                static int enemyHealthPercentage = 100;
                ImGui::InputInt("Enemy Health", &enemyHealthPercentage);

                if(ImGui::Button("Create new enemy")){
                    Entity enemy = registry->CreateEntity();
                    enemy.Group("enemies");
                    enemy.AddComponent<TransformComponent>(glm::vec2(enemyXPos, enemyYPos), glm::vec2(enemyXScale, enemyYScale), enemyRotation);
                    enemy.AddComponent<RigidBodyComponent>(glm::vec2(enemyVelocityX, enemyVelocityY));
                    enemy.AddComponent<SpriteComponent>(assets_ids[assetId], enemySpriteWidth, enemySpriteHeight, enemySpriteSourceRectX, enemySpriteSourceRectY, enemySpriteZIndex);
                    enemy.AddComponent<BoxColliderComponent>(enemyBoxColliderX, enemyBoxColliderY);
                    enemy.AddComponent<ProjectileEmitterComponent>(glm::vec2(enemyEmitterVelocityX, enemyEmitterVelocityY), enemyEmitterRateOfFire, enemyEmitterDuration, enemyEmitterDamagePercentage, enemyEmitterIsFriendly);
                    enemy.AddComponent<HealthComponent>(enemyHealthPercentage);
                }
            }
            ImGui::End();

            

            ImGui::Render();
            ImGuiSDL::Render(ImGui::GetDrawData());
        }
};

#endif