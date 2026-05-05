#ifndef GOMOKU_MISC_HPP
#define GOMOKU_MISC_HPP

#include <cstdint>
#include <limits>
#include <streambuf>
#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace Gomoku
{
    class NullBuffer final : public std::streambuf
    {
    public:
        int overflow(int c) override { return c; }
    };

    class SplitMix64Rng
    {
    public:
        using result_type = std::uint64_t;

        constexpr explicit SplitMix64Rng(const uint64_t seed = 0) : state_{seed} {}

        constexpr void seed(const uint64_t seed) { state_ = seed; }

        constexpr result_type operator()()
        {
            std::uint64_t z = state_ += 0x9E3779B97F4A7C15ULL;
            z = (z ^ z >> 30) * 0xBF584761CE4E5B9ULL;
            z = (z ^ z >> 27) * 0x94D049BB133111EBULL;
            return z ^ z >> 31;
        }

        constexpr void discard(unsigned long long n)
        {
            for (unsigned long long i = 0; i < n; ++i)
                operator()();
        }

        static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
        static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

    private:
        std::uint64_t state_;
    };

    constexpr std::uint64_t mul_hi64(std::uint64_t a, std::uint64_t b)
    {
#if defined(__SIZEOF_INT128__)
        // GCC, Clang (Linux/macOS)
        __uint128_t result = static_cast<__uint128_t>(a) * static_cast<__uint128_t>(b);
        return static_cast<std::uint64_t>(result >> 64);
#elif defined(_MSC_VER) && !defined(__clang__)
        // _umul128 is not constexpr, so this path is only valid at runtime.
        if (!std::is_constant_evaluated())
        {
            unsigned __int64 high;
            _umul128(a, b, &high);
            return high;
        }
        else
            goto ManualFallback;
#else
    ManualFallback:
#endif
        std::uint64_t a_lo = static_cast<std::uint32_t>(a);
        std::uint64_t a_hi = a >> 32;
        std::uint64_t b_lo = static_cast<std::uint32_t>(b);
        std::uint64_t b_hi = b >> 32;

        std::uint64_t lo_lo = a_lo * b_lo;
        std::uint64_t lo_hi = a_lo * b_hi;
        std::uint64_t hi_lo = a_hi * b_lo;
        std::uint64_t hi_hi = a_hi * b_hi;

        std::uint64_t cross = lo_hi + hi_lo;
        std::uint64_t carry = cross < lo_hi ? 1ULL << 32 : 0;

        std::uint64_t lo_lo_hi = lo_lo >> 32;
        cross += lo_lo_hi;

        if (cross < lo_lo_hi)
            carry += 1ULL << 32;

        return hi_hi + (cross >> 32) + carry;
    }

    template <typename T>
    constexpr std::pair<T, T> max_two(T a, T b, T c, T d)
    {
        if (a < b)
            std::swap(a, b);
        if (c < d)
            std::swap(c, d);
        if (a < c)
        {
            std::swap(a, c);
            std::swap(b, d);
        }
        return std::make_pair(a, std::max(b, c));
    }

    template <typename T>
    constexpr T one_and_a_half(T value) requires(std::is_integral_v<T>)
    {
        return value + value / 2;
    }

    constexpr std::uint64_t pdep_u64_fallback(const std::uint64_t val, std::uint64_t mask)
    {
        std::uint64_t result = 0;
        for (std::uint64_t bb = 1; mask != 0; bb += bb)
        {
            if (val & bb)
                result |= mask & -mask;  // Isolate the lowest set bit of mask
            mask &= mask - 1;            // Clear lowest set bit of mask
        }
        return result;
    }
}

#endif //GOMOKU_MISC_HPP
