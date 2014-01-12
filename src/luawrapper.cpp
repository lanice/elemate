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

void LuaWrapper::loadScript(std::string script)
{
    m_err = luaL_dofile(m_lua, script.c_str());
    luaError();
}

void LuaWrapper::callFunc(std::string func)
{
    lua_pushstring(m_lua, func.c_str());
    lua_getglobal(m_lua, func.c_str());
    
    m_err = lua_pcall(m_lua, 0, 0, 0);
    luaError();
}
