/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Heuristic.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AI/Heuristic.hpp"
#include "Game/Rules.hpp"
#include "Game/Constants.hpp"
#include "Debug.hpp"
#include <set>
#include <algorithm>

using namespace GameConstants;
using namespace EvalScores;
using namespace AIConfig;
using namespace Directions;

Heuristic::Heuristic() = default;
Heuristic::~Heuristic() = default;

int Heuristic::evaluatePosition(const Board& board, CellState player) {
    // BALANCED EVALUATION - Intelligence + Speed for <0.5s at depth 10

    CellState opponent = (player == BLACK) ? WHITE : BLACK;
    int score = 0;

    // 1. Check terminal states (mandatory)
    if (board.checkWin(player)) return WIN;
    if (board.checkWin(opponent)) return LOSS;

    // 2. TACTICAL: Count alignments (2, 3, 4 in line) - uses existing fast function
    int myAlignments = countAlignments(board, player);
    int opponentAlignments = countAlignments(board, opponent);
    score += myAlignments - opponentAlignments;

    // 3. STRATEGIC: Stone count (fast)
    int myStones = 0, opponentStones = 0;
    int size = board.getSize();

    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            CellState cell = board.getCell(x, y);
            if (cell == player) myStones++;
            else if (cell == opponent) opponentStones++;
        }
    }
    score += (myStones - opponentStones) * STONE_COUNT_MULTIPLIER;

    // 4. CAPTURES: Important bonus (fast lookup)
    int myCaptures = board.getCaptureCount(player);
    int opponentCaptures = board.getCaptureCount(opponent);
    score += (myCaptures - opponentCaptures) * CAPTURE_BONUS;

    return score;
}

std::vector<Position> Heuristic::getRelevantMoves(const Board& board, CellState player) {
    std::vector<Position> moves;

    // PRIORITY 1: Check for mandatory defensive moves (endgame capture rule)
    // If opponent has 5+ alignment that can be broken, we MUST play the capture moves
    std::vector<Position> mandatoryMoves = Rules::getMandatoryDefensiveMoves(board, player);
    if (!mandatoryMoves.empty()) {
        LOG_INFO("Heuristic returning " << mandatoryMoves.size() << " mandatory defensive moves only");
        return mandatoryMoves; // ONLY these moves are legal!
    }

    // No mandatory moves - generate normal move candidates
    std::set<std::pair<int,int>> moveSet;

    // Find all empty positions adjacent to existing pieces
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) != EMPTY) {
                // Add all adjacent empty positions
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        int nx = x + dx, ny = y + dy;
                        if (board.isValidMove(nx, ny)) {
                            moveSet.insert({nx, ny});
                        }
                    }
                }
            }
        }
    }

    // Convert set to vector with move scoring
    for (const auto& move : moveSet) {
        moves.push_back(Position(move.first, move.second));
    }

    // If no moves found (empty board), return center
    if (moves.empty()) {
        int center = board.getSize() / 2;
        moves.push_back(Position(center, center));
    }

    // Sort moves by INTELLIGENT priority for better Alpha-Beta pruning
    std::sort(moves.begin(), moves.end(), [&board, player, this](const Position& a, const Position& b) {
        return getMoveScore(board, a, player) > getMoveScore(board, b, player);
    });

    // SPEED OPTIMIZATION: Limit moves for <0.5s performance
    if (moves.size() > MAX_MOVES_PER_LEVEL) {
        moves.resize(MAX_MOVES_PER_LEVEL);
    }

    return moves;
}

bool Heuristic::hasAdjacentStone(const Board& board, int x, int y) {
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;

            int nx = x + dx, ny = y + dy;
            if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) != EMPTY) {
                return true;
            }
        }
    }
    return false;
}

int Heuristic::getMoveScore(const Board& board, const Position& pos, CellState player) {
    // ULTRA-FAST TACTICAL SCORING - NO expensive operations!
    // This runs for EVERY candidate move during sorting

    CellState opponent = (player == BLACK) ? WHITE : BLACK;
    int score = 0;

    // ONLY fast operations: count stones in 4 directions
    for (int d = 0; d < 4; d++) {
        int dx = VECTORS[d][0];
        int dy = VECTORS[d][1];

        // Count MY stones (limit scan to 4 for speed)
        int myStones = 0;
        for (int i = 1; i <= 4; i++) {
            int nx = pos.x + dx * i, ny = pos.y + dy * i;
            if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == player) myStones++;
            else break;
        }
        for (int i = 1; i <= 4; i++) {
            int nx = pos.x - dx * i, ny = pos.y - dy * i;
            if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == player) myStones++;
            else break;
        }

        // Simple scoring
        if (myStones >= 4) score += MOVE_WIN_OR_NEAR;
        else if (myStones == 3) score += MOVE_THREAT_4;
        else if (myStones == 2) score += MOVE_THREAT_3;
        else if (myStones == 1) score += MOVE_THREAT_2;

        // Count OPPONENT stones (limit scan to 4 for speed)
        int oppStones = 0;
        for (int i = 1; i <= 4; i++) {
            int nx = pos.x + dx * i, ny = pos.y + dy * i;
            if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == opponent) oppStones++;
            else break;
        }
        for (int i = 1; i <= 4; i++) {
            int nx = pos.x - dx * i, ny = pos.y - dy * i;
            if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == opponent) oppStones++;
            else break;
        }

        // Block bonus (slightly less than attack)
        if (oppStones >= 4) score += MOVE_BLOCK_4;
        else if (oppStones == 3) score += MOVE_BLOCK_3;
        else if (oppStones == 2) score += MOVE_BLOCK_2;
        else if (oppStones == 1) score += MOVE_BLOCK_1;
    }

    // Activity bonus (fast)
    int adjacent = 0;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            int nx = pos.x + dx, ny = pos.y + dy;
            if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) != EMPTY) {
                adjacent++;
            }
        }
    }
    score += adjacent * ACTIVITY_BONUS;

    // Center bonus (fast)
    int center = board.getSize() / 2;
    int distanceFromCenter = std::abs(pos.x - center) + std::abs(pos.y - center);
    score += std::max(0, MAX_CENTER_BONUS - distanceFromCenter);

    return score;
}

int Heuristic::countAlignments(const Board& board, CellState player) {
    // FAST alignment counting - essential for tactical play
    int score = 0;
    int size = board.getSize();

    // Scan board for player stones and check alignments
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            if (board.getCell(x, y) == player) {
                // Check 4 directions: horizontal, vertical, diagonal, anti-diagonal
                for (int d = 0; d < 4; d++) {
                    int dx = VECTORS[d][0];
                    int dy = VECTORS[d][1];

                    // Count forward only to avoid double counting
                    int count = 1;
                    int fx = x + dx, fy = y + dy;
                    while (fx >= 0 && fx < size && fy >= 0 && fy < size &&
                           board.getCell(fx, fy) == player) {
                        count++;
                        fx += dx;
                        fy += dy;
                    }

                    // Score based on alignment length (simplified scoring)
                    if (count >= 4) score += ALIGNMENT_4;
                    else if (count == 3) score += ALIGNMENT_3;
                    else if (count == 2) score += ALIGNMENT_2;
                }
            }
        }
    }

    return score;
}

// End of Heuristic.cpp - Dead code removed