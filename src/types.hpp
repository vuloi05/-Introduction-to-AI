#ifndef GOMOKU_TYPES_HPP
#define GOMOKU_TYPES_HPP

#include <cstdint>

#include "inplace_vector.hpp"

namespace Gomoku
{
    using Eval = std::int32_t;
    using Key = std::uint64_t;
    using Mask = std::uint16_t;

    constexpr std::size_t BOARD_SIZE = 16;

    enum Stone : std::int8_t
    {
        BLACK, WHITE, EMPTY,
        STONE_NB = 2
    };

    struct Coord
    {
        std::size_t row;
        std::size_t col;

        Coord() = default;
        constexpr Coord(const std::size_t row, const std::size_t col) : row{row}, col{col} {}

        bool operator==(const Coord& other) const { return row == other.row && col == other.col; }
        bool operator!=(const Coord& other) const { return !(*this == other); }
    };

    using CoordList = InplaceVector<Coord, BOARD_SIZE * BOARD_SIZE>;

    enum ThreatType : std::uint8_t
    {
        NONE = 0,

        ONE_SIMPLE = 1,
        ONE_BROKEN_SIMPLE = 2,
        ONE_BROKEN = 3,
        ONE_BROKEN_OPEN = 4,
        ONE_OPEN = 5,

        TWO_SIMPLE = 6,
        TWO_BROKEN_SIMPLE = 7,
        TWO_BROKEN_OPEN = 8,
        TWO_OPEN = 9,

        THREE_SIMPLE = 10,
        THREE_BROKEN = 11,
        THREE_OPEN = 12,

        FOUR_SIMPLE = 13,
        FOUR_OPEN = 14,

        FIVE = 15,

        THREAT_NB = 16
    };

    constexpr Eval EVAL_WIN_BASE = 1000000;
    constexpr Eval EVAL_MIN = -EVAL_WIN_BASE - 100;
    constexpr Eval EVAL_MAX = EVAL_WIN_BASE + 100;

    constexpr Eval EVAL[THREAT_NB] = {
        0,
        3, 6, 11, 19, 34,          // ONE: simple, broken_simple, broken, broken_open, open
        61, 110, 198, 357,         // TWO: simple, broken_simple, broken_open, open
        643, 1157, 2082,           // THREE: simple, broken, open
        3748, 6747,                // FOUR: simple, open
        12144                      // FIVE
    };

    enum Bound : std::int8_t
    {
        BOUND_NONE,
        BOUND_UPPER,
        BOUND_LOWER,
        BOUND_EXACT = BOUND_UPPER | BOUND_LOWER
    };

    constexpr Stone operator~(const Stone s) { return static_cast<Stone>(s ^ 1); }

    constexpr Coord COORD_NULL(-1, -1);
}

#endif //GOMOKU_TYPES_HPP
