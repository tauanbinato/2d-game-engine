#ifndef HEALTHCOMPONENT_H
#define HEALTHCOMPONENT_H

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

struct HealthComponent {
    int healthPercentage;

    HealthComponent (int healthPercentage = 0) {
        this->healthPercentage = healthPercentage;
    }
};

#endif