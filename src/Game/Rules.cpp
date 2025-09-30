/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Rules.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Game/Rules.hpp"
#include "Game/Constants.hpp"
#include "Debug.hpp"
#include <algorithm>

using namespace GameConstants;

// Constants
const int Rules::DIRECTIONS[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

// Core rule validation
bool Rules::isValidMove(const Board& board, int x, int y, CellState player) {
    if (!board.isValidMove(x, y)) {
        return false;
    }

    // PRIORITY: Check for mandatory defensive moves (endgame capture rule)
    std::vector<Position> mandatoryMoves = getMandatoryDefensiveMoves(board, player);
    if (!mandatoryMoves.empty()) {
        // There are mandatory moves - check if this is one of them
        bool isMandatory = false;
        for (const Position& mandatory : mandatoryMoves) {
            if (mandatory.x == x && mandatory.y == y) {
                isMandatory = true;
                break;
            }
        }

        if (!isMandatory) {
            LOG_RULE("Move (" << x << "," << y << ") rejected - player MUST play mandatory defensive capture");
            return false; // Invalid - not a mandatory move
        }
    }

    // Apply double-three rule
    if (violatesDoubleThreeRule(board, x, y, player)) {
        return false;
    }

    return true;
}

bool Rules::isGameWon(const Board& board, CellState player) {
    return hasAlignmentWin(board, player) || hasCaptureWin(board, player);
}

// Win condition checks
bool Rules::hasAlignmentWin(const Board& board, CellState player) {
    CellState opponent = (player == BLACK) ? WHITE : BLACK;
    
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) == player) {
                for (int i = 0; i < NUM_DIRECTIONS; i++) {
                    int dx = DIRECTIONS[i][0];
                    int dy = DIRECTIONS[i][1];
                    
                    int count = 1;
                    std::vector<Position> alignmentStones;
                    alignmentStones.push_back(Position(x, y));
                    
                    // Count forward
                    int nx = x + dx, ny = y + dy;
                    while (nx >= 0 && nx < board.getSize() && ny >= 0 && ny < board.getSize() && 
                           board.getCell(nx, ny) == player) {
                        count++;
                        alignmentStones.push_back(Position(nx, ny));
                        nx += dx;
                        ny += dy;
                    }
                    
                    // Count backward
                    nx = x - dx;
                    ny = y - dy;
                    while (nx >= 0 && nx < board.getSize() && ny >= 0 && ny < board.getSize() && 
                           board.getCell(nx, ny) == player) {
                        count++;
                        alignmentStones.insert(alignmentStones.begin(), Position(nx, ny));
                        nx -= dx;
                        ny -= dy;
                    }
                    
                    if (count >= WIN_ALIGNMENT) {
                        // Check endgame capture rule: alignment wins only if opponent cannot break it by immediate capture
                        bool canBeBroken = false;
                        LOG_DEBUG("Player " << (player == BLACK ? "BLACK" : "WHITE") << " has " << count << "-alignment");
                        
                        // Check if opponent can capture any stone in the alignment
                        // Check all possible moves for opponent
                        for (int ox = 0; ox < board.getSize() && !canBeBroken; ox++) {
                            for (int oy = 0; oy < board.getSize() && !canBeBroken; oy++) {
                                if (board.getCell(ox, oy) == EMPTY) {
                                    // Basic validation only (NO recursion via isValidMove!)
                                    if (!board.isValidMove(ox, oy)) {
                                        continue;
                                    }
                                    // Check double-three directly (avoid full isValidMove to prevent recursion)
                                    if (violatesDoubleThreeRule(board, ox, oy, opponent)) {
                                        continue;
                                    }
                                    std::vector<Position> captures = board.checkCaptures(ox, oy, opponent);
                                    // Check if any capture would remove a stone from our alignment
                                    for (const Position& capture : captures) {
                                        for (const Position& alignStone : alignmentStones) {
                                            if (capture.x == alignStone.x && capture.y == alignStone.y) {
                                                canBeBroken = true;
                                                LOG_DEBUG("Alignment breakable by capture at (" << ox << "," << oy << ")");
                                                break;
                                            }
                                        }
                                        if (canBeBroken) break;
                                    }
                                }
                            }
                        }
                        
                        // Determine if this is a win
                        if (!canBeBroken) {
                            LOG_RULE("Player " << (player == BLACK ? "BLACK" : "WHITE") << " wins - unbreakable alignment");
                            return true;
                        } else {
                            LOG_DEBUG("Alignment breakable - opponent has mandatory defensive moves");
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool Rules::hasCaptureWin(const Board& board, CellState player) {
    bool win = board.getCaptureCount(player) >= WIN_CAPTURES;
    if (win) {
        LOG_RULE("Player " << (player == BLACK ? "BLACK" : "WHITE") << " wins by capture (" << board.getCaptureCount(player) << " pairs)");
    }
    return win;
}

bool Rules::canBreakAlignment(const Board& board, int x, int y, CellState player) {
    // Check if capturing at this position can break a 5+ alignment
    CellState opponent = (player == BLACK) ? WHITE : BLACK;
    
    // Temporarily remove the stone to see if it breaks an alignment
    Board tempBoard = board;
    const_cast<Board&>(tempBoard).placePiece(x, y, EMPTY);
    
    // Check if opponent no longer has alignment win
    return !hasAlignmentWin(tempBoard, opponent);
}

// Capture detection and validation
std::vector<Position> Rules::detectCaptures(const Board& board, int x, int y, CellState player) {
    return board.checkCaptures(x, y, player);
}

bool Rules::isCaptureMove(const Board& board, int x, int y, CellState player) {
    std::vector<Position> captures = detectCaptures(board, x, y, player);
    return !captures.empty();
}

int Rules::countPotentialCaptures(const Board& board, CellState player) {
    int count = 0;
    
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.isValidMove(x, y)) {
                std::vector<Position> captures = detectCaptures(board, x, y, player);
                if (!captures.empty()) {
                    count++;
                }
            }
        }
    }
    
    return count;
}

// Free-three detection
bool Rules::isFreeThree(const Board& board, int x, int y, int dx, int dy, CellState player) {
    return board.isFreeThree(x, y, dx, dy, player);
}

std::vector<Position> Rules::findAllFreeThrees(const Board& board, int x, int y, CellState player) {
    return board.findFreeThrees(x, y, player);
}

bool Rules::createsFreeThree(const Board& board, int x, int y, CellState player) {
    std::vector<Position> freeThrees = findAllFreeThrees(board, x, y, player);
    return !freeThrees.empty();
}

// Double-three rule validation
bool Rules::violatesDoubleThreeRule(const Board& board, int x, int y, CellState player) {
    return countFreeThrees(board, x, y, player) >= 2;
}

int Rules::countFreeThrees(const Board& board, int x, int y, CellState player) {
    int count = 0;
    // Evaluate directly on the provided board. Board::isFreeThree simulates the placement internally
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        int dx = DIRECTIONS[i][0];
        int dy = DIRECTIONS[i][1];
        if (isFreeThree(board, x, y, dx, dy, player)) {
            count++;
        }
    }
    return count;
}

