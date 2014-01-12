#pragma once

#include <string>


class lua_State;

class LuaWrapper
{
public:
    LuaWrapper();
    ~LuaWrapper();

    void loadScript(std::string script);
    void callFunc(std::string func);


protected:
    void luaError();

    lua_State * m_lua;

    int m_err;

public:
    void operator=(LuaWrapper &) = delete;
};
