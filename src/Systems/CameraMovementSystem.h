#ifndef CAMERAMOVEMENTSYSTEM_H
#define CAMERAMOVEMENTSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/TransformComponent.h"
#include <SDL2/SDL.h>
#include "../Game/Game.h"

class CameraMovementSystem: public System {
    public:
        CameraMovementSystem(){
            RequireComponent<CameraFollowComponent>();
            RequireComponent<TransformComponent>();
        }

        void Update(SDL_Rect& camera) {
            for(auto entity : GetSystemEntities()){
                auto transform = entity.GetComponent<TransformComponent>();


                if (transform.position.x  + (camera.w / 2) < Game::m_mapWidth){
                     camera.x = transform.position.x - (Game::m_windowWidth / 2);
                }
                // Change camera.x and camera.y based on the entity transform position
                if (transform.position.y + (camera.h / 2) < Game::m_mapHeight){
                     camera.y = transform.position.y - (Game::m_windowHeight / 2);
                }

                
                camera.x = camera.x < 0 ? 0 : camera.x;
                camera.y = camera.y < 0 ? 0 : camera.y;
                camera.x = camera.x > camera.w ? camera.w : camera.x;
                camera.y = camera.y > camera.h ? camera.h : camera.y;
            }
        }
};
#endif