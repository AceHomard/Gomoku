/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Heuristic.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEURISTIC_HPP
#define HEURISTIC_HPP

#include "../Game/Board.hpp"
#include <vector>

// Forward declaration for compatibility
struct Threat {
    Position position;
    Threat(Position pos) : position(pos) {}
};

class Heuristic {
public:
    Heuristic();
    ~Heuristic();
    
    // Main evaluation function - returns 0 for non-terminal positions
    int evaluatePosition(const Board& board, CellState player);
    
    // Simple move generation - returns moves adjacent to existing pieces
    std::vector<Position> getRelevantMoves(const Board& board);
    
    // Stub methods for GameTree compatibility
    std::vector<Position> findWinningMoves(const Board& board, CellState player);
    std::vector<Position> findDefensiveMoves(const Board& board, CellState player);
    std::vector<Threat> findThreats(const Board& board, CellState player);
    int evaluateMove(const Board& board, const Position& move, CellState player);

private:
    // Helper to check if position has adjacent stones
    bool hasAdjacentStone(const Board& board, int x, int y);
};

#endif // HEURISTIC_HPP