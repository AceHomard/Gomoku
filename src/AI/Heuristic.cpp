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
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <set>

// Static initialization
const std::array<int, 9> Heuristic::PATTERN_VALUES = {
    50000,  // FIVE (win)
    5000,   // OPEN_FOUR
    1000,   // CLOSED_FOUR  
    500,    // OPEN_THREE
    50,     // CLOSED_THREE
    20,     // OPEN_TWO
    5,      // CLOSED_TWO
    1,      // SINGLE
    0       // INVALID
};

const std::array<int, 4> Heuristic::DIRECTION_VECTORS_X = {1, 0, 1, 1};
const std::array<int, 4> Heuristic::DIRECTION_VECTORS_Y = {0, 1, 1, -1};

const int Heuristic::CAPTURE_VALUE = 200;
const int Heuristic::WIN_VALUE = 50000;
const int Heuristic::POSITION_WEIGHT = 2;
const int Heuristic::THREAT_MULTIPLIER = 3;
const int Heuristic::MOBILITY_WEIGHT = 1;

Heuristic::Heuristic() {
    // Initialize evaluation cache
    evaluationCache.resize(BOARD_SIZE, std::vector<int>(BOARD_SIZE, -1));
}

Heuristic::~Heuristic() {
}

int Heuristic::evaluatePosition(const Board& board, CellState player) {
    // OPTIMIZATION: Fast evaluation with early termination
    
    // Check for immediate wins/losses first (fastest check)
    if (board.checkWin(player) || board.checkCaptureWin(player)) {
        return WIN_VALUE;
    }
    
    CellState opponent = (player == BLACK) ? WHITE : BLACK;
    if (board.checkWin(opponent) || board.checkCaptureWin(opponent)) {
        return -WIN_VALUE;
    }
    
    int score = 0;
    
    // OPTIMIZATION: Simplified evaluation for speed
    // Focus only on most important factors
    score += evaluatePatterns(board, player);           // Most important
    score += evaluateCaptures(board, player);           // Important for Gomoku
    
    // Skip expensive evaluations in deep search
    static int evalDepth = 0;
    evalDepth++;
    if (evalDepth % 4 == 0) {  // Only do full eval every 4th time
        score += evaluateThreats(board, player) * THREAT_MULTIPLIER;
        score += evaluatePositional(board, player) * POSITION_WEIGHT / 2; // Reduced weight
    }
    evalDepth--;
    
    return score;
}

int Heuristic::evaluatePatterns(const Board& board, CellState player) {
    std::vector<Pattern> patterns = findAllPatterns(board, player);
    std::vector<Pattern> opponentPatterns = findAllPatterns(board, (player == BLACK) ? WHITE : BLACK);
    
    int score = 0;
    
    // Evaluate own patterns
    for (const Pattern& pattern : patterns) {
        score += pattern.value;
        
        // Bonus for multiple open threes/fours (potential win threats)
        if (pattern.type == OPEN_THREE || pattern.type == OPEN_FOUR) {
            score += pattern.value / 4;
        }
    }
    
    // Penalize opponent patterns
    for (const Pattern& pattern : opponentPatterns) {
        score -= pattern.value;
        
        // Extra penalty for opponent threats
        if (pattern.type == OPEN_FOUR) {
            score -= pattern.value / 2; // Must block immediately
        } else if (pattern.type == OPEN_THREE) {
            score -= pattern.value / 3; // Serious threat
        }
    }
    
    return score;
}

int Heuristic::evaluateCaptures(const Board& board, CellState player) {
    int score = 0;
    
    // Direct capture count evaluation
    int playerCaptures = board.getCaptureCount(player);
    int opponentCaptures = board.getCaptureCount((player == BLACK) ? WHITE : BLACK);
    
    score += playerCaptures * CAPTURE_VALUE;
    score -= opponentCaptures * CAPTURE_VALUE;
    
    // Bonus for being close to capture win (5 pairs)
    if (playerCaptures >= 8) {
        score += 1000; // Very close to capture win
    } else if (playerCaptures >= 6) {
        score += 500;  // Close to capture win
    }
    
    // Penalty if opponent is close to capture win
    if (opponentCaptures >= 8) {
        score -= 1000;
    } else if (opponentCaptures >= 6) {
        score -= 500;
    }
    
    return score;
}

