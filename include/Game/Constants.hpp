#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

// Game constants
namespace GameConstants {
    constexpr int BOARD_SIZE = 19;
    constexpr int WIN_ALIGNMENT = 5;
    constexpr int WIN_CAPTURES = 10;
    constexpr int NUM_DIRECTIONS = 4;
}

// Directional vectors (used throughout the codebase)
namespace Directions {
    constexpr int VECTORS[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    // Index 0: Horizontal (right)
    // Index 1: Vertical (down)
    // Index 2: Diagonal (down-right)
    // Index 3: Anti-diagonal (down-left)
}

// AI evaluation scores
namespace EvalScores {
    // Terminal states (MinMax algorithm)
    constexpr int WIN = 100000;
    constexpr int LOSS = -100000;
    constexpr int WIN_VALUE = 500000;   // Used by MinMaxAI - MUST exceed any pattern score
    constexpr int LOSE_VALUE = -500000; // Used by MinMaxAI - MUST be lower than any pattern score

    // Pattern scores (used in position evaluation) - open vs blocked
    // Open = both ends free, Half-open = one end free, Blocked = both ends blocked
    constexpr int LIVE_FOUR = 100000;     // _XXXX_ : forced win next move
    constexpr int SPLIT_FOUR = 90000;    // _XX_XX_ or _X_XXX_ : gap-four, quasi-unstoppable
    constexpr int RUSH_FOUR = 5000;       // OXXXX_ : one way to complete
    constexpr int LIVE_THREE = 4000;      // _XXX__ : creates open-four next
    constexpr int STRETCH_THREE = 3500;   // _X_XX_ or _XX_X_ : gap-three, very dangerous
    constexpr int RUSH_THREE = 400;       // OXXX__ : only one dangerous extension
    constexpr int LIVE_TWO = 200;         // __XX__ : future potential
    constexpr int RUSH_TWO = 30;          // OXX___ : limited potential

    // Fork scores (multi-directional threats)
    constexpr int FORK_DOUBLE_FOUR = 200000;  // Two fours = instant win
    constexpr int FORK_FOUR_THREE = 150000;   // Four + open three = forced win
    constexpr int FORK_DOUBLE_THREE = 50000;  // Two open threes

    // Vulnerability
    constexpr int VULNERABLE_PAIR_PENALTY = 150; // Penalty per capturable pair created

    // Move ordering scores - Offensive (create threats)
    constexpr int MOVE_WIN_OR_NEAR = 10000;    // Win immediately or force win
    constexpr int MOVE_THREAT_4 = 1500;        // Create 4-alignment (very strong)
    constexpr int MOVE_THREAT_3 = 500;         // Create 3-alignment
    constexpr int MOVE_THREAT_2 = 100;         // Create 2-alignment

    // Move ordering scores - Defensive (block opponent)
    constexpr int MOVE_BLOCK_4 = 9000;         // MUST block 4-alignment
    constexpr int MOVE_BLOCK_3 = 2000;         // Block 3-alignment
    constexpr int MOVE_BLOCK_2 = 300;          // Block 2-alignment
    constexpr int MOVE_BLOCK_1 = 50;

    // Capture scores (tactical but not primary strategy)
    constexpr int CAPTURE_BONUS = 1000;        // Bonus per capture pair in evaluation
    constexpr int MOVE_CAPTURE = 1000;         // Move that captures opponent pair
    constexpr int MOVE_PREVENT_CAPTURE = 1000;  // Block opponent capture opportunity

    // Other bonuses
    constexpr int STONE_COUNT_MULTIPLIER = 5;
    constexpr int ACTIVITY_BONUS = 5;
    constexpr int MAX_CENTER_BONUS = 5;
}

// AI configuration
namespace AIConfig {
    constexpr int DEFAULT_DEPTH = 10;
    constexpr int MIN_DEPTH = 10;                  // Minimum depth guaranteed (subject requirement)
    constexpr int DEFAULT_TIME_LIMIT_MS = 490;    // Time budget per move (iterative deepening, applies after MIN_DEPTH)
    constexpr int TIME_CHECK_INTERVAL = 1000;     // Check time every N nodes
    constexpr int MAX_MOVES_PER_LEVEL = 5;       // Top N valid moves to keep
    constexpr int MAX_VALIDATION_ATTEMPTS = 15;  // Max candidates to validate before giving up
    constexpr int MAX_SCAN_DISTANCE = 4;         // For move scoring
}

#endif // CONSTANTS_HPP