/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HumanPlayer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/29 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/29 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HUMANPLAYER_HPP
#define HUMANPLAYER_HPP

#include "IPlayer.hpp"

class HumanPlayer : public IPlayer {
private:
    Position pendingMove;
    bool hasPendingMove;

public:
    HumanPlayer(CellState playerColor, const std::string& playerName = "Human");
    virtual ~HumanPlayer();
    
    virtual Position makeMove(const Board& board) override;
    
    // Human-specific input handling
    void setPendingMove(int x, int y);
    bool hasPendingMoveAvailable() const { return hasPendingMove; }
    void clearPendingMove() { hasPendingMove = false; }
};

#endif // HUMANPLAYER_HPP