int Heuristic::evaluateThreats(const Board& board, CellState player) {
    std::vector<Threat> playerThreats = findThreats(board, player);
    std::vector<Threat> opponentThreats = findThreats(board, (player == BLACK) ? WHITE : BLACK);
    
    int score = 0;
    
    // Evaluate player threats
    for (const Threat& threat : playerThreats) {
        score += threat.priority;
        
        // Multiple threats are exponentially better
        if (threat.type == OPEN_FOUR) {
            score += 2000; // Almost guaranteed win
        } else if (threat.type == OPEN_THREE) {
            score += 300;
        }
    }
    
    // Penalize opponent threats
    for (const Threat& threat : opponentThreats) {
        score -= threat.priority;
        
        if (threat.type == OPEN_FOUR) {
            score -= 3000; // Must defend immediately
        } else if (threat.type == OPEN_THREE) {
            score -= 400;
        }
    }
    
    // Bonus for multiple threats (tactical combinations)
    if (playerThreats.size() >= 2) {
        score += 200 * (playerThreats.size() - 1);
    }
    
    // Penalty for facing multiple opponent threats
    if (opponentThreats.size() >= 2) {
        score -= 300 * (opponentThreats.size() - 1);
    }
    
    return score;
}

int Heuristic::evaluatePositional(const Board& board, CellState player) {
    int score = 0;
    
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) == player) {
                score += getPositionalScore(x, y);
                
                // Bonus for pieces that support each other
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx == 0 && dy == 0) continue;
                        
                        int nx = x + dx, ny = y + dy;
                        if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == player) {
                            score += 5; // Connectivity bonus
                        }
                    }
                }
            }
        }
    }
    
    return score;
}

int Heuristic::evaluateMobility(const Board& board, CellState player) {
    // Count available moves near existing stones
    std::vector<Position> relevantMoves = getRelevantMoves(board);
    
    int score = relevantMoves.size() * 2;
    
    // Bonus for moves that create multiple options
    for (const Position& move : relevantMoves) {
        Board tempBoard = board;
        if (tempBoard.placePiece(move.x, move.y, player)) {
            // Count how many new tactical possibilities this creates
            int newOptions = 0;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    
                    int nx = move.x + dx, ny = move.y + dy;
                    if (tempBoard.isValidMove(nx, ny)) {
                        newOptions++;
                    }
                }
            }
            score += newOptions;
        }
    }
    
    return score;
}

std::vector<Pattern> Heuristic::findAllPatterns(const Board& board, CellState player) {
    std::vector<Pattern> patterns;
    
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) == player) {
                for (int dir = 0; dir < 4; dir++) {
                    Pattern pattern = analyzePattern(board, x, y, dir, player);
                    if (pattern.type != INVALID && pattern.length > 1) {
                        patterns.push_back(pattern);
                    }
                }
            }
        }
    }
    
    return patterns;
}

Pattern Heuristic::analyzePattern(const Board& board, int x, int y, int direction, CellState player) {
    int dx = DIRECTION_VECTORS_X[direction];
    int dy = DIRECTION_VECTORS_Y[direction];
    
    bool openStart, openEnd;
    int length = scanDirection(board, x, y, dx, dy, player, openStart, openEnd);
    
    PatternType type = classifyPattern(length, openStart, openEnd);
    int value = getPatternValue(type);
    
    return Pattern(type, length, openStart, openEnd, value);
}

PatternType Heuristic::classifyPattern(int length, bool openStart, bool openEnd) {
    if (length >= 5) {
        return FIVE;
    } else if (length == 4) {
        if (openStart && openEnd) {
            return OPEN_FOUR;
        } else if (openStart || openEnd) {
            return CLOSED_FOUR;
        }
    } else if (length == 3) {
        if (openStart && openEnd) {
            return OPEN_THREE;
        } else if (openStart || openEnd) {
            return CLOSED_THREE;
        }
    } else if (length == 2) {
        if (openStart && openEnd) {
            return OPEN_TWO;
        } else if (openStart || openEnd) {
            return CLOSED_TWO;
        }
    } else if (length == 1) {
        return SINGLE;
    }
    
    return INVALID;
}

std::vector<Threat> Heuristic::findThreats(const Board& board, CellState player) {
    std::vector<Threat> threats;
    
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.isValidMove(x, y)) {
                Board tempBoard = board;
                if (tempBoard.placePiece(x, y, player)) {
                    // Check what patterns this move would create
                    for (int dir = 0; dir < 4; dir++) {
                        Pattern pattern = analyzePattern(tempBoard, x, y, dir, player);
                        
                        if (pattern.type == OPEN_FOUR || pattern.type == CLOSED_FOUR) {
                            int priority = (pattern.type == OPEN_FOUR) ? 1000 : 500;
                            threats.emplace_back(Position(x, y), pattern.type, priority, dir);
                        } else if (pattern.type == OPEN_THREE) {
                            threats.emplace_back(Position(x, y), pattern.type, 200, dir);
                        }
                    }
                }
            }
        }
    }
    
    // Sort threats by priority
    std::sort(threats.begin(), threats.end(), 
              [](const Threat& a, const Threat& b) { return a.priority > b.priority; });
    
    return threats;
}

