#ifndef PROJECTILELIFECYCLESYSTEM_H
#define PROJECTILELIFECYCLESYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Logger/Logger.h"
#include <SDL2/SDL.h>
#include <string>
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "../Events/KeyPressedEvent.h"

class ProjectileLifeCycleSystem: public System {
    public:
        ProjectileLifeCycleSystem () {
            RequireComponent<ProjectileComponent>();
        }

        void Update() {
            for(auto entity : GetSystemEntities()) {
                auto projectile = entity.GetComponent<ProjectileComponent>();

                // Kill projectiles after they hit they duration limit
                if ((int)(SDL_GetTicks() - projectile.startTime) > (int)(projectile.duration)) {
                    entity.Kill();
                }
            }
        }
};

#endif