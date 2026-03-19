#include "memory.hpp"

#if defined(__linux__) && !defined(__ANDROID__)
#  define HAS_SYS_MMAN_H 1
#  define MAYBE_UNUSED
#  include <sys/mman.h>
#else
#  define MAYBE_UNUSED [[maybe_unused]]
#endif

namespace Gomoku
{
    void* allocate_huge_pages(MAYBE_UNUSED const std::size_t size)
    {
#ifdef HAS_SYS_MMAN_H
        void* addr = mmap(nullptr, size,
                          PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                          -1, 0);
        if (addr != MAP_FAILED)
            return addr;
        addr = mmap(nullptr, size,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1, 0);
        if (addr != MAP_FAILED)
        {
            madvise(addr, size, MADV_HUGEPAGE);
#ifdef MADV_POPULATE_WRITE
            madvise(addr, size, MADV_POPULATE_WRITE);
#endif
            return addr;
        }
#endif
        return nullptr;
    }

    void deallocate_huge_pages(MAYBE_UNUSED void* const ptr, MAYBE_UNUSED const std::size_t size)
    {
#ifdef HAS_SYS_MMAN_H
        if (ptr != nullptr)
            munmap(ptr, size);
#endif
    }

#undef HAS_SYS_MMAN_H
#undef MAYBE_UNUSED
}
