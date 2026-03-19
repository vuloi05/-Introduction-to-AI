#ifndef GOMOKU_CLI_GAME_HPP
#define GOMOKU_CLI_GAME_HPP

#include <memory>

#include "board.hpp"
#include "engine.hpp"
#include "types.hpp"

namespace Gomoku
{
    class CLIGame
    {
    public:
        enum PlayerType
        {
            HUMAN,
            ENGINE
        };

        enum Mode
        {
            INTERACTIVE,
            QUIET,
            BENCHMARK
        };

        CLIGame();

        CLIGame(PlayerType black, PlayerType white, int depth, Mode mode,
                std::size_t num_threads = 1, std::size_t tt_size_mb = 64, int time_ms = 0);

        CLIGame& parse_args(int argc, char* argv[]);

        void set_player(const std::pair<PlayerType, PlayerType>& types);
        void set_player(PlayerType black, PlayerType white);
        void set_player(Stone s, PlayerType type);
        [[nodiscard]] std::pair<PlayerType, PlayerType> player() const;
        [[nodiscard]] PlayerType player(Stone s) const;

        void set_depth(int depth);
        [[nodiscard]] int depth() const;

        void set_black_depth(int depth);
        [[nodiscard]] int black_depth() const;

        void set_white_depth(int depth);
        [[nodiscard]] int white_depth() const;

        void set_time(int time_ms);
        [[nodiscard]] int time_ms() const;

        void set_black_time(int time_ms);
        [[nodiscard]] int black_time() const;

        void set_white_time(int time_ms);
        [[nodiscard]] int white_time() const;

        void set_mode(Mode mode);
        [[nodiscard]] Mode mode() const;

        void run();

    private:
        Board board_;
        std::unique_ptr<SearchEngine> engine_black_;
        std::unique_ptr<SearchEngine> engine_white_;
        PlayerType player_type_[STONE_NB];

        // Common settings (fallback)
        int depth_;
        int time_ms_;

        // Per-side depth settings
        int black_depth_;
        int white_depth_;

        // Per-side time settings
        int black_time_;
        int white_time_;

        Mode mode_;

        [[nodiscard]] Coord get_human_player_move() const;
        [[nodiscard]] Coord get_engine_player_move() const;
        [[nodiscard]] SearchEngine& current_engine() const;
    };

    inline void CLIGame::set_player(const std::pair<PlayerType, PlayerType>& types)
    {
        set_player(types.first, types.second);
    }

    inline void CLIGame::set_player(const PlayerType black, const PlayerType white)
    {
        set_player(BLACK, black);
        set_player(WHITE, white);
    }

    inline void CLIGame::set_player(const Stone s, const PlayerType type) { player_type_[s] = type; }

    inline std::pair<CLIGame::PlayerType, CLIGame::PlayerType> CLIGame::player() const
    {
        return std::make_pair(player(BLACK), player(WHITE));
    }

    inline CLIGame::PlayerType CLIGame::player(const Stone s) const { return player_type_[s]; }

    inline void CLIGame::set_depth(const int depth) { depth_ = depth; }

    inline int CLIGame::depth() const { return depth_; }

    inline void CLIGame::set_black_depth(const int depth) { black_depth_ = depth; }

    inline int CLIGame::black_depth() const { return black_depth_; }

    inline void CLIGame::set_white_depth(const int depth) { white_depth_ = depth; }

    inline int CLIGame::white_depth() const { return white_depth_; }

    inline void CLIGame::set_time(const int time_ms) { time_ms_ = time_ms; }

    inline int CLIGame::time_ms() const { return time_ms_; }

    inline void CLIGame::set_black_time(const int time_ms) { black_time_ = time_ms; }

    inline int CLIGame::black_time() const { return black_time_; }

    inline void CLIGame::set_white_time(const int time_ms) { white_time_ = time_ms; }

    inline int CLIGame::white_time() const { return white_time_; }

    inline void CLIGame::set_mode(const Mode mode) { mode_ = mode; }

    inline CLIGame::Mode CLIGame::mode() const { return mode_; }

    inline SearchEngine& CLIGame::current_engine() const
    {
        return board_.side_to_move() == BLACK ? *engine_black_ : *engine_white_;
    }
}

#endif //GOMOKU_CLI_GAME_HPP

