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

void LuaWrapper::loadScript(const std::string & script)
{
    m_err = luaL_dofile(m_lua, script.c_str());
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
        m_err = luaL_dofile(m_lua, script.c_str());
        luaError();
    }
}

void LuaWrapper::pushFunc(const std::string & func)
{
    lua_pushstring(m_lua, func.c_str());
    lua_getglobal(m_lua, func.c_str());
}

void LuaWrapper::callFunction(const int & argCount, const int & resultCount)
{
    m_err = lua_pcall(m_lua, argCount, resultCount, 0);
    luaError();
}

void LuaWrapper::pushArgument(const std::string & arg)
{
    lua_pushstring(m_lua, arg.c_str());
}

void LuaWrapper::pushArgument(const char * arg)
{
    lua_pushstring(m_lua, arg);
}

void LuaWrapper::pushArgument(const int & arg)
{
    lua_pushinteger(m_lua, arg);
}

void LuaWrapper::pushArgument(const double & arg)
{
    lua_pushnumber(m_lua, arg);
}

void LuaWrapper::pushArgument(const unsigned long & arg)
{
    lua_pushunsigned(m_lua, arg);
}

void LuaWrapper::pushArgument(const bool & arg)
{
    lua_pushboolean(m_lua, arg);
}

void LuaWrapper::fetchResult(std::string & result)
{
    if (!lua_isstring(m_lua, -1))
    {
        glow::critical("LuaWrapper: Return value not a string.");
        return;
    }
    result = lua_tostring(m_lua, -1);
    lua_pop(m_lua, 1);
}

void LuaWrapper::fetchResult(int64_t & result)
{
    if (!lua_isnumber(m_lua, -1))
    {
        glow::critical("LuaWrapper: Return value not a number.");
        return;
    }
    result = lua_tointeger(m_lua, -1);
    lua_pop(m_lua, 1);
}

void LuaWrapper::fetchResult(double & result)
{
    if (!lua_isnumber(m_lua, -1))
    {
        glow::critical("LuaWrapper: Return value not a number.");
        return;
    }
    result = lua_tonumber(m_lua, -1);
    lua_pop(m_lua, 1);
}

void LuaWrapper::fetchResult(unsigned long & result)
{
    if (!lua_isnumber(m_lua, -1))
    {
        glow::critical("LuaWrapper: Return value not a number.");
        return;
    }
    result = lua_tounsigned(m_lua, -1);
    lua_pop(m_lua, 1);
}

void LuaWrapper::fetchResult(bool & result)
{
    if (!lua_isboolean(m_lua, -1))
    {
        glow::critical("LuaWrapper: Return value not a boolean.");
        return;
    }
    result = lua_toboolean(m_lua, -1) != 0;
    lua_pop(m_lua, 1);
}
