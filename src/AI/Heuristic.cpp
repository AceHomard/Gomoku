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
#include <set>
#include <algorithm>

Heuristic::Heuristic() {
}

Heuristic::~Heuristic() {
}

int Heuristic::evaluatePosition(const Board& board, CellState player) {
    // BALANCED EVALUATION - Intelligence + Speed for <0.5s at depth 10

    CellState opponent = (player == BLACK) ? WHITE : BLACK;
    int score = 0;

    // 1. Check terminal states (mandatory)
    if (board.checkWin(player)) return +100000;
    if (board.checkWin(opponent)) return -100000;

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
    score += (myStones - opponentStones) * 5;

    // 4. CAPTURES: Important bonus (fast lookup)
    int myCaptures = board.getCaptureCount(player);
    int opponentCaptures = board.getCaptureCount(opponent);
    score += (myCaptures - opponentCaptures) * 200;

    return score;
}

std::vector<Position> Heuristic::getRelevantMoves(const Board& board, CellState player) {
    std::vector<Position> moves;

    // PRIORITY 1: Check for mandatory defensive moves (endgame capture rule)
    // If opponent has 5+ alignment that can be broken, we MUST play the capture moves
    std::vector<Position> mandatoryMoves = Rules::getMandatoryDefensiveMoves(board, player);
    if (!mandatoryMoves.empty()) {
        std::cout << "[Heuristic] Returning " << mandatoryMoves.size()
                  << " mandatory defensive moves only!" << std::endl;
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

    // SPEED OPTIMIZATION: Limit to 3 moves maximum for <0.5s
    if (moves.size() > 3) {
        moves.resize(3); // 3^10 = 59K nodes - fast with Alpha-Beta
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
    int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};

    for (int d = 0; d < 4; d++) {
        int dx = directions[d][0];
        int dy = directions[d][1];

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
        if (myStones >= 4) score += 10000;  // Win or near-win
        else if (myStones == 3) score += 1000;
        else if (myStones == 2) score += 100;
        else if (myStones == 1) score += 10;

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
        if (oppStones >= 4) score += 8000;  // Must block
        else if (oppStones == 3) score += 800;
        else if (oppStones == 2) score += 80;
        else if (oppStones == 1) score += 8;
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
    score += adjacent * 5;

    // Center bonus (fast)
    int center = board.getSize() / 2;
    int distanceFromCenter = std::abs(pos.x - center) + std::abs(pos.y - center);
    score += std::max(0, 5 - distanceFromCenter);

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
                int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};

                for (int d = 0; d < 4; d++) {
                    int dx = directions[d][0];
                    int dy = directions[d][1];

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
                    if (count >= 4) score += 1000;
                    else if (count == 3) score += 100;
                    else if (count == 2) score += 10;
                }
            }
        }
    }

    return score;
}

int Heuristic::detectCriticalThreats(const Board& board, CellState player, CellState opponent) {
    // ULTRA-FAST threat detection - NO board copies!
    // Simply give bonus for 4-in-row patterns
    int threatScore = 0;

    // Scan for player's 4-in-row threats
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) == player) {
                int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};

                for (int d = 0; d < 4; d++) {
                    int dx = directions[d][0];
                    int dy = directions[d][1];

                    int count = 1;
                    int fx = x + dx, fy = y + dy;
                    while (board.isValidPosition(fx, fy) && board.getCell(fx, fy) == player) {
                        count++;
                        fx += dx; fy += dy;
                    }

                    if (count == 4) threatScore += 5000;
                }
            }
        }
    }

    // Scan for opponent's 4-in-row threats (need to block)
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) == opponent) {
                int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};

                for (int d = 0; d < 4; d++) {
                    int dx = directions[d][0];
                    int dy = directions[d][1];

                    int count = 1;
                    int fx = x + dx, fy = y + dy;
                    while (board.isValidPosition(fx, fy) && board.getCell(fx, fy) == opponent) {
                        count++;
                        fx += dx; fy += dy;
                    }

                    if (count == 4) threatScore -= 8000; // Penalty if opponent has threat
                }
            }
        }
    }

    return threatScore;
}

bool Heuristic::creates4InRowThreat(const Board& board, const Position& pos, CellState player) {
    // Quick check if this move creates a 4-in-a-row threat
    int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};

    for (int d = 0; d < 4; d++) {
        int dx = directions[d][0];
        int dy = directions[d][1];

        int count = 1; // The move itself

        // Count stones in both directions
        int bx = pos.x - dx, by = pos.y - dy;
        while (board.isValidPosition(bx, by) && board.getCell(bx, by) == player) {
            count++;
            bx -= dx; by -= dy;
        }

        int fx = pos.x + dx, fy = pos.y + dy;
        while (board.isValidPosition(fx, fy) && board.getCell(fx, fy) == player) {
            count++;
            fx += dx; fy += dy;
        }

        // If this creates 4 in a row, it's a strong threat
        if (count >= 4) return true;
    }

    return false;
}

