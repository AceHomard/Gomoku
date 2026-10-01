# Gomoku

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus)
![SFML 3](https://img.shields.io/badge/SFML-3.0.1-8CC445?logo=sfml)
![License: MIT](https://img.shields.io/badge/license-MIT-blue)

A Gomoku game with **Ninuki-renju rules** (captures, double-three ban, endgame capture) and a **Minimax / Alpha-Beta AI** searching up to 10 plies deep in about half a second per move, written in C++17 with an SFML graphical interface.

<!-- Add a screenshot or GIF of a game here, e.g. ![Gameplay](docs/gameplay.gif) -->

## Features

- **Three game modes**: Human vs Human (hotseat), Human vs AI (pick your color), AI vs AI
- **Full rule enforcement**: captures, forbidden double-threes, endgame capture rule
- **Move suggestion** in hotseat mode, computed by the AI
- **Move history and replay**: step backward and forward through the game
- **AI timer** showing the AI's thinking time for each move
- **Winning alignment highlight** and stone animations
- **AI debug visualizer**: an optional second window that renders the minimax search tree

## Rules

The game is played on a 19×19 board. Black moves first.

| Rule | Description |
|---|---|
| **Alignment win** | Line up 5 or more stones horizontally, vertically or diagonally. |
| **Capture** | Flanking exactly two opponent stones (`X O O X`) removes them from the board. |
| **Capture win** | Capturing 10 stones (5 pairs) wins the game. |
| **No double-three** | A move that creates two free-threes at once is forbidden, unless it also makes a capture. |
| **Endgame capture** | A five-in-a-row only wins if the opponent cannot break it with a capture on their next move. When they can, the capture is mandatory. |

## How the AI works

The AI (`src/AI/`) is a **Minimax search with Alpha-Beta pruning**, driven by iterative deepening within a time budget of about 0.5 s per move.

- **Iterative deepening**: the AI searches at depth 1, then 2, 3, … up to 10. When the time budget runs out, it plays the best move from the last fully completed depth. The best move from each depth is tried first at the next one.
- **Transposition table**: positions are identified by a Zobrist hash, updated with every move and including the capture counts. Positions already evaluated are reused instead of searched again.
- **Killer moves**: moves that caused an alpha-beta cutoff at a given depth are tried first in sibling positions, which makes cutoffs happen earlier.
- **Make / unmake moves**: the search updates a single board in place and reverts each move, including any captures, instead of copying the board at every node.
- **Candidate generation**: only empty cells next to existing stones are considered. Each candidate gets a quick tactical score (threats created, threats blocked, captures) and only the best ones are explored. This beam search keeps the tree small enough for a depth-10 search.
- **Pattern-based evaluation**: leaf positions are scored by recognizing classic Gomoku shapes in every direction (live four, split four, rush four, live three, stretched three, …) and multi-direction forks (double four, four-three, double three), together with capture count and vulnerable pairs.
- **Root-level shortcuts**: an immediately winning move is played without any search, and the forced defensive moves required by the endgame capture rule are applied before the search starts.
- **Opening book**: the first few moves are played instantly from predefined shapes.

All scoring constants live in [`include/Game/Constants.hpp`](include/Game/Constants.hpp).

### AI debug visualizer

Build with `make debug_visu` to open a second window next to the game. It shows the minimax tree of the deepest search pass for every AI move: nodes you can expand or collapse, scores, pruned branches, zoom and pan, plus a stats panel (nodes evaluated, cutoffs, search time).

### Benchmark

`make benchmark` builds a headless AI vs AI runner that reports win rates, time per move and depth reached:

```bash
make benchmark
./benchmark -n 10 -q      # 10 games, results only
./benchmark --noise 30    # more random variation between games (default: 15, 0 = deterministic)
```

## Getting started

### Requirements

- Linux with X11 (Ubuntu / Debian for `make deps`)
- `g++` with C++17 support, `cmake`, `make`, `wget`
- The DejaVu fonts (`fonts-dejavu-core`), usually installed by default

### Build and run

```bash
git clone https://github.com/AceHomard/Gomoku.git
cd Gomoku
make deps   # optional: install system libraries (Debian/Ubuntu, uses sudo)
make        # downloads and builds SFML 3.0.1 locally the first time, then builds the game
./Gomoku
```

SFML is downloaded into `lib/` and linked statically, so nothing needs to be installed system-wide besides the build tools.

| Make target | Description |
|---|---|
| `make` | Build the game |
| `make debug` | Build with debug symbols and logging |
| `make debug_visu` | Build with the AI debug visualizer window |
| `make benchmark` | Build the headless AI vs AI benchmark |
| `make re` | Full rebuild |
| `make fclean` | Remove build files, the binary and the downloaded SFML |

## Controls

| Key | Action |
|---|---|
| Mouse click | Place a stone |
| `1` / `2` / `3` | New game: Human vs Human / Human vs AI / AI vs AI |
| `P` or `Space` | Pause / resume |
| `R` | Pause during a game, restart after a game |
| `S` | Show a suggested move (Human vs Human) |
| `←` or `Z` / `→` or `Y` | Previous / next move in history |
| `Home` / `End` | Jump to the first / last move |
| `Enter` | Leave replay mode |
| `Esc` | Quit |

## Project structure

```
include/, src/
├── AI/        MinMaxAI (alpha-beta search) and Heuristic (move ordering and evaluation)
├── Game/      Board, Rules, Game loop, players (IPlayer → HumanPlayer / AIPlayer)
├── UI/        SFML rendering, timers, stone animations
└── Debug/     Minimax tree visualizer (only built with DEBUG_VISU)
```

## Team

This project was built by a team of two:

- **jgiampor** ([@AceHomard](https://github.com/AceHomard)): game rules (captures, double-three, endgame capture), move validation, AI search optimizations, move suggestion, UI features
- **glamazere** ([@quercyAP](https://github.com/quercyAP)): initial architecture, heuristic and move scoring, move history and replay system, AI debug visualizer, pattern-based evaluation, make/unmake move generation

## License

Released under the [MIT License](LICENSE).
