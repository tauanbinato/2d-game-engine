#ifndef LEVELLOADER_H
#define LEVELLOADER_H

#include <sol/sol.hpp>
#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include <SDL2/SDL.h>
#include <memory>


class LevelLoader {
    public:
        LevelLoader();
        ~LevelLoader();

        void LoadLevel(sol::state& m_lua, const std::unique_ptr<Registry>& m_registry, const std::unique_ptr<AssetStore>& m_assetStore, SDL_Renderer* m_ptrRenderer, int level);
};

#endif