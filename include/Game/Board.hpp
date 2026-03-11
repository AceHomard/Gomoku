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
#include <cstdint>

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

// Stores info needed to undo a move (for make/unmake pattern in AI search)
struct MoveUndo {
    int x, y;
    CellState player;
    std::vector<Position> capturedStones;  // positions of captured opponent stones
    CellState capturedColor;               // color of captured stones
    int capturedCount;                     // number of stones captured (for capture counter)
};

class Board {
private:
    std::vector<std::vector<CellState>> grid;
    int size;
    int blackCaptures;
    int whiteCaptures;
    Position lastMove;
    CellState lastPlayer;

    // Zobrist hashing
    uint64_t zobristHash;
    static uint64_t zobristTable[19][19][3];      // [x][y][EMPTY/BLACK/WHITE]
    static uint64_t zobristCaptures[2][11];        // [player_index][capture_count]
    static bool zobristInitialized;
    static void initZobrist();

public:
    Board(int boardSize = BOARD_SIZE);
    ~Board();

    // Basic board operations
    void clear();
    bool placePiece(int x, int y, CellState player);
    CellState getCell(int x, int y) const;
    void setCell(int x, int y, CellState state); // Direct cell manipulation (use with caution)
    bool isValidMove(int x, int y) const;
    int getSize() const { return size; }

    // Make/Unmake pattern for AI search (avoids board copy)
    MoveUndo makeMove(int x, int y, CellState player);
    void unmakeMove(const MoveUndo& undo);

    // Win condition checking
    bool checkWin(CellState player) const;
    bool checkWinFast(CellState player) const; // Only checks from lastMove (for AI search)
    bool checkAlignment(int x, int y, CellState player) const;
    bool checkCaptureWin(CellState player) const;
    Position getLastMove() const { return lastMove; }
    CellState getLastPlayer() const { return lastPlayer; }
    
    // Capture system
    std::vector<Position> checkCaptures(int x, int y, CellState player) const;
    int executeCaptures(const std::vector<Position>& captures);
    int getCaptureCount(CellState player) const;
    void setCaptureCount(CellState player, int count);
    
    // Double-three rule
    bool isDoubleThree(int x, int y, CellState player) const;
    std::vector<Position> findFreeThrees(int x, int y, CellState player) const;
    bool isFreeThree(int x, int y, int dx, int dy, CellState player) const;
    bool canFormUnstoppableFour(int x, int y, int dx, int dy, CellState player) const;
    bool canCreateUnstoppableFour(int x, int y, int dx, int dy, CellState player) const;
    
    // Zobrist hash
    uint64_t getZobristHash() const { return zobristHash; }

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