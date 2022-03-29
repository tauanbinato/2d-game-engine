#ifndef BOXCOLLIDERCOMPONENT_H
#define BOXCOLLIDERCOMPONENT_H

#include <glm/glm.hpp>
#include <SDL2/SDL.h>

struct BoxColliderComponent {
    public:
        int width;
        int height;
        glm::vec2 offset;
        bool isColliding;


        BoxColliderComponent(int width = 0, int height = 0, glm::vec2 offset = glm::vec2(0)) {
            this->height = height;
            this->width = width;
            this->offset = offset;
            this->isColliding = false;
        }
};
#endif