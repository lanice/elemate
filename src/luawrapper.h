#pragma once

#include <string>
#include <vector>
#include <tuple>

#include "lua.hpp"


struct lua_State;

class LuaWrapper
{
public:
    LuaWrapper();
    ~LuaWrapper();

    void loadScript(const std::string & script);
    void removeScript(const std::string & script);
    void reloadScripts();

    // template<typename... TArgs>
    // void callFunc(const std::string & func, const int & resultCount, const TArgs & ... args)
    // {
    //     pushFunc(func);
    //     push(args...);
    //     callFunction(sizeof...(args), resultCount);
    // };

    // void fetchResults(){};
    // template<typename T, typename... TResults>
    // void fetchResults(T & result, TResults & ... results)
    // {
    //     fetchResults(results...);
    //     fetchResult(result);
    // };


protected:
    void luaError();

    void pushFunc(const std::string & func);
    void callFunc(const int & numArgs, const int & numRet);

    template<typename T, typename... Ts>
    void push(const T value, const Ts... values)
    {
        push(value);
        push(values...);
    };

    void push();
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

    template<typename T> T fetch(const int index) const;


    // General type trait struct
    template <size_t, typename... Ts>
    struct _pop
    {
        typedef std::tuple<Ts...> type;

        // base case: creates a tuple containing one element
        template <typename T>
        static std::tuple<T> worker(const LuaWrapper &l, const int index)
        {
            return std::make_tuple(l.fetch<T>(index));
        }

        // inductive case
        template <typename T1, typename T2, typename... Rest>
        static std::tuple<T1, T2, Rest...> worker(const LuaWrapper &l, const int index)
        {
            std::tuple<T1> head = std::make_tuple(l.fetch<T1>(index));
            return std::tuple_cat(head, worker<T2, Rest...>(l, index + 1));
        }

        static type apply(LuaWrapper &l)
        {
            auto ret = worker<Ts...>(l, 1);
            lua_pop(l.m_state, sizeof...(Ts));
            return ret;
        }
    };

    template <typename... Ts>
    struct _pop<0, Ts...>
    {
        typedef void type;
        static type apply(LuaWrapper &l) {}
    };

    // Specialization for singular type
    template <typename T>
    struct _pop<1, T>
    {
        typedef T type;
        static type apply(LuaWrapper &l)
        {
            // fetch the top element (negative indices count from the top)
            T ret  = l.fetch<T>(-1);

            // Remove it from the stack
            lua_pop(l.m_state, 1);
            return ret;
        }
    };


    template <typename... T>
    typename _pop<sizeof...(T), T...>::type pop()
    {
        // Store the elements requested
        return _pop<sizeof...(T), T...>::apply(*this);
    }

public:
    template <typename... Ret, typename... Args>
    typename _pop<sizeof...(Ret), Ret...>::type call(const std::string &fun, const Args&... args)
    {
        // Push function to stack
        pushFunc(fun.c_str());

        const int numArgs = sizeof...(Args);
        const int numRet = sizeof...(Ret);

        // Push all arguments to the stack using our variadic Push
        push(args...);

        // Call the function
        callFunc(numArgs, numRet);

        // Return all the results and remove the from the stack
        return pop<Ret...>();
    }


protected:
    lua_State * m_state;

    std::vector<std::string> m_scripts;
    int m_err;

public:
    void operator=(LuaWrapper &) = delete;
};
