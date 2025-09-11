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
#include <set>

Heuristic::Heuristic() {
}

Heuristic::~Heuristic() {
}

int Heuristic::evaluatePosition(const Board& board, CellState player) {
    // Simple evaluation: return 0 for non-terminal positions
    // Terminal positions (wins/losses) are handled by MinMaxAI
    (void)board;  // Suppress unused parameter warning
    (void)player;
    return 0;
}

std::vector<Position> Heuristic::getRelevantMoves(const Board& board) {
    std::vector<Position> moves;
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
    
    // Convert set to vector
    for (const auto& move : moveSet) {
        moves.push_back(Position(move.first, move.second));
    }
    
    // If no moves found (empty board), return center
    if (moves.empty()) {
        int center = board.getSize() / 2;
        moves.push_back(Position(center, center));
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

// Stub implementations for GameTree compatibility
std::vector<Position> Heuristic::findWinningMoves(const Board& board, CellState player) {
    (void)board;
    (void)player;
    return std::vector<Position>(); // Empty - no complex analysis
}

std::vector<Position> Heuristic::findDefensiveMoves(const Board& board, CellState player) {
    (void)board;
    (void)player;
    return std::vector<Position>(); // Empty - no complex analysis
}

std::vector<Threat> Heuristic::findThreats(const Board& board, CellState player) {
    (void)board;
    (void)player;
    return std::vector<Threat>(); // Empty - no threat analysis
}

int Heuristic::evaluateMove(const Board& board, const Position& move, CellState player) {
    (void)board;
    (void)move;
    (void)player;
    return 0; // Neutral evaluation for all moves
}