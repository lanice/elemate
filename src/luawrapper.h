#pragma once

#include <string>
#include <vector>


struct lua_State;

class LuaWrapper
{
public:
    LuaWrapper();
    ~LuaWrapper();

    void loadScript(const std::string & script);
    void removeScript(const std::string & script);
    void reloadScripts();

    template<typename... TArgs>
    void callFunc(const std::string & func, const int & resultCount, const TArgs & ... args)
    {
        pushFunc(func);
        pushArguments(args...);
        callFunction(sizeof...(args), resultCount);
    };

    void fetchResults(){};
    template<typename T, typename... TResults>
    void fetchResults(T & result, TResults & ... results)
    {
        fetchResults(results...);
        fetchResult(result);
    };


protected:
    void luaError();

    void pushFunc(const std::string & func);
    void callFunction(const int & argCount, const int & resultCount);

    void pushArguments(){};
    template<typename T, typename... TArgs>
    void pushArguments(const T & head, const TArgs & ... tail)
    {
        push(head);
        pushArguments(tail...);
    };

    void push(const char * value);
    void push(const std::string & value);
    void push(const int value);
    void push(const double value);
    void push(const unsigned long value);
    void push(const bool value);

    void fetchResult(std::string & result);
    void fetchResult(int & result);
    void fetchResult(double & result);
    void fetchResult(unsigned long & result);
    void fetchResult(bool & result);


    lua_State * m_state;

    std::vector<std::string> m_scripts;
    int m_err;

public:
    void operator=(LuaWrapper &) = delete;
};
