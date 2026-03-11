/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AIPlayer.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/29 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/29 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AIPLAYER_HPP
#define AIPLAYER_HPP

#include "IPlayer.hpp"
#include <memory>

// Forward declaration
class MinMaxAI;

class AIPlayer : public IPlayer {
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
    
    // AI-specific configuration
    void setDifficulty(int level);
    int getDifficulty() const { return difficulty; }
    void setSearchDepth(int depth);
    int getSearchDepth() const;
    int getMaxDepthEverReached() const;
};

#endif // AIPLAYER_HPP