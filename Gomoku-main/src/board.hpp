#ifndef GOMOKU_BOARD_HPP
#define GOMOKU_BOARD_HPP

#include <algorithm>
#include <type_traits>
#include <vector>

#include "inplace_vector.hpp"
#include "types.hpp"

namespace Gomoku
{
    template <typename T, bool IV>
    using Container = std::conditional_t<IV, InplaceVector<T, BOARD_SIZE * BOARD_SIZE>, std::vector<T>>;

    struct PlayRange
    {
        std::size_t min_c, max_c, min_r, max_r;

        void add_coord(const Coord& c)
        {
            constexpr std::size_t RANGE = 2;
            if (c.col < min_c + RANGE)
                min_c = c.col > RANGE - 1 ? c.col - RANGE : 0;
            if (c.col + RANGE > max_c)
                max_c = std::min(c.col + RANGE, BOARD_SIZE - 1);
            if (c.row < min_r + RANGE)
                min_r = c.row > RANGE - 1 ? c.row - RANGE : 0;
            if (c.row + RANGE > max_r)
                max_r = std::min(c.row + RANGE, BOARD_SIZE - 1);
        }

        static PlayRange min()
        {
            return PlayRange{BOARD_SIZE - 1, 0, BOARD_SIZE - 1, 0};
        }
    };

    struct State
    {
        Coord c;
        PlayRange pr;

        State() : c{}, pr{PlayRange::min()} {}
        State(const Coord& c, const PlayRange& pr) : c{c}, pr{pr} {}
    };

    struct FullBoardPolicy
    {
        explicit FullBoardPolicy(const PlayRange&) {}
        static constexpr std::size_t row_begin() { return 0; }
        static constexpr std::size_t row_end() { return BOARD_SIZE; }
        static constexpr Mask get_row_mask() { return 0xFFFF; }
    };

    struct PlayRangePolicy
    {
        explicit PlayRangePolicy(const PlayRange& pr) :
            min_r_{pr.min_r}, max_r{pr.max_r},
            row_mask_{static_cast<Mask>((0xFFFFULL >> (BOARD_SIZE - (pr.max_c - pr.min_c + 1))) << pr.min_c)} {}

        [[nodiscard]] std::size_t row_begin() const { return min_r_; }
        [[nodiscard]] std::size_t row_end() const { return max_r + 1; }
        [[nodiscard]] Mask get_row_mask() const { return row_mask_; }

    private:
        std::size_t min_r_, max_r;
        Mask row_mask_;
    };

    struct EndTag {};

    class Board
    {
    public:
        template <typename Policy>
        class EmptyCellsIterator;

        template <typename Policy>
        struct EmptyCellsView
        {
            const Board& board;
            [[nodiscard]] EmptyCellsIterator<Policy> begin() const;
            [[nodiscard]] EmptyCellsIterator<Policy> end() const;
        };

        Board() : rows_{}, cols_{}, diag_{}, adiag_{}, trows_{}, tcols_{}, tdiag_{}, tadiag_{},
                  row_fives_{}, col_fives_{}, diag_fives_{}, adiag_fives_{},
                  row_four_opens_{}, col_four_opens_{}, diag_four_opens_{}, adiag_four_opens_{},
                  row_four_simples_{}, col_four_simples_{}, diag_four_simples_{}, adiag_four_simples_{},
                  evals_{}, history_{}, key_{0}, stm_{BLACK}, has_winner_{false} {}

        Board(const Board&) = default;
        Board(Board&&) = default;
        Board& operator=(const Board&) = default;
        Board& operator=(Board&&) = default;

        ~Board() = default;

