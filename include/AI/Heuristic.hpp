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
    std::vector<Position> getRelevantMoves(const Board& board, CellState player);

private:
    // Helper to check if position has adjacent stones
    bool hasAdjacentStone(const Board& board, int x, int y);

    // Move scoring for Alpha-Beta move ordering
    int getMoveScore(const Board& board, const Position& pos, CellState player);

    // Pattern evaluation methods
    int evaluatePatterns(const Board& board, CellState player);
    int evaluateDirection(const Board& board, int x, int y, int dx, int dy, CellState player);
    int getPatternScore(int length, bool openStart, bool openEnd);

    // Fast optimized methods for depth 10 performance
    int evaluatePatternsFast(const Board& board, CellState player);
    int evaluateDirectionFast(const Board& board, int x, int y, int dx, int dy, CellState player);

    // Smart but fast evaluation methods
    int countAlignments(const Board& board, CellState player);
    int detectCriticalThreats(const Board& board, CellState player, CellState opponent);
    bool creates4InRowThreat(const Board& board, const Position& pos, CellState player);

    // Threat detection methods
    int detectThreats(const Board& board, CellState player);
    bool isImmediateWin(const Board& board, const Position& move, CellState player);
    int countWinningThreats(const Board& board, const Position& move, CellState player);

    // Capture evaluation methods
    int evaluateCaptures(const Board& board, CellState player);

    // Positional evaluation methods
    int evaluatePositional(const Board& board, CellState player);
    int getDistanceFromCenter(int x, int y, int boardSize);
};

#endif // HEURISTIC_HPP