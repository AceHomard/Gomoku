/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Board.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BOARD_HPP
#define BOARD_HPP

#include "../Gomoku.hpp"
#include <vector>
#include <utility>

enum CellState {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

struct Position {
    int x, y;
    Position(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

class Board {
private:
    std::vector<std::vector<CellState>> grid;
    int size;
    int blackCaptures;
    int whiteCaptures;
    std::vector<Position> lastCaptures;  // For undo functionality

public:
    Board(int boardSize = BOARD_SIZE);
    ~Board();
    
    // Basic board operations
    void clear();
    bool placePiece(int x, int y, CellState player);
    CellState getCell(int x, int y) const;
    bool isValidMove(int x, int y) const;
    int getSize() const { return size; }
    
    // Win condition checking
    bool checkWin(CellState player) const;
    bool checkAlignment(int x, int y, CellState player) const;
    bool checkCaptureWin(CellState player) const;
    
    // Capture system
    std::vector<Position> checkCaptures(int x, int y, CellState player) const;
    int executeCaptures(const std::vector<Position>& captures);
    int getCaptureCount(CellState player) const;
    void setCaptureCount(CellState player, int count);
    
    // Double-three rule
    bool isDoubleThree(int x, int y, CellState player) const;
    std::vector<Position> findFreeThrees(int x, int y, CellState player) const;
    
    // Pattern detection
    bool isFreeThree(int x, int y, int dx, int dy, CellState player) const;
    bool canFormUnstoppableFour(int x, int y, int dx, int dy, CellState player) const;
    
    // Utility functions
    bool isValidPosition(int x, int y) const;
    CellState getOpponent(CellState player) const;
    
    // Public methods for Rules class
    int countConsecutive(int x, int y, int dx, int dy, CellState player) const;
    bool hasOpenEnds(int x, int y, int dx, int dy, int count, CellState player) const;
       
private:
    // Helper methods for win detection
    int countDirection(int x, int y, int dx, int dy, CellState player) const;
    bool checkDirectionAlignment(int x, int y, int dx, int dy, CellState player) const;
    
    // Helper methods for capture detection
    std::vector<Position> checkCaptureDirection(int x, int y, int dx, int dy, CellState player) const;
    
    // Helper methods for free-three detection
};

#endif // BOARD_HPP