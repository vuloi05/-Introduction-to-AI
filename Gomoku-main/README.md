# AI Project - Gomoku Engine

A high-performance Gomoku (Five in a Row) game engine demonstrating advanced game tree search algorithms, bitboard optimization, and parallel computing techniques.

## Project Overview

This application is a project for the **Introduction to Artificial Intelligence** course at Hanoi University of Science and Technology (HUST). It implements a competitive Gomoku AI capable of superhuman-level play under strict time constraints, employing modern C++20 features and hardware-optimized operations.

### Key Features

- **Negamax Framework**: Enhanced with Principal Variation Search (PVS) and aggressive Alpha-Beta pruning
- **Victory by Continuous Four (VCF)**: Tactical solver for forced win detection (similar to chess mate solvers)
- **Bitboard Representation**: Hardware-accelerated operations using SIMD intrinsics (`_pdep_u64`, `popcount`, `countr_zero`)
- **Lock-free Transposition Table**: Zobrist hashing with 64-bit atomic operations for position caching
- **Lazy SMP Parallelization**: Multi-threaded search with cache-line-aligned thread data to prevent false sharing
- **Zero-Allocation Search**: Custom `inplace_vector` container eliminates heap allocations during search
- **Memory Optimization**: Linux Huge Pages (2MB) support for minimizing TLB misses

## Search Levels

The engine supports progressive search algorithm complexity:

| Level | Description                     | Techniques                                   |
|-------|---------------------------------|----------------------------------------------|
| `abp` | Alpha-Beta Pruning              | Basic minimax with pruning                   |
| `mo`  | Move Ordering                   | ABP + heuristic move prioritization          |
| `tt`  | Transposition Table             | MO + position caching with Zobrist hashing   |
| `pvs` | Principal Variation Search      | TT + null-window searches                    |
| `vcf` | VCF Search (default, strongest) | PVS + tactical forcing sequence solver       |

## Project Structure

```
Gomoku/
├── CMakeLists.txt              # CMake build configuration
├── README.md                   # This file
├── main.tex                    # Project report (LaTeX)
├── include/
│   ├── argparse/
│   │   └── argparse.hpp        # Command-line argument parser library
│   └── ryehl/
│       └── inplace_vector.hpp  # Custom inplace vector implementation
└── src/
    ├── main.cpp                # Entry point
    ├── cli_game.cpp            # Command-line game interface
    ├── cli_game.hpp
    ├── board.cpp               # Board representation and threat detection
    ├── board.hpp
    ├── engine.cpp              # Search algorithms (PVS, VCF)
    ├── engine.hpp
    ├── tptable.cpp             # Transposition table implementation
    ├── tptable.hpp
    ├── zobrist.cpp             # Zobrist hashing
    ├── zobrist.hpp
    ├── thread.cpp              # Thread management for Lazy SMP
    ├── thread.hpp
    ├── memory.cpp              # Memory allocation (Huge Pages support)
    ├── memory.hpp
    ├── types.hpp               # Type definitions (Coord, ThreatType, etc.)
    ├── misc.hpp                # Miscellaneous utilities
    └── inplace_vector.hpp      # Local inplace vector copy
```

## Technologies

### Core Stack
- **C++20**: Modern C++ with concepts, ranges, and coroutines
- **CMake 4.0+**: Build system and dependency management
- **Bitboard Techniques**: Compact state representation with SIMD
- **Zobrist Hashing**: Fast position fingerprinting for transposition tables

### Libraries
- **argparse**: Modern C++ command-line argument parser
- **inplace_vector**: Custom STL-compatible container for zero-allocation

### Hardware Optimizations
- **SIMD Instructions**: AVX2, BMI, BMI2, POPCNT (x86-64 only)
- **Huge Pages**: 2MB pages for TLB optimization (Linux)
- **Cache-line Alignment**: False sharing prevention in multi-threading

## Prerequisites

- **Compiler**: C++20 compliant compiler
  - GCC 10+ (recommended for Linux)
  - Clang 12+ (recommended for macOS)
  - MSVC 19.29+ (for Windows)
- **CMake**: Version 4.0 or higher
- **Operating System**: Linux (recommended), macOS, Windows

### Optional (for x86-64 Performance)

For optimal performance on x86-64 processors:
- AVX2 support
- BMI/BMI2 instruction sets
- POPCNT hardware instruction

> **Note**: The engine includes software fallback for platforms without BMI2 (e.g., ARM, older x86), ensuring full portability.

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/ryehlmarshmallow/Gomoku.git
cd Gomoku
```

### 2. Build the Project

#### Using CMake (Recommended)

```bash
# Create build directory
mkdir build && cd build

# Configure for Release build (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build the project
cmake --build . -j$(nproc)
```

#### Alternative: Direct Make

```bash
cd build
make -j$(nproc)
```

The executable will be generated as:
- Linux/macOS: `./Gomoku`
- Windows: `Gomoku.exe`

### 3. Run the Application

```bash
# From build directory
./Gomoku

