/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Rules.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RULES_HPP
#define RULES_HPP

#include "../Gomoku.hpp"
#include "Board.hpp"
#include <vector>

class Rules {
public:
    // Core rule validation
    static bool isValidMove(const Board& board, int x, int y, CellState player);
    static bool isGameWon(const Board& board, CellState player);
    
    // Win condition checks
    static bool hasAlignmentWin(const Board& board, CellState player);
    static bool hasCaptureWin(const Board& board, CellState player);
    static bool canBreakAlignment(const Board& board, int x, int y, CellState player);
    
    // Capture detection and validation
    static std::vector<Position> detectCaptures(const Board& board, int x, int y, CellState player);
    static bool isCaptureMove(const Board& board, int x, int y, CellState player);
    static int countPotentialCaptures(const Board& board, CellState player);
    
    // Free-three detection
    static bool isFreeThree(const Board& board, int x, int y, int dx, int dy, CellState player);
    static std::vector<Position> findAllFreeThrees(const Board& board, int x, int y, CellState player);
    static bool createsFreeThree(const Board& board, int x, int y, CellState player);
    
    // Double-three rule validation
    static bool violatesDoubleThreeRule(const Board& board, int x, int y, CellState player);
    static int countFreeThrees(const Board& board, int x, int y, CellState player);
    
    // Pattern analysis
    static bool isUnstoppableFour(const Board& board, int x, int y, int dx, int dy, CellState player);
    static bool canFormWinningSequence(const Board& board, int x, int y, CellState player);
    static std::vector<Position> findThreats(const Board& board, CellState player);
    
    // Line analysis
    static int analyzeLinePattern(const Board& board, int x, int y, int dx, int dy, CellState player);
    static bool hasOpenEnd(const Board& board, int x, int y, int dx, int dy, CellState player);
    static int countConsecutiveStones(const Board& board, int x, int y, int dx, int dy, CellState player);
    
    // Game state analysis
    static bool isGameDrawn(const Board& board);
    static bool hasForcedWin(const Board& board, CellState player);
    static std::vector<Position> getMandatoryMoves(const Board& board, CellState player);

    // Endgame capture rule: mandatory defensive moves
    static std::vector<Position> getMandatoryDefensiveMoves(const Board& board, CellState player);
    
    // Constants
    static const int WIN_ALIGNMENT = 5;
    static const int WIN_CAPTURES = 10;
    static const int FREE_THREE_LENGTH = 3;
    static const int DIRECTIONS[][2];
    static const int NUM_DIRECTIONS = 4;
    
private:
    // Helper methods for pattern detection
    static bool checkPattern(const Board& board, int x, int y, int dx, int dy, 
                           const std::vector<int>& pattern, CellState player);
    static std::vector<int> getLineSegment(const Board& board, int x, int y, int dx, int dy, 
                                         int length, CellState player);
    static bool hasGap(const std::vector<int>& segment, CellState player);
    static int countStones(const std::vector<int>& segment, CellState player);
    
    // Helper methods for capture detection
    static bool isFlankingCapture(const Board& board, int x, int y, int dx, int dy, CellState player);
    static Position findCapturedStone(const Board& board, int x, int y, int dx, int dy, CellState player);
    
    // Helper methods for free-three validation
    static bool hasTwoOpenEnds(const Board& board, int x, int y, int dx, int dy, 
                              int length, CellState player);
    static bool canExtendToFour(const Board& board, int x, int y, int dx, int dy, CellState player);
    static bool isBlockedByCapture(const Board& board, int x, int y, int dx, int dy, CellState player);
};

#endif // RULES_HPP