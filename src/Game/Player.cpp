/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Player.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Game/Player.hpp"
#include "AI/MinMaxAI.hpp"
#include "Game/Rules.hpp"
#include <iostream>
#include <algorithm>
#include <random>

// Base Player class implementation
Player::Player(CellState playerColor, PlayerType playerType, const std::string& playerName)
    : color(playerColor), type(playerType), name(playerName), captureCount(0) {
}

Player::~Player() {
}

CellState Player::getOpponentColor() const {
    return (color == BLACK) ? WHITE : BLACK;
}

// HumanPlayer implementation
HumanPlayer::HumanPlayer(CellState playerColor, const std::string& playerName)
    : Player(playerColor, HUMAN, playerName), pendingMove(-1, -1), hasPendingMove(false) {
}

HumanPlayer::~HumanPlayer() {
}

Position HumanPlayer::makeMove(const Board& board) {
    (void)board;
    // For human players, this method waits for input
    // The actual move is set via setPendingMove() from the game's event handling
    if (hasPendingMove) {
        Position move = pendingMove;
        clearPendingMove();
        return move;
    }
    
    // Return invalid position if no move is pending
    return Position(-1, -1);
}

void HumanPlayer::setPendingMove(int x, int y) {
    pendingMove = Position(x, y);
    hasPendingMove = true;
}

Position HumanPlayer::getMouseMove(int mouseX, int mouseY) const {
    // Convert mouse coordinates to board coordinates
    int boardX = (mouseX - 50) / CELL_SIZE;
    int boardY = (mouseY - 50) / CELL_SIZE;
    
    return Position(boardX, boardY);
}

bool HumanPlayer::isValidMousePosition(int mouseX, int mouseY) const {
    Position pos = getMouseMove(mouseX, mouseY);
    return pos.x >= 0 && pos.x < BOARD_SIZE && pos.y >= 0 && pos.y < BOARD_SIZE;
}

// AIPlayer implementation
AIPlayer::AIPlayer(CellState playerColor, int difficultyLevel, const std::string& playerName)
    : Player(playerColor, AI, playerName), difficulty(difficultyLevel), searchDepth(10) {
    
    // Create MinMaxAI engine with appropriate depth and time limit
    double timeLimit = 0.5; // 500ms time limit
    int aiDepth = std::max(10, searchDepth); // Minimum depth 10 as required
    
    minMaxEngine = std::make_unique<MinMaxAI>(playerColor, aiDepth, playerName + " (MinMax)");
    
    std::cout << "[AIPlayer] Created with difficulty " << difficulty 
              << ", depth " << aiDepth << ", time limit " << timeLimit << "s" << std::endl;
}

AIPlayer::~AIPlayer() {
}

Position AIPlayer::makeMove(const Board& board) {
    return minMaxEngine->makeMove(board);
}

void AIPlayer::onGameStart() {
    Player::onGameStart();
    if (minMaxEngine) {
        minMaxEngine->onGameStart();
    }
}

void AIPlayer::onGameEnd(bool won) {
    Player::onGameEnd(won);
    if (minMaxEngine) {
        minMaxEngine->onGameEnd(won);
    }
}

void AIPlayer::setDifficulty(int level) {
    difficulty = level;
    searchDepth = 10 + level * 2;
    if (minMaxEngine) {
        minMaxEngine->setSearchDepth(std::max(10, searchDepth));
    }
}

void AIPlayer::setSearchDepth(int depth) {
    searchDepth = std::max(10, depth); // Enforce minimum depth of 10
    if (minMaxEngine) {
        minMaxEngine->setSearchDepth(searchDepth);
    }
}

int AIPlayer::getSearchDepth() const {
    if (minMaxEngine) {
        return minMaxEngine->getSearchDepth();
    }
    return searchDepth;
}

void AIPlayer::setTimeLimit(double /*seconds*/) {
    // Time limit not used in basic MinMax
}

double AIPlayer::getTimeLimit() const {
    return 0.5; // Default value for compatibility
}

int AIPlayer::getNodesEvaluated() const {
    return 0; // Not tracked in basic MinMax
}

int AIPlayer::getTTHitRate() const {
    return 0; // No transposition table in basic MinMax
}

std::vector<Position> AIPlayer::getPrincipalVariation() const {
    return std::vector<Position>(); // Not tracked in basic MinMax
}

