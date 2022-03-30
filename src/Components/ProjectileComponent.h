#ifndef PROJECTILECOMPONENT_H
#define PROJECTILECOMPONENT_H

#include <SDL2/SDL.h>

struct ProjectileComponent {
    bool isFriendly;
    int hitPercentDamage;
    int duration;
    int startTime;

    ProjectileComponent(bool isFriendly = false, int hitPercentDamage = 0, int duration = 0) {
        this->isFriendly = isFriendly;
        this->startTime = SDL_GetTicks();
        this->duration = duration;
        this->hitPercentDamage = hitPercentDamage;
    }
};

#endif