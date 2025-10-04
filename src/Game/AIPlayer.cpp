/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AIPlayer.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/29 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/29 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Game/AIPlayer.hpp"
#include "Game/Board.hpp"
#include "AI/MinMaxAI.hpp"
#include <iostream>
#include <algorithm>

AIPlayer::AIPlayer(CellState playerColor, int difficultyLevel, const std::string& playerName)
    : IPlayer(playerColor, AI, playerName), difficulty(difficultyLevel), searchDepth(10) {
    
    // Create MinMaxAI engine with appropriate depth
    int aiDepth = std::max(10, searchDepth); // Minimum depth 10 as required by project
    
    minMaxEngine = std::make_unique<MinMaxAI>(playerColor, aiDepth, playerName + " (MinMax)");
    
    std::cout << "[AIPlayer] Created with difficulty " << difficulty 
              << ", depth " << aiDepth << std::endl;
}

AIPlayer::~AIPlayer() {
}

Position AIPlayer::makeMove(const Board& board) {
    return minMaxEngine->makeMove(board);
}

void AIPlayer::onGameStart() {
    if (minMaxEngine) {
        minMaxEngine->onGameStart();
    }
}

void AIPlayer::onGameEnd(bool won) {
    if (minMaxEngine) {
        minMaxEngine->onGameEnd(won);
    }
}

void AIPlayer::setDifficulty(int level) {
    difficulty = level;
    searchDepth = 10 + level * 2; // Increase depth with difficulty
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