int Heuristic::evaluatePatternsFast(const Board& board, CellState player) {
    int score = 0;

    // FAST PATTERN EVALUATION - optimized for speed
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) == player) {
                // Check 4 directions: horizontal, vertical, diagonal, anti-diagonal
                int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};

                for (int d = 0; d < 4; d++) {
                    int dx = directions[d][0];
                    int dy = directions[d][1];

                    // Fast direction evaluation
                    score += evaluateDirectionFast(board, x, y, dx, dy, player);
                }
            }
        }
    }

    return score;
}

int Heuristic::evaluateDirectionFast(const Board& board, int x, int y, int dx, int dy, CellState player) {
    // Simplified fast evaluation - count consecutive stones only
    int count = 1; // Current stone

    // Count backwards
    int bx = x - dx, by = y - dy;
    while (board.isValidPosition(bx, by) && board.getCell(bx, by) == player) {
        count++;
        bx -= dx; by -= dy;
    }

    // Count forwards
    int fx = x + dx, fy = y + dy;
    while (board.isValidPosition(fx, fy) && board.getCell(fx, fy) == player) {
        count++;
        fx += dx; fy += dy;
    }

    // Simple scoring - avoid complex openness calculations for speed
    static int fastScores[] = {0, 1, 10, 100, 1000, 10000};

    if (count >= 5) return 10000; // Win
    if (count <= 0) return 0;

    int score = fastScores[count];

    // Only count once per pattern (when at leftmost position)
    int backCount = count - 1;
    for (int i = 1; i < count; i++) {
        if (board.isValidPosition(x - i*dx, y - i*dy) &&
            board.getCell(x - i*dx, y - i*dy) == player) {
            backCount--;
        } else break;
    }

    return (backCount == 0) ? score : 0;
}

int Heuristic::evaluateDirection(const Board& board, int x, int y, int dx, int dy, CellState player) {
    // Count consecutive stones in this direction
    int count = 1; // Current stone counts as 1

    // Count backwards
    int backCount = 0;
    int bx = x - dx, by = y - dy;
    while (board.isValidPosition(bx, by) && board.getCell(bx, by) == player) {
        backCount++;
        bx -= dx;
        by -= dy;
    }

    // Count forwards
    int forwardCount = 0;
    int fx = x + dx, fy = y + dy;
    while (board.isValidPosition(fx, fy) && board.getCell(fx, fy) == player) {
        forwardCount++;
        fx += dx;
        fy += dy;
    }

    count = 1 + backCount + forwardCount;

    // Check if the pattern has open ends
    bool openStart = board.isValidPosition(bx, by) && board.getCell(bx, by) == EMPTY;
    bool openEnd = board.isValidPosition(fx, fy) && board.getCell(fx, fy) == EMPTY;

    // Get score for this pattern
    int patternScore = getPatternScore(count, openStart, openEnd);

    // Only count each pattern once (avoid double counting)
    // We do this by only scoring when we're at the leftmost/topmost stone of the pattern
    if (backCount == 0) {
        return patternScore;
    }

    return 0; // Already counted by a previous stone
}

int Heuristic::getPatternScore(int length, bool openStart, bool openEnd) {
    // Base scores for different pattern lengths
    int baseScores[] = {0, 1, 10, 100, 1000, 10000}; // 0-5 stones

    if (length >= 5) {
        return 10000; // Winning pattern
    }

    int score = baseScores[length];

    // Apply bonuses based on openness
    if (openStart && openEnd) {
        // Completely open pattern (both ends free)
        score *= 3;
    } else if (openStart || openEnd) {
        // Semi-open pattern (one end free)
        score *= 2;
    }
    // Closed pattern (both ends blocked) gets no bonus

    return score;
}

// Stub implementations for Phase 3.2-3.4

int Heuristic::detectThreats(const Board& board, CellState player) {
    int threatScore = 0;
    std::vector<Position> moves = getRelevantMoves(board, player);

    for (const Position& move : moves) {
        // Test immediate win
        if (isImmediateWin(board, move, player)) {
            threatScore += 50000; // Immediate win is highest priority
            continue;
        }

        // Count winning threats (4-in-a-row with one gap)
        int winningThreats = countWinningThreats(board, move, player);
        if (winningThreats >= 2) {
            threatScore += 5000 * 2; // Double threat is devastating
        } else if (winningThreats == 1) {
            threatScore += 5000; // Single winning threat
        }

        // Check if this move blocks opponent's immediate win
        CellState opponent = (player == BLACK) ? WHITE : BLACK;
        if (isImmediateWin(board, move, opponent)) {
            threatScore += 1000; // Must block opponent's win
        }
    }

    return threatScore;
}

bool Heuristic::isImmediateWin(const Board& board, const Position& move, CellState player) {
    // Simulate the move
    Board tempBoard = board;
    if (!tempBoard.placePiece(move.x, move.y, player)) {
        return false; // Invalid move
    }

    // Handle captures if any
    std::vector<Position> captures = tempBoard.checkCaptures(move.x, move.y, player);
    tempBoard.executeCaptures(captures);

    // Check for alignment win
    if (tempBoard.checkWin(player)) {
        return true;
    }

    // Check for capture win (10 captures)
    if (tempBoard.checkCaptureWin(player)) {
        return true;
    }

    return false;
}

