#include "cli_game.hpp"

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>

#include <argparse/argparse.hpp>
#include "misc.hpp"

namespace Gomoku
{
    CLIGame::CLIGame()
        : engine_black_(create_engine(SearchLevel::VCF, true, 1, 64)),
          engine_white_(create_engine(SearchLevel::VCF, true, 1, 64)),
          player_type_{HUMAN, ENGINE},
          depth_{4}, time_ms_{0},
          black_depth_{4}, white_depth_{4},
          black_time_{0}, white_time_{0},
          mode_{INTERACTIVE} {}

    CLIGame::CLIGame(const PlayerType black, const PlayerType white, const int depth, const Mode mode,
                     const std::size_t num_threads, const std::size_t tt_size_mb, const int time_ms)
        : engine_black_(create_engine(SearchLevel::VCF, true, num_threads, tt_size_mb)),
          engine_white_(create_engine(SearchLevel::VCF, true, num_threads, tt_size_mb)),
          player_type_{black, white},
          depth_{depth}, time_ms_{time_ms},
          black_depth_{depth}, white_depth_{depth},
          black_time_{time_ms}, white_time_{time_ms},
          mode_{mode} {}

    inline std::ostream& cout_if_interactive(const CLIGame::Mode mode)
    {
        static NullBuffer null_buffer;
        static std::ostream null_stream(&null_buffer);
        return mode == CLIGame::INTERACTIVE ? std::cout : null_stream;
    }

    CLIGame::PlayerType parse_player_type(const std::string& arg)
    {
        if (arg == "human")
            return CLIGame::HUMAN;
        if (arg == "engine")
            return CLIGame::ENGINE;
        throw std::runtime_error("Invalid player type: " + arg + ". Must be 'human' or 'engine'.");
    }

    CLIGame::Mode parse_mode(const std::string& arg)
    {
        if (arg == "interactive")
            return CLIGame::INTERACTIVE;
        if (arg == "quiet")
            return CLIGame::QUIET;
        if (arg == "benchmark")
            return CLIGame::BENCHMARK;
        throw std::runtime_error("Invalid mode: " + arg + ". Must be 'interactive', 'quiet', or 'benchmark'.");
    }

    SearchLevel parse_search_level(const std::string& arg)
    {
        if (arg == "abp" || arg == "ABP")
            return SearchLevel::ABP;
        if (arg == "mo" || arg == "MO")
            return SearchLevel::MO;
        if (arg == "tt" || arg == "TT")
            return SearchLevel::TT;
        if (arg == "pvs" || arg == "PVS")
            return SearchLevel::PVS;
        if (arg == "vcf" || arg == "VCF")
            return SearchLevel::VCF;
        throw std::runtime_error("Invalid search level: " + arg + ". Must be 'abp', 'mo', 'tt', 'pvs', or 'vcf'.");
    }

    bool parse_container_type(const std::string& arg)
    {
        if (arg == "inplace" || arg == "iv")
            return true;
        if (arg == "vector" || arg == "std")
            return false;
        throw std::runtime_error("Invalid container type: " + arg + ". Must be 'inplace' (or 'iv') or 'vector' (or 'std').");
    }

