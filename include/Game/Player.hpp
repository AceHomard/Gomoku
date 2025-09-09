/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Player.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "../Gomoku.hpp"
#include "Board.hpp"
#include <string>
#include <memory>
#include <vector>

enum PlayerType {
    HUMAN = 0,
    AI = 1
};

class Player {
protected:
    CellState color;
    PlayerType type;
    std::string name;
    int captureCount;

public:
    Player(CellState playerColor, PlayerType playerType, const std::string& playerName);
    virtual ~Player();
    
    // Pure virtual method - must be implemented by derived classes
    virtual Position makeMove(const Board& board) = 0;
    
    // Getters
    CellState getColor() const { return color; }
    PlayerType getType() const { return type; }
    std::string getName() const { return name; }
    int getCaptureCount() const { return captureCount; }
    
    // Setters
    void setCaptureCount(int count) { captureCount = count; }
    void addCapture(int count = 1) { captureCount += count; }
    
    // Utility methods
    bool isHuman() const { return type == HUMAN; }
    bool isAI() const { return type == AI; }
    CellState getOpponentColor() const;
    
    // Virtual methods for different player behavior
    virtual void onGameStart() {}
    virtual void onGameEnd(bool won) { (void)won; }
    virtual void onCapture(int capturedCount) { (void)capturedCount; }
};

class HumanPlayer : public Player {
private:
    Position pendingMove;
    bool hasPendingMove;

public:
    HumanPlayer(CellState playerColor, const std::string& playerName = "Human");
    virtual ~HumanPlayer();
    
    virtual Position makeMove(const Board& board) override;
    
    // Human-specific methods
    void setPendingMove(int x, int y);
    bool hasPendingMoveAvailable() const { return hasPendingMove; }
    void clearPendingMove() { hasPendingMove = false; }
    
    // Input handling
    Position getMouseMove(int mouseX, int mouseY) const;
    bool isValidMousePosition(int mouseX, int mouseY) const;
};

// Forward declaration for MinMaxAI
class MinMaxAI;

// AI player using MinMax algorithm
class AIPlayer : public Player {
private:
    int difficulty;
    int searchDepth;
    std::unique_ptr<MinMaxAI> minMaxEngine;

public:
    AIPlayer(CellState playerColor, int difficultyLevel = 1, const std::string& playerName = "AI");
    virtual ~AIPlayer();
    
    virtual Position makeMove(const Board& board) override;
    virtual void onGameStart() override;
    virtual void onGameEnd(bool won) override;
    
    // AI-specific methods
    void setDifficulty(int level);
    int getDifficulty() const { return difficulty; }
    void setSearchDepth(int depth);
    int getSearchDepth() const;
    void setTimeLimit(double seconds);
    double getTimeLimit() const;
    
    // Performance and debug information
    int getNodesEvaluated() const;
    int getTTHitRate() const;
    std::vector<Position> getPrincipalVariation() const;

private:
    // Legacy methods (kept for compatibility but use MinMaxAI internally)
    Position findBestMove(const Board& board);
    int evaluatePosition(const Board& board, CellState player);
    std::vector<Position> generateMoves(const Board& board);
};

#endif // PLAYER_HPP