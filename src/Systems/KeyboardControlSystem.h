#ifndef KEYBOARDCONTROLSYSTEM_H
#define KEYBOARDCONTROLSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Logger/Logger.h"
#include <SDL2/SDL.h>
#include <string>
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "../Events/KeyPressedEvent.h"

class KeyboardControlSystem: public System {
    public:
        KeyboardControlSystem() {
            RequireComponent<KeyboardControlledComponent>();
            RequireComponent<RigidBodyComponent>();
            RequireComponent<SpriteComponent>();
        }

        void SubscribeToKeyPressedEvents(std::unique_ptr<EventBus>& eventBus) {
            eventBus->SubscribeToEvent<KeyPressedEvent>(this, &KeyboardControlSystem::onKeyPressed);
        }

        void onKeyPressed(KeyPressedEvent& event) {
            // Change the sprite and the velocity of interested entities
            for (auto entity : GetSystemEntities()) {
                const auto keyboardControl = entity.GetComponent<KeyboardControlledComponent>();
                auto& sprite = entity.GetComponent<SpriteComponent>();
                auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

                switch (event.symbol) {
                    case SDLK_w:
                        rigidbody.velocity = keyboardControl.upVelocity;
                        sprite.srcRect.y = sprite.height * 0;
                        break;
                    case SDLK_d:
                        rigidbody.velocity = keyboardControl.rightVelocity;
                        sprite.srcRect.y = sprite.height * 1;
                        break;
                    case SDLK_s:
                        rigidbody.velocity = keyboardControl.downVelocity;
                        sprite.srcRect.y = sprite.height * 2;
                        break;
                    case SDLK_a:
                        rigidbody.velocity = keyboardControl.leftVelocity;
                        sprite.srcRect.y = sprite.height * 3;
                        break;
                }
            }
        }

        void Update() {
            
        }
};

#endif