#include <doctest/doctest.h>
#include <Phoenix/MortonCode.h>

using namespace Phoenix;

// Helper: returns true if 'code' falls within any range in the array.
static bool CodeInRanges(uint64 code, const TMortonCodeRangeArray& ranges)
{
    for (auto [lo, hi] : ranges)
    {
        if (code >= lo && code <= hi)
            return true;
    }
    return false;
}

TEST_SUITE("MortonCode")
{
    // -------------------------------------------------------------------------
    // ExpandBits / CollapseBits — bit-level round-trip
    // -------------------------------------------------------------------------

    TEST_CASE("ExpandBits zero is zero")
    {
        CHECK(ExpandBits(0u) == 0u);
    }

    TEST_CASE("ExpandBits spreads individual bits to even positions")
    {
        // Bit 0 of input → bit 0 of output
        CHECK(ExpandBits(1u) == 1u);
        // Bit 1 of input → bit 2 of output
        CHECK(ExpandBits(2u) == 4u);
        // Bit 2 of input → bit 4 of output
        CHECK(ExpandBits(4u) == 16u);
        // Both bits 0 and 1 set → bits 0 and 2 set
        CHECK(ExpandBits(3u) == 5u);
    }

    TEST_CASE("CollapseBits is the inverse of ExpandBits")
    {
        for (uint32 x : {0u, 1u, 2u, 3u, 7u, 15u, 255u, 0xFFFFu, 0xFFFFFFFFu})
        {
            CHECK(CollapseBits(ExpandBits(x)) == x);
        }
    }

    TEST_CASE("CollapseBits ignores odd-position bits")
    {
        // ExpandBits only sets even-position bits; odd-position bits are noise.
        // Adding 0xAAAA... (all odd bits set) should not change Collapse(Expand(x)).
        uint64 noise = 0xAAAAAAAAAAAAAAAAULL;
        CHECK(CollapseBits(ExpandBits(42u) | noise) == 42u);
    }

    // -------------------------------------------------------------------------
    // ToMortonCode / FromMortonCode — unsigned round-trip (lshift = 0)
    // -------------------------------------------------------------------------

    TEST_CASE("ToMortonCode unsigned origin is zero")
    {
        CHECK(ToMortonCode(0u, 0u, 0) == 0u);
    }

    TEST_CASE("ToMortonCode unsigned interleaves x into odd bits and y into even bits")
    {
        // x=1 (bit 0) → result bit 1; y=0 → result bit 0 = 0
        CHECK(ToMortonCode(1u, 0u, 0) == 2u);
        // x=0; y=1 (bit 0) → result bit 0
        CHECK(ToMortonCode(0u, 1u, 0) == 1u);
        // x=1, y=1 → bits 1 and 0 both set
        CHECK(ToMortonCode(1u, 1u, 0) == 3u);
    }

    TEST_CASE("FromMortonCode unsigned round-trip")
    {
        for (uint32 x : {0u, 1u, 3u, 7u, 255u, 1023u})
        {
            for (uint32 y : {0u, 1u, 2u, 5u, 100u, 500u})
            {
                uint64 code = ToMortonCode(x, y, 0);
                uint32 rx = 0, ry = 0;
                FromMortonCode(code, rx, ry, 0);
                CHECK(rx == x);
                CHECK(ry == y);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Signed coordinates — quadrant encoding
    // -------------------------------------------------------------------------

    TEST_CASE("ToMortonCode signed positive quadrant has quad bits = 0")
    {
        uint64 code = ToMortonCode(3, 5, 0);
        CHECK(GetMortonCodeQuad(code) == 0);
    }

    TEST_CASE("ToMortonCode signed negative-x has quad bit 0 set")
    {
        uint64 code = ToMortonCode(-1, 2, 0);
        CHECK(GetMortonCodeQuad(code) == 1);
    }

    TEST_CASE("ToMortonCode signed negative-y has quad bit 1 set")
    {
        uint64 code = ToMortonCode(4, -3, 0);
        CHECK(GetMortonCodeQuad(code) == 2);
    }

    TEST_CASE("ToMortonCode signed both-negative has quad bits 0 and 1 set")
    {
        uint64 code = ToMortonCode(-7, -8, 0);
        CHECK(GetMortonCodeQuad(code) == 3);
    }

    TEST_CASE("FromMortonCode signed round-trip all quadrants")
    {
        const int32 coords[] = {-100, -1, 0, 1, 100};

        for (int32 x : coords)
        {
            for (int32 y : coords)
            {
                uint64 code = ToMortonCode(x, y, 0);
                int32 rx = 0, ry = 0;
                FromMortonCode(code, rx, ry, 0);
                CHECK(rx == x);
                CHECK(ry == y);
            }
        }
    }

    TEST_CASE("Signed codes from different quadrants are in separate value ranges")
    {
        // Quadrant is stored in the top 3 bits, so all codes in quad 0 are less
        // than all codes in quad 1 (when the value parts are equal).
        uint64 q0 = ToMortonCode(1, 1, 0);   // quad 0
        uint64 q1 = ToMortonCode(-1, 1, 0);  // quad 1
        uint64 q2 = ToMortonCode(1, -1, 0);  // quad 2
        uint64 q3 = ToMortonCode(-1, -1, 0); // quad 3

        CHECK(GetMortonCodeQuad(q0) < GetMortonCodeQuad(q1));
        CHECK(GetMortonCodeQuad(q1) < GetMortonCodeQuad(q2));
        CHECK(GetMortonCodeQuad(q2) < GetMortonCodeQuad(q3));
    }

    // -------------------------------------------------------------------------
    // ScaleToMortonCode
    // -------------------------------------------------------------------------

    TEST_CASE("ScaleToMortonCode zero is zero")
    {
        CHECK(ScaleToMortonCode(0) == 0);
    }

    TEST_CASE("ScaleToMortonCode positive values shift right by MortonCodeGridBits")
    {
        int32 shift = MortonCodeGridBits;
        CHECK(ScaleToMortonCode(1 << shift) == 1);
        CHECK(ScaleToMortonCode(4 << shift) == 4);
    }

    TEST_CASE("ScaleToMortonCode negative values preserve sign")
    {
        int32 shift = MortonCodeGridBits;
        // Need |val| - 1 >= (1 << shift) so the shifted result is non-zero.
        // -(1 << shift) gives xu = (1<<shift)-1, which shifts to 0.
        // -(4 << shift) gives xu = (4<<shift)-1, which shifts to a positive value.
        int32 val = -(4 << shift);
        CHECK(ScaleToMortonCode(val) < 0);
    }

    // -------------------------------------------------------------------------
    // GetMortonCodeValue
    // -------------------------------------------------------------------------

    TEST_CASE("GetMortonCodeValue strips quad bits")
    {
        uint64 code = ToMortonCode(-3, 5, 0);
        uint64 value = GetMortonCodeValue(code);
        // Top 3 bits must be clear.
        CHECK((value >> 61) == 0u);
    }

    // -------------------------------------------------------------------------
    // MortonCodeQuery
    // -------------------------------------------------------------------------

    TEST_CASE("MortonCodeQuery single point returns range containing that code")
    {
        // A single-cell AABB at the positive origin in Morton grid space.
        MortonCodeAABB query{0, 0, 0, 0};
        TMortonCodeRangeArray ranges;
        MortonCodeQuery(query, ranges);

        CHECK_FALSE(ranges.empty());
        uint64 code = ToMortonCode(0, 0, 0);
        CHECK(CodeInRanges(code, ranges));
    }

    TEST_CASE("MortonCodeQuery 2x2 region covers all four corner codes")
    {
        // AABB covering (0,0)-(1,1) in Morton grid space (positive quadrant).
        MortonCodeAABB query{0, 0, 1, 1};
        TMortonCodeRangeArray ranges;
        MortonCodeQuery(query, ranges);

        CHECK(CodeInRanges(ToMortonCode(0, 0, 0), ranges));
        CHECK(CodeInRanges(ToMortonCode(1, 0, 0), ranges));
        CHECK(CodeInRanges(ToMortonCode(0, 1, 0), ranges));
        CHECK(CodeInRanges(ToMortonCode(1, 1, 0), ranges));
    }

    TEST_CASE("MortonCodeQuery 2x2 region excludes adjacent cells just outside")
    {
        MortonCodeAABB query{0, 0, 1, 1};
        TMortonCodeRangeArray ranges;
        MortonCodeQuery(query, ranges);

        CHECK_FALSE(CodeInRanges(ToMortonCode(2, 0, 0), ranges));
        CHECK_FALSE(CodeInRanges(ToMortonCode(0, 2, 0), ranges));
        CHECK_FALSE(CodeInRanges(ToMortonCode(2, 2, 0), ranges));
    }

    TEST_CASE("MortonCodeQuery empty ranges for degenerate gridBits=0")
    {
        MortonCodeAABB query{0, 0, 1, 1};
        TMortonCodeRangeArray ranges;
        MortonCodeQuery(query, ranges, 0);  // invalid gridBits → returns immediately
        CHECK(ranges.empty());
    }

    TEST_CASE("MortonCodeQuery fully-inside cell produces single range")
    {
        // An AABB aligned to a power-of-two cell boundary triggers the
        // "fully inside" fast path and must return exactly one range.
        MortonCodeAABB query{0, 0, 3, 3};
        TMortonCodeRangeArray ranges;
        MortonCodeQuery(query, ranges);

        CHECK(ranges.size() == 1);

        // Verify all 16 cells inside are covered.
        for (int32 x = 0; x <= 3; ++x)
        {
            for (int32 y = 0; y <= 3; ++y)
            {
                CHECK(CodeInRanges(ToMortonCode(x, y, 0), ranges));
            }
        }
    }
}