// Pattern analysis
bool Rules::isUnstoppableFour(const Board& board, int x, int y, int dx, int dy, CellState player) {
    return board.canFormUnstoppableFour(x, y, dx, dy, player);
}

bool Rules::canFormWinningSequence(const Board& board, int x, int y, CellState player) {
    // Check if placing at this position leads to a forced win
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        int dx = DIRECTIONS[i][0];
        int dy = DIRECTIONS[i][1];
        
        if (isUnstoppableFour(board, x, y, dx, dy, player)) {
            return true;
        }
    }
    
    // Check for capture-based winning sequences
    std::vector<Position> captures = detectCaptures(board, x, y, player);
    if (!captures.empty()) {
        int newCaptureCount = board.getCaptureCount(player) + captures.size();
        if (newCaptureCount >= WIN_CAPTURES) {
            return true;
        }
    }
    
    return false;
}

std::vector<Position> Rules::findThreats(const Board& board, CellState player) {
    std::vector<Position> threats;
    
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.isValidMove(x, y)) {
                if (canFormWinningSequence(board, x, y, player)) {
                    threats.push_back(Position(x, y));
                }
            }
        }
    }
    
    return threats;
}

// Line analysis
int Rules::analyzeLinePattern(const Board& board, int x, int y, int dx, int dy, CellState player) {
    int count = 0;
    int nx = x, ny = y;
    
    // Count consecutive stones in this direction
    while (nx >= 0 && nx < board.getSize() && ny >= 0 && ny < board.getSize() && 
           board.getCell(nx, ny) == player) {
        count++;
        nx += dx;
        ny += dy;
    }
    
    return count;
}

