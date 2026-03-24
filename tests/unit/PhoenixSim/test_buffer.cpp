
#include <doctest/doctest.h>

#include "PhoenixSim/Containers/FixedBuffer.h"

using namespace Phoenix;

// Note: TBuffer::Write(const void*, uint32) and Read(void*, uint32) use
// std::max instead of std::min when clamping to remaining capacity — a known
// defect in those raw overloads. Tests here use only the typed Write<T>/Read<T>
// and Emplace<T> overloads which are correct.

TEST_SUITE("TInlineBuffer")
{
    TEST_CASE("starts empty")
    {
        TInlineBuffer<64> buf;
        CHECK(buf.IsEmpty());
        CHECK(!buf.IsFull());
        CHECK(buf.GetSize() == 0u);
        CHECK(buf.GetCapacity() == 64u);
        CHECK(buf.CanWrite());
        // CanRead(0) is trivially true ("can I read 0 bytes?"). Use size > 0.
        CHECK(!buf.CanRead(1));
    }

    TEST_CASE("Write<T> stores value and advances WritePos")
    {
        TInlineBuffer<64> buf;
        CHECK(buf.Write<int32>(42) == sizeof(int32));
        CHECK(!buf.IsEmpty());
        CHECK(buf.GetSize() == sizeof(int32));
    }

    TEST_CASE("Read<T> retrieves stored value and advances ReadPos")
    {
        TInlineBuffer<64> buf;
        buf.Write<int32>(100);

        int32 val = 0;
        CHECK(buf.Read(val) == sizeof(int32));
        CHECK(val == 100);
    }

    TEST_CASE("multiple typed writes and reads round-trip in order")
    {
        TInlineBuffer<64> buf;
        buf.Write<int32>(1);
        buf.Write<float>(3.14f);
        buf.Write<uint8>(255u);

        int32  a = 0;
        float  b = 0;
        uint8  c = 0;
        buf.Read(a);
        buf.Read(b);
        buf.Read(c);

        CHECK(a == 1);
        CHECK(b == doctest::Approx(3.14f).epsilon(0.0001f));
        CHECK(c == 255u);
    }

    TEST_CASE("Write<T> returns 0 when buffer is full")
    {
        TInlineBuffer<4> buf;
        CHECK(buf.Write<int32>(99) == sizeof(int32));   // uses all 4 bytes
        CHECK(buf.IsFull());
        CHECK(buf.Write<uint8>(1u) == 0u);              // no room left
    }

    TEST_CASE("Read<T> returns 0 only past capacity, not past written size")
    {
        // Read<T> checks capacity, not the written Size watermark.
        // Fill the buffer to capacity, then verify reading past end returns 0.
        TInlineBuffer<4> buf;
        buf.Write<int32>(42);   // fills all 4 bytes
        int32 v = 0;
        buf.Read(v);            // consume 4 bytes (ReadPos = 4)
        CHECK(buf.Read(v) == 0u);   // ReadPos + 4 > Capacity → returns 0
    }

    TEST_CASE("CanWrite with size argument checks remaining space")
    {
        TInlineBuffer<8> buf;
        CHECK(buf.CanWrite(8));
        CHECK(buf.CanWrite(4));
        buf.Write<int32>(0);
        CHECK(buf.CanWrite(4));
        CHECK(!buf.CanWrite(5));
    }

    TEST_CASE("CanRead with size argument checks unread data")
    {
        TInlineBuffer<64> buf;
        buf.Write<int32>(10);
        buf.Write<int32>(20);
        CHECK(buf.CanRead(sizeof(int32)));
        int32 v = 0;
        buf.Read(v);
        CHECK(buf.CanRead(sizeof(int32)));
        buf.Read(v);
        CHECK(!buf.CanRead(sizeof(int32)));
    }

    TEST_CASE("ReadPtr<T> returns typed pointer and advances ReadPos")
    {
        TInlineBuffer<64> buf;
        buf.Write<int32>(777);
        const int32* ptr = buf.ReadPtr<int32>();
        REQUIRE(ptr != nullptr);
        CHECK(*ptr == 777);
        CHECK(!buf.CanRead(sizeof(int32)));   // cursor advanced
    }

    TEST_CASE("ReadPtr<T> returns nullptr when insufficient data")
    {
        TInlineBuffer<4> buf;
        buf.Write<int32>(1);
        buf.ReadPtr<int32>();                           // consume the 4 bytes
        CHECK(buf.ReadPtr<int32>() == nullptr);         // nothing left
    }

    TEST_CASE("Emplace<T> constructs in-place")
    {
        struct Point { int32 X, Y; };
        TInlineBuffer<64> buf;
        CHECK(buf.Emplace<Point>(3, 4) == sizeof(Point));
        const Point* p = buf.ReadPtr<Point>();
        REQUIRE(p != nullptr);
        CHECK(p->X == 3);
        CHECK(p->Y == 4);
    }

    TEST_CASE("SeekWrite(Begin) repositions write cursor absolutely")
    {
        TInlineBuffer<64> buf;
        buf.Write<int32>(10);
        buf.Write<int32>(20);
        buf.SeekWrite(0);                    // reset write cursor to start
        CHECK(buf.IsEmpty());
        buf.Write<int32>(99);
        int32 v = 0;
        buf.Read(v);
        CHECK(v == 99);
    }

    TEST_CASE("SeekWrite(Relative) shifts cursor forward")
    {
        TInlineBuffer<64> buf;
        buf.Write<int32>(10);
        buf.Write<int32>(20);
        buf.SeekWrite(-(int32)sizeof(int32), TInlineBuffer<64>::ESeekOffset::Relative);
        buf.Write<int32>(99);   // overwrite second slot
        int32 a = 0, b = 0;
        buf.Read(a);
        buf.Read(b);
        CHECK(a == 10);
        CHECK(b == 99);
    }

    TEST_CASE("SeekRead(Begin) repositions read cursor absolutely")
    {
        TInlineBuffer<64> buf;
        buf.Write<int32>(42);
        int32 v1 = 0, v2 = 0;
        buf.Read(v1);
        CHECK(v1 == 42);
        buf.SeekRead(0);   // rewind
        buf.Read(v2);
        CHECK(v2 == 42);
    }

    TEST_CASE("Reset resets both cursors and clears size")
    {
        TInlineBuffer<64> buf;
        buf.Write<int32>(1);
        buf.Write<int32>(2);
        buf.Reset();
        CHECK(buf.IsEmpty());
        CHECK(buf.GetSize() == 0u);
        CHECK(!buf.CanRead(1));
    }
}