std::vector<Threat> Heuristic::findImmediateThreats(const Board& board, CellState player) {
    std::vector<Threat> allThreats = findThreats(board, player);
    std::vector<Threat> immediateThreats;
    
    for (const Threat& threat : allThreats) {
        if (threat.type == OPEN_FOUR || threat.type == CLOSED_FOUR) {
            immediateThreats.push_back(threat);
        }
    }
    
    return immediateThreats;
}

std::vector<Position> Heuristic::findWinningMoves(const Board& board, CellState player) {
    std::vector<Position> winningMoves;
    
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.isValidMove(x, y)) {
                Board tempBoard = board;
                if (tempBoard.placePiece(x, y, player)) {
                    // Execute any captures
                    std::vector<Position> captures = tempBoard.checkCaptures(x, y, player);
                    tempBoard.executeCaptures(captures);
                    
                    // Check for win conditions
                    if (tempBoard.checkWin(player) || tempBoard.checkCaptureWin(player)) {
                        winningMoves.push_back(Position(x, y));
                    }
                }
            }
        }
    }
    
    return winningMoves;
}

std::vector<Position> Heuristic::findDefensiveMoves(const Board& board, CellState player) {
    CellState opponent = (player == BLACK) ? WHITE : BLACK;
    return findWinningMoves(board, opponent);
}

int Heuristic::evaluateMove(const Board& board, const Position& move, CellState player) {
    if (!board.isValidMove(move.x, move.y)) {
        return -10000;
    }
    
    int score = 0;
    
    // Create temporary board
    Board tempBoard = board;
    if (!tempBoard.placePiece(move.x, move.y, player)) {
        return -10000;
    }
    
    // Check for immediate win
    if (tempBoard.checkWin(player)) {
        return 50000;
    }
    
    // Evaluate captures
    score += evaluateMoveCaptures(tempBoard, move, player);
    
    // Evaluate patterns created
    for (int dir = 0; dir < 4; dir++) {
        Pattern pattern = analyzePattern(tempBoard, move.x, move.y, dir, player);
        score += pattern.value;
    }
    
    // Evaluate threats
    score += evaluateMoveThreats(tempBoard, move, player);
    
    // Positional value
    score += getPositionalScore(move.x, move.y);
    
    return score;
}

int Heuristic::evaluateMoveCaptures(const Board& board, const Position& move, CellState player) {
    std::vector<Position> captures = board.checkCaptures(move.x, move.y, player);
    return captures.size() * CAPTURE_VALUE;
}

int Heuristic::evaluateMoveThreats(const Board& board, const Position& move, CellState player) {
    int score = 0;
    
    // Check if this move creates threats
    std::vector<Threat> newThreats = findThreats(board, player);
    for (const Threat& threat : newThreats) {
        if (threat.position == move) {
            score += threat.priority / 2; // Partial credit for creating the threat
        }
    }
    
    return score;
}

std::vector<Position> Heuristic::getRelevantMoves(const Board& board) {
    std::vector<Position> moves;
    std::set<std::pair<int,int>> moveSet;
    
    // OPTIMIZATION: Limit search radius and use early termination
    int stonesFound = 0;
    for (int x = 0; x < board.getSize() && stonesFound < 50; x++) {
        for (int y = 0; y < board.getSize() && stonesFound < 50; y++) {
            if (board.getCell(x, y) != EMPTY) {
                stonesFound++;
                // Reduced to 3x3 area for performance (radius=1)
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        int nx = x + dx, ny = y + dy;
                        if (board.isValidMove(nx, ny)) {
                            moveSet.insert({nx, ny});
                            // Early exit if we have enough moves
                            if (moveSet.size() >= 20) goto done;
                        }
                    }
                }
            }
        }
    }
    done:
    
    // Convert set to vector with size limit
    moves.reserve(std::min(static_cast<size_t>(20), moveSet.size()));
    int count = 0;
    for (const auto& move : moveSet) {
        moves.push_back(Position(move.first, move.second));
        if (++count >= 20) break; // Hard limit
    }
    
    // If no moves found (empty board), return smaller center area
    if (moves.empty()) {
        int center = BOARD_SIZE / 2;
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int x = center + dx, y = center + dy;
                if (board.isValidMove(x, y)) {
                    moves.push_back(Position(x, y));
                }
            }
        }
    }
    
    return moves;
}

