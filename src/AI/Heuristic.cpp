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

    // 3. DOUBLE WIN THREAT: capture victory + alignment threat synergy (O(1) checks)
    if (myCaptures >= 8 && myPatterns >= LIVE_THREE) score += 50000;
    else if (myCaptures >= 6 && myPatterns >= LIVE_FOUR) score += 30000;

    if (opponentCaptures >= 8 && opponentPatterns >= LIVE_THREE) score -= 50000;
    else if (opponentCaptures >= 6 && opponentPatterns >= LIVE_FOUR) score -= 30000;

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

    // Fork detection counters
    int myFours = 0, myOpenThrees = 0;
    int oppFours = 0, oppOpenThrees = 0;
    int totalOffensive = 0, totalDefensive = 0;

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

        // Fork classification: track multi-directional threats
        if (myStones >= 3 && (beforeEmpty || afterEmpty)) myFours++;
        else if (myStones == 2 && beforeEmpty && afterEmpty) myOpenThrees++;
        totalOffensive += baseScore;

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

        // Fork classification: opponent threats
        if (oppStones >= 3 && (oppBeforeEmpty || oppAfterEmpty)) oppFours++;
        else if (oppStones == 2 && oppBeforeEmpty && oppAfterEmpty) oppOpenThrees++;
        totalDefensive += blockScore;

        score += blockScore;
    }

    // FORK DETECTION - offensive (multi-directional threats = winning patterns)
    if (myFours >= 2) score += FORK_DOUBLE_FOUR;
    else if (myFours >= 1 && myOpenThrees >= 1) score += FORK_FOUR_THREE;
    else if (myOpenThrees >= 2) score += FORK_DOUBLE_THREE;

    // FORK DETECTION - defensive (block opponent forks)
    if (oppFours >= 2) score += FORK_DOUBLE_FOUR - 10000;
    else if (oppFours >= 1 && oppOpenThrees >= 1) score += FORK_FOUR_THREE - 10000;
    else if (oppOpenThrees >= 2) score += FORK_DOUBLE_THREE - 5000;

    // DUAL-PURPOSE BONUS: moves that block AND threaten simultaneously
    if (totalOffensive > MOVE_THREAT_3 && totalDefensive > MOVE_BLOCK_2)
        score += totalDefensive / 2;

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

    // ===== VULNERABLE PAIR AVOIDANCE =====
    // Penalize moves that create capturable pairs (XX) - Gomoku Ninuki error #1
    {
        int oppCaptures = board.getCaptureCount(opponent);
        int vulnPenalty = VULNERABLE_PAIR_PENALTY;
        if (oppCaptures >= 6) vulnPenalty *= 3;
        else if (oppCaptures >= 4) vulnPenalty *= 2;

        for (int d = 0; d < 4; d++) {
            int ddx = VECTORS[d][0];
            int ddy = VECTORS[d][1];

            // Check if placing here creates a pair with neighbor in +dir
            int nx = pos.x + ddx, ny = pos.y + ddy;
            if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == player) {
                // Pair would exist at pos-(nx,ny). Check if capturable:
                int bx = pos.x - ddx, by = pos.y - ddy; // before pos
                int ax = nx + ddx, ay = ny + ddy;         // after neighbor
                // Capturable if opponent_before + empty_after or empty_before + opponent_after
                bool capt1 = board.isValidPosition(bx, by) && board.getCell(bx, by) == opponent &&
                             board.isValidPosition(ax, ay) && board.getCell(ax, ay) == EMPTY;
                bool capt2 = board.isValidPosition(bx, by) && board.getCell(bx, by) == EMPTY &&
                             board.isValidPosition(ax, ay) && board.getCell(ax, ay) == opponent;
                if (capt1 || capt2) score -= vulnPenalty;
            }

            // Check if placing here creates a pair with neighbor in -dir
            nx = pos.x - ddx; ny = pos.y - ddy;
            if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == player) {
                int bx = nx - ddx, by = ny - ddy;         // before neighbor
                int ax = pos.x + ddx, ay = pos.y + ddy;   // after pos
                bool capt1 = board.isValidPosition(bx, by) && board.getCell(bx, by) == opponent &&
                             board.isValidPosition(ax, ay) && board.getCell(ax, ay) == EMPTY;
                bool capt2 = board.isValidPosition(bx, by) && board.getCell(bx, by) == EMPTY &&
                             board.isValidPosition(ax, ay) && board.getCell(ax, ay) == opponent;
                if (capt1 || capt2) score -= vulnPenalty;
            }
        }
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

    // Random noise for varied games (only when enabled)
    if (noiseRange > 0) {
        std::uniform_int_distribution<int> dist(-noiseRange, noiseRange);
        score += dist(rng);
    }

    return score;
}

