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

    // Alignment scores
    constexpr int ALIGNMENT_4 = 1000;
    constexpr int ALIGNMENT_3 = 100;
    constexpr int ALIGNMENT_2 = 10;

    // Move ordering scores
    constexpr int MOVE_WIN_OR_NEAR = 10000;
    constexpr int MOVE_THREAT_4 = 1000;
    constexpr int MOVE_THREAT_3 = 100;
    constexpr int MOVE_THREAT_2 = 10;

    constexpr int MOVE_BLOCK_4 = 8000;
    constexpr int MOVE_BLOCK_3 = 800;
    constexpr int MOVE_BLOCK_2 = 80;
    constexpr int MOVE_BLOCK_1 = 8;

    // Other bonuses
    constexpr int STONE_COUNT_MULTIPLIER = 5;
    constexpr int CAPTURE_BONUS = 200;
    constexpr int ACTIVITY_BONUS = 5;
    constexpr int MAX_CENTER_BONUS = 5;
}

// AI configuration
namespace AIConfig {
    constexpr int DEFAULT_DEPTH = 10;
    constexpr int MAX_MOVES_PER_LEVEL = 4;  // For performance <0.5s
    constexpr int MAX_SCAN_DISTANCE = 4;     // For move scoring
}

#endif // CONSTANTS_HPP