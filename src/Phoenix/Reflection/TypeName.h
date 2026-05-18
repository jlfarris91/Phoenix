#pragma once

#include <string>
#include <source_location>

#include "Phoenix/Name.h"

namespace Phoenix
{
    namespace detail
    {
        // Returns the fully qualified C++ name of T (e.g. "Phoenix::RTS::FeatureUnit")
        template <class T>
        constexpr std::string_view GetQualifiedTypeName()
        {
            auto loc = std::source_location::current();
            std::string_view f = loc.function_name();
#ifdef _MSC_VER
            constexpr auto funcName = std::string_view("GetQualifiedTypeName");
            auto start = f.find(funcName);
            if (start == std::string_view::npos)
            {
                return f.data();
            }
            start += funcName.length() + 1;
            const auto end = f.find_last_of('>');
            if (end == std::string_view::npos)
            {
                return f.data();
            }
            return std::string_view(f.substr(start, end - start));
#else
            const auto start = f.find("T = ");
            if (start == std::string_view::npos) return f.data();
            const auto end = f.find_first_of(';', start + 4);
            return std::string_view(f.substr(start + 4, end - (start + 4)));
#endif
            // auto loc = std::source_location::current();
            // std::string_view f = loc.function_name();
            // const auto start = f.find("T = ");
            // if (start == std::string_view::npos) return f;
            // const auto end = f.find_first_of(';', start + 4);
            // return std::string_view(f.substr(start + 4, end - (start + 4)));
        }

        constexpr size_t FindFirstCharAtLevel(const char* str, char c, size_t s, size_t e, size_t l = 0)
        {
            size_t i = s;
            size_t d = 0;
            size_t f = std::string::npos;
            if (c == '<') ++l;
            for (; i <= e; ++i)
            {
                if (str[i] == '<') ++d;
                if (str[i] == '>') --d;
                if (str[i] == c && d == l) { f = i; break; }
            }
            return f;
        }

        constexpr size_t FindLastCharAtLevel(const char* str, char c, size_t s, size_t e, size_t l = 0)
        {
            size_t i = s;
            size_t d = 0;
            size_t f = std::string::npos;
            if (c == '<') ++l;
            for (; i <= e; ++i)
            {
                if (str[i] == '<') ++d;
                if (str[i] == '>') --d;
                if (str[i] == c && d == l) f = i;
            }
            return f;
        }
    }

    class FTypeName
    {
    public:

        constexpr FTypeName() = default;

        constexpr FTypeName(const FTypeName& other)
        {
            std::ranges::copy(other.Name, Name);
            std::ranges::copy(other.TemplateName, TemplateName);
            std::ranges::copy(other.Parent, Parent);
            std::ranges::copy(other.QualifiedName, QualifiedName);
            NumTemplateArgs = other.NumTemplateArgs;
            for (size_t i = 0; i < other.NumTemplateArgs; ++i)
                std::ranges::copy(other.TemplateArgs[i], TemplateArgs[i]);
        }

        constexpr FTypeName(FTypeName&& other) noexcept
        {
            std::ranges::copy(other.Name, Name);
            std::ranges::copy(other.TemplateName, TemplateName);
            std::ranges::copy(other.Parent, Parent);
            std::ranges::copy(other.QualifiedName, QualifiedName);
            NumTemplateArgs = other.NumTemplateArgs;
            for (size_t i = 0; i < other.NumTemplateArgs; ++i)
                std::ranges::copy(other.TemplateArgs[i], TemplateArgs[i]);
        }
        
        FTypeName& operator=(const FTypeName& other) noexcept
        {
            std::ranges::copy(other.Name, Name);
            std::ranges::copy(other.TemplateName, TemplateName);
            std::ranges::copy(other.Parent, Parent);
            std::ranges::copy(other.QualifiedName, QualifiedName);
            NumTemplateArgs = other.NumTemplateArgs;
            for (size_t i = 0; i < other.NumTemplateArgs; ++i)
                std::ranges::copy(other.TemplateArgs[i], TemplateArgs[i]);
            return *this;
        }
        
        FTypeName& operator=(FTypeName&& other) noexcept
        {
            std::ranges::copy(other.Name, Name);
            std::ranges::copy(other.TemplateName, TemplateName);
            std::ranges::copy(other.Parent, Parent);
            std::ranges::copy(other.QualifiedName, QualifiedName);
            NumTemplateArgs = other.NumTemplateArgs;
            for (size_t i = 0; i < other.NumTemplateArgs; ++i)
                std::ranges::copy(other.TemplateArgs[i], TemplateArgs[i]);
            return *this;
        }

