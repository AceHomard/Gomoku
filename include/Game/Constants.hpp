/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Constants.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/30 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
    constexpr int WIN_VALUE = 10000;   // Used by MinMaxAI
    constexpr int LOSE_VALUE = -10000; // Used by MinMaxAI

    // Alignment scores (used in position evaluation)
    constexpr int ALIGNMENT_4 = 1500;
    constexpr int ALIGNMENT_3 = 500;
    constexpr int ALIGNMENT_2 = 50;

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
    constexpr int MAX_MOVES_PER_LEVEL = 3;       // Top N valid moves to keep
    constexpr int MAX_VALIDATION_ATTEMPTS = 10;  // Max candidates to validate before giving up
    constexpr int MAX_SCAN_DISTANCE = 4;         // For move scoring
}

#endif // CONSTANTS_HPP