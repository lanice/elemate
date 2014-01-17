#pragma once

#include <string>
#include <vector>
#include <tuple>


struct lua_State;

class LuaWrapper
{
public:
    LuaWrapper();
    ~LuaWrapper();

    void loadScript(const std::string & script);
    void removeScript(const std::string & script);
    void reloadScripts();


protected:
    void luaError();
    void pushFunc(const std::string & func) const;
    void callFunc(const int numArgs, const int numRet);

    void popStack(const int index);

    template<typename T, typename... Ts>
    void push(const T value, const Ts... values) const
    {
        push(value);
        push(values...);
    };

    void push() const;
    void push(const std::string & value) const;
    void push(const char * value) const;
    void push(const int value) const;
    void push(const double value) const;
    void push(const unsigned long value) const;
    void push(const bool value) const;

    template<typename T> T fetch(const int index) const;

    template <size_t, typename... Ts>
    struct _pop
    {
        typedef std::tuple<Ts...> type;

        template <typename T>
        static std::tuple<T> worker(const LuaWrapper &instance, const int index)
        {
            return std::make_tuple(instance.fetch<T>(index));
        }

        template <typename T1, typename T2, typename... Rest>
        static std::tuple<T1, T2, Rest...> worker(const LuaWrapper &instance, const int index)
        {
            std::tuple<T1> head = std::make_tuple(instance.fetch<T1>(index));
            return std::tuple_cat(head, worker<T2, Rest...>(instance, index + 1));
        }

        static type apply(LuaWrapper &instance)
        {
            auto ret = worker<Ts...>(instance, 1);
            instance.popStack(sizeof...(Ts));
            return ret;
        }
    };

    template <typename... Ts>
    struct _pop<0, Ts...>
    {
        typedef void type;
        static type apply(LuaWrapper &instance)
        {
            // necessary because just commenting out '&instance' (to prevent unused parameter warning) would cause the lua state to crash when calling a function after calling a function with no return value... o.0
            instance.popStack(0);
        }
    };

    template <typename T>
    struct _pop<1, T>
    {
        typedef T type;
        static T apply(LuaWrapper &instance)
        {
            // fetch the top element (negative indices count from the top)
            T ret  = instance.fetch<T>(-1);

            instance.popStack(1);
            return ret;
        }
    };

    template <typename... T>
    typename _pop<sizeof...(T), T...>::type pop()
    {
        return _pop<sizeof...(T), T...>::apply(*this);
    }


public:
    template <typename... Ret, typename... Args>
    typename _pop<sizeof...(Ret), Ret...>::type call(const std::string &fun, const Args&... args)
    {
        pushFunc(fun.c_str());
        push(args...);

        const int numArgs = sizeof...(Args);
        const int numRet = sizeof...(Ret);

        callFunc(numArgs, numRet);

        return pop<Ret...>();
    }


protected:
    lua_State * m_state;

    std::vector<std::string> m_scripts;
    int m_err;


public:
    void operator=(LuaWrapper &) = delete;
};
