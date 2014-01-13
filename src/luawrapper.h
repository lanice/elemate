#pragma once

#include <string>


class lua_State;

class LuaWrapper
{
public:
    LuaWrapper();
    ~LuaWrapper();

    void loadScript(const std::string & script);

    template<typename... TArgs>
    void callFunc(const std::string & func, const TArgs & ... args)
    {
        pushFunc(func);
        pushArguments(args...);
        callFunction(sizeof...(args));
    };


protected:
    void luaError();

    void pushFunc(const std::string & func);
    void pushArgument(const std::string & arg);
    void callFunction(const int & argCount);

    void pushArguments(){};

    template<typename T, typename... TArgs>
    void pushArguments(const T & head, const TArgs & ... tail)
    {
        pushArgument(head);
        pushArguments(tail...);
    };

    lua_State * m_lua;

    int m_err;

public:
    void operator=(LuaWrapper &) = delete;
};
