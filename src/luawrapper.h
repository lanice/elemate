#pragma once

#include <string>
#include <vector>
#include <sstream>


class lua_State;

class LuaWrapper
{
public:
    LuaWrapper();
    ~LuaWrapper();

    void loadScript(const std::string & script);
    void reloadScripts();

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
        std::stringstream ss;
        std::string argument;
        ss << head;
        ss >> argument;
        pushArgument(argument);
        pushArguments(tail...);
    };

    lua_State * m_lua;

    std::vector<std::string> m_scripts;
    int m_err;

public:
    void operator=(LuaWrapper &) = delete;
};
