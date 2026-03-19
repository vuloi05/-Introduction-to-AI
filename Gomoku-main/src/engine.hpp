#ifndef GOMOKU_ENGINE_HPP
#define GOMOKU_ENGINE_HPP

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "board.hpp"
#include "thread.hpp"
#include "tptable.hpp"
#include "types.hpp"

namespace Gomoku
{
    // Cache line size for alignment purposes.
    // 64 bytes is the standard cache line size on x86/x86-64, and most ARM processors.
    inline constexpr std::size_t CACHE_LINE_SIZE = 64;
    inline constexpr std::uint64_t POLL_NODES_MASK = 0xFFF;

    // =========================================================================
    // SearchLevel Enum
    // =========================================================================
    enum class SearchLevel
    {
        ABP, // Basic Alpha-Beta Pruning
        MO, // Alpha-Beta + Move Ordering
        TT, // Alpha-Beta + Move Ordering + Transposition Table
        PVS, // Alpha-Beta + Move Ordering + TT + Principal Variation Search
        VCF // All features including Victory by Continuous Four
    };

    struct alignas(CACHE_LINE_SIZE) ThreadData
    {
        std::uint64_t ab_nodes;
        std::uint64_t tt_hits;
        std::uint64_t vcf_nodes;

        ThreadData() : ab_nodes{0}, tt_hits{0}, vcf_nodes{0} {}

        void reset()
        {
            ab_nodes = 0;
            tt_hits = 0;
            vcf_nodes = 0;
        }

        [[nodiscard]] std::uint64_t total_nodes() const
        {
            return ab_nodes + vcf_nodes;
        }

    private:
        static constexpr std::size_t DATA_SIZE = 3 * sizeof(std::uint64_t);
        static constexpr std::size_t PADDING_SIZE =
            (DATA_SIZE % CACHE_LINE_SIZE == 0) ? 0 : (CACHE_LINE_SIZE - DATA_SIZE % CACHE_LINE_SIZE);
        [[maybe_unused]] std::array<char, PADDING_SIZE> padding_{};
    };

    static_assert(sizeof(ThreadData) % CACHE_LINE_SIZE == 0, "ThreadData must be a multiple of cache line size");

    struct SearchResult
    {
        Coord best_move;
        int depth;
        Eval eval;
    };

    class SearchEngine
    {
    public:
        virtual ~SearchEngine() = default;

        // Configuration
        virtual void set_num_threads(std::size_t num_threads) = 0;
        virtual void set_tt_size(std::size_t size_mb) = 0;

        // Statistics
        [[nodiscard]] virtual std::uint64_t nodes_searched() const = 0;
        [[nodiscard]] virtual std::uint64_t ab_nodes_searched() const = 0;
        [[nodiscard]] virtual std::uint64_t tt_hits_total() const = 0;
        [[nodiscard]] virtual std::uint64_t vcf_nodes_searched() const = 0;
        [[nodiscard]] virtual int last_search_depth() const = 0;

        // Control
        virtual void stop() = 0;
        virtual void new_game() = 0;

        // Search
        virtual Coord search(const Board& board, int max_depth) = 0;
        virtual Coord search_timed(const Board& board, int time_ms, int max_depth) = 0;
    };

    namespace detail
    {
        template <bool IV>
        Coord find_winning_move(const Board& board, Stone player)
        {
            if (auto fives = board.get_five_moves<IV>(player); !fives.empty())
                return fives[0];
            if (auto open_fours = board.get_open_four_moves<IV>(player); !open_fours.empty())
                return open_fours[0];
            return COORD_NULL;
        }

        template <bool IV>
        auto get_four_block_moves(const Board& board, Stone player)
        {
            return board.get_four_defense_moves<IV>(~player);
        }

