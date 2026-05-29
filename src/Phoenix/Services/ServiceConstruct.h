#pragma once

#include <memory>
#include <tuple>

#include "IServiceLocator.h"

namespace Phoenix
{
    // Opt-in automatic dependency resolution.
    // Define inside your class: using Dependencies = std::tuple<Dep1, Dep2, ...>;
    template <class T>
    concept HasDependencies = requires { typename T::Dependencies; };

    namespace Detail
    {
        // Resolve TDeps from loc, prepend a child scope if the constructor takes one,
        // then append any extra TValues. Called by both MakeService and WithConstructor.
        template <class T, class... TDeps, class... TValues>
        std::shared_ptr<T> ConstructWithDeps(
            IServiceLocator& loc,
            std::tuple<TDeps...>*,    // tag — deduces TDeps without passing a real tuple
            TValues&&... values)
        {
            if constexpr (std::is_constructible_v<T,
                std::shared_ptr<IServiceLocator>,
                std::shared_ptr<TDeps>...,
                TValues...>)
            {
                return std::make_shared<T>(
                    loc.CreateChildScope(),
                    loc.ResolveService<TDeps>()...,
                    std::forward<TValues>(values)...);
            }
            else
            {
                return std::make_shared<T>(
                    loc.ResolveService<TDeps>()...,
                    std::forward<TValues>(values)...);
            }
        }
    }

    // Default construction path for Register<T>().
    //   1. If T::Dependencies exists — resolve those deps and construct.
    //   2. Else if T(shared_ptr<IServiceLocator>, values...) exists — inject a child scope.
    //   3. Else — construct directly with values.
    template <class T, class... TValues>
    std::shared_ptr<T> MakeService(IServiceLocator& loc, TValues&&... values)
    {
        if constexpr (HasDependencies<T>)
        {
            using Deps = typename T::Dependencies;
            return Detail::ConstructWithDeps<T>(
                loc,
                static_cast<Deps*>(nullptr),
                std::forward<TValues>(values)...);
        }
        else if constexpr (std::is_constructible_v<T, std::shared_ptr<IServiceLocator>, TValues...>)
        {
            return std::make_shared<T>(loc.CreateChildScope(), std::forward<TValues>(values)...);
        }
        else
        {
            return std::make_shared<T>(std::forward<TValues>(values)...);
        }
    }

    // Explicit construction path for WithConstructor<TDeps...>().
    // Use when T has multiple constructors and auto-detection picks the wrong one.
    template <class T, class... TDeps, class... TValues>
    std::shared_ptr<T> MakeServiceExplicit(IServiceLocator& loc, TValues&&... values)
    {
        return Detail::ConstructWithDeps<T>(
            loc,
            static_cast<std::tuple<TDeps...>*>(nullptr),
            std::forward<TValues>(values)...);
    }
}