Position AIPlayer::findBestMove(const Board& board) {
    std::vector<Position> moves = generateMoves(board);
    
    if (moves.empty()) {
        return Position(-1, -1);
    }
    
    Position bestMove = moves[0];
    int bestScore = -10000;
    
    // Simple evaluation for now - prioritize center and avoid edges
    for (const Position& move : moves) {
        int score = evaluatePosition(board, color);
        
        // Add positional bonuses
        int centerDistance = abs(move.x - BOARD_SIZE/2) + abs(move.y - BOARD_SIZE/2);
        score += (BOARD_SIZE - centerDistance) * 2;
        
        // Check for immediate threats and opportunities
        Board tempBoard = board;
        const_cast<Board&>(tempBoard).placePiece(move.x, move.y, color);
        
        // Prioritize winning moves
        if (tempBoard.checkWin(color)) {
            return move;
        }
        
        // Block opponent's winning moves
        const_cast<Board&>(tempBoard).placePiece(move.x, move.y, getOpponentColor());
        if (tempBoard.checkWin(getOpponentColor())) {
            score += 5000;
        }
        
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }
    
    return bestMove;
}

int AIPlayer::evaluatePosition(const Board& board, CellState player) {
    int score = 0;
    
    // Basic evaluation based on captures
    score += board.getCaptureCount(player) * 100;
    score -= board.getCaptureCount(board.getOpponent(player)) * 100;
    
    // Check for potential alignments
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) == player) {
                // Add points for potential alignments
                int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
                
                for (int i = 0; i < 4; i++) {
                    int count = 1;
                    int dx = directions[i][0], dy = directions[i][1];
                    
                    // Count in positive direction
                    int nx = x + dx, ny = y + dy;
                    while (nx >= 0 && nx < board.getSize() && ny >= 0 && ny < board.getSize() && 
                           board.getCell(nx, ny) == player) {
                        count++;
                        nx += dx;
                        ny += dy;
                    }
                    
                    // Count in negative direction
                    nx = x - dx;
                    ny = y - dy;
                    while (nx >= 0 && nx < board.getSize() && ny >= 0 && ny < board.getSize() && 
                           board.getCell(nx, ny) == player) {
                        count++;
                        nx -= dx;
                        ny -= dy;
                    }
                    
                    // Score based on alignment length
                    if (count >= 2) {
                        score += count * count * 10;
                    }
                }
            }
        }
    }
    
    return score;
}

std::vector<Position> AIPlayer::generateMoves(const Board& board) {
    std::vector<Position> moves;
    
    // Generate all valid moves (filter with Rules to exclude forbidden double-threes)
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (Rules::isValidMove(board, x, y, color)) {
                moves.push_back(Position(x, y));
            }
        }
    }
    
    // If this is the first move, prefer center
    if (moves.size() == BOARD_SIZE * BOARD_SIZE) {
        moves.clear();
        moves.push_back(Position(BOARD_SIZE/2, BOARD_SIZE/2));
        return moves;
    }
    
    // Sort moves by proximity to existing stones (more tactical)
    std::vector<std::pair<Position, int>> scoredMoves;
    
    for (const Position& move : moves) {
        int proximityScore = 0;
        
        // Check 3x3 neighborhood for existing stones
        for (int dx = -2; dx <= 2; dx++) {
            for (int dy = -2; dy <= 2; dy++) {
                int nx = move.x + dx;
                int ny = move.y + dy;
                
                if (nx >= 0 && nx < board.getSize() && ny >= 0 && ny < board.getSize()) {
                    if (board.getCell(nx, ny) != EMPTY) {
                        int distance = abs(dx) + abs(dy);
                        proximityScore += (3 - distance);
                    }
                }
            }
        }
        
        if (proximityScore > 0) {
            scoredMoves.push_back({move, proximityScore});
        }
    }
    
    // Sort by score (higher is better)
    std::sort(scoredMoves.begin(), scoredMoves.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Return top candidates (limit to reduce search space)
    moves.clear();
    int maxMoves = std::min(20, static_cast<int>(scoredMoves.size()));
    
    for (int i = 0; i < maxMoves; i++) {
        moves.push_back(scoredMoves[i].first);
    }
    
    // If no moves near existing stones, return a few center-biased moves
    if (moves.empty()) {
        int center = BOARD_SIZE / 2;
        for (int r = 1; r <= 3; r++) {
            for (int dx = -r; dx <= r; dx++) {
                for (int dy = -r; dy <= r; dy++) {
                    int x = center + dx;
                    int y = center + dy;
                    if (Rules::isValidMove(board, x, y, color)) {
                        moves.push_back(Position(x, y));
                        if (moves.size() >= 10) return moves;
                    }
                }
            }
        }
    }
    
    return moves;
}