bool Rules::hasOpenEnd(const Board& board, int x, int y, int dx, int dy, CellState player) {
    // Find the end of the sequence
    while (x >= 0 && x < board.getSize() && y >= 0 && y < board.getSize() && 
           board.getCell(x, y) == player) {
        x += dx;
        y += dy;
    }
    
    // Check if the position after the sequence is empty
    return (x >= 0 && x < board.getSize() && y >= 0 && y < board.getSize() && 
            board.getCell(x, y) == EMPTY);
}

int Rules::countConsecutiveStones(const Board& board, int x, int y, int dx, int dy, CellState player) {
    return board.countConsecutive(x, y, dx, dy, player);
}

// Game state analysis
bool Rules::isGameDrawn(const Board& board) {
    // Check if board is full
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) == EMPTY) {
                return false; // Game not drawn if there are empty spaces
            }
        }
    }
    
    // Board is full and no one has won
    return !hasAlignmentWin(board, BLACK) && !hasAlignmentWin(board, WHITE) &&
           !hasCaptureWin(board, BLACK) && !hasCaptureWin(board, WHITE);
}

bool Rules::hasForcedWin(const Board& board, CellState player) {
    // Check if player has an immediate winning move
    std::vector<Position> threats = findThreats(board, player);
    
    if (threats.size() >= 2) {
        // Multiple threats - forced win
        return true;
    }
    
    if (threats.size() == 1) {
        // Check if opponent can defend
        CellState opponent = (player == BLACK) ? WHITE : BLACK;
        Position threat = threats[0];
        
        // Simulate opponent's response
        Board tempBoard = board;
        const_cast<Board&>(tempBoard).placePiece(threat.x, threat.y, opponent);
        
        // If player still has winning moves after opponent's defense, it's forced
        std::vector<Position> newThreats = findThreats(tempBoard, player);
        return !newThreats.empty();
    }
    
    return false;
}

std::vector<Position> Rules::getMandatoryMoves(const Board& board, CellState player) {
    std::vector<Position> mandatory;
    CellState opponent = (player == BLACK) ? WHITE : BLACK;

    // PRIORITY 1: Check for mandatory defensive moves (opponent has breakable 5+ alignment)
    std::vector<Position> defensiveMoves = getMandatoryDefensiveMoves(board, player);
    if (!defensiveMoves.empty()) {
        return defensiveMoves; // MUST defend, no other choice
    }

    // Find opponent's immediate threats
    std::vector<Position> opponentThreats = findThreats(board, opponent);

    // Must defend against immediate threats
    for (const Position& threat : opponentThreats) {
        mandatory.push_back(threat);
    }

    // If no threats, look for winning moves
    if (mandatory.empty()) {
        std::vector<Position> winningMoves = findThreats(board, player);
        mandatory.insert(mandatory.end(), winningMoves.begin(), winningMoves.end());
    }

    return mandatory;
}