        template <SearchLevel SL, bool IV>
        Eval vcf_search(Board& board, const int depth, const Stone attacker,
                        const std::atomic<bool>& stop_flag, ThreadData& td)
        {
            if constexpr (SL != SearchLevel::VCF)
                return 0; // VCF disabled for lower search levels
            ++td.vcf_nodes;

            if ((td.vcf_nodes & POLL_NODES_MASK) == 0)
            {
                if (stop_flag.load(std::memory_order_relaxed))
                    return 0;
            }

            if (board.has_winner())
            {
                const Stone winner = board.winner();
                return winner == attacker ? (EVAL_WIN_BASE + depth) : -(EVAL_WIN_BASE + depth);
            }

            if (depth <= 0)
                return 0;

            const Stone stm = board.side_to_move();
            const Stone defender = ~attacker;

            if (stm == attacker)
            {
                if (const Coord win = find_winning_move<IV>(board, attacker); win != COORD_NULL)
                    return EVAL_WIN_BASE + depth;
                if (const Coord defender_win = find_winning_move<IV>(board, defender); defender_win != COORD_NULL)
                    return 0;
                if (const auto defender_threats = board.get_four_defense_moves<IV>(defender); !defender_threats.
                    empty())
                    return 0;

                for (auto fours = board.get_forcing_moves<IV>(attacker); const Coord& move : fours)
                {
                    board.do_move(move);
                    const Eval eval = vcf_search<SL, IV>(board, depth - 1, attacker, stop_flag, td);
                    board.undo_move();

                    if (eval > 0)
                        return eval;
                }
                return 0;
            }
            if (const Coord attacker_win = find_winning_move<IV>(board, attacker); attacker_win != COORD_NULL)
                return EVAL_WIN_BASE + depth;

            if (const Coord defender_win = find_winning_move<IV>(board, defender); defender_win != COORD_NULL)
                return 0;

            auto blocks = board.get_four_defense_moves<IV>(attacker);
            if (blocks.empty())
                return 0;

            if (blocks.size() > 1)
            {
                bool can_survive = false;
                for (const Coord& block : blocks)
                {
                    board.do_move(block);
                    Coord attacker_win_after = find_winning_move<IV>(board, attacker);
                    board.undo_move();
                    if (attacker_win_after == COORD_NULL)
                    {
                        can_survive = true;
                        break;
                    }
                }
                if (!can_survive)
                    return EVAL_WIN_BASE + depth;
            }

            Eval best = EVAL_WIN_BASE + depth;
            for (const Coord& block : blocks)
            {
                board.do_move(block);
                const Eval eval = vcf_search<SL, IV>(board, depth - 1, attacker, stop_flag, td);
                board.undo_move();

                if (eval <= 0)
                    return 0;
                if (eval < best)
                    best = eval;
            }
            return best;
        }

        // Get moves based on search level
        template <SearchLevel SL, bool IV>
        auto get_moves_for_search(const Board& board)
        {
            if constexpr (SL == SearchLevel::ABP)
                return board.empty_cells_in_play_range<IV>();
            else
                return board.empty_cells_with_move_ordering<IV>();
        }

