#ifndef LUASCRIPSYSTEM_H
#define LUASCRIPSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/LuaScriptComponent.h"
#include "../Components/TransformComponent.h"

// First declare some native C++ functions that we will bind with Lua functions
// int GetEntityPosition(Entity entity) {
//     if (entity.HasComponent<TransformComponent>()){
//         auto& transform = entity.GetComponent<TransformComponent>();
//         return;
//     } else {
//         Logger::Error("LuaScriptSystemError! This entity does not have transform component.");
//         return;
//     }
// };

void SetEntityPosition(Entity entity, double x, double y) {
    if (entity.HasComponent<TransformComponent>()){
        auto& transform = entity.GetComponent<TransformComponent>();
        transform.position.x = x;
        transform.position.y = y;
    } else {
        Logger::Error("LuaScriptSystemError! This entity does not have transform component.");
        return;
    }
};


class LuaScriptSystem: public System {
    public:
        LuaScriptSystem() {
            RequireComponent<LuaScriptComponent>();
        }

        void CreateLuaBindings(sol::state& lua) {

            //Create the "Entity" usertype so Lua knows what an entity is
            lua.new_usertype<Entity>(
                "entity",
                "get_id", &Entity::GetId,
                "destroy", &Entity::Kill,
                "has_tag", &Entity::HasTag,
                "belongs_to_group", &Entity::BelongsToGroup
            );

            // Create the bindings between C++ and Lua functions
            lua.set_function("set_position", SetEntityPosition);
        }

        void Update(double deltaTime, int ellapsedTime) {
            // loop all entities that have a script component and invoke their lua function
            for (auto entity : GetSystemEntities()) {
                auto script = entity.GetComponent<LuaScriptComponent>();
                script.func(entity, deltaTime, ellapsedTime);
            }
        }
};

#endif