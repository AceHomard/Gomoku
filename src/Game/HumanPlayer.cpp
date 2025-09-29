/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HumanPlayer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/29 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/29 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Game/HumanPlayer.hpp"
#include "Game/Board.hpp"

HumanPlayer::HumanPlayer(CellState playerColor, const std::string& playerName)
    : IPlayer(playerColor, HUMAN, playerName), pendingMove(-1, -1), hasPendingMove(false) {
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