        // PVS with stop flag for interruptible search
        template <SearchLevel SL, bool IV>
        Eval pvs(Board& board, TranspositionTable& tt, const int depth, Eval alpha, const Eval beta,
                 const bool is_pv, const std::atomic<bool>& stop_flag, ThreadData& td)
        {
            ++td.ab_nodes;

            if ((td.ab_nodes & POLL_NODES_MASK) == 0)
            {
                if (stop_flag.load(std::memory_order_relaxed))
                    return 0;
            }

            const Key key = board.zobrist_key();
            Coord tt_move = COORD_NULL;

            if constexpr (SL >= SearchLevel::TT)
            {
                if (TTData tt_data{}; tt.probe(key, tt_data))
                {
                    ++td.tt_hits;
                    tt_move = tt_data.best_move();

                    if (!is_pv && tt_data.depth >= depth)
                    {
                        const Eval tt_eval = tt_data.eval;
                        if (tt_data.bound == BOUND_EXACT)
                            return tt_eval;
                        if (tt_data.bound == BOUND_LOWER && tt_eval >= beta)
                            return tt_eval;
                        if (tt_data.bound == BOUND_UPPER && tt_eval <= alpha)
                            return tt_eval;
                    }
                }
            }

            if (board.has_winner())
                return -(EVAL_WIN_BASE + depth);

            const Stone stm = board.side_to_move();

            if (const Coord win_move = find_winning_move<IV>(board, stm); win_move != COORD_NULL)
                return EVAL_WIN_BASE + depth;

            auto must_block = get_four_block_moves<IV>(board, stm);

            if (depth == 0)
                return board.evaluate();

            if constexpr (SL == SearchLevel::VCF)
            {
                if (const Eval vcf_result = vcf_search<SL, IV>(board, std::min(depth * 2, 10), stm, stop_flag, td);
                    vcf_result > 0)
                    return vcf_result;

                if (const Eval opp_vcf_result = vcf_search<SL, IV>(board, std::min(depth * 2, 10), ~stm, stop_flag, td);
                    opp_vcf_result > 0)
                {
                    return -opp_vcf_result + 100;
                }
            }

            if (must_block.size() > 1)
            {
                bool can_survive = false;
                for (const Coord& block : must_block)
                {
                    board.do_move(block);
                    Coord check_opp_win = find_winning_move<IV>(board, ~board.side_to_move());
                    board.undo_move();
                    if (check_opp_win == COORD_NULL)
                    {
                        can_survive = true;
                        break;
                    }
                }
                if (!can_survive)
                    return -(EVAL_WIN_BASE + depth);
            }

            Eval max_eval = EVAL_MIN;
            Coord best_move = COORD_NULL;
            Bound bound = BOUND_UPPER;
            bool searched_first = false;

            auto search_move = [&](const Coord& move, const bool first_move) -> bool
            {
                if (stop_flag.load(std::memory_order_relaxed))
                    return true;

                board.do_move(move);
                Eval eval;

                // Use PVS null-window search only for PVS and VCF levels
                if constexpr (SL >= SearchLevel::PVS)
                {
                    if (first_move || !is_pv)
                        eval = -pvs<SL, IV>(board, tt, depth - 1, -beta, -alpha, is_pv, stop_flag, td);
                    else
                    {
                        eval = -pvs<SL, IV>(board, tt, depth - 1, -alpha - 1, -alpha, false, stop_flag, td);
                        if (!stop_flag.load(std::memory_order_relaxed) && eval > alpha && eval < beta)
                            eval = -pvs<SL, IV>(board, tt, depth - 1, -beta, -alpha, true, stop_flag, td);
                    }
                }
                else
                {
                    eval = -pvs<SL, IV>(board, tt, depth - 1, -beta, -alpha, is_pv, stop_flag, td);
                }
                board.undo_move();

                if (stop_flag.load(std::memory_order_relaxed))
                    return true;

                if (eval > max_eval)
                {
                    max_eval = eval;
                    best_move = move;
                }

                if (max_eval > alpha)
                {
                    alpha = max_eval;
                    bound = BOUND_EXACT;
                }

                return alpha >= beta;
            };

            if constexpr (SL >= SearchLevel::TT)
            {
                if (tt_move != COORD_NULL && board.stone_at(tt_move) == EMPTY)
                {
                    if (search_move(tt_move, true))
                    {
                        if (!stop_flag.load(std::memory_order_relaxed))
                        {
                            bound = BOUND_LOWER;
                            tt.store(key, best_move, is_pv, bound, depth, max_eval);
                        }
                        return max_eval;
                    }
                    searched_first = true;
                }
            }

            if (!must_block.empty())
            {
                for (const Coord& move : must_block)
                {
                    if constexpr (SL >= SearchLevel::TT)
                        if (move == tt_move)
                            continue;

                    if (search_move(move, !searched_first))
                    {
                        if (!stop_flag.load(std::memory_order_relaxed))
                            bound = BOUND_LOWER;
                        break;
                    }
                    searched_first = true;
                }
                if constexpr (SL >= SearchLevel::TT)
                    if (!stop_flag.load(std::memory_order_relaxed))
                        tt.store(key, best_move, is_pv, bound, depth, max_eval);
                return max_eval;
            }

            auto moves = get_moves_for_search<SL, IV>(board);
            for (const Coord& move : moves)
            {
                if constexpr (SL >= SearchLevel::TT)
                    if (move == tt_move)
                        continue;

                if (search_move(move, !searched_first))
                {
                    if (!stop_flag.load(std::memory_order_relaxed))
                        bound = BOUND_LOWER;
                    break;
                }
                searched_first = true;
            }

            if constexpr (SL >= SearchLevel::TT)
                if (!stop_flag.load(std::memory_order_relaxed))
                    tt.store(key, best_move, is_pv, bound, depth, max_eval);

            return max_eval;
        }
    } // namespace detail


