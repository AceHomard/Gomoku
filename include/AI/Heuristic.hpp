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
    std::vector<Position> getRelevantMoves(const Board& board);

private:
    // Helper to check if position has adjacent stones
    bool hasAdjacentStone(const Board& board, int x, int y);
};

#endif // HEURISTIC_HPP