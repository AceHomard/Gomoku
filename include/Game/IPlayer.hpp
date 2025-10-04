/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IPlayer.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/29 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/29 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IPLAYER_HPP
#define IPLAYER_HPP

#include "../Gomoku.hpp"
#include "Board.hpp"
#include <string>

enum PlayerType {
    HUMAN = 0,
    AI = 1
};

class IPlayer {
protected:
    CellState color;
    PlayerType type;
    std::string name;
    int captureCount;

public:
    IPlayer(CellState playerColor, PlayerType playerType, const std::string& playerName)
        : color(playerColor), type(playerType), name(playerName), captureCount(0) {}
    
    virtual ~IPlayer() = default;
    
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
    CellState getOpponentColor() const {
        return (color == BLACK) ? WHITE : BLACK;
    }
    
    // Virtual methods for game events (optional overrides)
    virtual void onGameStart() {}
    virtual void onGameEnd(bool won) { (void)won; }
};

#endif // IPLAYER_HPP