    CLIGame& CLIGame::parse_args(const int argc, char* argv[])
    {
        argparse::ArgumentParser program("Gomoku");

        // Player configuration
        program.add_argument("-b", "--black")
               .help("Player type for black: human or engine")
               .default_value(std::string("human"));

        program.add_argument("-w", "--white")
               .help("Player type for white: human or engine")
               .default_value(std::string("engine"));

        // Mode selection
        program.add_argument("-m", "--mode")
               .help("Game mode: interactive, quiet, or benchmark")
               .default_value(std::string("interactive"));

        // Common settings
        program.add_argument("-d", "--depth")
               .help("Search depth for both engines")
               .default_value(4)
               .scan<'i', int>();

        program.add_argument("-t", "--time")
               .help("Time limit per move in seconds for both engines (0 = use depth instead)")
               .default_value(0.0)
               .scan<'g', double>();

        program.add_argument("-T", "--threads")
               .help("Number of threads for both engines (-1 = use all available cores)")
               .default_value(-1)
               .scan<'i', int>();

        program.add_argument("-H", "--hash")
               .help("Transposition table size in megabytes for both engines")
               .default_value(64)
               .scan<'i', int>();

        program.add_argument("-l", "--level")
               .help("Search level for both engines: abp, mo, tt, pvs, or vcf")
               .default_value(std::string("vcf"));

        program.add_argument("-c", "--container")
               .help("Container type for both engines: inplace (or iv) or vector (or std)")
               .default_value(std::string("inplace"));

        // Per-side depth settings
        program.add_argument("--black-depth")
               .help("Search depth for black engine (overrides --depth)")
               .default_value(-1)
               .scan<'i', int>();

        program.add_argument("--white-depth")
               .help("Search depth for white engine (overrides --depth)")
               .default_value(-1)
               .scan<'i', int>();

        // Per-side time settings
        program.add_argument("--black-time")
               .help("Time limit per move in seconds for black engine (overrides --time)")
               .default_value(-1.0)
               .scan<'g', double>();

        program.add_argument("--white-time")
               .help("Time limit per move in seconds for white engine (overrides --time)")
               .default_value(-1.0)
               .scan<'g', double>();

        // Per-side thread settings
        program.add_argument("--black-threads")
               .help("Number of threads for black engine (overrides --threads)")
               .default_value(-1)
               .scan<'i', int>();

        program.add_argument("--white-threads")
               .help("Number of threads for white engine (overrides --threads)")
               .default_value(-1)
               .scan<'i', int>();

        // Per-side hash settings
        program.add_argument("--black-hash")
               .help("Transposition table size in MB for black engine (overrides --hash)")
               .default_value(-1)
               .scan<'i', int>();

        program.add_argument("--white-hash")
               .help("Transposition table size in MB for white engine (overrides --hash)")
               .default_value(-1)
               .scan<'i', int>();

        // Per-side search level settings
        program.add_argument("--black-level")
               .help("Search level for black engine (overrides --level)")
               .default_value(std::string(""));

        program.add_argument("--white-level")
               .help("Search level for white engine (overrides --level)")
               .default_value(std::string(""));

        // Per-side container type settings
        program.add_argument("--black-container")
               .help("Container type for black engine (overrides --container)")
               .default_value(std::string(""));

        program.add_argument("--white-container")
               .help("Container type for white engine (overrides --container)")
               .default_value(std::string(""));

        try
        {
            program.parse_args(argc, argv);
        }
        catch (const std::exception& err)
        {
            std::cerr << err.what() << '\n';
            std::cerr << program;
            std::terminate();
        }

        // Parse player types
        set_player(BLACK, parse_player_type(program.get<std::string>("--black")));
        set_player(WHITE, parse_player_type(program.get<std::string>("--white")));

        // Parse mode
        set_mode(parse_mode(program.get<std::string>("--mode")));

        // Parse common settings
        const int common_depth = program.get<int>("--depth");
        set_depth(common_depth);

        const auto common_time_sec = program.get<double>("--time");
        if (common_time_sec < 0)
            throw std::runtime_error("Invalid time: " + std::to_string(common_time_sec) + ". Must be non-negative.");
        set_time(static_cast<int>(common_time_sec * 1000));

        int common_threads = program.get<int>("--threads");
        if (common_threads == -1)
            common_threads = static_cast<int>(std::thread::hardware_concurrency());
        if (common_threads <= 0)
            common_threads = 1;

        const int common_hash = program.get<int>("--hash");
        if (common_hash <= 0)
            throw std::runtime_error("Invalid hash size: " + std::to_string(common_hash) + ". Must be greater than 0.");

        // Resolve per-side depth
        const int black_depth_arg = program.get<int>("--black-depth");
        black_depth_ = (black_depth_arg >= 0) ? black_depth_arg : common_depth;

        const int white_depth_arg = program.get<int>("--white-depth");
        white_depth_ = (white_depth_arg >= 0) ? white_depth_arg : common_depth;

        // Resolve per-side time
        const auto black_time_arg = program.get<double>("--black-time");
        black_time_ = (black_time_arg >= 0) ? static_cast<int>(black_time_arg * 1000) : time_ms_;

        const auto white_time_arg = program.get<double>("--white-time");
        white_time_ = (white_time_arg >= 0) ? static_cast<int>(white_time_arg * 1000) : time_ms_;

        // Resolve per-side threads
        int black_threads_arg = program.get<int>("--black-threads");
        if (black_threads_arg == -1)
            black_threads_arg = common_threads;
        else if (black_threads_arg <= 0)
            black_threads_arg = 1;

        int white_threads_arg = program.get<int>("--white-threads");
        if (white_threads_arg == -1)
            white_threads_arg = common_threads;
        else if (white_threads_arg <= 0)
            white_threads_arg = 1;

        // Resolve per-side hash
        const int black_hash_arg = program.get<int>("--black-hash");
        const int black_hash = (black_hash_arg > 0) ? black_hash_arg : common_hash;

        const int white_hash_arg = program.get<int>("--white-hash");
        const int white_hash = (white_hash_arg > 0) ? white_hash_arg : common_hash;

        // Parse common search level and container type
        const SearchLevel common_level = parse_search_level(program.get<std::string>("--level"));
        const bool common_container = parse_container_type(program.get<std::string>("--container"));

        // Resolve per-side search level
        const auto black_level_arg = program.get<std::string>("--black-level");
        const SearchLevel black_level = black_level_arg.empty() ? common_level : parse_search_level(black_level_arg);

        const auto white_level_arg = program.get<std::string>("--white-level");
        const SearchLevel white_level = white_level_arg.empty() ? common_level : parse_search_level(white_level_arg);

        // Resolve per-side container type
        const auto black_container_arg = program.get<std::string>("--black-container");
        const bool black_container = black_container_arg.empty() ? common_container : parse_container_type(black_container_arg);

        const auto white_container_arg = program.get<std::string>("--white-container");
        const bool white_container = white_container_arg.empty() ? common_container : parse_container_type(white_container_arg);

        // Create engines with resolved settings
        engine_black_ = create_engine(black_level, black_container,
                                       static_cast<std::size_t>(black_threads_arg),
                                       static_cast<std::size_t>(black_hash));
        engine_white_ = create_engine(white_level, white_container,
                                       static_cast<std::size_t>(white_threads_arg),
                                       static_cast<std::size_t>(white_hash));

        return *this;
    }

