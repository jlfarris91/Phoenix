
#pragma once

#include <tuple>
#include <type_traits>

namespace Phoenix
{
    template <size_t I = 0, typename... Ts>
    std::enable_if_t<I == sizeof...(Ts), bool> ContainsNullptr(const std::tuple<Ts...>&)
    {
        return false;
    }

    template <size_t I = 0, typename... Ts>
    std::enable_if_t<I < sizeof...(Ts), bool> ContainsNullptr(const std::tuple<Ts...>& t)
    {
        using T = std::tuple_element_t<I, std::tuple<Ts...>>;
        if constexpr (std::is_pointer_v<T>)
        {
            if (std::get<I>(t) == nullptr)
            {
                return true;
            }
        }
        return ContainsNullptr<I + 1>(t);
    }

    template <class Fn, class ...TArgs>
    concept FnReturnsBool = requires (Fn f)
    {
        { f(std::declval<TArgs>()...) } -> std::same_as<bool>;
    };

    template <class TCallback, class TIndex, class ...TArgs>
    concept FirstParamIsIndex = requires (const TCallback& cb, TIndex index, TArgs&&... args)
    {
        cb(index, std::forward<TArgs>(args)...);
    };

    // Returns true if the for-each loop should break.
    template <class TCallback, class TIndex, class ...TArgs>
    bool InvokeForEachCallbackWithIndex(const TCallback& callback, TIndex index, TArgs&&... args)
    {
        // Handle the case where the callback takes an index as the first argument.
        if constexpr ( FirstParamIsIndex<TCallback, TIndex, TArgs...> )
        {
            if constexpr ( FnReturnsBool<TCallback, TIndex, TArgs...>)
            {
                return callback(index, std::forward<TArgs>(args)...);
            }
            else
            {
                callback(index, std::forward<TArgs>(args)...);
                return false;
            }
        }
        else
        {
            if constexpr ( FnReturnsBool<TCallback, TArgs...> )
            {
                return callback(std::forward<TArgs>(args)...);
            }
            else
            {
                callback(std::forward<TArgs>(args)...);
                return false;
            }
        }
    }

    template <class TCallback>
    void TEST(const TCallback& callback)
    {
        for (int i = 0; i < 10; ++i)
        {
            double a = i * 100;
            if (InvokeForEachCallbackWithIndex(callback, i, a))
            {
                return;
            }
        }
    }

    inline void TEST2()
    {
        TEST([](int index, double a)
        {
            
        });

        TEST([](double a)
        {
            
        });

        TEST([](int index, double a)
        {
            return a > 500.0;
        });

        TEST([](double a)
        {
            return a > 500.0;
        });
    }

    // Returns true if the for-each loop should break.
    template <class TCallback, class ...TArgs>
    bool InvokeForEachCallbackNoIndex(const TCallback& callback, TArgs&&... args)
    {
        // Handle the case where the callback returns a bool
        if constexpr ( FnReturnsBool<TCallback, TArgs...>)
        {
            return callback(std::forward<TArgs>(args)...);
        }

        callback(std::forward<TArgs>(args)...);
        return false;
    }

    template <class T>
    void Swap(T& a, T& b)
    {
        T temp = a;
        a = b;
        b = temp;
    }
}
