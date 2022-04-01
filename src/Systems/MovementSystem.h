#ifndef MOVEMENTSYSTEM_H
#define MOVEMENTSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"

class MovementSystem: public System {
    public:
        MovementSystem() {
            RequireComponent<TransformComponent>();
            RequireComponent<RigidBodyComponent>();
        }

        // Subscription to collision events
        void SubscribeToCollisionEvent(const std::unique_ptr<EventBus>& eventBus){
            eventBus->SubscribeToEvent<CollisionEvent>(this, &MovementSystem::onCollision);
        }

        // On Collision
        void onCollision(CollisionEvent& event) {
            Entity a = event.a;
            Entity b = event.b;

            if (a.BelongsToGroup("enemies") && b.BelongsToGroup("obstacles")) {
                OnEnemyHitsObstacle(a, b);
            }

            if (a.BelongsToGroup("obstacles") && b.BelongsToGroup("enemies")) {
                OnEnemyHitsObstacle(b, a);
            }
        }

        void OnEnemyHitsObstacle(Entity enemy, Entity obstacle) {
            if (enemy.HasComponent<RigidBodyComponent>() && enemy.HasComponent<SpriteComponent>()) {
                auto& rigidBody = enemy.GetComponent<RigidBodyComponent>();
                auto& sprite = enemy.GetComponent<SpriteComponent>();

                if(rigidBody.velocity.x != 0) {
                    rigidBody.velocity.x *= -1;
                    sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
                }

                if(rigidBody.velocity.y != 0) {
                    rigidBody.velocity.y *= -1;
                    sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
                }
            }
        }

        void Update(double deltaTime) {
            // TODO:
            
            // Loop all entities that the system is interested in..
            for (auto entity: GetSystemEntities()) {
                // Update entity pos based on its velocity every frame
                auto& transform = entity.GetComponent<TransformComponent>();
                const auto rigidbody = entity.GetComponent<RigidBodyComponent>();

                transform.position.x += rigidbody.velocity.x * deltaTime;
                transform.position.y += rigidbody.velocity.y * deltaTime;

                if (entity.HasTag("player")) {
                    int paddingLeft = 10;
                    int paddingTop = 10;
                    int paddingRight = 30;
                    int paddingBottom = 30;
                    transform.position.x = transform.position.x < paddingLeft ? paddingLeft : transform.position.x;
                    transform.position.x = transform.position.x > Game::m_mapWidth - paddingRight ? Game::m_mapWidth - paddingRight : transform.position.x;
                    transform.position.y = transform.position.y < paddingTop ? paddingTop : transform.position.y;
                    transform.position.y = transform.position.y > Game::m_mapHeight - paddingBottom ? Game::m_mapHeight - paddingBottom : transform.position.y;
                }

                bool isEntityOutsideMap = (
                    transform.position.x < 0 || 
                    transform.position.x > Game::m_mapWidth ||
                    transform.position.y < 0 ||
                    transform.position.y > Game::m_mapHeight
                );
                if (isEntityOutsideMap && !entity.HasTag("player")) {
                    entity.Kill();
                }
            }
        }
};

#endif