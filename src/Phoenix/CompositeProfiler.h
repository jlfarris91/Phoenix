
#pragma once

#include "Phoenix/Profiling.h"

#include <initializer_list>
#include <vector>

namespace Phoenix::Profiling
{

// Fans out every IProfiler call to a fixed list of backends.
// Ownership is not taken — callers must ensure backends outlive this object.
class CompositeProfiler : public IProfiler
{
public:
    CompositeProfiler() = default;

    explicit CompositeProfiler(std::initializer_list<IProfiler*> profilers)
        : m_profilers(profilers) {}

    void Add(IProfiler* profiler)
    {
        m_profilers.push_back(profiler);
    }

    void SetThreadName(const char* txt, int32_t hint) override
    {
        for (auto* p : m_profilers) p->SetThreadName(txt, hint);
    }

    void BeginZone(const SourceLocation* srcLoc, int32_t depth) override
    {
        for (auto* p : m_profilers) p->BeginZone(srcLoc, depth);
    }

    void EndZone() override
    {
        for (auto* p : m_profilers) p->EndZone();
    }

    void Text(const char* txt, size_t size) override
    {
        for (auto* p : m_profilers) p->Text(txt, size);
    }

    void TextFmt(const char* fmt, ...) override
    {
        va_list args;
        va_start(args, fmt);
        char buf[512];
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        for (auto* p : m_profilers) p->Text(buf, strlen(buf));
    }

    void Name(const char* txt, size_t size) override
    {
        for (auto* p : m_profilers) p->Name(txt, size);
    }

    void NameFmt(const char* fmt, ...) override
    {
        va_list args;
        va_start(args, fmt);
        char buf[512];
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        for (auto* p : m_profilers) p->Name(buf, strlen(buf));
    }

    void Color(uint32_t color) override
    {
        for (auto* p : m_profilers) p->Color(color);
    }

    void Value(uint64_t value) override
    {
        for (auto* p : m_profilers) p->Value(value);
    }

private:
    std::vector<IProfiler*> m_profilers;
};

} // namespace Phoenix::Profiling
