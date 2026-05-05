#ifndef GOMOKU_ZOBRIST_HPP
#define GOMOKU_ZOBRIST_HPP

#include "types.hpp"

namespace Gomoku
{
    Key get_zobrist_key(Stone stone, const Coord& coord);
}

#endif //GOMOKU_ZOBRIST_HPP