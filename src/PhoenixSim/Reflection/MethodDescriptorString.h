#pragma once

#include <array>

namespace Phoenix
{
    // ── consteval error sentinels ─────────────────────────────────────────────
    // These functions are intentionally never defined.  Calling them from a
    // consteval context produces a compile error whose name IS the message.
    consteval void MethodDeclarationString_ERROR_unmatched_parentheses_in_method_name();
    consteval void MethodDeclarationString_ERROR_too_many_param_names_for_template_args();
    consteval void MethodDeclarationString_ERROR_param_name_count_does_not_match_template_arg_count();

    template <class ...TArgs>
    struct MethodDeclarationString
    {
        static constexpr size_t NumParams = sizeof...(TArgs);

        // kFallbackArgNames provides stable string_views for unnamed params.
        // Supports up to 16 params; add more if needed.
        static constexpr const char* kFallbackArgNames[] = {
            "arg0","arg1","arg2","arg3","arg4","arg5","arg6","arg7",
            "arg8","arg9","arg10","arg11","arg12","arg13","arg14","arg15"
        };

        // Accepts a string literal: either a bare name ("MyMethod") or a name
        // with comma-separated param names ("MyMethod(paramA, paramB)").
        // constexpr so MSVC can evaluate it during static initialisation; the
        // throw() inside acts as a compile-time assertion when called in a
        // constexpr/consteval context, and aborts at startup otherwise.
        template <size_t N>
        consteval MethodDeclarationString(const char (&str)[N])
        {
            std::string_view view(str, N - 1);  // strip null terminator

            const size_t openParen = view.find('(');
            if (openParen != std::string_view::npos)
            {
                const size_t closeParen = view.rfind(')');
                if (closeParen == std::string_view::npos || closeParen < openParen)
                {
                    MethodDeclarationString_ERROR_unmatched_parentheses_in_method_name();
                }

                Name = view.substr(0, openParen);

                // Split the param list by ',' manually (std::views::split not reliably consteval).
                const std::string_view paramList = view.substr(openParen + 1, closeParen - openParen - 1);
                size_t argIdx = 0;
                size_t start  = 0;
                for (size_t i = 0; i <= paramList.size(); ++i)
                {
                    if (i == paramList.size() || paramList[i] == ',')
                    {
                        // Trim leading/trailing spaces.
                        size_t s = start, e = i;
                        while (s < e && paramList[s] == ' ') ++s;
                        while (e > s && paramList[e - 1] == ' ') --e;
                        if (argIdx >= NumParams)
                        {
                            MethodDeclarationString_ERROR_too_many_param_names_for_template_args();
                        }
                        ParamNames[argIdx++] = paramList.substr(s, e - s);
                        start = i + 1;
                    }
                }
                if (argIdx != NumParams)
                {
                    MethodDeclarationString_ERROR_param_name_count_does_not_match_template_arg_count();
                }
            }
            else
            {
                Name = view;
                for (size_t i = 0; i < NumParams; ++i)
                {
                    ParamNames[i] = std::string_view(kFallbackArgNames[i]);
                }
            }
        }

        std::string_view Name;
        std::array<std::string_view, NumParams> ParamNames = {};
    };
}