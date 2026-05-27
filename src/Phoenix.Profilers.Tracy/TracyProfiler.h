
#pragma once

#include "Phoenix/Profiling.h"

namespace Phoenix::Profiling
{
    class TracyProfiler : public IProfiler
    {
    public:
        void SetThreadName(const char* txt, int32_t hint) override;
        void BeginZone(const SourceLocation* srcLoc, int32_t depth) override;
        void EndZone() override;
        void Text(const char* txt, size_t size) override;
        void TextFmt(const char* fmt, ...) override;
        void Name(const char* txt, size_t size) override;
        void NameFmt(const char* fmt, ...) override;
        void Color(uint32_t color) override;
        void Value(uint64_t value) override;
    };
}

