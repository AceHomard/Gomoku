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

    // Capture detection
    static std::vector<Position> detectCaptures(const Board& board, int x, int y, CellState player);
    static bool isCaptureMove(const Board& board, int x, int y, CellState player);

    // Free-three detection (for double-three rule)
    static bool isFreeThree(const Board& board, int x, int y, int dx, int dy, CellState player);
    static std::vector<Position> findAllFreeThrees(const Board& board, int x, int y, CellState player);

    // Double-three rule validation
    static bool violatesDoubleThreeRule(const Board& board, int x, int y, CellState player, bool checkCaptureException = true);
    static int countFreeThrees(const Board& board, int x, int y, CellState player);

    // Game state analysis
    static bool isGameDrawn(const Board& board);

    // Endgame capture rule: mandatory defensive moves
    static std::vector<Position> getMandatoryDefensiveMoves(const Board& board, CellState player);
};

#endif // RULES_HPP