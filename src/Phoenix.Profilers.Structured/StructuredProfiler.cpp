
#include "StructuredProfiler.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <chrono>

using namespace Phoenix::Profiling;

// ── Timing ────────────────────────────────────────────────────────────────────

static int64_t NowNs()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

// ── Binary zone record (32 bytes) ─────────────────────────────────────────────
// Layout must match phxcap_to_db.py struct format '<qqIIQ'.

#pragma pack(push, 1)
struct BinZone
{
    int64_t  startNs;    // 8
    int64_t  endNs;      // 8
    uint32_t threadId;   // 4
    uint32_t pad;        // 4 — reserved
    uint64_t srcLocPtr;  // 8 — raw SourceLocation* used as key in string table
};
#pragma pack(pop)
static_assert(sizeof(BinZone) == 32, "BinZone must be 32 bytes");

// ── Active zone (on-stack while zone is open) ─────────────────────────────────

struct ActiveZone
{
    const SourceLocation* srcLoc;
    int64_t startNs;
};

// ── Global file state ─────────────────────────────────────────────────────────

static std::mutex                                s_fileMutex;
static FILE*                                     s_file      = nullptr;
static uint64_t                                  s_zoneCount = 0;
static std::unordered_set<const SourceLocation*> s_srcLocs;
static std::unordered_map<uint32_t, std::string> s_threadNames;

static uint32_t AssignThreadId()
{
    static std::mutex                                     s_tidMutex;
    static std::unordered_map<std::thread::id, uint32_t> s_tidMap;
    static uint32_t                                       s_next = 1;

    auto id = std::this_thread::get_id();
    std::lock_guard<std::mutex> lk(s_tidMutex);
    auto [it, inserted] = s_tidMap.emplace(id, 0u);
    if (inserted) it->second = s_next++;
    return it->second;
}

// ── Thread-local state — destructor auto-flushes when thread exits ────────────

static constexpr int kBufZones  = 1024;
static constexpr int kStackSize = 256;

struct ThreadLocalState
{
    ActiveZone stack[kStackSize];
    int        stackDepth = 0;
    BinZone    buf[kBufZones];
    int        bufCount   = 0;
    uint32_t   threadId   = 0;

    void Flush()
    {
        if (bufCount == 0) return;
        std::lock_guard<std::mutex> lk(s_fileMutex);
        if (!s_file) { bufCount = 0; return; }
        fwrite(buf, sizeof(BinZone), (size_t)bufCount, s_file);
        s_zoneCount += (uint64_t)bufCount;
        for (int i = 0; i < bufCount; ++i)
        {
            const auto* loc = reinterpret_cast<const SourceLocation*>(buf[i].srcLocPtr);
            if (loc) s_srcLocs.insert(loc);
        }
        bufCount = 0;
    }

    // Called automatically by the CRT when the owning thread exits.
    ~ThreadLocalState() { Flush(); }
};

static thread_local ThreadLocalState t_tls;

// ── File format constants ─────────────────────────────────────────────────────

static const char k_magic[8]   = { 'P','H','X','C','A','P','\0','\1' };
static const char k_strMark[8] = { 'P','H','X','S','T','R','T','\0' };
static const char k_tail[8]    = { 'P','H','X','T','A','I','L','\0' };

// ── StructuredProfiler ────────────────────────────────────────────────────────

StructuredProfiler::StructuredProfiler(const char* path, int /*flushEvery*/, bool clearOnOpen)
    : m_path(path)
{
    if (clearOnOpen)
        (void)::remove(path);

    std::lock_guard<std::mutex> lk(s_fileMutex);
    s_file = fopen(path, "wb");
    assert(s_file && "StructuredProfiler: failed to open file");
    if (s_file)
        fwrite(k_magic, 1, sizeof(k_magic), s_file);
}

StructuredProfiler::~StructuredProfiler()
{
    Close();
}