    template <SearchLevel SL, bool IV>
    class Engine final : public SearchEngine
    {
    public:
        explicit Engine(std::size_t num_threads = 1, std::size_t tt_size_mb = 64)
            : threads_(num_threads), tt_(tt_size_mb), last_depth_{0}
        {
            ensure_thread_data_size(std::max(num_threads, std::size_t{1}));
        }

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&) = delete;
        Engine& operator=(Engine&&) = delete;

        ~Engine() override = default;

        void set_num_threads(std::size_t num_threads) override
        {
            threads_.resize(num_threads);
            ensure_thread_data_size(std::max(num_threads, std::size_t{1}));
        }

        void set_tt_size(std::size_t size_mb) override
        {
            tt_.resize(size_mb);
        }

        [[nodiscard]] std::uint64_t nodes_searched() const override
        {
            std::uint64_t total = 0;
            for (const auto& td : thread_data_)
                total += td.total_nodes();
            return total;
        }

        [[nodiscard]] std::uint64_t ab_nodes_searched() const override
        {
            std::uint64_t total = 0;
            for (const auto& td : thread_data_)
                total += td.ab_nodes;
            return total;
        }

        [[nodiscard]] std::uint64_t tt_hits_total() const override
        {
            std::uint64_t total = 0;
            for (const auto& td : thread_data_)
                total += td.tt_hits;
            return total;
        }

        [[nodiscard]] std::uint64_t vcf_nodes_searched() const override
        {
            std::uint64_t total = 0;
            for (const auto& td : thread_data_)
                total += td.vcf_nodes;
            return total;
        }

        [[nodiscard]] int last_search_depth() const override
        {
            return last_depth_;
        }

        void stop() override
        {
            stop_flag_.store(true, std::memory_order_relaxed);
        }

        [[nodiscard]] bool is_stopped() const
        {
            return stop_flag_.load(std::memory_order_relaxed);
        }

        void new_game() override
        {
            tt_.clear();
        }

        Coord search(const Board& board, int max_depth) override
        {
            stop_flag_.store(false, std::memory_order_relaxed);
            reset_node_count();
            results_.clear();
            last_depth_ = 0;
            tt_.new_search();

            if (const std::size_t num_threads = threads_.size(); num_threads == 0)
                worker_search(board, max_depth, 0);
            else
            {
                for (std::size_t i = 0; i < num_threads; ++i)
                {
                    threads_.assign_job(i, [this, board, max_depth, i]
                    {
                        worker_search(board, max_depth, static_cast<int>(i));
                    });
                }
                threads_.wait_for_all();
            }

            update_last_depth();
            return get_best_move();
        }