int Heuristic::countPatterns(const Board& board, CellState player) {
    int score = 0;
    int size = board.getSize();

    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            if (board.getCell(x, y) != player) continue;

            for (int d = 0; d < 4; d++) {
                int dx = VECTORS[d][0];
                int dy = VECTORS[d][1];

                // Only count forward to avoid double-counting
                int px = x - dx, py = y - dy;
                if (px >= 0 && px < size && py >= 0 && py < size &&
                    board.getCell(px, py) == player) {
                    continue;
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

                // Check ends
                bool beforeOpen = (px >= 0 && px < size && py >= 0 && py < size &&
                                   board.getCell(px, py) == EMPTY);
                bool afterOpen = (fx >= 0 && fx < size && fy >= 0 && fy < size &&
                                  board.getCell(fx, fy) == EMPTY);
                int openEnds = (beforeOpen ? 1 : 0) + (afterOpen ? 1 : 0);

                // Score consecutive patterns
                int consecScore = 0;
                if (count >= 5) {
                    consecScore = LIVE_FOUR;
                } else if (count == 4) {
                    if (openEnds == 2) consecScore = LIVE_FOUR;
                    else if (openEnds == 1) consecScore = RUSH_FOUR;
                } else if (count == 3) {
                    if (openEnds == 2) consecScore = LIVE_THREE;
                    else if (openEnds == 1) consecScore = RUSH_THREE;
                } else if (count == 2) {
                    if (openEnds == 2) consecScore = LIVE_TWO;
                    else if (openEnds == 1) consecScore = RUSH_TWO;
                }

                // GAP DETECTION: scan 5-cell and 6-cell windows from start position
                // to catch patterns like X_XXX, XX_XX, X_XX_, _XX_X
                int gapScore = 0;

                // 5-cell window starting from (x,y) in direction (dx,dy)
                {
                    int pCount = 0, eCount = 0, bCount = 0;
                    for (int k = 0; k < 5; k++) {
                        int wx = x + k * dx, wy = y + k * dy;
                        if (!board.isValidPosition(wx, wy)) { bCount++; continue; }
                        CellState c = board.getCell(wx, wy);
                        if (c == player) pCount++;
                        else if (c == EMPTY) eCount++;
                        else bCount++;
                    }

                    if (bCount == 0 && eCount >= 1) {
                        // Check outer ends of the 5-cell window
                        bool wBefore = beforeOpen; // cell before window = cell before start
                        int wax = x + 5 * dx, way = y + 5 * dy;
                        bool wAfter = board.isValidPosition(wax, way) &&
                                      board.getCell(wax, way) == EMPTY;

                        if (pCount == 4 && eCount == 1) {
                            // Gap-four: 4 player + 1 gap in 5 cells
                            if (wBefore && wAfter) gapScore = std::max(gapScore, SPLIT_FOUR);
                            else if (wBefore || wAfter) gapScore = std::max(gapScore, RUSH_FOUR);
                        } else if (pCount == 3 && eCount == 2) {
                            // Gap-three: 3 player + 2 gaps in 5 cells
                            if (wBefore || wAfter) gapScore = std::max(gapScore, STRETCH_THREE);
                            else gapScore = std::max(gapScore, RUSH_THREE);
                        }
                    }
                }

                // 6-cell window for wider split patterns like _XX_XX_
                {
                    int pCount = 0, eCount = 0, bCount = 0;
                    for (int k = 0; k < 6; k++) {
                        int wx = x + k * dx, wy = y + k * dy;
                        if (!board.isValidPosition(wx, wy)) { bCount++; continue; }
                        CellState c = board.getCell(wx, wy);
                        if (c == player) pCount++;
                        else if (c == EMPTY) eCount++;
                        else bCount++;
                    }

                    if (bCount == 0 && pCount == 4 && eCount == 2) {
                        // 4 player + 2 gaps in 6 cells = strong split pattern
                        bool wBefore = beforeOpen;
                        int wax = x + 6 * dx, way = y + 6 * dy;
                        bool wAfter = board.isValidPosition(wax, way) &&
                                      board.getCell(wax, way) == EMPTY;
                        if (wBefore || wAfter) gapScore = std::max(gapScore, SPLIT_FOUR);
                    }
                }

                // Take the best score between consecutive and gap patterns
                score += std::max(consecScore, gapScore);
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