        [[nodiscard]] Stone stone_at(const Coord& c) const;
        [[nodiscard]] Stone stone_at_safe(const Coord& c) const;
        [[nodiscard]] Stone side_to_move() const;
        [[nodiscard]] Stone winner() const;
        [[nodiscard]] bool has_winner() const;
        [[nodiscard]] Eval evaluate() const;
        [[nodiscard]] Eval evaluate_at(Stone s, const Coord& c) const;
        [[nodiscard]] Key zobrist_key() const;
        [[nodiscard]] PlayRange play_range() const;
        [[nodiscard]] CoordList empty_cells() const;
        [[nodiscard]] EmptyCellsView<FullBoardPolicy> empty_cells_view() const;
        [[nodiscard]] EmptyCellsView<PlayRangePolicy> empty_cells_in_play_range_view() const;
        template <bool IV>
        [[nodiscard]] Container<Coord, IV> empty_cells_in_play_range() const;
        template <bool IV>
        [[nodiscard]] Container<Coord, IV> empty_cells_with_move_ordering() const;

        [[nodiscard]] ThreatType max_threat_at(Stone s, const Coord& c) const;

        template <bool IV>
        [[nodiscard]] Container<Coord, IV> get_five_moves(Stone s) const;
        template <bool IV>
        [[nodiscard]] Container<Coord, IV> get_open_four_moves(Stone s) const;
        template <bool IV>
        [[nodiscard]] Container<Coord, IV> get_four_defense_moves(Stone s) const;
        template <bool IV>
        [[nodiscard]] Container<Coord, IV> get_forcing_moves(Stone s) const;

        void do_move(const Coord& c);
        void undo_move();

        void clear();

        static void init();

    private:
        Mask rows_[STONE_NB][BOARD_SIZE];
        Mask cols_[STONE_NB][BOARD_SIZE];
        Mask diag_[STONE_NB][2 * BOARD_SIZE - 1];
        Mask adiag_[STONE_NB][2 * BOARD_SIZE - 1];

        ThreatType trows_[STONE_NB][BOARD_SIZE][BOARD_SIZE + 8];
        ThreatType tcols_[STONE_NB][BOARD_SIZE][BOARD_SIZE + 8];
        ThreatType tdiag_[STONE_NB][2 * BOARD_SIZE - 1][BOARD_SIZE + 8];
        ThreatType tadiag_[STONE_NB][2 * BOARD_SIZE - 1][BOARD_SIZE + 8];

        Mask row_fives_[STONE_NB][BOARD_SIZE];
        Mask col_fives_[STONE_NB][BOARD_SIZE];
        Mask diag_fives_[STONE_NB][2 * BOARD_SIZE - 1];
        Mask adiag_fives_[STONE_NB][2 * BOARD_SIZE - 1];

        Mask row_four_opens_[STONE_NB][BOARD_SIZE];
        Mask col_four_opens_[STONE_NB][BOARD_SIZE];
        Mask diag_four_opens_[STONE_NB][2 * BOARD_SIZE - 1];
        Mask adiag_four_opens_[STONE_NB][2 * BOARD_SIZE - 1];

        Mask row_four_simples_[STONE_NB][BOARD_SIZE];
        Mask col_four_simples_[STONE_NB][BOARD_SIZE];
        Mask diag_four_simples_[STONE_NB][2 * BOARD_SIZE - 1];
        Mask adiag_four_simples_[STONE_NB][2 * BOARD_SIZE - 1];

        Eval evals_[STONE_NB];

        InplaceVector<State, BOARD_SIZE * BOARD_SIZE> history_;

        Key key_;
        Stone stm_;
        bool has_winner_;

        void update_threats(std::size_t row, std::size_t col, std::size_t drow,
                            std::size_t dlen, std::size_t adrow, std::size_t adlen);
        void update_evaluation(const Coord& c, bool subtract);
        void update_winner_after_do_move(const Coord& c);

        template <typename Policy>
        friend class EmptyCellsIterator;
    };

    template <typename Policy>
    class Board::EmptyCellsIterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = Coord;
        using difference_type = std::ptrdiff_t;
        using pointer = const Coord*;
        using reference = const Coord&;

        EmptyCellsIterator() : board_{nullptr}, row_{0}, bits_{0}, policy_{} {}

        EmptyCellsIterator(const Board& board, Policy policy) :
            board_{&board}, row_{policy.row_begin()}, bits_{0}, policy_{policy}
        {
            prepare_row();
            while (bits_ == 0 && row_ < policy_.row_end())
            {
                ++row_;
                prepare_row();
            }
        }