        Coord search_timed(const Board& board, int time_ms, int max_depth) override
        {
            stop_flag_.store(false, std::memory_order_relaxed);
            reset_node_count();
            results_.clear();
            last_depth_ = 0;
            tt_.new_search();

            if (const std::size_t num_threads = threads_.size(); num_threads == 0)
            {
                std::thread timer_thread([this, time_ms]
                {
                    std::unique_lock lock(timer_mutex_);
                    if (!timer_cv_.wait_for(lock, std::chrono::milliseconds(time_ms),
                                            [this] { return is_stopped(); }))
                        stop();
                });

                worker_search(board, max_depth, 0);
                stop();
                timer_cv_.notify_all();
                timer_thread.join();
            }
            else
            {
                for (std::size_t i = 0; i < num_threads; ++i)
                {
                    threads_.assign_job(i, [this, board, max_depth, i]
                    {
                        worker_search(board, max_depth, static_cast<int>(i));
                    });
                }

                std::thread timer_thread([this, time_ms]
                {
                    std::unique_lock lock(timer_mutex_);
                    if (!timer_cv_.wait_for(lock, std::chrono::milliseconds(time_ms),
                                            [this] { return is_stopped(); }))
                        stop();
                });

                threads_.wait_for_all();
                stop();
                timer_cv_.notify_all();
                timer_thread.join();
            }

            update_last_depth();
            return get_best_move();
        }

    private:
        ThreadPool threads_;
        TranspositionTable tt_;
        std::atomic<bool> stop_flag_{false};
        std::vector<ThreadData> thread_data_;
        std::mutex results_mutex_;
        std::vector<SearchResult> results_;
        int last_depth_;
        std::mutex timer_mutex_;
        std::condition_variable timer_cv_;

        void ensure_thread_data_size(std::size_t min_size)
        {
            if (thread_data_.size() < min_size)
                thread_data_.resize(min_size);
        }

        void reset_node_count()
        {
            for (auto& td : thread_data_)
                td.reset();
        }

        void add_result(const SearchResult& result)
        {
            std::lock_guard lock(results_mutex_);
            results_.push_back(result);
        }

        void update_last_depth()
        {
            std::lock_guard lock(results_mutex_);
            last_depth_ = 0;
            for (const auto& r : results_)
                if (r.depth > last_depth_)
                    last_depth_ = r.depth;
        }

        [[nodiscard]] Coord get_best_move() const
        {
            if (results_.empty())
                return COORD_NULL;

            int max_depth = results_[0].depth;
            for (const auto& r : results_)
                if (r.depth > max_depth)
                    max_depth = r.depth;

            std::map<std::pair<std::size_t, std::size_t>, int> move_counts;
            for (const auto& r : results_)
                if (r.depth == max_depth)
                {
                    auto key = std::make_pair(r.best_move.row, r.best_move.col);
                    move_counts[key]++;
                }

            Coord best_move = COORD_NULL;
            int best_count = 0;
            for (const auto& [coord_pair, count] : move_counts)
                if (count > best_count)
                {
                    best_count = count;
                    best_move = Coord{coord_pair.first, coord_pair.second};
                }

            return best_move;
        }

