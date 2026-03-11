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
    // PATTERN-AWARE EVALUATION - Distinguishes open vs blocked alignments
    // NOTE: Terminal state checks removed - handled by MinMaxAI::evaluateBoard()

    CellState opponent = (player == BLACK) ? WHITE : BLACK;
    int score = 0;

    // 1. TACTICAL: Pattern-aware alignment scoring (open vs blocked)
    int myPatterns = countPatterns(board, player);
    int opponentPatterns = countPatterns(board, opponent);
    // Slight defensive bias: opponent threats weighted 1.1x
    score += myPatterns - (opponentPatterns * 11 / 10);

    // 2. CAPTURES: Progressive bonus (fast lookup)
    int myCaptures = board.getCaptureCount(player);
    int opponentCaptures = board.getCaptureCount(opponent);

    int captureScore = 0;

    // My captures: exponential bonus when close to victory
    if (myCaptures >= 8) {
        captureScore += myCaptures * CAPTURE_BONUS * 5;
    } else if (myCaptures >= 6) {
        captureScore += myCaptures * CAPTURE_BONUS * 3;
    } else if (myCaptures >= 4) {
        captureScore += myCaptures * CAPTURE_BONUS * 2;
    } else {
        captureScore += myCaptures * CAPTURE_BONUS;
    }

    // Opponent captures: same progressive penalty
    if (opponentCaptures >= 8) {
        captureScore -= opponentCaptures * CAPTURE_BONUS * 5;
    } else if (opponentCaptures >= 6) {
        captureScore -= opponentCaptures * CAPTURE_BONUS * 3;
    } else if (opponentCaptures >= 4) {
        captureScore -= opponentCaptures * CAPTURE_BONUS * 2;
    } else {
        captureScore -= opponentCaptures * CAPTURE_BONUS;
    }

    score += captureScore;

    return score;
}