int Heuristic::countWinningThreats(const Board& board, const Position& move, CellState player) {
    // Simulate the move
    Board tempBoard = board;
    if (!tempBoard.placePiece(move.x, move.y, player)) {
        return 0; // Invalid move
    }

    int threats = 0;
    int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};

    // Check each direction for winning threats (4-in-a-row patterns)
    for (int d = 0; d < 4; d++) {
        int dx = directions[d][0];
        int dy = directions[d][1];

        // Count consecutive stones in this direction from the placed move
        int count = 1; // The move itself

        // Count backwards
        int bx = move.x - dx, by = move.y - dy;
        while (tempBoard.isValidPosition(bx, by) && tempBoard.getCell(bx, by) == player) {
            count++;
            bx -= dx;
            by -= dy;
        }

        // Count forwards
        int fx = move.x + dx, fy = move.y + dy;
        while (tempBoard.isValidPosition(fx, fy) && tempBoard.getCell(fx, fy) == player) {
            count++;
            fx += dx;
            fy += dy;
        }

        // If we have 4 in a row, check if it creates a winning threat
        if (count == 4) {
            // Check if both ends are open (or at least one creates unstoppable threat)
            bool backOpen = tempBoard.isValidPosition(bx, by) && tempBoard.getCell(bx, by) == EMPTY;
            bool frontOpen = tempBoard.isValidPosition(fx, fy) && tempBoard.getCell(fx, fy) == EMPTY;

            if (backOpen || frontOpen) {
                threats++; // This is a winning threat - opponent must block one end
            }
        }

        // Also check for open 3 that becomes winning threat
        if (count == 3) {
            bool backOpen = tempBoard.isValidPosition(bx, by) && tempBoard.getCell(bx, by) == EMPTY;
            bool frontOpen = tempBoard.isValidPosition(fx, fy) && tempBoard.getCell(fx, fy) == EMPTY;

            // Open 3 with space to extend both ways is a strong threat
            if (backOpen && frontOpen) {
                bool canExtendBack = tempBoard.isValidPosition(bx - dx, by - dy) &&
                                   tempBoard.getCell(bx - dx, by - dy) == EMPTY;
                bool canExtendFront = tempBoard.isValidPosition(fx + dx, fy + dy) &&
                                     tempBoard.getCell(fx + dx, fy + dy) == EMPTY;

                if (canExtendBack || canExtendFront) {
                    threats++; // This creates multiple winning opportunities
                }
            }
        }
    }

    return threats;
}

int Heuristic::evaluateCaptures(const Board& board, CellState player) {
    int score = 0;

    // 1. Evaluate current capture count
    int myCaptures = board.getCaptureCount(player);
    CellState opponent = (player == BLACK) ? WHITE : BLACK;
    int opponentCaptures = board.getCaptureCount(opponent);

    // Bonus for being close to capture win (10 captures)
    if (myCaptures >= 8) {
        score += 2000; // Very close to capture win
    } else if (myCaptures >= 6) {
        score += 500;  // Good progress
    } else {
        score += myCaptures * 50; // Base capture value
    }

    // Penalty for opponent captures
    if (opponentCaptures >= 8) {
        score -= 2500; // Opponent very close to win
    } else {
        score -= opponentCaptures * 60; // Each opponent capture hurts more
    }

    // 2. Evaluate capture opportunities
    std::vector<Position> moves = getRelevantMoves(board, player);
    for (const Position& move : moves) {
        // Check if this move creates captures
        std::vector<Position> captures = board.checkCaptures(move.x, move.y, player);
        if (!captures.empty()) {
            score += captures.size() * 25; // Bonus per stone captured

            // Extra bonus if the capture breaks an opponent alignment
            // (this is heuristic - we assume capturing in active areas is good)
            if (captures.size() >= 2) {
                score += 100; // Multiple captures likely disrupt opponent plans
            }
        }

        // 3. Check vulnerability to opponent captures
        std::vector<Position> vulnCaptures = board.checkCaptures(move.x, move.y, opponent);
        if (!vulnCaptures.empty()) {
            score -= 75; // Penalty for placing vulnerable stones
        }
    }

    return score;
}

int Heuristic::evaluatePositional(const Board& board, CellState player) {
    int score = 0;

    // Center control bonus (only when no critical threats exist)
    int centerBonus[10] = {50, 40, 30, 20, 10, 5, 2, 1, 0, 0};

    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) == player) {
                int distance = getDistanceFromCenter(x, y, board.getSize());
                if (distance < 10) {
                    score += centerBonus[distance];
                }
            }
        }
    }

    return score;
}

int Heuristic::getDistanceFromCenter(int x, int y, int boardSize) {
    int centerX = boardSize / 2;
    int centerY = boardSize / 2;
    return std::abs(x - centerX) + std::abs(y - centerY);
}