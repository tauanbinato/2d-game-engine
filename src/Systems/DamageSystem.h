#ifndef DAMAGESYSTEM_H
#define DAMAGESYSTEM_H

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/HealthComponent.h"
#include "../Logger/Logger.h"

class DamageSystem: public System {
    public:
        DamageSystem() {
            RequireComponent<BoxColliderComponent>();
        }

        void SubscribeToCollisionEvent(std::unique_ptr<EventBus>& eventBus){
            eventBus->SubscribeToEvent<CollisionEvent>(this, &DamageSystem::onCollision);
        }

        void onCollision(CollisionEvent& event) {
            Entity a = event.a;
            Entity b = event.b;

            Logger::Log("The Damage System recieved an event collision between entities " + std::to_string(a.GetId()) + " and " + std::to_string(b.GetId()));
            if (a.BelongsToGroup("projectiles") && b.HasTag("player")) {
                OnProjectileHitsPlayer(a, b); // a is the projectile and b is the player 
            }

            if (b.BelongsToGroup("projectiles") && a.HasTag("player")) {
                OnProjectileHitsPlayer(b, a); // b is the projectile and a is the player 
            }


            if (a.BelongsToGroup("projectiles") && b.BelongsToGroup("enemies")) {

            }

            if (b.BelongsToGroup("projectiles") && a.BelongsToGroup("enemies")) {

            }

        }

        void OnProjectileHitsPlayer(Entity projectile, Entity player) {
            auto projectileComponent = projectile.GetComponent<ProjectileComponent>();

            if(!projectileComponent.isFriendly) {
                // Reduce the health of the player by the projectile hit percentDamage
                auto& health = player.GetComponent<HealthComponent>();
                health.healthPercentage -= projectileComponent.hitPercentDamage;

                if(health.healthPercentage <= 0) {
                    player.Kill();
                }

                projectile.Kill();
            }
        }

        void Update() {

        }
};

#endif