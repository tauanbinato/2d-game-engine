#ifndef SPRITECOMPONENT_H
#define SPRITECOMPONENT_H

#include <string>
#include <SDL2/SDL.h>

struct SpriteComponent {
    int width;
    int height;
    std::string assetId;
    SDL_Rect srcRect;
    int zIndex;
    bool isFixed;
    SDL_RendererFlip flip;
   
    SpriteComponent(std::string assetId = "", int width = 0, int height = 0, int srcRectX = 0, int srcRectY = 0, int zIndex = 0, bool isFixed = false) {
        this->assetId = assetId;
        this->width = width;
        this->height = height;
        this->srcRect = { srcRectX, srcRectY, width, height };
        this->zIndex = zIndex;
        this->isFixed = isFixed;
        this->flip = SDL_FLIP_NONE;
    };
};

#endif