        static constexpr FTypeName Construct(std::string qualifiedTypeName, const std::string& alias = {})
        {
#ifdef _MSC_VER
            if (qualifiedTypeName.starts_with("class "))   qualifiedTypeName = qualifiedTypeName.substr(6);
            if (qualifiedTypeName.starts_with("struct "))  qualifiedTypeName = qualifiedTypeName.substr(7);
            if (qualifiedTypeName.starts_with("enum "))    qualifiedTypeName = qualifiedTypeName.substr(5);

            // Normalize MSVC-specific integer spellings to cross-platform names.
            // unsigned must be checked before the signed variants to avoid
            // partial replacement of "unsigned __int64" → "unsigned int64".
            {
                struct { const char* From; const char* To; } kIntNorm[] = {
                    { "unsigned __int64", "uint64" },
                    { "unsigned __int32", "uint32" },
                    { "unsigned __int16", "uint16" },
                    { "unsigned __int8",  "uint8"  },
                    { "__int64",          "int64"  },
                    { "__int32",          "int32"  },
                    { "__int16",          "int16"  },
                    { "__int8",           "int8"   },
                };
                for (auto& [from, to] : kIntNorm)
                {
                    size_t pos = 0;
                    const size_t fromLen = std::string_view(from).size();
                    const size_t toLen   = std::string_view(to).size();
                    while ((pos = qualifiedTypeName.find(from, pos)) != std::string::npos)
                    {
                        qualifiedTypeName.replace(pos, fromLen, to);
                        pos += toLen;
                    }
                }
            }
#endif
            if (qualifiedTypeName[0] == ' ')                    qualifiedTypeName = qualifiedTypeName.substr(1);

            size_t len = qualifiedTypeName.size();
            const char* start = qualifiedTypeName.c_str();

            FTypeName result;
            std::ranges::copy(qualifiedTypeName, result.QualifiedName);

            // Recurse into template args
            auto firstLT = detail::FindLastCharAtLevel(start, '<', 0, len);
            if (firstLT != std::string::npos)
            {
                auto lastGT = detail::FindLastCharAtLevel(start, '>', 0, len);

                auto readPos = firstLT + 1;

                // Construct a TypeName object for each template arg
                auto nextComma = detail::FindFirstCharAtLevel(start, ',', readPos, lastGT);
                while (readPos < lastGT)
                {
                    nextComma = nextComma > lastGT ? lastGT : nextComma;
                    auto argLen = nextComma - readPos;
                
                    std::string templateArg(start + readPos, argLen);
                    std::ranges::copy(templateArg, result.TemplateArgs[result.NumTemplateArgs++]);

                    readPos = nextComma + 1;
                    nextComma = detail::FindFirstCharAtLevel(start, ',', readPos, lastGT);
                }

                // Store just the template name, ie TFixed (without the template args)
                auto lastColon = detail::FindLastCharAtLevel(start, ':', 0, len);
                if (lastColon != std::string::npos && lastColon > firstLT)
                {
                    std::ranges::copy(qualifiedTypeName.substr(lastColon + 1, firstLT - lastColon - 1), result.TemplateName);
                }
                else
                {
                    std::ranges::copy(qualifiedTypeName.substr(0, firstLT), result.TemplateName);
                }
            }

            // Find the actual type name
            auto lastColon = detail::FindLastCharAtLevel(start, ':', 0, len);
            if (lastColon != std::string::npos)
            {
                std::ranges::copy(qualifiedTypeName.substr(lastColon + 1), result.Name);
                std::ranges::copy(qualifiedTypeName.substr(0, lastColon - 1), result.Parent);
            }
            else
            {
                std::ranges::copy(qualifiedTypeName, result.Name);
                result.Parent[0] = '\0';
            }

            return result;
        }

        template <class T>
        static constexpr FTypeName Construct(const std::string& alias = {})
        {
            return Construct(std::string{ detail::GetQualifiedTypeName<T>() }, alias);
        }

        constexpr bool IsEmpty() const
        {
            return Name[0] == '\0';
        }

        constexpr const char* GetName() const
        {
            return Name;
        }

        constexpr const char* GetTemplateName() const
        {
            return TemplateName;
        }

        constexpr const char* GetParent() const
        {
            return Parent;
        }

        constexpr const char* GetQualifiedName() const
        {
            return QualifiedName;
        }

        constexpr const auto& GetTemplateArgs() const
        {
            return TemplateArgs;
        }

        constexpr size_t GetNumTemplateArgs() const
        {
            return NumTemplateArgs;
        }

        constexpr FTypeName GetParentTypeName() const
        {
            return Parent[0] == '\0' ? FTypeName() : Construct(Parent);
        }

    private:
        char Name[256] = {};
        char TemplateName[256] = {};
        char Parent[256] = {};
        char QualifiedName[512] = {};
        char TemplateArgs[8][512] = {};
        uint8 NumTemplateArgs = 0;
    };

    template <class T>
    struct StaticTypeName
    {
        static constexpr FTypeName TypeName = FTypeName::Construct<T>();
        static constexpr FName TypeId = FName(TypeName.GetQualifiedName());

        static constexpr const FTypeName& Get()
        {
            return TypeName;
        }

        static constexpr bool IsEmpty()
        {
            return TypeName.IsEmpty();
        }

        static constexpr const char* GetName()
        {
            return TypeName.GetName();
        }

        static constexpr const char* GetTemplateName()
        {
            return TypeName.GetName();
        }

        static constexpr const char* GetParent()
        {
            return TypeName.GetParent();
        }

        static constexpr const char* GetQualifiedName()
        {
            return TypeName.GetQualifiedName();
        }

        static constexpr const auto& GetTemplateArgs()
        {
            return TypeName.GetTemplateArgs();
        }

        static constexpr size_t GetNumTemplateArgs()
        {
            return TypeName.GetNumTemplateArgs();
        }
    };
}
