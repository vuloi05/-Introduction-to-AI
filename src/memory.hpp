#ifndef GOMOKU_MEMORY_HPP
#define GOMOKU_MEMORY_HPP
#include <cstddef>

namespace Gomoku
{
    void *allocate_huge_pages(std::size_t size);
    void deallocate_huge_pages(void *ptr, std::size_t size);
}

#endif //GOMOKU_MEMORY_HPP