std::vector<Position> Heuristic::getRelevantMoves(const Board& board, CellState player, bool fastMode) {
    std::vector<Position> moves;

    // PRIORITY 1: Check for mandatory defensive moves (endgame capture rule)
    // ONLY at root level - too expensive for inner search nodes (O(n^4))
    if (!fastMode) {
        std::vector<Position> mandatoryMoves = Rules::getMandatoryDefensiveMoves(board, player);
        if (!mandatoryMoves.empty()) {
            LOG_INFO("Heuristic returning " << mandatoryMoves.size() << " mandatory defensive moves only");
            return mandatoryMoves;
        }
    }

    // Generate normal move candidates
    std::set<std::pair<int,int>> moveSet;

    // STEP 1: Generate candidates (FAST - only check if empty)
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) != EMPTY) {
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

    for (const auto& move : moveSet) {
        moves.push_back(Position(move.first, move.second));
    }

    if (moves.empty()) {
        int center = board.getSize() / 2;
        moves.push_back(Position(center, center));
    }

    // STEP 2: Pre-compute scores to avoid inconsistent ordering from random noise
    std::vector<std::pair<int, size_t>> scoredIndices;
    scoredIndices.reserve(moves.size());
    for (size_t i = 0; i < moves.size(); i++) {
        scoredIndices.push_back({getMoveScore(board, moves[i], player), i});
    }
    std::sort(scoredIndices.begin(), scoredIndices.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });
    std::vector<Position> sortedMoves;
    sortedMoves.reserve(moves.size());
    for (const auto& si : scoredIndices) {
        sortedMoves.push_back(moves[si.second]);
    }
    moves = std::move(sortedMoves);

    // STEP 3: Filter top candidates
    if (fastMode) {
        // FAST MODE: Skip expensive Rules::isValidMove (mandatory moves + double-three)
        // Just take top N moves that are empty cells (already guaranteed by moveSet)
        size_t count = std::min((size_t)MAX_MOVES_PER_LEVEL, moves.size());
        moves.resize(count);
        return moves;
    }

    // FULL MODE (root): Validate with Rules::isValidMove for double-three etc.
    std::vector<Position> validMoves;
    validMoves.reserve(MAX_MOVES_PER_LEVEL);

    size_t maxAttempts = std::min((size_t)MAX_VALIDATION_ATTEMPTS, moves.size());

    for (size_t i = 0; i < maxAttempts; i++) {
        const Position& move = moves[i];
        if (Rules::isValidMove(board, move.x, move.y, player)) {
            validMoves.push_back(move);
            if (validMoves.size() >= MAX_MOVES_PER_LEVEL) {
                break;
            }
        }
    }

    return validMoves;
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

    // NEW: Count adjacent stones to avoid isolated positions (captured stone spots)
    int adjacentMyStones = 0;
    int adjacentOpponentStones = 0;

    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            int nx = pos.x + dx, ny = pos.y + dy;
            if (board.isValidPosition(nx, ny)) {
                if (board.getCell(nx, ny) == player) adjacentMyStones++;
                if (board.getCell(nx, ny) == opponent) adjacentOpponentStones++;
            }
        }
    }

    // PENALTY for isolated positions (often just-captured spots with only 1 adjacent stone)
    int totalAdjacent = adjacentMyStones + adjacentOpponentStones;
    if (totalAdjacent == 0) {
        score -= 1000;  // Empty area = very bad
    } else if (totalAdjacent == 1) {
        score -= 500;   // Isolated (1 stone) = bad (likely captured stone)
    }

    // BONUS for dense areas (strategic crossroads)
    score += adjacentMyStones * 50;
    score += adjacentOpponentStones * 20;  // Also value blocking positions

    // ONLY fast operations: count stones in 4 directions
    for (int d = 0; d < 4; d++) {
        int dx = VECTORS[d][0];
        int dy = VECTORS[d][1];

        // Count MY stones (limit scan to 4 for speed) + track endpoints
        int myStones = 0;
        int lastPosX = pos.x, lastPosY = pos.y;
        int firstPosX = pos.x, firstPosY = pos.y;

        // Scan forward
        for (int i = 1; i <= 4; i++) {
            int nx = pos.x + dx * i, ny = pos.y + dy * i;
            if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == player) {
                myStones++;
                lastPosX = nx;
                lastPosY = ny;
            }
            else break;
        }
        // Scan backward
        for (int i = 1; i <= 4; i++) {
            int nx = pos.x - dx * i, ny = pos.y - dy * i;
            if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == player) {
                myStones++;
                firstPosX = nx;
                firstPosY = ny;
            }
            else break;
        }

        // ULTRA-FAST: Check only IMMEDIATE neighbors (distance 1)
        int beforeX = firstPosX - dx, beforeY = firstPosY - dy;
        int afterX = lastPosX + dx, afterY = lastPosY + dy;

        bool beforeEmpty = board.isValidPosition(beforeX, beforeY) &&
                           board.getCell(beforeX, beforeY) == EMPTY;
        bool afterEmpty = board.isValidPosition(afterX, afterY) &&
                          board.getCell(afterX, afterY) == EMPTY;

        // Calculate base score
        int baseScore = 0;
        if (myStones >= 4) {
            // 5+ in a row = WIN, regardless of blocking (subject: "5 or more is a win")
            // Return immediately with maximum score - no penalty can override this
            return 100000;
        }
        else if (myStones == 3) baseScore = MOVE_THREAT_4;
        else if (myStones == 2) baseScore = MOVE_THREAT_3;
        else if (myStones == 1) baseScore = MOVE_THREAT_2;

        // Apply simple penalty: immediate adjacency only (never for winning moves - already returned above)
        if (!beforeEmpty && !afterEmpty) {
            baseScore = 0; // Both sides immediately blocked
        } else if (!beforeEmpty || !afterEmpty) {
            baseScore = baseScore / 2; // One side blocked
        }

        score += baseScore;

        // Count OPPONENT stones (limit scan to 4 for speed) + track endpoints
        int oppStones = 0;
        int oppLastPosX = pos.x, oppLastPosY = pos.y;
        int oppFirstPosX = pos.x, oppFirstPosY = pos.y;

        // Scan forward
        for (int i = 1; i <= 4; i++) {
            int nx = pos.x + dx * i, ny = pos.y + dy * i;
            if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == opponent) {
                oppStones++;
                oppLastPosX = nx;
                oppLastPosY = ny;
            }
            else break;
        }
        // Scan backward
        for (int i = 1; i <= 4; i++) {
            int nx = pos.x - dx * i, ny = pos.y - dy * i;
            if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == opponent) {
                oppStones++;
                oppFirstPosX = nx;
                oppFirstPosY = ny;
            }
            else break;
        }

        // ULTRA-FAST: Check only IMMEDIATE neighbors (distance 1) for opponent
        int oppBeforeX = oppFirstPosX - dx, oppBeforeY = oppFirstPosY - dy;
        int oppAfterX = oppLastPosX + dx, oppAfterY = oppLastPosY + dy;

        bool oppBeforeEmpty = board.isValidPosition(oppBeforeX, oppBeforeY) &&
                              board.getCell(oppBeforeX, oppBeforeY) == EMPTY;
        bool oppAfterEmpty = board.isValidPosition(oppAfterX, oppAfterY) &&
                             board.getCell(oppAfterX, oppAfterY) == EMPTY;

        // Calculate block score
        int blockScore = 0;
        if (oppStones >= 4) {
            // Opponent has 5+ in a row if we don't block = MUST block immediately
            return 90000;
        }
        else if (oppStones == 3) blockScore = MOVE_BLOCK_3;
        else if (oppStones == 2) blockScore = MOVE_BLOCK_2;
        else if (oppStones == 1) blockScore = MOVE_BLOCK_1;

        // Apply simple penalty: immediate adjacency only
        // BUT: never penalize blocking a winning alignment (4+ opponent stones)
        if (oppStones < 4) {
            if (!oppBeforeEmpty && !oppAfterEmpty) {
                blockScore = 0; // Opponent blocked both sides - low priority
            } else if (!oppBeforeEmpty || !oppAfterEmpty) {
                blockScore = blockScore / 2; // One side blocked
            }
        }

        score += blockScore;
    }

    // ===== CAPTURE DETECTION (CRITICAL TACTICAL FEATURE) =====
    // Detect if this move would capture opponent stones
    // Pattern: X - O - O - X (playing at X captures the 2 O's)
    int captureScore = 0;
    for (int d = 0; d < 4; d++) {
        int dx = VECTORS[d][0];
        int dy = VECTORS[d][1];

        // Pattern: (pos) - O - O - X
        if (board.isValidPosition(pos.x + dx, pos.y + dy) &&
            board.isValidPosition(pos.x + 2*dx, pos.y + 2*dy) &&
            board.isValidPosition(pos.x + 3*dx, pos.y + 3*dy)) {
            if (board.getCell(pos.x + dx, pos.y + dy) == opponent &&
                board.getCell(pos.x + 2*dx, pos.y + 2*dy) == opponent &&
                board.getCell(pos.x + 3*dx, pos.y + 3*dy) == player) {
                captureScore += MOVE_CAPTURE;
            }
        }

        // Pattern: X - O - O - (pos)
        if (board.isValidPosition(pos.x - dx, pos.y - dy) &&
            board.isValidPosition(pos.x - 2*dx, pos.y - 2*dy) &&
            board.isValidPosition(pos.x - 3*dx, pos.y - 3*dy)) {
            if (board.getCell(pos.x - dx, pos.y - dy) == opponent &&
                board.getCell(pos.x - 2*dx, pos.y - 2*dy) == opponent &&
                board.getCell(pos.x - 3*dx, pos.y - 3*dy) == player) {
                captureScore += MOVE_CAPTURE;
            }
        }
    }
    // ADAPTIVE: Scale capture priority based on OUR capture count
    if (captureScore > 0) {
        int myCaptures = board.getCaptureCount(player);
        // Each capture takes 2 stones, win at 10. So 8+ = next capture wins!
        if (myCaptures >= 8) {
            captureScore *= 10; // WINNING CAPTURE - highest priority
        } else if (myCaptures >= 6) {
            captureScore *= 5;  // Very close to win
        } else if (myCaptures >= 4) {
            captureScore *= 3;  // Getting dangerous
        }
        // 0-3 captures: base score (1000)
    }
    score += captureScore;

    // ===== PREVENT OPPONENT CAPTURE (ADAPTIVE DEFENSE) =====
    // Detect if opponent could capture us if we don't play here
    // Defense priority scales with opponent's capture count (win at 10)
    int preventCaptureScore = 0;
    for (int d = 0; d < 4; d++) {
        int dx = VECTORS[d][0];
        int dy = VECTORS[d][1];

        // Pattern: O - X - X - (pos)
        if (board.isValidPosition(pos.x - dx, pos.y - dy) &&
            board.isValidPosition(pos.x - 2*dx, pos.y - 2*dy) &&
            board.isValidPosition(pos.x - 3*dx, pos.y - 3*dy)) {
            if (board.getCell(pos.x - dx, pos.y - dy) == player &&
                board.getCell(pos.x - 2*dx, pos.y - 2*dy) == player &&
                board.getCell(pos.x - 3*dx, pos.y - 3*dy) == opponent) {
                preventCaptureScore += MOVE_PREVENT_CAPTURE;
            }
        }

        // Pattern: (pos) - X - X - O
        if (board.isValidPosition(pos.x + dx, pos.y + dy) &&
            board.isValidPosition(pos.x + 2*dx, pos.y + 2*dy) &&
            board.isValidPosition(pos.x + 3*dx, pos.y + 3*dy)) {
            if (board.getCell(pos.x + dx, pos.y + dy) == player &&
                board.getCell(pos.x + 2*dx, pos.y + 2*dy) == player &&
                board.getCell(pos.x + 3*dx, pos.y + 3*dy) == opponent) {
                preventCaptureScore += MOVE_PREVENT_CAPTURE;
            }
        }
    }

    // ADAPTIVE: Scale defense priority based on opponent's capture count
    if (preventCaptureScore > 0) {
        int opponentCaptures = board.getCaptureCount(opponent);

        // Urgency multiplier - closer to 10 captures = higher priority
        if (opponentCaptures >= 8) {
            preventCaptureScore *= 5;  // CRITICAL: 8-9 captures (score ~3000)
        } else if (opponentCaptures >= 6) {
            preventCaptureScore *= 3;  // HIGH: 6-7 captures (score ~1800)
        } else if (opponentCaptures >= 4) {
            preventCaptureScore *= 2;  // MEDIUM: 4-5 captures (score ~1200)
        }
        // 0-3 captures: base score (600)
    }
    score += preventCaptureScore;

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

    // Random noise for varied games (only when enabled)
    if (noiseRange > 0) {
        std::uniform_int_distribution<int> dist(-noiseRange, noiseRange);
        score += dist(rng);
    }

    return score;
}

