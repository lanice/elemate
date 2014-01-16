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

void LuaWrapper::push(const std::string & value)
{
    lua_pushstring(m_state, value.c_str());
}

void LuaWrapper::push(const char * value)
{
    lua_pushstring(m_state, value);
}

void LuaWrapper::push(const int value)
{
    lua_pushinteger(m_state, value);
}

void LuaWrapper::push(const double value)
{
    lua_pushnumber(m_state, value);
}

void LuaWrapper::push(const unsigned long value)
{
    lua_pushunsigned(m_state, value);
}

void LuaWrapper::push(const bool value)
{
    lua_pushboolean(m_state, value);
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
