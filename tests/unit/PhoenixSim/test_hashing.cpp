#include <doctest/doctest.h>
#include <PhoenixSim/Hashing.h>

using namespace Phoenix;

TEST_SUITE("Hashing")
{
    // FNV-1a spec: hashing an empty byte sequence returns the basis constant.
    TEST_CASE("FNV1A32 empty input returns basis")
    {
        CHECK(Hashing::FNV1A32("", 0) == Hashing::basis32);
    }

    TEST_CASE("FNV1A64 empty input returns basis")
    {
        CHECK(Hashing::FNV1A64("", 0) == Hashing::basis64);
    }

    // Same input must always produce the same output (determinism).
    TEST_CASE("FNV1A32 is deterministic")
    {
        CHECK(Hashing::FNV1A32("phoenix", 7) == Hashing::FNV1A32("phoenix", 7));
    }

    TEST_CASE("FNV1A64 is deterministic")
    {
        CHECK(Hashing::FNV1A64("phoenix", 7) == Hashing::FNV1A64("phoenix", 7));
    }

    // Different inputs must produce different hashes (no trivial collision).
    TEST_CASE("FNV1A32 different inputs produce different hashes")
    {
        CHECK(Hashing::FNV1A32("hello", 5) != Hashing::FNV1A32("world", 5));
        CHECK(Hashing::FNV1A32("a", 1)     != Hashing::FNV1A32("b", 1));
        CHECK(Hashing::FNV1A32("ab", 2)    != Hashing::FNV1A32("ba", 2));
    }

    TEST_CASE("FNV1A64 different inputs produce different hashes")
    {
        CHECK(Hashing::FNV1A64("hello", 5) != Hashing::FNV1A64("world", 5));
        CHECK(Hashing::FNV1A64("ab", 2)    != Hashing::FNV1A64("ba", 2));
    }

    // The 32-bit and 64-bit variants must not collide with each other
    // (they use different basis/prime values).
    TEST_CASE("FNV1A32 and FNV1A64 produce different values for the same input")
    {
        hash32_t h32 = Hashing::FNV1A32("test", 4);
        hash64_t h64 = Hashing::FNV1A64("test", 4);
        CHECK(static_cast<hash64_t>(h32) != h64);
    }

    // FNV1A incremental property: hashing "ab" must equal hashing "a" then appending "b".
    TEST_CASE("FNV1A32 Append chains correctly")
    {
        hash32_t full = Hashing::FNV1A32("ab", 2);
        hash32_t step = Hashing::FNV1A32Append(Hashing::FNV1A32("a", 1), "b", 1);
        CHECK(full == step);
    }

    TEST_CASE("FNV1A64 Append chains correctly")
    {
        hash64_t full = Hashing::FNV1A64("ab", 2);
        hash64_t step = Hashing::FNV1A64Append(Hashing::FNV1A64("a", 1), "b", 1);
        CHECK(full == step);
    }

    // Appending a single character must match appending a 1-byte string.
    TEST_CASE("FNV1A32 Append single char matches Append 1-byte string")
    {
        hash32_t basis = Hashing::FNV1A32("x", 1);
        CHECK(Hashing::FNV1A32Append(basis, 'y') == Hashing::FNV1A32Append(basis, "y", 1));
    }

    TEST_CASE("FNV1A64 Append single char matches Append 1-byte string")
    {
        hash64_t basis = Hashing::FNV1A64("x", 1);
        CHECK(Hashing::FNV1A64Append(basis, 'y') == Hashing::FNV1A64Append(basis, "y", 1));
    }

    // Multi-step appending across three segments must equal a single full hash.
    TEST_CASE("FNV1A32 Append of three segments equals full hash")
    {
        hash32_t full = Hashing::FNV1A32("abc", 3);
        hash32_t step = Hashing::FNV1A32Append(Hashing::FNV1A32Append(Hashing::FNV1A32("a", 1), "b", 1), "c", 1);
        CHECK(full == step);
    }

    // Combine(basis, "x") must equal FNV1A32("x") applied with basis.
    TEST_CASE("FNV1A32 Combine with single literal matches direct hash with same basis")
    {
        hash32_t combined = Hashing::FNV1A32Combine(Hashing::basis32, "hello");
        hash32_t direct   = Hashing::FNV1A32("hello");
        CHECK(combined == direct);
    }

    // Compile-time (constexpr) evaluation must produce the same value as runtime.
    TEST_CASE("FNV1A32 constexpr matches runtime")
    {
        constexpr hash32_t compile_time = Hashing::FNV1A32("constexpr_test", 14);
        hash32_t           run_time     = Hashing::FNV1A32("constexpr_test", 14);
        CHECK(compile_time == run_time);
    }

    TEST_CASE("FNV1A64 constexpr matches runtime")
    {
        constexpr hash64_t compile_time = Hashing::FNV1A64("constexpr_test", 14);
        hash64_t           run_time     = Hashing::FNV1A64("constexpr_test", 14);
        CHECK(compile_time == run_time);
    }

    // Array template overload must match the explicit-length pointer overload.
    TEST_CASE("FNV1A32 array template overload matches pointer overload")
    {
        constexpr const char literal[] = "array_test";
        CHECK(Hashing::FNV1A32(literal) == Hashing::FNV1A32(literal, sizeof(literal) - 1));
    }

    TEST_CASE("FNV1A64 array template overload matches pointer overload")
    {
        constexpr const char literal[] = "array_test";
        CHECK(Hashing::FNV1A64(literal) == Hashing::FNV1A64(literal, sizeof(literal) - 1));
    }
}
