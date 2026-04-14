#pragma once

namespace Phoenix
{
    template <class T>
    class ExternalTypeRegistration;

    // ── detail helpers ────────────────────────────────────────────────────────
    namespace detail
    {
        // Detects whether T has a StaticTypeName member.
        template <class T>
        concept HasStaticTypeName =
            requires { typename decltype(T::StaticTypeName); } ||
            requires { typename decltype(ExternalTypeRegistration<T>::StaticTypeName); };
    } // namespace detail
}
