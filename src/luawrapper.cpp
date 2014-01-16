#include "luawrapper.h"

#include <cstdint>

#include <glow/logging.h>

#include "lua.hpp"


LuaWrapper::LuaWrapper()
: m_state(luaL_newstate())
, m_err(LUA_OK)
{
    luaL_openlibs(m_state);
}

LuaWrapper::~LuaWrapper()
{
    if (m_state == nullptr) return;
    lua_close(m_state);
}


void LuaWrapper::luaError()
{
    if (m_err != LUA_OK)
    {
        glow::critical("A Lua error occured: %;", lua_tostring(m_state, -1));
        lua_pop(m_state, 1);
    }
}

void LuaWrapper::loadScript(const std::string & script)
{
    m_err = luaL_dofile(m_state, script.c_str());
    luaError();
    m_scripts.push_back(script);
}

void LuaWrapper::removeScript(const std::string & script)
{
    for (std::vector<std::string>::iterator it = m_scripts.begin(); it != m_scripts.end(); ++it)
        if (*it == script) it = m_scripts.erase(it) - 1;
}

void LuaWrapper::reloadScripts()
{
    for (auto script : m_scripts)
    {
        m_err = luaL_dofile(m_state, script.c_str());
        luaError();
    }
}

void LuaWrapper::pushFunc(const std::string & func)
{
    lua_getglobal(m_state, func.c_str());
}

void LuaWrapper::callFunction(const int & argCount, const int & resultCount)
{
    m_err = lua_pcall(m_state, argCount, resultCount, 0);
    luaError();
}

void LuaWrapper::pushArgument(const std::string & arg)
{
    lua_pushstring(m_state, arg.c_str());
}

void LuaWrapper::pushArgument(const char * arg)
{
    lua_pushstring(m_state, arg);
}

void LuaWrapper::pushArgument(const int & arg)
{
    lua_pushinteger(m_state, arg);
}

void LuaWrapper::pushArgument(const double & arg)
{
    lua_pushnumber(m_state, arg);
}

void LuaWrapper::pushArgument(const unsigned long & arg)
{
    lua_pushunsigned(m_state, arg);
}

void LuaWrapper::pushArgument(const bool & arg)
{
    lua_pushboolean(m_state, arg);
}

void LuaWrapper::fetchResult(std::string & result)
{
    if (!lua_isstring(m_state, -1))
    {
        glow::critical("LuaWrapper: Return value not a string.");
        return;
    }
    result = lua_tostring(m_state, -1);
    lua_pop(m_state, 1);
}

void LuaWrapper::fetchResult(int & result)
{
    if (!lua_isnumber(m_state, -1))
    {
        glow::critical("LuaWrapper: Return value not a number.");
        return;
    }
    assert( static_cast<int>(lua_tointeger(m_state, -1)) == lua_tointeger(m_state, -1));
    result = static_cast<int>(lua_tointeger(m_state, -1));
    lua_pop(m_state, 1);
}

void LuaWrapper::fetchResult(double & result)
{
    if (!lua_isnumber(m_state, -1))
    {
        glow::critical("LuaWrapper: Return value not a number.");
        return;
    }
    result = lua_tonumber(m_state, -1);
    lua_pop(m_state, 1);
}

void LuaWrapper::fetchResult(unsigned long & result)
{
    if (!lua_isnumber(m_state, -1))
    {
        glow::critical("LuaWrapper: Return value not a number.");
        return;
    }
    result = lua_tounsigned(m_state, -1);
    lua_pop(m_state, 1);
}

void LuaWrapper::fetchResult(bool & result)
{
    if (!lua_isboolean(m_state, -1))
    {
        glow::critical("LuaWrapper: Return value not a boolean.");
        return;
    }
    result = lua_toboolean(m_state, -1) != 0;
    lua_pop(m_state, 1);
}
