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
using namespace Directions;

// Core rule validation
bool Rules::isValidMove(const Board& board, int x, int y, CellState player) {
    if (!board.isValidMove(x, y)) {
        return false;
    }

    // PRIORITY: Check for mandatory defensive moves (endgame capture rule)
    // BUT: if this move itself creates a winning alignment, allow it (winning takes priority)
    std::vector<Position> mandatoryMoves = getMandatoryDefensiveMoves(board, player);
    if (!mandatoryMoves.empty()) {
        // Check if this move creates a winning alignment for us
        Board tempBoard = board;
        tempBoard.setCell(x, y, player);
        // Apply captures from this move
        std::vector<Position> captures = board.checkCaptures(x, y, player);
        for (const Position& cap : captures) {
            tempBoard.setCell(cap.x, cap.y, EMPTY);
        }
        bool thisMovWins = hasAlignmentWin(tempBoard, player) ||
                           (board.getCaptureCount(player) + (int)captures.size() >= 10);

        if (!thisMovWins) {
            // There are mandatory moves and we're not winning - check if this is one of them
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
    }

    // Apply double-three rule (with capture exception by default)
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
                    int dx = VECTORS[i][0];
                    int dy = VECTORS[i][1];
                    
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

std::vector<Position> Rules::getWinningAlignment(const Board& board, CellState player) {
    CellState opponent = (player == BLACK) ? WHITE : BLACK;
    
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) == player) {
                for (int i = 0; i < NUM_DIRECTIONS; i++) {
                    int dx = VECTORS[i][0];
                    int dy = VECTORS[i][1];
                    
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
                        
                        // Check if opponent can capture any stone in the alignment
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
                                                break;
                                            }
                                        }
                                        if (canBeBroken) break;
                                    }
                                }
                            }
                        }
                        
                        // Return alignment if it's a valid win (cannot be broken)
                        if (!canBeBroken) {
                            return alignmentStones;
                        }
                    }
                }
            }
        }
    }
    
    // No winning alignment found
    return std::vector<Position>();
}

// Capture detection and validation
std::vector<Position> Rules::detectCaptures(const Board& board, int x, int y, CellState player) {
    return board.checkCaptures(x, y, player);
}

bool Rules::isCaptureMove(const Board& board, int x, int y, CellState player) {
    std::vector<Position> captures = detectCaptures(board, x, y, player);
    return !captures.empty();
}

// Free-three detection
bool Rules::isFreeThree(const Board& board, int x, int y, int dx, int dy, CellState player) {
    return board.isFreeThree(x, y, dx, dy, player);
}

std::vector<Position> Rules::findAllFreeThrees(const Board& board, int x, int y, CellState player) {
    return board.findFreeThrees(x, y, player);
}

// Double-three rule validation
bool Rules::violatesDoubleThreeRule(const Board& board, int x, int y, CellState player, bool checkCaptureException) {
    // First check if we have a double-three
    if (countFreeThrees(board, x, y, player) < 2) {
        return false; // No double-three
    }

    // We have a double-three. If capture exception is disabled, reject it
    if (!checkCaptureException) {
        return true; // Double-three violation
    }

    // EXCEPTION: It is NOT forbidden to introduce a double-three by capturing a pair (subject appendix)
    // Check if this move would capture stones
    Board tempBoard = board;
    tempBoard.setCell(x, y, player); // Temporarily place stone
    std::vector<Position> wouldCapture = tempBoard.checkCaptures(x, y, player);
    tempBoard.setCell(x, y, EMPTY); // Remove temporary placement

    // If the move captures at least one pair, double-three is allowed
    if (!wouldCapture.empty()) {
        LOG_RULE("Move (" << x << "," << y << ") creates double-three BUT captures "
                 << wouldCapture.size() << " stones - LEGAL per subject rules");
        return false; // NOT a violation (exception applies)
    }

    // Double-three without capture = violation
    LOG_RULE("Move (" << x << "," << y << ") rejected - violates double-three rule (no capture)");
    return true;
}

int Rules::countFreeThrees(const Board& board, int x, int y, CellState player) {
    int count = 0;
    // Evaluate directly on the provided board. Board::isFreeThree simulates the placement internally
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        int dx = VECTORS[i][0];
        int dy = VECTORS[i][1];
        if (isFreeThree(board, x, y, dx, dy, player)) {
            count++;
        }
    }
    return count;
}

// Pattern analysis - simplified to only what's needed

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

// Mandatory moves - only endgame capture rule (other threat detection unused by AI)

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
                    int dx = VECTORS[i][0];
                    int dy = VECTORS[i][1];

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

// End of Rules.cpp - Dead code removed for clean architecture