# Or with options
./Gomoku -b engine -w engine -d 6
```

## Usage

### Basic Command

```bash
./Gomoku [options]
```

### Command-Line Arguments

The engine uses the `argparse` library for command-line argument parsing. All options support both short (`-x`) and long (`--xxx`) forms.

#### Player Configuration

| Option        | Description                                | Default  |
|---------------|--------------------------------------------|----------|
| `-b, --black` | Player type for black: `human` or `engine` | `human`  |
| `-w, --white` | Player type for white: `human` or `engine` | `engine` |

#### Game Mode

| Option       | Description                                       | Default       |
|--------------|---------------------------------------------------|---------------|
| `-m, --mode` | Game mode: `interactive`, `quiet`, or `benchmark` | `interactive` |

#### Engine Settings (Common)

| Option            | Description                                      | Default   |
|-------------------|--------------------------------------------------|-----------|
| `-d, --depth`     | Search depth for both engines                    | `4`       |
| `-t, --time`      | Time limit per move in seconds (0 = use depth)   | `0`       |
| `-T, --threads`   | Number of threads (-1 = all available cores)     | `-1`      |
| `-H, --hash`      | Transposition table size in MB                   | `64`      |
| `-l, --level`     | Search level: `abp`, `mo`, `tt`, `pvs`, or `vcf` | `vcf`     |
| `-c, --container` | Container type: `inplace`/`iv` or `vector`/`std` | `inplace` |

#### Per-Side Engine Settings

Override common settings for individual players:

| Option              | Description                      |
|---------------------|----------------------------------|
| `--black-depth`     | Search depth for black engine    |
| `--white-depth`     | Search depth for white engine    |
| `--black-time`      | Time limit for black engine      |
| `--white-time`      | Time limit for white engine      |
| `--black-threads`   | Thread count for black engine    |
| `--white-threads`   | Thread count for white engine    |
| `--black-hash`      | Hash table size for black engine |
| `--white-hash`      | Hash table size for white engine |
| `--black-level`     | Search level for black engine    |
| `--white-level`     | Search level for white engine    |
| `--black-container` | Container type for black engine  |
| `--white-container` | Container type for white engine  |

### Usage Examples

**Human vs. Engine (default):**
```bash
./Gomoku
```

**Engine vs. Engine with depth 6:**
```bash
./Gomoku -b engine -w engine -d 6
```

**Human (Black) vs Engine with 5 seconds per move:**
```bash
./Gomoku -b human -w engine -t 5
```

**Benchmark mode with custom settings:**
```bash
./Gomoku -b engine -w engine -m benchmark -d 8 -T 4 -H 128
```

**Engine vs. Engine with different depths:**
```bash
./Gomoku -b engine -w engine --black-depth 6 --white-depth 4
```

**Use all CPU cores with VCF search:**
```bash
./Gomoku -b engine -w engine -T -1 -l vcf
```

### Playing the Game

In interactive mode, enter moves in the format `row col` (0-indexed):
```
Your move (row col): 7 7
```

## Technical Details

### Board Representation

- **Board Size**: 16×16
- **Bitboard Storage**: Separate 16-bit masks for each row, column, diagonal, and anti-diagonal
- **Threat Detection**: Pre-computed lookup tables for all possible line configurations

### Search Algorithm

1. **Iterative Deepening**: Progressively increases search depth
2. **Principal Variation Search**: Narrow-window searches after first move
3. **VCF Search**: Specialized forcing sequence detector (like chess mate solvers)
4. **Move Ordering**: TT best move → forcing moves → heuristically scored moves

### Memory Optimization

- **Transposition Table**: Lock-free with Zobrist hashing
- **Huge Pages**: Reduces TLB misses on Linux systems
- **Inplace Vectors**: Eliminates heap allocations in hot paths

## Troubleshooting

### Compilation Errors

**Missing BMI2 instructions on older CPUs:**
```bash
# The engine includes software fallback, but if compilation fails:
cmake -DCMAKE_BUILD_TYPE=Release -DDISABLE_BMI2=ON ..
```

**CMake version issues:**
```bash
# Check your CMake version
cmake --version

# If too old, install newer version or adjust CMakeLists.txt
```

### Runtime Issues

**Segmentation fault on startup:**
- Ensure sufficient stack size for deep searches
- Check available memory for transposition table allocation

**Slow performance:**
- Verify Release build mode is used (`-DCMAKE_BUILD_TYPE=Release`)
- Check CPU supports SIMD instructions
- Increase thread count with `-T` option

**Huge Pages not working (Linux):**
```bash
# Check if huge pages are enabled
cat /proc/meminfo | grep Huge

# Enable huge pages
sudo sysctl -w vm.nr_hugepages=128
```

## Authors

**Group 01** — Introduction to Artificial Intelligence, Semester 20251

- **Dao Minh Tam**
- **Dinh Tuan Minh**
- **Dang Xuan Bach**
- **Truong Tue Minh**

**Instructor**: Dr. Dam Quang Tuan

## License

This project is created for educational purposes as part of the Introduction to Artificial Intelligence course at Hanoi University of Science and Technology.

## Acknowledgments

We would like to express our sincere gratitude to:

- **Dr. Dam Quang Tuan** — Our instructor, for guidance and support throughout this project.
- Allis, L. V. (1994) — For foundational work on game tree search algorithms.
- Czajka (2020) — For comprehensive threat classification in Gomoku.
- Marsland, T. A. (1983) — For Principal Variation Search algorithm.
- Intel Corporation — For SIMD instruction set documentation and optimization guides.
- The C++ community — For modern C++20 features and best practices.

