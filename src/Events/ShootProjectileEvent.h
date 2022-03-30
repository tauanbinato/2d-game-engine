#ifndef SHOOTPROJECTILEEVENT_H
#define SHOOTPROJECTILEEVENT_H

#include "../ECS/ECS.h"
#include "../EventBus/Event.h"

#include <SDL2/SDL.h>

class ShootProjectileEvent : public Event{
    public:
        std::unique_ptr<Registry>& m_registry;

        ShootProjectileEvent(std::unique_ptr<Registry>& m_registry): m_registry(m_registry) {};
};

#endif