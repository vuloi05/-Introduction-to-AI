#ifndef GOMOKU_TPTABLE_HPP
#define GOMOKU_TPTABLE_HPP

#include <atomic>
#include <cstdint>

#include "misc.hpp"
#include "types.hpp"

namespace Gomoku
{
    struct TTData
    {
        Key key : 16;
        std::size_t best_move_row : 4;
        std::size_t best_move_col : 4;
        int generation : 5;
        bool is_pv : 1;
        Bound bound : 2;
        int depth : 8;
        Eval eval : 24;

        [[nodiscard]] bool is_valid() const { return depth != 0; }
        [[nodiscard]] Coord best_move() const { return Coord{best_move_row, best_move_col}; }
    };

    union TTEntry
    {
        TTData data;
        std::uint64_t raw;

        TTEntry() : raw{0} {}
        explicit TTEntry(std::uint64_t r) : raw{r} {}
    };

    class TranspositionTable
    {
    public:
        using Entry = std::atomic<std::uint64_t>;

        TranspositionTable() = default;
        explicit TranspositionTable(std::size_t size_in_mb);

        TranspositionTable(const TranspositionTable&) = delete;
        TranspositionTable& operator=(const TranspositionTable&) = delete;

        ~TranspositionTable();

        [[nodiscard]] std::size_t size() const;

        // Probe the TT for a given key. Returns true if a valid entry was found.
        [[nodiscard]] bool probe(Key key, TTData& data) const;

        // Store an entry in the TT
        void store(Key key, Coord best_move, bool is_pv, Bound bound, int depth, Eval eval) const;

        // Increment generation counter (call at start of each search)
        void new_search();

        void resize(std::size_t size_in_mb);
        void clear();

    private:
        Entry* table_ = nullptr;
        std::size_t size_ = 0;
        int gen_ = 0;
        bool is_huge_pages_ = false;

        [[nodiscard]] std::size_t index(Key key) const;

        void allocate(std::size_t size_in_mb);
        void deallocate();
    };
}

inline std::size_t Gomoku::TranspositionTable::size() const { return size_; }

inline std::size_t Gomoku::TranspositionTable::index(const Key key) const
{
    return mul_hi64(key, size_);
}

#endif //GOMOKU_TPTABLE_HPP
