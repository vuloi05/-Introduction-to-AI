#include "zobrist.hpp"

#include <array>

#include "misc.hpp"
#include "types.hpp"

namespace Gomoku
{
    constexpr auto ZOBRIST_PIECE_COORDS = []() consteval
    {
        constexpr std::uint64_t seed = 42;
        std::array<Key, STONE_NB * BOARD_SIZE * BOARD_SIZE> table{};
        SplitMix64Rng rng{seed};

        for (auto& i : table)
            i = rng();

        return table;
    }();

    Key get_zobrist_key(const Stone stone, const Coord& coord)
    {
        return ZOBRIST_PIECE_COORDS[
            static_cast<std::size_t>(stone) * BOARD_SIZE * BOARD_SIZE +
            static_cast<std::size_t>(coord.row) * BOARD_SIZE +
            static_cast<std::size_t>(coord.col)];
    }
}
