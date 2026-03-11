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

class Heuristic {
public:
    Heuristic();
    ~Heuristic();
    
    // Main evaluation function - currently returns 0 for non-terminal positions
    // This will be improved during the learning iterations
    int evaluatePosition(const Board& board, CellState player);
    
    // Move generation - returns moves adjacent to existing pieces
    // fastMode=true skips expensive validation (mandatory moves, double-three) for inner search nodes
    std::vector<Position> getRelevantMoves(const Board& board, CellState player, bool fastMode = false);

private:
    // Helper to check if position has adjacent stones
    bool hasAdjacentStone(const Board& board, int x, int y);

    // Move scoring for Alpha-Beta move ordering
    int getMoveScore(const Board& board, const Position& pos, CellState player);

    // Pattern-aware evaluation: distinguishes open vs blocked alignments
    int countPatterns(const Board& board, CellState player);

    // Legacy alignment counting (kept for reference)
    int countAlignments(const Board& board, CellState player);
};

#endif // HEURISTIC_HPP