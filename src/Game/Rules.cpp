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
#include <algorithm>

// Constants
const int Rules::DIRECTIONS[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

// Core rule validation
bool Rules::isValidMove(const Board& board, int x, int y, CellState player) {
    if (!board.isValidMove(x, y)) {
        return false;
    }
    
    // Check double-three rule
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
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) == player) {
                for (int i = 0; i < NUM_DIRECTIONS; i++) {
                    int dx = DIRECTIONS[i][0];
                    int dy = DIRECTIONS[i][1];
                    
                    int count = 1;
                    // Count forward
                    int nx = x + dx, ny = y + dy;
                    while (nx >= 0 && nx < board.getSize() && ny >= 0 && ny < board.getSize() && 
                           board.getCell(nx, ny) == player) {
                        count++;
                        nx += dx;
                        ny += dy;
                    }
                    
                    // Count backward
                    nx = x - dx;
                    ny = y - dy;
                    while (nx >= 0 && nx < board.getSize() && ny >= 0 && ny < board.getSize() && 
                           board.getCell(nx, ny) == player) {
                        count++;
                        nx -= dx;
                        ny -= dy;
                    }
                    
                    if (count >= WIN_ALIGNMENT) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool Rules::hasCaptureWin(const Board& board, CellState player) {
    return board.getCaptureCount(player) >= WIN_CAPTURES;
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
    // Create a temporary board with the move placed
    Board tempBoard = board;
    const_cast<Board&>(tempBoard).placePiece(x, y, player);
    
    int count = 0;
    
    // Check each direction for free-threes
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        int dx = DIRECTIONS[i][0];
        int dy = DIRECTIONS[i][1];
        
        if (isFreeThree(tempBoard, x, y, dx, dy, player)) {
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