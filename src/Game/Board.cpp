/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Board.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Game/Board.hpp"
#include <iostream>

Board::Board(int boardSize) : size(boardSize), blackCaptures(0), whiteCaptures(0) {
    grid.resize(size, std::vector<CellState>(size, EMPTY));
}

Board::~Board() {
}

void Board::clear() {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            grid[i][j] = EMPTY;
        }
    }
    blackCaptures = 0;
    whiteCaptures = 0;
    lastCaptures.clear();
}

bool Board::placePiece(int x, int y, CellState player) {
    if (!isValidMove(x, y)) {
        return false;
    }
    
    // Check double-three rule before placing
    if (isDoubleThree(x, y, player)) {
        return false;
    }
    
    grid[x][y] = player;
    
    // Check and execute captures
    std::vector<Position> captures = checkCaptures(x, y, player);
    if (!captures.empty()) {
        int capturedCount = executeCaptures(captures);
        if (player == BLACK) {
            blackCaptures += capturedCount;
        } else {
            whiteCaptures += capturedCount;
        }
    }
    
    return true;
}

CellState Board::getCell(int x, int y) const {
    if (!isValidPosition(x, y)) {
        return EMPTY;
    }
    return grid[x][y];
}

bool Board::isValidMove(int x, int y) const {
    return isValidPosition(x, y) && grid[x][y] == EMPTY;
}

bool Board::isValidPosition(int x, int y) const {
    return x >= 0 && x < size && y >= 0 && y < size;
}

CellState Board::getOpponent(CellState player) const {
    if (player == BLACK) return WHITE;
    if (player == WHITE) return BLACK;
    return EMPTY;
}

// Win condition checking
bool Board::checkWin(CellState player) const {
    return checkCaptureWin(player) || checkAlignment(-1, -1, player);
}

