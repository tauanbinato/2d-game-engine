#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include <SDL2/SDL.h>
#include "../AssetStore/AssetStore.h"
#include <algorithm>


class RenderSystem: public System {
    public:
        RenderSystem() {
            RequireComponent<TransformComponent>();
            RequireComponent<SpriteComponent>();
        }

        void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, SDL_Rect& camera) {

            // THIS SORTING CAN BE POLISHED, RIGHT NOW CAN BE PERFORMATIC SORT ALL ENTITIES ON EACH FRAME------------------------------------
            // Create a vector copy with both sprite and transform components
            struct RenderableEntity {
                TransformComponent transformComponent;
                SpriteComponent spriteComponent;
            };

            std::vector<RenderableEntity> renderableEntities;

            for (auto entity: GetSystemEntities()) {
                RenderableEntity re;
                re.spriteComponent = entity.GetComponent<SpriteComponent>();
                re.transformComponent = entity.GetComponent<TransformComponent>();

                // Bypass rendering entitites if they are outside the cameraview (culling)
                bool isEntityOutsideCameraView = (
                    re.transformComponent.position.x + (re.transformComponent.scale.x * re.spriteComponent.width) < camera.x ||
                    re.transformComponent.position.x > camera.x + camera.w ||
                    re.transformComponent.position.y + ((re.transformComponent.scale.y * re.spriteComponent.height)) < camera.y ||
                    re.transformComponent.position.y > camera.y + camera.h
                );

                // Culling sprites outside camera view and not fixed
                if (isEntityOutsideCameraView && !re.spriteComponent.isFixed) {
                    continue;
                }
                renderableEntities.emplace_back(re);
            }

            // Sort the copied vector by z-index
            std::sort(renderableEntities.begin(), renderableEntities.end(), [](const RenderableEntity& a, const RenderableEntity& b) {
                return a.spriteComponent.zIndex < b.spriteComponent.zIndex;
            });
            

            // Loop all entities that the system is interested in.. based on the sorted vector
            for (auto entity: renderableEntities) {
                const auto transform = entity.transformComponent;
                const auto sprite = entity.spriteComponent;

                // Set the source rectangle of our origial sprite texture
                SDL_Rect srcRect = sprite.srcRect;

                // Set the destination rectangle with the x.y position to be rendered
                SDL_Rect dstRect = {
                    static_cast<int>(transform.position.x - (sprite.isFixed ? 0 : camera.x)),
                    static_cast<int>(transform.position.y - (sprite.isFixed ? 0 : camera.y)),
                    static_cast<int>(sprite.width * transform.scale.x),
                    static_cast<int>(sprite.height * transform.scale.y)
                };

                SDL_RenderCopyEx(
                    renderer,
                    assetStore->GetTexture(sprite.assetId),
                    &srcRect,
                    &dstRect,
                    transform.rotation,
                    NULL,
                    sprite.flip
                );

            }
        }
};

#endif