#include "tptable.hpp"

#include <cstring>

#include "memory.hpp"

namespace Gomoku
{
    TranspositionTable::TranspositionTable(std::size_t size_in_mb) { allocate(size_in_mb); }

    TranspositionTable::~TranspositionTable() { deallocate(); }

    bool TranspositionTable::probe(const Key key, TTData& data) const
    {
        if (table_ == nullptr || size_ == 0)
            return false;

        const std::size_t idx = index(key);
        const TTEntry entry{table_[idx].load(std::memory_order_relaxed)};

        // Check if entry is valid (depth != 0) and key matches (lower 16 bits)
        if (!entry.data.is_valid())
            return false;

        if (entry.data.key != (key & 0xFFFF))
            return false;

        data = entry.data;
        return true;
    }

    void TranspositionTable::store(const Key key, const Coord best_move, const bool is_pv,
                                   const Bound bound, const int depth, const Eval eval) const
    {
        if (table_ == nullptr || size_ == 0)
            return;

        const std::size_t idx = index(key);
        const TTEntry old_entry{table_[idx].load(std::memory_order_relaxed)};
        TTEntry new_entry;

        // Replacement scheme: always replace if
        // 1. Entry is empty (depth == 0)
        // 2. Old entry is from a previous generation
        // 3. New depth >= old depth
        // 4. New entry is exact bound and old is not.
        const bool should_replace = !old_entry.data.is_valid() ||
            (old_entry.data.generation != gen_) ||
            depth >= old_entry.data.depth ||
            (bound == BOUND_EXACT && old_entry.data.bound != BOUND_EXACT);

        if (!should_replace)
            return;

        new_entry.data.key = key & 0xFFFF;
        new_entry.data.best_move_row = best_move.row;
        new_entry.data.best_move_col = best_move.col;
        new_entry.data.generation = gen_;
        new_entry.data.is_pv = is_pv;
        new_entry.data.bound = bound;
        new_entry.data.depth = depth;
        new_entry.data.eval = eval;

        table_[idx].store(new_entry.raw, std::memory_order_relaxed);
    }

    void TranspositionTable::new_search()
    {
        gen_ = (gen_ + 1) & 0x1F; // 5-bit generation, wrap around
    }

    void TranspositionTable::resize(const std::size_t size_in_mb)
    {
        if (size_in_mb == size_)
            return;
        deallocate();
        clear();
        size_ = size_in_mb;
        if (size_ != 0)
            allocate(size_);
    }

    void TranspositionTable::clear()
    {
        if (table_ != nullptr)
            std::fill_n(table_, size_, 0);
        gen_ = 0;
    }

    void TranspositionTable::allocate(const std::size_t size_in_mb)
    {
        size_ = size_in_mb * 1024 * 1024 / sizeof(Entry);
        if (void* addr = allocate_huge_pages(size_ * sizeof(Entry)); addr != nullptr)
        {
            table_ = static_cast<Entry*>(addr);
            is_huge_pages_ = true;
            return;
        }
        // Fallback to regular allocation
        table_ = new Entry[size_];
        is_huge_pages_ = false;
    }

    void TranspositionTable::deallocate()
    {
        if (table_ != nullptr)
        {
            if (is_huge_pages_)
                deallocate_huge_pages(table_, size_ * sizeof(Entry));
            else
                delete[] table_;
            table_ = nullptr;
        }
        size_ = 0;
        is_huge_pages_ = false;
    }
}
