#ifndef DEBUGCOLLISIONSYSTEM_H
#define DEBUGCOLLISIONSYSTEM_H

#include "../Components/TransformComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../ECS/ECS.h"
#include "../Logger/Logger.h"
#include <SDL2/SDL.h>
#include <string>

class DebugCollisionSystem: public System {
    public:
        DebugCollisionSystem() {
            RequireComponent<BoxColliderComponent>();
        }

        void Update(SDL_Renderer* renderer, SDL_Rect& camera) {

            for (auto entity : GetSystemEntities()) {
                auto entityTransform = entity.GetComponent<TransformComponent>();
                auto entityCollider = entity.GetComponent<BoxColliderComponent>();

                SDL_Rect debugRect{
                    static_cast<int>(entityTransform.position.x + entityCollider.offset.x - camera.x),
                    static_cast<int>(entityTransform.position.y + entityCollider.offset.y - camera.y), 
                    static_cast<int>(entityCollider.width * entityTransform.scale.x), 
                    static_cast<int>(entityCollider.height * entityTransform.scale.y) 
                 };
                
                if(entityCollider.isColliding) {
                    // Red
                    SDL_SetRenderDrawColor(renderer,255,0,0,0);
                    SDL_RenderDrawRect(renderer, &debugRect);
                } else {
                    // Blue
                    SDL_SetRenderDrawColor(renderer,0,255,255,0);
                    SDL_RenderDrawRect(renderer, &debugRect);
                }
            }
        }
};

#endif