bool Heuristic::isOpeningPhase(const Board& board) {
    int stoneCount = 0;
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) != EMPTY) {
                stoneCount++;
            }
        }
    }
    return stoneCount < 8;
}

bool Heuristic::isEndgamePhase(const Board& board) {
    int stoneCount = 0;
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) != EMPTY) {
                stoneCount++;
            }
        }
    }
    return stoneCount > (BOARD_SIZE * BOARD_SIZE) / 2;
}

int Heuristic::evaluateOpening(const Board& board, CellState player) {
    int score = 0;
    int center = BOARD_SIZE / 2;
    
    // Prefer center control in opening
    for (int x = center - 2; x <= center + 2; x++) {
        for (int y = center - 2; y <= center + 2; y++) {
            if (board.isValidPosition(x, y) && board.getCell(x, y) == player) {
                int distance = abs(x - center) + abs(y - center);
                score += (5 - distance) * 10;
            }
        }
    }
    
    return score;
}

int Heuristic::evaluateEndgame(const Board& board, CellState player) {
    // In endgame, focus on forcing wins and preventing opponent wins
    int score = 0;
    
    std::vector<Threat> threats = findThreats(board, player);
    std::vector<Threat> opponentThreats = findThreats(board, (player == BLACK) ? WHITE : BLACK);
    
    // Heavily weight immediate tactical threats
    for (const Threat& threat : threats) {
        if (threat.type == OPEN_FOUR) {
            score += 5000;
        } else if (threat.type == OPEN_THREE) {
            score += 1000;
        }
    }
    
    for (const Threat& threat : opponentThreats) {
        if (threat.type == OPEN_FOUR) {
            score -= 6000; // Must defend
        } else if (threat.type == OPEN_THREE) {
            score -= 1200;
        }
    }
    
    return score;
}

// Helper function implementations
int Heuristic::scanDirection(const Board& board, int x, int y, int dx, int dy, CellState player, 
                            bool& openStart, bool& openEnd) {
    int count = 1; // Count the current piece
    
    // Scan in positive direction
    int nx = x + dx, ny = y + dy;
    while (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == player) {
        count++;
        nx += dx;
        ny += dy;
    }
    
    // Check if end is open
    openEnd = board.isValidPosition(nx, ny) && board.getCell(nx, ny) == EMPTY;
    
    // Scan in negative direction
    nx = x - dx;
    ny = y - dy;
    while (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == player) {
        count++;
        nx -= dx;
        ny -= dy;
    }
    
    // Check if start is open
    openStart = board.isValidPosition(nx, ny) && board.getCell(nx, ny) == EMPTY;
    
    return count;
}

int Heuristic::getPatternValue(PatternType type) {
    if (type < PATTERN_VALUES.size()) {
        return PATTERN_VALUES[type];
    }
    return 0;
}

int Heuristic::getPositionalScore(int x, int y) {
    int center = BOARD_SIZE / 2;
    int centerDistance = abs(x - center) + abs(y - center);
    
    // Prefer center positions, but not too heavily
    int baseScore = std::max(0, center - centerDistance);
    
    // Slight bonus for positions that are not on the edge
    if (x > 0 && x < BOARD_SIZE - 1 && y > 0 && y < BOARD_SIZE - 1) {
        baseScore += 2;
    }
    
    return baseScore;
}

bool Heuristic::isValidAndEmpty(const Board& board, int x, int y) {
    return board.isValidPosition(x, y) && board.getCell(x, y) == EMPTY;
}

bool Heuristic::isValidAndPlayer(const Board& board, int x, int y, CellState player) {
    return board.isValidPosition(x, y) && board.getCell(x, y) == player;
}

std::string Heuristic::patternTypeToString(PatternType type) {
    switch (type) {
        case FIVE: return "FIVE";
        case OPEN_FOUR: return "OPEN_FOUR";
        case CLOSED_FOUR: return "CLOSED_FOUR";
        case OPEN_THREE: return "OPEN_THREE";
        case CLOSED_THREE: return "CLOSED_THREE";
        case OPEN_TWO: return "OPEN_TWO";
        case CLOSED_TWO: return "CLOSED_TWO";
        case SINGLE: return "SINGLE";
        default: return "INVALID";
    }
}