int Heuristic::countPatterns(const Board& board, CellState player) {
    // PATTERN-AWARE scoring: distinguishes live (open) vs rush (half-open) vs dead (blocked)
    // A "live" pattern has BOTH ends empty = much more dangerous
    // A "rush" pattern has ONE end empty = still threatening
    // A "dead" pattern has BOTH ends blocked = harmless, score 0

    int score = 0;
    int size = board.getSize();

    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            if (board.getCell(x, y) != player) continue;

            for (int d = 0; d < 4; d++) {
                int dx = VECTORS[d][0];
                int dy = VECTORS[d][1];

                // Only count forward to avoid double-counting
                // Skip if previous cell in this direction is also player (not start of group)
                int px = x - dx, py = y - dy;
                if (px >= 0 && px < size && py >= 0 && py < size &&
                    board.getCell(px, py) == player) {
                    continue; // Not the start of this alignment
                }

                // Count consecutive stones forward
                int count = 1;
                int fx = x + dx, fy = y + dy;
                while (fx >= 0 && fx < size && fy >= 0 && fy < size &&
                       board.getCell(fx, fy) == player) {
                    count++;
                    fx += dx;
                    fy += dy;
                }

                // Check ends: before the start and after the end
                // "before" = cell before (x,y) in this direction
                bool beforeOpen = (px >= 0 && px < size && py >= 0 && py < size &&
                                   board.getCell(px, py) == EMPTY);
                // "after" = cell after last stone (fx, fy)
                bool afterOpen = (fx >= 0 && fx < size && fy >= 0 && fy < size &&
                                  board.getCell(fx, fy) == EMPTY);

                int openEnds = (beforeOpen ? 1 : 0) + (afterOpen ? 1 : 0);

                // Score based on count + openness
                if (count >= 5) {
                    score += LIVE_FOUR; // Already won
                } else if (count == 4) {
                    if (openEnds == 2) score += LIVE_FOUR;      // _XXXX_ = forced win
                    else if (openEnds == 1) score += RUSH_FOUR; // OXXXX_ = one way
                    // openEnds == 0: dead four, score 0
                } else if (count == 3) {
                    if (openEnds == 2) score += LIVE_THREE;      // _XXX_ = very dangerous
                    else if (openEnds == 1) score += RUSH_THREE;  // OXXX_ = still a threat
                } else if (count == 2) {
                    if (openEnds == 2) score += LIVE_TWO;         // _XX_ = potential
                    else if (openEnds == 1) score += RUSH_TWO;    // OXX_ = limited
                }
            }
        }
    }

    return score;
}

int Heuristic::countAlignments(const Board& board, CellState player) {
    // Legacy function kept for compatibility - delegates to countPatterns
    return countPatterns(board, player);
}

// End of Heuristic.cpp - Dead code removed