        void worker_search(Board board, int max_depth, int thread_id)
        {
            ThreadData& td = thread_data_[thread_id];
            const Stone stm = board.side_to_move();

            if (const Coord win_move = detail::find_winning_move<IV>(board, stm); win_move != COORD_NULL)
            {
                add_result({win_move, 0, EVAL_WIN_BASE});
                return;
            }

            auto must_block = detail::get_four_block_moves<IV>(board, stm);

            if (must_block.size() > 1)
            {
                Coord best_block = must_block[0];
                Eval best_eval = EVAL_MIN;
                for (const Coord& block : must_block)
                {
                    if (is_stopped())
                        break;
                    board.do_move(block);
                    const Eval eval = -detail::pvs<SL, IV>(board, tt_, max_depth - 1, EVAL_MIN, EVAL_MAX, true,
                                                           stop_flag_, td);
                    board.undo_move();
                    if (eval > best_eval)
                    {
                        best_eval = eval;
                        best_block = block;
                    }
                }
                add_result({best_block, 1, best_eval});
                return;
            }

            if (must_block.size() == 1)
            {
                add_result({must_block[0], 0, 0});
                return;
            }

            if constexpr (SL == SearchLevel::VCF)
            {
                if (const Eval vcf_result = detail::vcf_search<SL, IV>(board, std::min(max_depth * 2, 10),
                                                                       stm, stop_flag_, td); vcf_result > 0)
                {
                    for (auto fours = board.get_forcing_moves<IV>(stm); const Coord& move : fours)
                    {
                        if (is_stopped())
                            break;
                        board.do_move(move);
                        const Eval eval = detail::vcf_search<SL, IV>(board, std::min(max_depth * 2, 10) - 1, stm,
                                                                     stop_flag_, td);
                        board.undo_move();
                        if (eval > 0)
                        {
                            add_result({move, 0, eval});
                            return;
                        }
                    }
                }
            }

            const int start_depth = 1 + thread_id % 3;

            for (int depth = start_depth; depth <= max_depth; ++depth)
            {
                if (is_stopped())
                    break;

                Eval alpha = EVAL_MIN;
                constexpr Eval beta = EVAL_MAX;
                Coord current_best = COORD_NULL;
                Eval current_best_eval = EVAL_MIN;

                Coord tt_move = COORD_NULL;
                if constexpr (SL >= SearchLevel::TT)
                    if (TTData tt_data{}; tt_.probe(board.zobrist_key(), tt_data))
                        tt_move = tt_data.best_move();

                bool searched_first = false;

                if constexpr (SL >= SearchLevel::TT)
                {
                    if (tt_move != COORD_NULL && board.stone_at(tt_move) == EMPTY)
                    {
                        board.do_move(tt_move);
                        const Eval eval = -detail::pvs<SL, IV>(board, tt_, depth - 1, -beta, -alpha,
                                                               true, stop_flag_, td);
                        board.undo_move();

                        if (is_stopped())
                            break;

                        current_best = tt_move;
                        current_best_eval = eval;
                        alpha = eval;
                        searched_first = true;
                    }
                }

                for (auto moves = detail::get_moves_for_search<SL, IV>(board); const Coord& move : moves)
                {
                    if (is_stopped())
                        break;

                    if constexpr (SL >= SearchLevel::TT)
                        if (move == tt_move)
                            continue;

                    board.do_move(move);
                    Eval eval;

                    if constexpr (SL >= SearchLevel::PVS)
                    {
                        if (!searched_first)
                            eval = -detail::pvs<SL, IV>(board, tt_, depth - 1, -beta, -alpha, true, stop_flag_, td);
                        else
                        {
                            eval = -detail::pvs<SL, IV>(board, tt_, depth - 1, -alpha - 1, -alpha,
                                                        false, stop_flag_, td);
                            if (!is_stopped() && eval > alpha && eval < beta)
                                eval = -detail::pvs<SL, IV>(board, tt_, depth - 1, -beta, -alpha, true, stop_flag_, td);
                        }
                    }
                    else
                        eval = -detail::pvs<SL, IV>(board, tt_, depth - 1, -beta, -alpha, true, stop_flag_, td);
                    board.undo_move();

                    if (is_stopped())
                        break;

                    if (eval > current_best_eval)
                    {
                        current_best_eval = eval;
                        current_best = move;
                    }

                    if (eval > alpha)
                        alpha = eval;

                    searched_first = true;
                }

                if (!is_stopped() && current_best != COORD_NULL)
                {
                    if constexpr (SL >= SearchLevel::TT)
                        tt_.store(board.zobrist_key(), current_best, true, BOUND_EXACT, depth, current_best_eval);

                    add_result({current_best, depth, current_best_eval});

                    if (current_best_eval >= EVAL_WIN_BASE)
                        break;
                }
            }
        }
    };

    // =========================================================================
    // Factory Declaration
    // =========================================================================
    std::unique_ptr<SearchEngine> create_engine(SearchLevel level, bool inplace_vector,
                                                std::size_t num_threads, std::size_t tt_size);
} // namespace Gomoku

#endif //GOMOKU_ENGINE_HPP
