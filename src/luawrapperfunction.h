#pragma once

#include <string>
#include <tuple>
#include <functional>

#include <lua.hpp>


struct BaseFunction
{
    virtual ~BaseFunction() {}
    virtual int apply(lua_State * state) = 0;
};


namespace Lua
{
    int _lua_dispatcher(lua_State * state);

    template <std::size_t... Is>
    struct _indices {};

    template <std::size_t N, std::size_t... Is>
    struct _indices_builder : _indices_builder<N-1, N-1, Is...> {};

    template <std::size_t... Is>
    struct _indices_builder<0, Is...>
    {
        using type = _indices<Is...>;
    };

    template <typename T> T _check_get(lua_State * state, const int index);

    void _push(lua_State * state); // Necessary in the case no arguments are passed
    void _push(lua_State * state, bool &&value);
    void _push(lua_State * state, int &&value);
    void _push(lua_State * state, unsigned int &&value);
    void _push(lua_State * state, float &&value);
    void _push(lua_State * state, double &&value);
    void _push(lua_State * state, std::string &&value);

    template <typename Return, typename... Args, std::size_t... N>
    Return _lift(std::function<Return(Args...)> function, std::tuple<Args...> args, _indices<N...>)
    {
        return function(std::get<N>(args)...);
    }

    template <typename Return, typename... Args>
    Return _lift(std::function<Return(Args...)> function, std::tuple<Args...> args)
    {
        return _lift(function, args, typename _indices_builder<sizeof...(Args)>::type());
    }

    template <typename... T, std::size_t... N>
    std::tuple<T...> _get_args(lua_State * state, _indices<N...>)
    {
        return std::make_tuple(_check_get<T>(state, N+1)...);
    }

    template <typename... T>
    std::tuple<T...> _get_args(lua_State * state)
    {
        constexpr std::size_t num_args = sizeof...(T);
        return _get_args<T...>(state, typename _indices_builder<num_args>::type());
    }

    inline void _push_n(lua_State *) {}

    template <typename T, typename... Rest>
    void _push_n(lua_State * state, T value, Rest... rest)
    {
        _push(state, std::forward<T>(value));
        _push_n(state, rest...);
    }

    template <typename... T, std::size_t... N>
    void _push_dispatcher(lua_State * state, std::tuple<T...> &&values, _indices<N...>)
    {
        _push_n(state, std::get<N>(values)...);
    }

    template <typename... T>
    void _push(lua_State * state, std::tuple<T...> &&values)
    {
        constexpr int num_values = sizeof...(T);
        lua_settop(state, num_values);
        _push_dispatcher(state, std::forward<std::tuple<T...>>(values), typename _indices_builder<num_values>::type());
    }
}


template <int N, typename Return, typename... Args>
class Function : public BaseFunction
{
private:
    std::function<Return(Args...)> m_function;
    std::string m_name;
    lua_State ** m_state;

public:
    Function(lua_State * &state, const std::string & name, Return(*function)(Args...)) : Function(state, name, std::function<Return(Args...)>{function}) {}

    Function(lua_State * &state, const std::string & name, std::function<Return(Args...)> function) : m_function(function), m_name(name), m_state(&state)
    {
        lua_pushlightuserdata(state, (void *)static_cast<BaseFunction *>(this));

        lua_pushcclosure(state, &Lua::_lua_dispatcher, 1);

        lua_setglobal(state, name.c_str());
    }

    ~Function()
    {
        if (m_state != nullptr && *m_state != nullptr)
        {
            lua_pushnil(*m_state);
            lua_setglobal(*m_state, m_name.c_str());
        }
    }

    int apply(lua_State * state)
    {
        std::tuple<Args...> args = Lua::_get_args<Args...>(state);
        Return value = Lua::_lift(m_function, args);
        Lua::_push(state, std::forward<Return>(value));
        return N;
    }

public:
    void operator=(Function &) = delete;
};
