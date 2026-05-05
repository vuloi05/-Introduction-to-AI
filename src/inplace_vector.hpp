#ifndef GOMOKU_INPLACE_VECTOR_HPP
#define GOMOKU_INPLACE_VECTOR_HPP

#include "ryehl/inplace_vector.hpp"

namespace Gomoku
{
    template <typename T, std::size_t N>
    using InplaceVector = ryehl::inplace_vector<T, N>;
}

#endif //GOMOKU_INPLACE_VECTOR_HPP
