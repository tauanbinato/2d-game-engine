#ifndef LUASCRIPTCOMPONENT_H
#define LUASCRIPTCOMPONENT_H

#include <sol/sol.hpp>

struct LuaScriptComponent
{
    sol::function func;

    LuaScriptComponent(sol::function func = sol::lua_nil) {
        this->func = func;
    }
};
#endif