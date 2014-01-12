#include "luawrapper.h"

#include <glow/logging.h>

#include "lua.hpp"


LuaWrapper::LuaWrapper()
: m_lua(luaL_newstate())
, m_err(0)
{
    luaL_openlibs(m_lua);
}

LuaWrapper::~LuaWrapper()
{
    lua_close(m_lua);
}


void LuaWrapper::luaError()
{
    if (m_err != LUA_OK)
    {
        glow::critical("A Lua error occured: %;", lua_tostring(m_lua, -1));
        lua_pop(m_lua, 1);
    }
}

void LuaWrapper::callScript()
{
    if ((m_err = luaL_dofile(m_lua, "scripts/luascript.lua")) == LUA_OK)
    {
        lua_pushstring(m_lua, "helloWorld");
        lua_getglobal(m_lua, "helloWorld");
        lua_pcall(m_lua, 0, 0, 0);
    }
    luaError();
}
