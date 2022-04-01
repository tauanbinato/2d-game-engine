#ifndef PROJECTILEEMITSYSTEM_H
#define PROJECTILEEMITSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Logger/Logger.h"
#include <SDL2/SDL.h>
#include <string>
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "../Events/ShootProjectileEvent.h"
#include <iostream>

class ProjectileEmitSystem: public System {

    public:
        ProjectileEmitSystem () {
            RequireComponent<ProjectileEmitterComponent>();
            RequireComponent<TransformComponent>();
            RequireComponent<RigidBodyComponent>();
        }

        void SubscribeToSpaceBarEvent(std::unique_ptr<EventBus>& eventBus){
            eventBus->SubscribeToEvent<ShootProjectileEvent>(this, &ProjectileEmitSystem::OnShoot);
        }

        void OnShoot(ShootProjectileEvent& event) {
            // Shoot projectile for the Player
            for (auto entity : GetSystemEntities()) {

                // Is the player
                if(entity.HasTag("player")) {

                    const auto transform = entity.GetComponent<TransformComponent>();
                    const auto rigidBody = entity.GetComponent<RigidBodyComponent>();
                    auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();

                    glm::vec2 projectilePosition = transform.position;
                    CenterProjectile(entity, projectilePosition);

                    glm::vec2 projectileVelocity = projectileEmitter.projectileVelocity;
                    int directionX = 0;
                    int directionY = 0;
                    if (rigidBody.velocity.x > 0) directionX = +1;
                    if (rigidBody.velocity.x < 0) directionX = -1;
                    if (rigidBody.velocity.y > 0) directionY = +1;
                    if (rigidBody.velocity.y < 0) directionY = -1;
                    
                    projectileVelocity.x = projectileEmitter.projectileVelocity.x * directionX + rigidBody.velocity.x;
                    projectileVelocity.y = projectileEmitter.projectileVelocity.y * directionY + rigidBody.velocity.y;

                    ShootProjectile(entity, event.m_registry, projectilePosition, projectileVelocity);
                }
                
                
            }
        }

        // Update function
        void Update (std::unique_ptr<Registry>& registry) {

            // For every enimy entity we leave them shooting
            for (auto entity : GetSystemEntities()) {
                const auto rigidBody = entity.GetComponent<RigidBodyComponent>();
                auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
                if (!projectileEmitter.isFriendly) {

                    glm::vec2 projectilePosition = glm::vec2(0);
                    CenterProjectile(entity, projectilePosition);
                    ShootProjectile(entity, registry, projectilePosition, (projectileEmitter.projectileVelocity + rigidBody.velocity)); 
                }
                
            }
        }

    private:

        // If the entity has Sprite we position our new projectile in the center
        void CenterProjectile(Entity entity, glm::vec2& projectilePosition) {
            const auto transform = entity.GetComponent<TransformComponent>();
            projectilePosition = transform.position;
            if (entity.HasComponent<SpriteComponent>()) {

                auto sprite = entity.GetComponent<SpriteComponent>();
                projectilePosition.x += ((transform.scale.x * sprite.width)  / 2);
                projectilePosition.y += ((transform.scale.y * sprite.height) / 2);
            }
        }

        void ShootProjectile(Entity entity, std::unique_ptr<Registry>& registry, glm::vec2& projectilePosition, glm::vec2 projectileVelocity) {
            
            auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
            // Check if its time to re-emit a new projectile
            if ((int)(SDL_GetTicks() - projectileEmitter.lastEmissionTime) > (int)(projectileEmitter.projectileRateOfFire)) {

                // Add a new projectile entity to the registry
                Entity projectile = registry->CreateEntity();
                projectile.Group("projectiles");
                projectile.AddComponent<TransformComponent>(projectilePosition, glm::vec2(1.0), 0.0);
                projectile.AddComponent<RigidBodyComponent>(projectileVelocity);
                projectile.AddComponent<SpriteComponent>("bullet-texture", 4, 4, 0, 0, 4);
                projectile.AddComponent<BoxColliderComponent>(4,4);
                projectile.AddComponent<ProjectileComponent>(projectileEmitter.isFriendly, projectileEmitter.hitPercentDamage, projectileEmitter.projectileDuration);

                projectileEmitter.lastEmissionTime = SDL_GetTicks();
            }
        }
    
};

#endif