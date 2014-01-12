#pragma once


class lua_State;

class LuaWrapper
{
public:
    LuaWrapper();
    ~LuaWrapper();

    void callScript();


protected:
    void luaError();

    lua_State * m_lua;

    int m_err;

public:
    void operator=(LuaWrapper &) = delete;
};