std::vector<Position> Rules::getMandatoryDefensiveMoves(const Board& board, CellState player) {
    // Check if OPPONENT has a 5+ alignment that can be broken by capture
    // If yes, return the mandatory capture moves (player MUST play one of these)

    std::vector<Position> mandatoryMoves;
    CellState opponent = (player == BLACK) ? WHITE : BLACK;

    // Scan for opponent alignments of 5+
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) == opponent) {
                for (int i = 0; i < NUM_DIRECTIONS; i++) {
                    int dx = DIRECTIONS[i][0];
                    int dy = DIRECTIONS[i][1];

                    int count = 1;
                    std::vector<Position> alignmentStones;
                    alignmentStones.push_back(Position(x, y));

                    // Count forward
                    int nx = x + dx, ny = y + dy;
                    while (nx >= 0 && nx < board.getSize() && ny >= 0 && ny < board.getSize() &&
                           board.getCell(nx, ny) == opponent) {
                        count++;
                        alignmentStones.push_back(Position(nx, ny));
                        nx += dx;
                        ny += dy;
                    }

                    // Count backward
                    nx = x - dx;
                    ny = y - dy;
                    while (nx >= 0 && nx < board.getSize() && ny >= 0 && ny < board.getSize() &&
                           board.getCell(nx, ny) == opponent) {
                        count++;
                        alignmentStones.insert(alignmentStones.begin(), Position(nx, ny));
                        nx -= dx;
                        ny -= dy;
                    }

                    // Opponent has 5+ alignment
                    if (count >= WIN_ALIGNMENT) {
                        // Check if I (player) can break it by capturing
                        for (int ox = 0; ox < board.getSize(); ox++) {
                            for (int oy = 0; oy < board.getSize(); oy++) {
                                if (board.getCell(ox, oy) == EMPTY) {
                                    // Basic validation only (NO recursion!)
                                    if (!board.isValidMove(ox, oy)) {
                                        continue;
                                    }
                                    // Check double-three directly
                                    if (violatesDoubleThreeRule(board, ox, oy, player)) {
                                        continue;
                                    }

                                    std::vector<Position> captures = board.checkCaptures(ox, oy, player);

                                    // Check if any capture would remove a stone from opponent's alignment
                                    for (const Position& capture : captures) {
                                        for (const Position& alignStone : alignmentStones) {
                                            if (capture.x == alignStone.x && capture.y == alignStone.y) {
                                                // This is a mandatory defensive move!
                                                mandatoryMoves.push_back(Position(ox, oy));
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // If we found mandatory moves, return them immediately
                        if (!mandatoryMoves.empty()) {
                            LOG_MANDATORY("Player " << (player == BLACK ? "BLACK" : "WHITE")
                                          << " MUST play one of " << mandatoryMoves.size() << " defensive captures");
                            return mandatoryMoves;
                        }
                    }
                }
            }
        }
    }

    return mandatoryMoves; // Empty if no mandatory moves
}

// Helper methods for pattern detection
bool Rules::checkPattern(const Board& board, int x, int y, int dx, int dy, 
                        const std::vector<int>& pattern, CellState player) {
    for (size_t i = 0; i < pattern.size(); i++) {
        int nx = x + i * dx;
        int ny = y + i * dy;
        
        if (nx < 0 || nx >= board.getSize() || ny < 0 || ny >= board.getSize()) {
            return false;
        }
        
        CellState expectedState = EMPTY;
        if (pattern[i] == 1) expectedState = player;
        else if (pattern[i] == 2) expectedState = (player == BLACK) ? WHITE : BLACK;
        
        if (board.getCell(nx, ny) != expectedState) {
            return false;
        }
    }
    
    return true;
}

std::vector<int> Rules::getLineSegment(const Board& board, int x, int y, int dx, int dy, 
                                      int length, CellState player) {
    std::vector<int> segment;
    
    for (int i = 0; i < length; i++) {
        int nx = x + i * dx;
        int ny = y + i * dy;
        
        if (nx < 0 || nx >= board.getSize() || ny < 0 || ny >= board.getSize()) {
            segment.push_back(0); // Out of bounds = empty
        } else {
            CellState cell = board.getCell(nx, ny);
            if (cell == EMPTY) segment.push_back(0);
            else if (cell == player) segment.push_back(1);
            else segment.push_back(2); // Opponent
        }
    }
    
    return segment;
}

bool Rules::hasGap(const std::vector<int>& segment, CellState player) {
    (void)player;
    bool foundPlayer = false;
    bool foundGap = false;
    
    for (int cell : segment) {
        if (cell == 1) { // Player stone
            if (foundGap) return true; // Gap followed by player stone
            foundPlayer = true;
        } else if (cell == 0 && foundPlayer) { // Empty after player stone
            foundGap = true;
        } else if (cell == 2) { // Opponent stone
            break;
        }
    }
    
    return false;
}

int Rules::countStones(const std::vector<int>& segment, CellState player) {
    (void)player;
    int count = 0;
    for (int cell : segment) {
        if (cell == 1) count++; // Player stone
    }
    return count;
}

// Helper methods for capture detection
bool Rules::isFlankingCapture(const Board& board, int x, int y, int dx, int dy, CellState player) {
    CellState opponent = (player == BLACK) ? WHITE : BLACK;
    
    // Check pattern: Player - Opponent - Opponent - Player (after placing current move)
    if (board.isValidPosition(x + dx, y + dy) &&
        board.isValidPosition(x + 2*dx, y + 2*dy) &&
        board.isValidPosition(x + 3*dx, y + 3*dy)) {
        
        return board.getCell(x + dx, y + dy) == opponent &&
               board.getCell(x + 2*dx, y + 2*dy) == opponent &&
               board.getCell(x + 3*dx, y + 3*dy) == player;
    }
    
    return false;
}

Position Rules::findCapturedStone(const Board& board, int x, int y, int dx, int dy, CellState player) {
    if (isFlankingCapture(board, x, y, dx, dy, player)) {
        return Position(x + dx, y + dy); // First opponent stone in the capture
    }
    return Position(-1, -1);
}

// Helper methods for free-three validation
bool Rules::hasTwoOpenEnds(const Board& board, int x, int y, int dx, int dy, 
                          int length, CellState player) {
    return board.hasOpenEnds(x, y, dx, dy, length, player);
}

bool Rules::canExtendToFour(const Board& board, int x, int y, int dx, int dy, CellState player) {
    return board.canFormUnstoppableFour(x, y, dx, dy, player);
}

bool Rules::isBlockedByCapture(const Board& board, int x, int y, int dx, int dy, CellState player) {
    CellState opponent = (player == BLACK) ? WHITE : BLACK;
    
    // Check if extending this three would create a capture opportunity for opponent
    int nx = x + dx, ny = y + dy;
    int nx2 = x - dx, ny2 = y - dy;
    
    // Check both ends of the potential four
    if (board.isValidPosition(nx, ny) && board.getCell(nx, ny) == EMPTY) {
        // Simulate placing there and check for opponent captures
        Board tempBoard = board;
        const_cast<Board&>(tempBoard).placePiece(nx, ny, player);
        
        std::vector<Position> captures = detectCaptures(tempBoard, nx, ny, opponent);
        if (!captures.empty()) return true;
    }
    
    if (board.isValidPosition(nx2, ny2) && board.getCell(nx2, ny2) == EMPTY) {
        Board tempBoard = board;
        const_cast<Board&>(tempBoard).placePiece(nx2, ny2, player);
        
        std::vector<Position> captures = detectCaptures(tempBoard, nx2, ny2, opponent);
        if (!captures.empty()) return true;
    }
    
    return false;
}