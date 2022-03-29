#ifndef PROJECTILEEMITSYSTEM_H
#define PROJECTILEEMITSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Logger/Logger.h"
#include <SDL2/SDL.h>
#include <string>
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "../Events/KeyPressedEvent.h"

class ProjectileEmitSystem: public System {
    public:
        ProjectileEmitSystem () {
            RequireComponent<ProjectileEmitterComponent>();
            RequireComponent<TransformComponent>();
        }

        void Update (std::unique_ptr<Registry>& registry) {

            // For every entity That fits
            for (auto entity : GetSystemEntities()) {
                auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
                const auto transform = entity.GetComponent<TransformComponent>();

                // Check if its time to re-emit a new projectile
                if ((int)(SDL_GetTicks() - projectileEmitter.lastEmissionTime) > (int)(projectileEmitter.projectileRateOfFire)) {

                    glm::vec2 projectilePosition = transform.position;
                    // If the entity has Sprite we position our new projectile in the center
                    if (entity.HasComponent<SpriteComponent>()) {
                        auto sprite = entity.GetComponent<SpriteComponent>();
                        projectilePosition.x += ((transform.scale.x * sprite.width)  / 2);
                        projectilePosition.y += ((transform.scale.y * sprite.height) / 2);
                    }
                    // Add a new projectile entity to the registry
                    Entity projectile = registry->CreateEntity();
                    projectile.AddComponent<TransformComponent>(projectilePosition, glm::vec2(1.0), 0.0);
                    projectile.AddComponent<RigidBodyComponent>(projectileEmitter.projectileVelocity);
                    projectile.AddComponent<SpriteComponent>("bullet-image", 4, 4, 0, 0, 4);
                    projectile.AddComponent<BoxColliderComponent>(4,4);

                    projectileEmitter.lastEmissionTime = SDL_GetTicks();
                }
            }
        }
};

#endif