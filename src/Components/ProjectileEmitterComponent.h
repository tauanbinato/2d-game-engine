#ifndef PROJECTILEEMITTERCOMPONENT_H
#define PROJECTILEEMITTERCOMPONENT_H

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

struct ProjectileEmitterComponent {
    
    glm::vec2 projectileVelocity;
    int projectileRateOfFire;
    int projectileDuration;
    bool isFriendly;
    int hitPercentDamage;
    int lastEmissionTime; // When was the last time this component shoot


    ProjectileEmitterComponent (glm::vec2 projectileVelocity = glm::vec2(0), int projectileRateOfFire = 0,int projectileDuration = 10000, int hitPercentDamage = 10, bool isFriendly = false) {
        this->projectileVelocity = projectileVelocity;
        this->projectileRateOfFire = projectileRateOfFire;
        this->hitPercentDamage = hitPercentDamage;
        this->isFriendly = isFriendly;
        this->projectileDuration = projectileDuration;
        this->lastEmissionTime = SDL_GetTicks();
    }
};

#endif