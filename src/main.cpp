#include "cli_game.hpp"

int main(const int argc, char* argv[])
{
    Gomoku::Board::init();

    Gomoku::CLIGame().parse_args(argc, argv).run();

    return 0;
}
