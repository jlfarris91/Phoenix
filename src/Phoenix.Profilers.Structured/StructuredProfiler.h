
#pragma once

#include <cstdint>
#include <string>

#include "Phoenix/Profiling.h"

namespace Phoenix::Profiling
{

// IProfiler backend that captures every zone into a compact binary .phxcap file.
// Convert to SQLite for analysis: python3 tools/phxcap_to_db.py capture.phxcap
//
// Thread-safe: each thread maintains a local 24 KB zone buffer; completed buffers
// are written to the capture file under a single file mutex (amortised, not per zone).
// No SQLite overhead on the hot path — bounded only by sequential disk write speed.
class StructuredProfiler : public IProfiler
{
public:
    // path        — filesystem path to the .phxcap output file
    // flushEvery  — kept for API compatibility; ignored (buffer size is fixed)
    // clearOnOpen — delete an existing file before opening
    explicit StructuredProfiler(const char* path, int flushEvery = 4096, bool clearOnOpen = false);
    ~StructuredProfiler() override;

    // Finalise the capture file (write string table + footer). Called by destructor.
    void Close();

    void SetThreadName(const char* txt, int32_t hint) override;
    void BeginZone(const SourceLocation* srcLoc, int32_t depth) override;
    void EndZone() override;
    void Text(const char* txt, size_t size) override;
    void TextFmt(const char* fmt, ...) override;
    void Name(const char* txt, size_t size) override;
    void NameFmt(const char* fmt, ...) override;
    void Color(uint32_t color) override;
    void Value(uint64_t value) override;

private:
    std::string m_path;
};

} // namespace Phoenix::Profiling