        EmptyCellsIterator(const Board& board, Policy policy, EndTag) :
            board_{&board}, row_{policy.row_end()}, bits_{0}, policy_{policy} {}

        [[nodiscard]] value_type operator*() const
        {
            const std::size_t col = std::countr_zero(bits_);
            return Coord{row_, col};
        }

        EmptyCellsIterator& operator++()
        {
            bits_ &= bits_ - 1;
            while (bits_ == 0)
            {
                ++row_;
                if (row_ >= BOARD_SIZE)
                    break;
                prepare_row();
            }
            return *this;
        }

        EmptyCellsIterator operator++(int)
        {
            EmptyCellsIterator temp = *this;
            ++(*this);
            return temp;
        }

        [[nodiscard]] bool operator==(const EmptyCellsIterator& other) const { return bits_ == other.bits_; }

    private:
        const Board* board_;
        std::size_t row_;
        Mask bits_;
        Policy policy_;

        void prepare_row()
        {
            const Mask occupied = board_->rows_[BLACK][row_] | board_->rows_[WHITE][row_];
            bits_ = ~occupied & policy_.get_row_mask();
        }
    };


    inline Stone Board::side_to_move() const { return stm_; }

    inline Stone Board::winner() const { return has_winner() ? ~side_to_move() : EMPTY; }

    inline bool Board::has_winner() const { return has_winner_; }

    inline Key Board::zobrist_key() const { return key_; }

    inline PlayRange Board::play_range() const
    {
        if (history_.empty())
            return PlayRange{BOARD_SIZE / 2, BOARD_SIZE / 2, BOARD_SIZE / 2, BOARD_SIZE / 2};
        return history_.back().pr;
    }

    inline CoordList Board::empty_cells() const
    {
        const auto view = empty_cells_view();
        return {view.begin(), view.end()};
    }

    inline Board::EmptyCellsView<FullBoardPolicy> Board::empty_cells_view() const
    {
        return EmptyCellsView<FullBoardPolicy>{*this};
    }

    inline Board::EmptyCellsView<PlayRangePolicy> Board::empty_cells_in_play_range_view() const
    {
        return EmptyCellsView<PlayRangePolicy>{*this};
    }

    std::ostream& operator<<(std::ostream& os, const Board& board);

    template <typename Policy>
    Board::EmptyCellsIterator<Policy> Board::EmptyCellsView<Policy>::begin() const
    {
        return EmptyCellsIterator<Policy>{board, Policy{board.play_range()}};
    }

    template <typename Policy>
    Board::EmptyCellsIterator<Policy> Board::EmptyCellsView<Policy>::end() const
    {
        return EmptyCellsIterator<Policy>{board, Policy{board.play_range()}, EndTag{}};
    }

    template <bool IV>
    Container<Coord, IV> Board::empty_cells_in_play_range() const
    {
        const auto view = empty_cells_in_play_range_view();
        return {view.begin(), view.end()};
    }

    template <bool IV>
    Container<Coord, IV> Board::empty_cells_with_move_ordering() const
    {
        struct ScoredCoord
        {
            Coord coord;
            Eval score;

            ScoredCoord() = default;
            ScoredCoord(const Coord& c, const Eval s) : coord{c}, score{s} {}

            bool operator<(const ScoredCoord& other) const { return score > other.score; }
        };

        if (history_.empty())
            return {Coord{BOARD_SIZE / 2, BOARD_SIZE / 2}};

        bool is_neighbor[BOARD_SIZE][BOARD_SIZE] = {};

        for (const auto& state : history_)
        {
            const auto [row, col] = state.c;
            const std::size_t r_min = row > 2 ? row - 2 : 0;
            const std::size_t r_max = row + 2 < BOARD_SIZE ? row + 2 : BOARD_SIZE - 1;
            const std::size_t c_min = col > 2 ? col - 2 : 0;
            const std::size_t c_max = col + 2 < BOARD_SIZE ? col + 2 : BOARD_SIZE - 1;

            for (std::size_t r = r_min; r <= r_max; ++r)
                for (std::size_t c = c_min; c <= c_max; ++c)
                    is_neighbor[r][c] = true;
        }

        Container<ScoredCoord, IV> scored_list;

        for (std::size_t r = 0; r < BOARD_SIZE; ++r)
        {
            const Mask occupied = rows_[BLACK][r] | rows_[WHITE][r];
            for (std::size_t c = 0; c < BOARD_SIZE; ++c)
            {
                if (!is_neighbor[r][c])
                    continue;
                if (occupied >> c & 1)
                    continue;

                const Coord coord{r, c};
                const Eval offense = evaluate_at(stm_, coord);
                const Eval defense = evaluate_at(~stm_, coord);
                scored_list.emplace_back(coord, offense + defense + defense / 5);
            }
        }

        std::sort(scored_list.begin(), scored_list.end());

        Container<Coord, IV> result;
        for (const auto& sc : scored_list)
            result.push_back(sc.coord);
        return result;
    }

    // =========================================================================
    // Helper functions for diagonal coordinate conversion
    // =========================================================================

    namespace detail
    {
        inline Coord coord_from_diag(const std::size_t d, const std::size_t i)
        {
            return d < BOARD_SIZE ? Coord{i, i + BOARD_SIZE - 1 - d} : Coord{i + d - BOARD_SIZE + 1, i};
        }

        inline Coord coord_from_adiag(const std::size_t d, const std::size_t i)
        {
            if (d < BOARD_SIZE)
                return Coord{i, d - i};
            const std::size_t c = BOARD_SIZE - 1 - i;
            return Coord{d - c, c};
        }
    }

    // =========================================================================
    // Template implementations for threat move generation
    // =========================================================================

    template <bool IV>
    Container<Coord, IV> Board::get_five_moves(const Stone s) const
    {
        Container<Coord, IV> result;
        Mask added[BOARD_SIZE] = {};

        auto try_add = [&](const Coord& coord)
        {
            const auto [r, c] = coord;
            if (const Mask bit = static_cast<Mask>(1) << c; !(added[r] & bit))
            {
                added[r] |= bit;
                result.push_back(coord);
            }
        };

        auto scan = [&]<typename GetMask, typename ToCoord>(
            const std::size_t limit, GetMask&& get_mask, ToCoord&& to_coord)
        {
            for (std::size_t idx = 0; idx < limit; ++idx)
            {
                for (Mask m = get_mask(idx); m; m &= m - 1)
                    try_add(to_coord(idx, std::countr_zero(m)));
            }
        };

        scan(BOARD_SIZE,
             [&](std::size_t r) { return row_fives_[s][r] & ~(rows_[BLACK][r] | rows_[WHITE][r]); },
             [](std::size_t r, std::size_t c) { return Coord{r, c}; });

        scan(BOARD_SIZE,
             [&](std::size_t c) { return col_fives_[s][c] & ~(cols_[BLACK][c] | cols_[WHITE][c]); },
             [](std::size_t c, std::size_t r) { return Coord{r, c}; });

        scan(2 * BOARD_SIZE - 1,
             [&](std::size_t d) { return diag_fives_[s][d] & ~(diag_[BLACK][d] | diag_[WHITE][d]); },
             [](std::size_t d, std::size_t i) { return detail::coord_from_diag(d, i); });

        scan(2 * BOARD_SIZE - 1,
             [&](std::size_t d) { return adiag_fives_[s][d] & ~(adiag_[BLACK][d] | adiag_[WHITE][d]); },
             [](std::size_t d, std::size_t i) { return detail::coord_from_adiag(d, i); });

        return result;
    }

    template <bool IV>
    Container<Coord, IV> Board::get_open_four_moves(const Stone s) const
    {
        Container<Coord, IV> result;
        Mask added[BOARD_SIZE] = {};

        auto try_add = [&](const Coord& coord)
        {
            const auto [r, c] = coord;
            if (const Mask bit = static_cast<Mask>(1) << c; !(added[r] & bit))
            {
                added[r] |= bit;
                result.push_back(coord);
            }
        };

        auto scan = [&]<typename GetMask, typename ToCoord>(
            const std::size_t limit, GetMask&& get_mask, ToCoord&& to_coord)
        {
            for (std::size_t idx = 0; idx < limit; ++idx)
            {
                for (Mask m = get_mask(idx); m; m &= m - 1)
                    try_add(to_coord(idx, std::countr_zero(m)));
            }
        };

        scan(BOARD_SIZE,
             [&](std::size_t r) { return row_four_opens_[s][r] & ~(rows_[BLACK][r] | rows_[WHITE][r]); },
             [](std::size_t r, std::size_t c) { return Coord{r, c}; });

        scan(BOARD_SIZE,
             [&](std::size_t c) { return col_four_opens_[s][c] & ~(cols_[BLACK][c] | cols_[WHITE][c]); },
             [](std::size_t c, std::size_t r) { return Coord{r, c}; });

        scan(2 * BOARD_SIZE - 1,
             [&](std::size_t d) { return diag_four_opens_[s][d] & ~(diag_[BLACK][d] | diag_[WHITE][d]); },
             [](std::size_t d, std::size_t i) { return detail::coord_from_diag(d, i); });

        scan(2 * BOARD_SIZE - 1,
             [&](std::size_t d) { return adiag_four_opens_[s][d] & ~(adiag_[BLACK][d] | adiag_[WHITE][d]); },
             [](std::size_t d, std::size_t i) { return detail::coord_from_adiag(d, i); });

        return result;
    }

    template <bool IV>
    Container<Coord, IV> Board::get_four_defense_moves(const Stone s) const
    {
        if (auto five_moves = get_five_moves<IV>(s); !five_moves.empty())
            return five_moves;
        return get_open_four_moves<IV>(s);
    }

    template <bool IV>
    Container<Coord, IV> Board::get_forcing_moves(const Stone s) const
    {
        Container<Coord, IV> result;
        Mask added[BOARD_SIZE] = {};

        auto try_add = [&](const Coord& coord)
        {
            const auto [r, c] = coord;
            if (const Mask bit = static_cast<Mask>(1) << c; !(added[r] & bit))
            {
                added[r] |= bit;
                result.push_back(coord);
            }
        };

        auto scan = [&]<typename GetMask, typename ToCoord>(
            const std::size_t limit, GetMask&& get_mask, ToCoord&& to_coord)
        {
            for (std::size_t idx = 0; idx < limit; ++idx)
            {
                for (Mask m = get_mask(idx); m; m &= m - 1)
                    try_add(to_coord(idx, std::countr_zero(m)));
            }
        };

        scan(BOARD_SIZE,
             [&](std::size_t r) { return (row_four_opens_[s][r] | row_four_simples_[s][r]) & ~(rows_[BLACK][r] | rows_[WHITE][r]); },
             [](std::size_t r, std::size_t c) { return Coord{r, c}; });

        scan(BOARD_SIZE,
             [&](std::size_t c) { return (col_four_opens_[s][c] | col_four_simples_[s][c]) & ~(cols_[BLACK][c] | cols_[WHITE][c]); },
             [](std::size_t c, std::size_t r) { return Coord{r, c}; });

        scan(2 * BOARD_SIZE - 1,
             [&](std::size_t d) { return (diag_four_opens_[s][d] | diag_four_simples_[s][d]) & ~(diag_[BLACK][d] | diag_[WHITE][d]); },
             [](std::size_t d, std::size_t i) { return detail::coord_from_diag(d, i); });

        scan(2 * BOARD_SIZE - 1,
             [&](std::size_t d) { return (adiag_four_opens_[s][d] | adiag_four_simples_[s][d]) & ~(adiag_[BLACK][d] | adiag_[WHITE][d]); },
             [](std::size_t d, std::size_t i) { return detail::coord_from_adiag(d, i); });

        return result;
    }
}

#endif //GOMOKU_BOARD_HPP
