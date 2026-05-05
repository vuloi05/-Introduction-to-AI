#include "board.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstring>
#include <iostream>

#ifdef __BMI2__
#include <immintrin.h>
#endif

#include "misc.hpp"
#include "types.hpp"
#include "zobrist.hpp"

namespace Gomoku
{
    constexpr std::size_t TABLE_SIZE = 1 << BOARD_SIZE;

    constexpr int row_dir[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    constexpr int col_dir[8] = {1, 1, 0, -1, -1, -1, 0, 1};

    struct DiagCoord
    {
        std::size_t row;
        std::size_t col;
        std::size_t len;
    };

    struct Segment
    {
        std::uint8_t start;
        std::uint8_t length;

        Segment() : start{0}, length{0} {}
        Segment(const std::uint8_t start, const std::uint8_t length) : start{start}, length{length} {}

        [[nodiscard]] bool is_null() const { return start == 0 && length == 0; }
    };

    struct SplitResult
    {
        Segment left;
        Segment right;
    };

    SplitResult SPLIT[TABLE_SIZE];
    std::uint64_t POTENTIAL_THREAT[TABLE_SIZE << 1];
    std::uint16_t POTENTIAL_FIVE_MASK[TABLE_SIZE << 1];
    std::uint16_t POTENTIAL_FOUR_OPEN_MASK[TABLE_SIZE << 1];
    std::uint16_t POTENTIAL_FOUR_SIMPLE_MASK[TABLE_SIZE << 1];

    inline std::uint64_t potential_threat(const std::size_t length, const std::size_t mask)
    {
        return POTENTIAL_THREAT[(1 << length) + mask];
    }

    inline bool is_coord_valid(const Coord& c) { return c.row < BOARD_SIZE && c.col < BOARD_SIZE; }

    inline DiagCoord diag_coord(const Coord& c)
    {
        const int row = static_cast<int>(c.row);
        const int col = static_cast<int>(c.col);
        return DiagCoord{
            row - col + (BOARD_SIZE - 1),
            std::min(c.row, c.col),
            BOARD_SIZE - std::abs(row - col)
        };
    }

    inline DiagCoord adiag_coord(const Coord& c)
    {
        const int row = static_cast<int>(c.row);
        const int col = static_cast<int>(c.col);
        return DiagCoord{
            c.row + c.col,
            std::min(c.row, BOARD_SIZE - 1 - c.col),
            BOARD_SIZE - std::abs(row + col - static_cast<int>(BOARD_SIZE - 1))
        };
    }

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

    inline void update_line(ThreatType* row, Mask& five_mask, Mask& four_open_mask, Mask& four_simple_mask,
                            const Mask self, const Mask opp, const std::size_t length)
    {
        std::memset(row, NONE, 16 * sizeof(ThreatType));
        five_mask = 0;
        four_open_mask = 0;
        four_simple_mask = 0;

        const Mask walls = length >= 16 ? 0 : 0xFFFF << length;

        for (const auto& [left, right] = SPLIT[opp | walls]; const Segment& seg : {left, right})
        {
            if (seg.is_null())
                break;

            constexpr std::uint64_t SPLIT_MASK = 0x0F0F0F0F0F0F0F0FULL;

            const Mask local = self >> seg.start & ((1 << seg.length) - 1);
            const std::size_t idx = (1 << seg.length) + local;

            const std::uint64_t pot = POTENTIAL_THREAT[idx];

#ifdef __BMI2__
            const std::uint64_t low = _pdep_u64(pot, SPLIT_MASK);
#else
            const std::uint64_t low = pdep_u64_fallback(pot, SPLIT_MASK);
#endif

            std::memcpy(row + seg.start, &low, 8);

            if (seg.length > 8)
            {
#ifdef __BMI2__
                const std::uint64_t high = _pdep_u64(pot >> 32, SPLIT_MASK);
#else
                const std::uint64_t high = pdep_u64_fallback(pot >> 32, SPLIT_MASK);
#endif
                std::memcpy(row + seg.start + 8, &high, 8);
            }

            five_mask |= POTENTIAL_FIVE_MASK[idx] << seg.start;
            four_open_mask |= POTENTIAL_FOUR_OPEN_MASK[idx] << seg.start;
            four_simple_mask |= POTENTIAL_FOUR_SIMPLE_MASK[idx] << seg.start;
        }
    }

    Stone Board::stone_at(const Coord& c) const
    {
        assert(is_coord_valid(c));
        const std::uint64_t bit = 1ULL << c.col;
        if (rows_[BLACK][c.row] & bit)
            return BLACK;
        if (rows_[WHITE][c.row] & bit)
            return WHITE;
        return EMPTY;
    }

    Stone Board::stone_at_safe(const Coord& c) const
    {
        return c.row < BOARD_SIZE && c.col < BOARD_SIZE ? stone_at(c) : EMPTY;
    }

    Eval Board::evaluate() const { return evals_[stm_] - evals_[~stm_]; }

    Eval Board::evaluate_at(const Stone s, const Coord& c) const
    {
        assert(is_coord_valid(c));

        if (stone_at(c) != EMPTY)
            return 0;

        const auto [row, col] = c;
        const auto [drow, dcol, dlen] = diag_coord(c);
        const auto [adrow, adcol, adlen] = adiag_coord(c);

        const auto trow = trows_[s][row][col];
        const auto tcol = tcols_[s][col][row];
        const auto tdiag = tdiag_[s][drow][dcol];
        const auto tadiag = tadiag_[s][adrow][adcol];

        const auto [t1, t2] = max_two(trow, tcol, tdiag, tadiag);

        return one_and_a_half(EVAL[t1]) + EVAL[t2];
    }


    ThreatType Board::max_threat_at(const Stone s, const Coord& c) const
    {
        assert(is_coord_valid(c));

        if (stone_at(c) != EMPTY)
            return NONE;

        const auto [row, col] = c;
        const auto [drow, dcol, dlen] = diag_coord(c);
        const auto [adrow, adcol, adlen] = adiag_coord(c);

        const auto trow = trows_[s][row][col];
        const auto tcol = tcols_[s][col][row];
        const auto tdiag = tdiag_[s][drow][dcol];
        const auto tadiag = tadiag_[s][adrow][adcol];

        return std::max({trow, tcol, tdiag, tadiag});
    }


    void Board::do_move(const Coord& c)
    {
        assert(!has_winner());

        const auto [row, col] = c;
        const auto [drow, dcol, dlen] = diag_coord(c);
        const auto [adrow, adcol, adlen] = adiag_coord(c);

        update_evaluation(c, true);

        rows_[stm_][row] |= 1ULL << col;
        cols_[stm_][col] |= 1ULL << row;
        diag_[stm_][drow] |= 1ULL << dcol;
        adiag_[stm_][adrow] |= 1ULL << adcol;

        update_threats(row, col, drow, dlen, adrow, adlen);
        update_evaluation(c, false);

        update_winner_after_do_move(c);

        key_ ^= get_zobrist_key(stm_, c);

        PlayRange pr = history_.empty() ? PlayRange::min() : history_.back().pr;
        pr.add_coord(c);
        history_.emplace_back(c, pr);

        stm_ = ~stm_;
    }

    void Board::undo_move()
    {
        assert(!history_.empty());

        const auto c = history_.back().c;
        history_.pop_back();

        const auto [row, col] = c;
        const auto [drow, dcol, dlen] = diag_coord(c);
        const auto [adrow, adcol, adlen] = adiag_coord(c);

        update_evaluation(c, true);

        stm_ = ~stm_;

        rows_[stm_][row] &= ~(1ULL << col);
        cols_[stm_][col] &= ~(1ULL << row);
        diag_[stm_][drow] &= ~(1ULL << dcol);
        adiag_[stm_][adrow] &= ~(1ULL << adcol);

        update_threats(row, col, drow, dlen, adrow, adlen);

        update_evaluation(c, false);

        key_ ^= get_zobrist_key(stm_, c);

        has_winner_ = false;
    }

    void Board::clear()
    {
        std::memset(rows_, 0, sizeof(rows_));
        std::memset(cols_, 0, sizeof(cols_));
        std::memset(diag_, 0, sizeof(diag_));
        std::memset(adiag_, 0, sizeof(adiag_));

        std::memset(trows_, 0, sizeof(trows_));
        std::memset(tcols_, 0, sizeof(tcols_));
        std::memset(tdiag_, 0, sizeof(tdiag_));
        std::memset(tadiag_, 0, sizeof(tadiag_));

        std::memset(row_fives_, 0, sizeof(row_fives_));
        std::memset(col_fives_, 0, sizeof(col_fives_));
        std::memset(diag_fives_, 0, sizeof(diag_fives_));
        std::memset(adiag_fives_, 0, sizeof(adiag_fives_));

        std::memset(row_four_opens_, 0, sizeof(row_four_opens_));
        std::memset(col_four_opens_, 0, sizeof(col_four_opens_));
        std::memset(diag_four_opens_, 0, sizeof(diag_four_opens_));
        std::memset(adiag_four_opens_, 0, sizeof(adiag_four_opens_));

        std::memset(row_four_simples_, 0, sizeof(row_four_simples_));
        std::memset(col_four_simples_, 0, sizeof(col_four_simples_));
        std::memset(diag_four_simples_, 0, sizeof(diag_four_simples_));
        std::memset(adiag_four_simples_, 0, sizeof(adiag_four_simples_));

        evals_[BLACK] = evals_[WHITE] = 0;

        history_.clear();

        key_ = 0;
        stm_ = BLACK;
        has_winner_ = false;
    }

    void Board::init()
    {
        constexpr ThreatType THREAT[6][6] = {
            {NONE, NONE, NONE, NONE, NONE, NONE},
            {NONE, ONE_SIMPLE, ONE_BROKEN_SIMPLE, ONE_BROKEN, ONE_BROKEN_OPEN, ONE_OPEN},
            {NONE, TWO_SIMPLE, TWO_BROKEN_SIMPLE, TWO_BROKEN_OPEN, TWO_OPEN},
            {NONE, THREE_SIMPLE, THREE_BROKEN, THREE_OPEN},
            {NONE, FOUR_SIMPLE, FOUR_OPEN},
            {NONE, FIVE}
        };

        for (std::size_t mask = 0; mask < TABLE_SIZE; ++mask)
        {
            Segment& left = SPLIT[mask].left;
            Segment& right = SPLIT[mask].right;

            std::size_t start = 0, length = 0;
            for (std::size_t i = 0; i < BOARD_SIZE; ++i)
            {
                if (mask >> i & 1)
                {
                    if (length >= 5)
                        (left.is_null() ? left : right) = Segment{
                            static_cast<std::uint8_t>(start),
                            static_cast<std::uint8_t>(length)
                        };
                    start = i + 1;
                    length = 0;
                }
                else
                    ++length;
            }
            if (length >= 5)
                (left.is_null() ? left : right) = Segment{
                    static_cast<std::uint8_t>(start),
                    static_cast<std::uint8_t>(length)
                };
        }

        for (int length = 5; length <= static_cast<int>(BOARD_SIZE); ++length)
        {
            for (std::size_t mask = 0; mask < 1ULL << length; ++mask)
            {
                std::uint64_t pot = 0;
                std::uint16_t five_m = 0;
                std::uint16_t four_open_m = 0;
                std::uint16_t four_simple_m = 0;

                for (int i = 0; i < length; ++i)
                {
                    if (mask >> i & 1)
                        continue;
                    const std::size_t toggled_mask = mask | 1ULL << i;
                    int severity = 0;
                    int way = 0;
                    for (int j = std::max(i - 4, 0); j <= std::min(i, length - 5); ++j)
                    {
                        if (const int c = std::popcount(toggled_mask >> j & 0x1F); c > severity)
                        {
                            severity = c;
                            way = 1;
                        }
                        else if (c == severity)
                            ++way;
                    }
                    const ThreatType threat = THREAT[severity][way];
                    pot |= static_cast<std::uint64_t>(threat) << (i * 4);

                    if (threat == FIVE)
                        five_m |= 1 << i;
                    else if (threat == FOUR_OPEN)
                        four_open_m |= 1 << i;
                    else if (threat == FOUR_SIMPLE)
                        four_simple_m |= 1 << i;
                }
                const std::size_t idx = (1 << length) + mask;
                POTENTIAL_THREAT[idx] = pot;
                POTENTIAL_FIVE_MASK[idx] = five_m;
                POTENTIAL_FOUR_OPEN_MASK[idx] = four_open_m;
                POTENTIAL_FOUR_SIMPLE_MASK[idx] = four_simple_m;
            }
        }
    }

    void Board::update_threats(const std::size_t row, const std::size_t col, const std::size_t drow,
                               const std::size_t dlen, const std::size_t adrow, const std::size_t adlen)
    {
        update_line(trows_[BLACK][row], row_fives_[BLACK][row], row_four_opens_[BLACK][row],
                    row_four_simples_[BLACK][row],
                    rows_[BLACK][row], rows_[WHITE][row], BOARD_SIZE);
        update_line(trows_[WHITE][row], row_fives_[WHITE][row], row_four_opens_[WHITE][row],
                    row_four_simples_[WHITE][row],
                    rows_[WHITE][row], rows_[BLACK][row], BOARD_SIZE);
        update_line(tcols_[BLACK][col], col_fives_[BLACK][col], col_four_opens_[BLACK][col],
                    col_four_simples_[BLACK][col],
                    cols_[BLACK][col], cols_[WHITE][col], BOARD_SIZE);
        update_line(tcols_[WHITE][col], col_fives_[WHITE][col], col_four_opens_[WHITE][col],
                    col_four_simples_[WHITE][col],
                    cols_[WHITE][col], cols_[BLACK][col], BOARD_SIZE);
        update_line(tdiag_[BLACK][drow], diag_fives_[BLACK][drow], diag_four_opens_[BLACK][drow],
                    diag_four_simples_[BLACK][drow],
                    diag_[BLACK][drow], diag_[WHITE][drow], dlen);
        update_line(tdiag_[WHITE][drow], diag_fives_[WHITE][drow], diag_four_opens_[WHITE][drow],
                    diag_four_simples_[WHITE][drow],
                    diag_[WHITE][drow], diag_[BLACK][drow], dlen);
        update_line(tadiag_[BLACK][adrow], adiag_fives_[BLACK][adrow], adiag_four_opens_[BLACK][adrow],
                    adiag_four_simples_[BLACK][adrow],
                    adiag_[BLACK][adrow], adiag_[WHITE][adrow], adlen);
        update_line(tadiag_[WHITE][adrow], adiag_fives_[WHITE][adrow], adiag_four_opens_[WHITE][adrow],
                    adiag_four_simples_[WHITE][adrow],
                    adiag_[WHITE][adrow], adiag_[BLACK][adrow], adlen);
    }

    void Board::update_evaluation(const Coord& c, const bool subtract)
    {
        const Eval factor = subtract ? -1 : 1;
        for (const Stone s : {BLACK, WHITE})
        {
            Eval eval = evaluate_at(s, c);
            for (int i = 0; i < 8; ++i)
            {
                auto [row, col] = c;
                for (int j = 0; j < 16; ++j)
                {
                    row += row_dir[i];
                    col += col_dir[i];
                    if (!is_coord_valid(Coord{row, col}))
                        break;
                    eval += evaluate_at(s, Coord{row, col});
                }
            }
            eval *= factor;
            evals_[s] += eval;
        }
    }

    void Board::update_winner_after_do_move(const Coord& c)
    {
        const auto [row, col] = c;
        const auto [drow, dcol, dlen] = diag_coord(c);
        const auto [adrow, adcol, adlen] = adiag_coord(c);

        for (Mask mask : {rows_[stm_][row], cols_[stm_][col], diag_[stm_][drow], adiag_[stm_][adrow]})
        {
            mask &= mask >> 1;
            mask &= mask >> 2;
            mask &= mask >> 1;

            if (mask != 0)
            {
                has_winner_ = true;
                return;
            }
        }
    }


    std::ostream& operator<<(std::ostream& os, const Board& board)
    {
        os << "   ";
        for (std::size_t c = 0; c < BOARD_SIZE; ++c)
            os << (c < 10 ? " " : "") << c << " ";
        os << "\n";
        for (std::size_t r = 0; r < BOARD_SIZE; ++r)
        {
            os << (r < 10 ? " " : "") << r << " ";
            for (std::size_t c = 0; c < BOARD_SIZE; ++c)
            {
                const Stone s = board.stone_at(Coord{r, c});
                const char ch = s == BLACK ? 'X' : s == WHITE ? 'O' : '.';
                os << " " << ch << " ";
            }
            os << "\n";
        }
        return os;
    }
}