    void CLIGame::run()
    {
        // Benchmark mode validation
        if (mode_ == BENCHMARK)
        {
            if (player_type_[BLACK] != ENGINE || player_type_[WHITE] != ENGINE)
            {
                throw std::runtime_error("Benchmark mode requires both players to be ENGINE.");
            }
            // Print column headers for benchmark output
            std::cout << "time_sec ab_nodes vcf_nodes tt_hits max_depth nps\n";
        }

        auto& out = cout_if_interactive(mode_);

        if (mode_ == INTERACTIVE)
        {
            out << "Starting a new game of Gomoku!\n";
        }

        board_.clear();
        constexpr std::string_view STONE_SYMBOLS("XO");

        while (true)
        {
            if (mode_ == INTERACTIVE)
            {
                out << board_;
                out << "Eval: " << board_.evaluate() << "\n";
            }

            if (board_.has_winner())
            {
                if (mode_ == INTERACTIVE)
                {
                    out << "Game over! Winner: " << STONE_SYMBOLS[board_.winner()] << '\n';
                }
                else if (mode_ == QUIET)
                {
                    std::cout << "winner " << STONE_SYMBOLS[board_.winner()] << '\n';
                }
                // In benchmark mode, no output for winner
                break;
            }

            Coord move{};
            if (player_type_[board_.side_to_move()] == HUMAN)
            {
                move = get_human_player_move();
            }
            else
            {
                move = get_engine_player_move();
            }

            if (move == COORD_NULL)
            {
                if (mode_ == INTERACTIVE)
                {
                    out << "Game over! Draw.\n";
                }
                else if (mode_ == QUIET)
                {
                    std::cout << "draw\n";
                }
                break;
            }

            board_.do_move(move);
        }
    }

    Coord CLIGame::get_human_player_move() const
    {
        auto& out = cout_if_interactive(mode_);
        std::size_t row, col;
        while (true)
        {
            out << "Enter your move (row col): ";
            if (!(std::cin >> row >> col))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                out << "Invalid input. Please enter two numbers.\n";
                continue;
            }
            if (row >= BOARD_SIZE || col >= BOARD_SIZE)
            {
                out << "Invalid move. Coordinates must be between 0 and " << (BOARD_SIZE - 1) << ".\n";
                continue;
            }
            Coord move{row, col};
            if (board_.stone_at(move) != EMPTY)
            {
                out << "Invalid move. Cell is already occupied.\n";
                continue;
            }
            return move;
        }
    }

    Coord CLIGame::get_engine_player_move() const
    {
        auto& out = cout_if_interactive(mode_);

        if (mode_ == INTERACTIVE)
        {
            out << "Engine is thinking...\n";
        }

        const auto start_time = std::chrono::high_resolution_clock::now();

        SearchEngine& engine = current_engine();

        // Determine depth and time based on side to move
        const Stone stm = board_.side_to_move();
        const int current_depth = stm == BLACK ? black_depth_ : white_depth_;
        const int current_time = stm == BLACK ? black_time_ : white_time_;

        const Coord move = (current_time > 0)
                               ? engine.search_timed(board_, current_time, 100) // Time-based search
                               : engine.search(board_, current_depth);          // Depth-based search

        const auto end_time = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double> elapsed = end_time - start_time;

        const std::uint64_t ab_nodes = engine.ab_nodes_searched();
        const std::uint64_t vcf_nodes = engine.vcf_nodes_searched();
        const std::uint64_t tt_hits = engine.tt_hits_total();
        const int max_depth = engine.last_search_depth();
        const std::uint64_t total_nodes = engine.nodes_searched();
        const double nps = elapsed.count() > 0 ? static_cast<double>(total_nodes) / elapsed.count() : 0;

        if (mode_ == BENCHMARK)
        {
            std::cout << std::fixed << std::setprecision(6)
                      << elapsed.count() << " "
                      << ab_nodes << " "
                      << vcf_nodes << " "
                      << tt_hits << " "
                      << max_depth << " "
                      << static_cast<std::uint64_t>(nps) << "\n";
        }
        else if (mode_ == QUIET)
        {
            // Print only the move coordinates
            std::cout << move.row << " " << move.col << "\n";
        }
        else // INTERACTIVE
        {
            out << "Engine computed move in " << elapsed.count() << " seconds.\n";
            out << "Nodes searched: " << total_nodes << " (AB: " << ab_nodes << ", VCF: " << vcf_nodes << ")\n";
            out << "TT hits: " << tt_hits << ", NPS: " << static_cast<std::uint64_t>(nps) << "\n";
            out << "Max depth reached: " << max_depth << "\n";
            out << "Engine plays: ";
            std::cout << move.row << " " << move.col << "\n";
        }

        return move;
    }
}

