
#include "PhoenixSim/Name.h"

using namespace Phoenix;

const FName FName::None = FName();
const FName FName::Empty = ""_n;

#if PHOENIX_SIM_NAME_ENTRIES

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
#include <vector>
#include <string>

// Helper: Checks if a char is printable ASCII
static bool IsPrintable(char c) {
    return (c >= 32 && c <= 126);
}

// Scans the .rdata section for likely string literals
std::vector<const char*> ScanRDataForStrings(size_t minLength = 4) {
    std::vector<const char*> foundStrings;
    HMODULE hModule = GetModuleHandleA(NULL);
    if (!hModule) return foundStrings;

    // Get DOS header
    auto* dosHeader = (PIMAGE_DOS_HEADER)hModule;
    auto* ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
    auto* section = IMAGE_FIRST_SECTION(ntHeaders);
    WORD numSections = ntHeaders->FileHeader.NumberOfSections;

    for (WORD i = 0; i < numSections; ++i, ++section) {
        // Look for .rdata section
        if (strncmp((const char*)section->Name, ".rdata", 6) == 0) {
            BYTE* start = (BYTE*)hModule + section->VirtualAddress;
            BYTE* end = start + section->Misc.VirtualSize;
            BYTE* p = start;
            while (p < end) {
                // Look for ASCII string
                BYTE* s = p;
                size_t len = 0;
                while (s < end && IsPrintable(*s)) { ++s; ++len; }
                if (len >= minLength && s < end && *s == '\0') {
                    foundStrings.push_back((const char*)p);
                    p = s + 1;
                } else {
                    ++p;
                }
            }
            break;
        }
    }
    return foundStrings;
}

struct Foo
{
    Foo()
    {
        auto strings = ScanRDataForStrings(4);
        for (const char* s : strings)
        {
            FName name(s, strlen(s));
            NameToString()[name] = s;
        }
    }
} g_Foo;
#endif

#endif

const char* FName::GetNameEntry(hash32_t hash)
{
#if PHOENIX_SIM_NAME_ENTRIES
    auto iter = NameToString().find(hash);
    if (iter == NameToString().end())
    {
        std::string string = std::to_string(hash);
        return RecordNameEntryAs(string.c_str(), string.size(), hash);
    }
    return iter->second.c_str();
#else
    return "???";
#endif
}

#if PHOENIX_SIM_NAME_ENTRIES

const char* FName::RecordNameEntryAs(const char* chars, size_t len, hash32_t value)
{
    std::string& string = NameToString()[value];
    string.assign(chars, len);
    return string.c_str();
}

const char* FName::AppendNameEntryAs(hash32_t base, const char* chars, size_t len, hash32_t value)
{
    auto iter = NameToString().find(base);
    std::string string = iter != NameToString().end() ? iter->second.c_str() : "?";
    string.append(chars, len);
    return RecordNameEntryAs(string.c_str(), string.size(), value);
}

const char* FName::CombineNameEntryAs(hash32_t base, hash32_t other, hash32_t value)
{
    auto baseIter = NameToString().find(base);
    auto otherIter = NameToString().find(other);
    std::string baseString = baseIter != NameToString().end() ? baseIter->second.c_str() : "?";
    std::string otherString = otherIter != NameToString().end() ? otherIter->second.c_str() : "?";
    std::string string = baseString + "+" + otherString;
    return RecordNameEntryAs(string.c_str(), string.size(), value);
}

#endif