bool Board::checkAlignment(int x, int y, CellState player) const {
    // If specific position given, check from that position
    if (x >= 0 && y >= 0) {
        return checkDirectionAlignment(x, y, 1, 0, player) ||  // horizontal
               checkDirectionAlignment(x, y, 0, 1, player) ||  // vertical
               checkDirectionAlignment(x, y, 1, 1, player) ||  // diagonal
               checkDirectionAlignment(x, y, 1, -1, player);   // anti-diagonal
    }
    
    // Check entire board for any alignment
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (grid[i][j] == player) {
                if (checkDirectionAlignment(i, j, 1, 0, player) ||
                    checkDirectionAlignment(i, j, 0, 1, player) ||
                    checkDirectionAlignment(i, j, 1, 1, player) ||
                    checkDirectionAlignment(i, j, 1, -1, player)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Board::checkCaptureWin(CellState player) const {
    return getCaptureCount(player) >= 10;
}

int Board::getCaptureCount(CellState player) const {
    return (player == BLACK) ? blackCaptures : whiteCaptures;
}

void Board::setCaptureCount(CellState player, int count) {
    if (player == BLACK) {
        blackCaptures = count;
    } else if (player == WHITE) {
        whiteCaptures = count;
    }
}

// Capture system
std::vector<Position> Board::checkCaptures(int x, int y, CellState player) const {
    std::vector<Position> allCaptures;
    
    // Check all four directions
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    
    for (int i = 0; i < 4; i++) {
        int dx = directions[i][0];
        int dy = directions[i][1];
        
        // Check both directions along this axis
        std::vector<Position> captures1 = checkCaptureDirection(x, y, dx, dy, player);
        std::vector<Position> captures2 = checkCaptureDirection(x, y, -dx, -dy, player);
        
        allCaptures.insert(allCaptures.end(), captures1.begin(), captures1.end());
        allCaptures.insert(allCaptures.end(), captures2.begin(), captures2.end());
    }
    
    return allCaptures;
}

std::vector<Position> Board::checkCaptureDirection(int x, int y, int dx, int dy, CellState player) const {
    std::vector<Position> captures;
    CellState opponent = getOpponent(player);
    
    // Check for pattern: Player - Opponent - Opponent - Player
    if (isValidPosition(x + dx, y + dy) && 
        isValidPosition(x + 2*dx, y + 2*dy) && 
        isValidPosition(x + 3*dx, y + 3*dy)) {
        
        if (grid[x + dx][y + dy] == opponent &&
            grid[x + 2*dx][y + 2*dy] == opponent &&
            grid[x + 3*dx][y + 3*dy] == player) {
            
            captures.push_back(Position(x + dx, y + dy));
            captures.push_back(Position(x + 2*dx, y + 2*dy));
        }
    }
    
    return captures;
}

int Board::executeCaptures(const std::vector<Position>& captures) {
    lastCaptures = captures;
    for (const Position& pos : captures) {
        if (isValidPosition(pos.x, pos.y)) {
            grid[pos.x][pos.y] = EMPTY;
        }
    }
    return captures.size();
}

// Double-three rule
bool Board::isDoubleThree(int x, int y, CellState player) const {
    std::vector<Position> freeThrees = findFreeThrees(x, y, player);
    return freeThrees.size() >= 2;
}

std::vector<Position> Board::findFreeThrees(int x, int y, CellState player) const {
    std::vector<Position> freeThrees;
    
    // Temporarily place the piece
    const_cast<Board*>(this)->grid[x][y] = player;
    
    // Check all four directions for free-threes
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    
    for (int i = 0; i < 4; i++) {
        int dx = directions[i][0];
        int dy = directions[i][1];
        
        if (isFreeThree(x, y, dx, dy, player)) {
            freeThrees.push_back(Position(x, y));
        }
    }
    
    // Remove the temporary piece
    const_cast<Board*>(this)->grid[x][y] = EMPTY;
    
    return freeThrees;
}

bool Board::isFreeThree(int x, int y, int dx, int dy, CellState player) const {
    int count = countConsecutive(x, y, dx, dy, player);
    
    if (count == 3) {
        return canFormUnstoppableFour(x, y, dx, dy, player);
    }
    
    return false;
}

bool Board::canFormUnstoppableFour(int x, int y, int dx, int dy, CellState player) const {
    // Find the bounds of the three consecutive stones
    int start = 0, end = 0;
    
    // Find start of sequence
    while (isValidPosition(x + (start-1)*dx, y + (start-1)*dy) && 
           grid[x + (start-1)*dx][y + (start-1)*dy] == player) {
        start--;
    }
    
    // Find end of sequence
    while (isValidPosition(x + (end+1)*dx, y + (end+1)*dy) && 
           grid[x + (end+1)*dx][y + (end+1)*dy] == player) {
        end++;
    }
    
    // Check if we can extend to form an unstoppable four
    bool canExtendBefore = isValidPosition(x + (start-1)*dx, y + (start-1)*dy) && 
                          grid[x + (start-1)*dx][y + (start-1)*dy] == EMPTY;
    bool canExtendAfter = isValidPosition(x + (end+1)*dx, y + (end+1)*dy) && 
                         grid[x + (end+1)*dx][y + (end+1)*dy] == EMPTY;
    
    return canExtendBefore && canExtendAfter;
}

// Helper methods
int Board::countDirection(int x, int y, int dx, int dy, CellState player) const {
    int count = 0;
    int nx = x, ny = y;
    
    while (isValidPosition(nx, ny) && grid[nx][ny] == player) {
        count++;
        nx += dx;
        ny += dy;
    }
    
    return count;
}

bool Board::checkDirectionAlignment(int x, int y, int dx, int dy, CellState player) const {
    int count = 1; // Count the current piece
    
    // Count in positive direction
    count += countDirection(x + dx, y + dy, dx, dy, player);
    
    // Count in negative direction
    count += countDirection(x - dx, y - dy, -dx, -dy, player);
    
    return count >= 5;
}

int Board::countConsecutive(int x, int y, int dx, int dy, CellState player) const {
    int count = 0;
    
    if (grid[x][y] == player) {
        count = 1;
        
        // Count forward
        int nx = x + dx, ny = y + dy;
        while (isValidPosition(nx, ny) && grid[nx][ny] == player) {
            count++;
            nx += dx;
            ny += dy;
        }
        
        // Count backward
        nx = x - dx;
        ny = y - dy;
        while (isValidPosition(nx, ny) && grid[nx][ny] == player) {
            count++;
            nx -= dx;
            ny -= dy;
        }
    }
    
    return count;
}

bool Board::hasOpenEnds(int x, int y, int dx, int dy, int count, CellState player) const {
    (void)count;
    // Find the bounds of the sequence
    int startX = x, startY = y, endX = x, endY = y;
    
    // Find actual start and end of the sequence
    while (isValidPosition(startX - dx, startY - dy) && grid[startX - dx][startY - dy] == player) {
        startX -= dx;
        startY -= dy;
    }
    
    while (isValidPosition(endX + dx, endY + dy) && grid[endX + dx][endY + dy] == player) {
        endX += dx;
        endY += dy;
    }
    
    // Check if both ends are open (empty)
    bool startOpen = isValidPosition(startX - dx, startY - dy) && 
                     grid[startX - dx][startY - dy] == EMPTY;
    bool endOpen = isValidPosition(endX + dx, endY + dy) && 
                   grid[endX + dx][endY + dy] == EMPTY;
    
    return startOpen && endOpen;
}

// Rendering
void Board::draw(sf::RenderWindow& window) const {
    // Draw board background
    sf::RectangleShape background(sf::Vector2f(size * CELL_SIZE, size * CELL_SIZE));
    background.setPosition(sf::Vector2f(50, 50));
    background.setFillColor(BOARD_COLOR);
    window.draw(background);
    
    // Draw grid lines
    for (int i = 0; i <= size; i++) {
        // Vertical lines
        sf::RectangleShape vLine(sf::Vector2f(1, size * CELL_SIZE));
        vLine.setPosition(sf::Vector2f(50 + i * CELL_SIZE, 50));
        vLine.setFillColor(LINE_COLOR);
        window.draw(vLine);
        
        // Horizontal lines
        sf::RectangleShape hLine(sf::Vector2f(size * CELL_SIZE, 1));
        hLine.setPosition(sf::Vector2f(50, 50 + i * CELL_SIZE));
        hLine.setFillColor(LINE_COLOR);
        window.draw(hLine);
    }
    
    // Draw pieces
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            if (grid[x][y] != EMPTY) {
                sf::CircleShape piece(CELL_SIZE / 2 - 2);
                piece.setPosition(sf::Vector2f(50 + x * CELL_SIZE + 2, 50 + y * CELL_SIZE + 2));
                
                if (grid[x][y] == BLACK) {
                    piece.setFillColor(BLACK_STONE);
                    piece.setOutlineColor(sf::Color::White);
                } else {
                    piece.setFillColor(WHITE_STONE);
                    piece.setOutlineColor(sf::Color::Black);
                }
                piece.setOutlineThickness(1);
                
                window.draw(piece);
            }
        }
    }
}

void Board::drawCaptureCount(sf::RenderWindow& window, sf::Font& font) const {
    // Draw black captures
    sf::Text blackText(font, "Black Captures: " + std::to_string(blackCaptures), 20);
    blackText.setPosition(sf::Vector2f(size * CELL_SIZE + 70, 50));
    blackText.setFillColor(sf::Color::Black);
    window.draw(blackText);
    
    // Draw white captures
    sf::Text whiteText(font, "White Captures: " + std::to_string(whiteCaptures), 20);
    whiteText.setPosition(sf::Vector2f(size * CELL_SIZE + 70, 80));
    whiteText.setFillColor(sf::Color::White);
    window.draw(whiteText);
}