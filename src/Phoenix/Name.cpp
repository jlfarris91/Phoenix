
#include "Phoenix/Name.h"

#include <mutex>

using namespace Phoenix;

const FName FName::None = FName();
const FName FName::Empty = ""_n;

#if PHOENIX_SIM_NAME_ENTRIES

#define PHOENIX_DUMP_SIM_NAMES 0

namespace
{
    std::unordered_map<hash32_t, std::string>& NameToString()
    {
        static std::unordered_map<hash32_t, std::string> s_map;
        return s_map;
    }
}

#ifdef _WIN32
#include <windows.h>
#include <string>
#include <fstream>

namespace
{
    // Helper: Checks if a char is printable ASCII
    static bool IsPrintable(char c)
    {
        return (c >= 32 && c <= 126);
    }

    // Scans the .rdata section for likely string literals
    std::unordered_set<const char*> ScanRDataForStrings(size_t minLength = 4)
    {
        std::unordered_set<const char*> foundStrings;
        HMODULE hModule = GetModuleHandleA(NULL);
        if (!hModule)
        {
            return foundStrings;
        }

        // Get DOS header
        auto* dosHeader = (PIMAGE_DOS_HEADER)hModule;
        auto* ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
        auto* section = IMAGE_FIRST_SECTION(ntHeaders);
        WORD numSections = ntHeaders->FileHeader.NumberOfSections;

        for (WORD i = 0; i < numSections; ++i, ++section)
        {
            // Look for .rdata section
            if (strncmp((const char*)section->Name, ".rdata", 6) == 0)
            {
                BYTE* start = (BYTE*)hModule + section->VirtualAddress;
                BYTE* end = start + section->Misc.VirtualSize;
                BYTE* p = start;
                while (p < end)
                {
                    // Look for ASCII string
                    BYTE* s = p;
                    size_t len = 0;
                    while (s < end && IsPrintable(*s))
                    {
                        ++s; ++len;
                    }

                    if (len >= minLength && s < end && *s == '\0')
                    {
                        foundStrings.emplace((const char*)p);
                        p = s + 1;
                    }
                    else
                    {
                        ++p;
                    }
                }
                break;
            }
        }
        return foundStrings;
    }

    static bool _scanned_for_strings = []() {
        auto strings = ScanRDataForStrings();
        for (const char* str : strings)
        {
            hash32_t hash = FName(str);
            NameToString()[hash] = str;
        }
#if PHOENIX_DUMP_SIM_NAMES
        std::ofstream out("Names.txt");
        for (const auto& [hash, str] : NameToString())
        {
            out << std::hex << hash << " (" << hash << ") : " << str << "\n";
        }
#endif
        return true;
    }();
}

#endif // _WIN32

namespace
{
    std::recursive_mutex g_nameEntryMutex;
}

#endif // PHOENIX_SIM_NAME_ENTRIES

const char* FName::GetNameEntry(hash32_t hash)
{
    if (hash == 0)
    {
        return "";
    }
#if PHOENIX_SIM_NAME_ENTRIES
    {
        std::scoped_lock lock(g_nameEntryMutex);
        auto iter = NameToString().find(hash);
        if (iter == NameToString().end())
        {
            std::string string = std::to_string(hash);
            return RecordNameEntryAs(string.c_str(), string.size(), hash);
        }
        return iter->second.c_str();
    }
#else
    return "???";
#endif
}

const char* FName::ToString() const
{
    return GetNameEntry(Value);
}

#if PHOENIX_SIM_NAME_ENTRIES

const char* FName::RecordNameEntryAs(const char* chars, size_t len, hash32_t asHash)
{
    if (asHash == 0)
    {
        return "";
    }
    std::scoped_lock lock(g_nameEntryMutex);
    std::string& string = NameToString()[asHash];
    string.assign(chars, len);
    return string.c_str();
}

const char* FName::AppendNameEntryAs(hash32_t base, const char* chars, size_t len, hash32_t asHash)
{
    std::scoped_lock lock(g_nameEntryMutex);
    std::string string = GetNameEntry(base);
    string.append(chars, len);
    return RecordNameEntryAs(string.c_str(), string.size(), asHash);
}

const char* FName::CombineNameEntryAs(hash32_t base, hash32_t other, hash32_t asHash)
{
    std::scoped_lock lock(g_nameEntryMutex);
    std::string baseString = GetNameEntry(base);
    std::string otherString = GetNameEntry(other);
    std::string string = baseString + "+" + otherString;
    return RecordNameEntryAs(string.c_str(), string.size(), asHash);
}

#endif