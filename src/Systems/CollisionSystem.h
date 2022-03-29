#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include "../Components/TransformComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../ECS/ECS.h"
#include "../Logger/Logger.h"
#include <SDL2/SDL.h>
#include <string>
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"

class CollisionSystem: public System {

    public:
        CollisionSystem() {
            RequireComponent<BoxColliderComponent>();
            RequireComponent<TransformComponent>();
        }

        void Update(bool SDLCollision, std::unique_ptr<EventBus>& ptr_eventBus) {
            // Check all entities that has a boxcollider
            // to see if they are colliding with each other
            auto entities = GetSystemEntities();
            bool collided = false;

            for(auto i = entities.begin(); i != entities.end(); i++) {
                Entity a = *i;
                auto aTransform = a.GetComponent<TransformComponent>();
                auto& aCollider = a.GetComponent<BoxColliderComponent>();

                // Checking only the ones to the right, that still needs to be checked
                for(auto j = i; j != entities.end(); j++) {
                    Entity b = *j;

                    // Bypass if they are the same entity 
                    if (a == b) {
                        continue;
                    }

                    auto bTransform = b.GetComponent<TransformComponent>();
                    auto& bCollider = b.GetComponent<BoxColliderComponent>();
                    

                    if (!SDLCollision) {
                        // Check AABB Collision
                        collided = CheckAABBCollision(
                            aTransform.position.x + aCollider.offset.x,
                            aTransform.position.y + aCollider.offset.y,
                            aCollider.width,
                            aCollider.height,
                            bTransform.position.x + bCollider.offset.x,
                            bTransform.position.y + bCollider.offset.y,
                            bCollider.width,
                            bCollider.height
                        );
                    } 

                    // Use SDL Interssections to check collisions
                    else {
                        SDL_Rect rectA{ (int)aTransform.position.x + (int)aCollider.offset.x, (int)aTransform.position.y + (int)aCollider.offset.y, aCollider.width, aCollider.height };
                        SDL_Rect rectB{ (int)bTransform.position.x + (int)bCollider.offset.x, (int)bTransform.position.y + (int)bCollider.offset.y, bCollider.width, bCollider.height };
                        SDL_bool sdl_collided = SDL_HasIntersection(&rectA, &rectB);
                        collided = sdl_collided;
                    }

                    // Collision Detected
                    if (collided) {
                        
                        aCollider.isColliding = true;
                        bCollider.isColliding = true;

                        // Emit an event
                        ptr_eventBus->EmitEvent<CollisionEvent>(a, b);

                    } 
                    // Not Colliding
                    else {
                        aCollider.isColliding = false;
                        bCollider.isColliding = false;
                    }
                    
                }
            }
        }

        bool CheckAABBCollision(double aX, double aY, double aW, double aH, double bX, double bY, double bW, double bH) {
            return (
                aX < bX + bW &&
                aX + aW > bX &&
                aY < bY + bH &&
                aY + aH > bY
            );
        }

       

};

#endif