void StructuredProfiler::Close()
{
    // Flush the calling thread's last partial buffer.
    t_tls.Flush();

    std::lock_guard<std::mutex> lk(s_fileMutex);
    if (!s_file)
        return;

    // ── String section ─────────────────────────────────────────────────────
    fwrite(k_strMark, 1, sizeof(k_strMark), s_file);

    uint32_t numSrcLocs = (uint32_t)s_srcLocs.size();
    fwrite(&numSrcLocs, sizeof(numSrcLocs), 1, s_file);

    for (const SourceLocation* loc : s_srcLocs)
    {
        uint64_t ptr  = reinterpret_cast<uint64_t>(loc);
        uint32_t line = loc->Line;
        fwrite(&ptr,  sizeof(ptr),  1, s_file);
        fwrite(&line, sizeof(line), 1, s_file);
        const char* name = loc->Name     ? loc->Name     : "";
        const char* func = loc->Function ? loc->Function : "";
        const char* file = loc->File     ? loc->File     : "";
        fwrite(name, 1, strlen(name) + 1, s_file);
        fwrite(func, 1, strlen(func) + 1, s_file);
        fwrite(file, 1, strlen(file) + 1, s_file);
    }

    // ── Thread name section ────────────────────────────────────────────────
    uint32_t numThreads = (uint32_t)s_threadNames.size();
    fwrite(&numThreads, sizeof(numThreads), 1, s_file);
    for (const auto& [tid, name] : s_threadNames)
    {
        fwrite(&tid, sizeof(tid), 1, s_file);
        fwrite(name.c_str(), 1, name.size() + 1, s_file);
    }

    // ── Footer ────────────────────────────────────────────────────────────
    fwrite(&s_zoneCount, sizeof(s_zoneCount), 1, s_file);
    fwrite(k_tail, 1, sizeof(k_tail), s_file);

    fclose(s_file);
    s_file = nullptr;
}

void StructuredProfiler::SetThreadName(const char* txt, int32_t /*hint*/)
{
    if (t_tls.threadId == 0)
        t_tls.threadId = AssignThreadId();

    std::lock_guard<std::mutex> lk(s_fileMutex);
    s_threadNames[t_tls.threadId] = txt;
}

void StructuredProfiler::BeginZone(const SourceLocation* srcLoc, int32_t /*depth*/)
{
    if (t_tls.threadId == 0)
        t_tls.threadId = AssignThreadId();

    if (t_tls.stackDepth < kStackSize)
    {
        t_tls.stack[t_tls.stackDepth].srcLoc  = srcLoc;
        t_tls.stack[t_tls.stackDepth].startNs = NowNs();
    }
    ++t_tls.stackDepth;
}

void StructuredProfiler::EndZone()
{
    if (t_tls.stackDepth <= 0)
        return;
    --t_tls.stackDepth;
    if (t_tls.stackDepth >= kStackSize)
        return;  // stack was overflowing; zone was not recorded

    int64_t endNs = NowNs();

    BinZone& z  = t_tls.buf[t_tls.bufCount++];
    z.startNs   = t_tls.stack[t_tls.stackDepth].startNs;
    z.endNs     = endNs;
    z.threadId  = t_tls.threadId;
    z.pad       = 0;
    z.srcLocPtr = reinterpret_cast<uint64_t>(t_tls.stack[t_tls.stackDepth].srcLoc);

    if (t_tls.bufCount >= kBufZones)
        t_tls.Flush();
}

void StructuredProfiler::Text(const char* /*txt*/, size_t /*size*/) {}
void StructuredProfiler::TextFmt(const char* /*fmt*/, ...) {}
void StructuredProfiler::Name(const char* /*txt*/, size_t /*size*/) {}
void StructuredProfiler::NameFmt(const char* /*fmt*/, ...) {}
void StructuredProfiler::Color(uint32_t /*color*/) {}
void StructuredProfiler::Value(uint64_t /*value*/) {}
