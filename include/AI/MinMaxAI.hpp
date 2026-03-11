/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MinMaxAI.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINMAXAI_HPP
#define MINMAXAI_HPP

#include "../Game/IPlayer.hpp"
#include "../Game/Board.hpp"
#include "Heuristic.hpp"
#include <memory>

class MinMaxAI : public IPlayer {
private:
    int searchDepth;
    std::unique_ptr<Heuristic> heuristic;

    // Debug et performance
    mutable int nodesEvaluated;
    mutable int cutoffsCount;
    mutable bool debugMode;

public:
    MinMaxAI(CellState playerColor, int depth = 4, 
             const std::string& playerName = "MinMax AI");
    virtual ~MinMaxAI();
    
    // Override Player methods
    virtual Position makeMove(const Board& board) override;
    virtual void onGameStart() override;
    virtual void onGameEnd(bool won) override;
    
    // AI-specific configuration
    void setSearchDepth(int depth) { searchDepth = std::max(1, depth); }
    int getSearchDepth() const { return searchDepth; }

    // Debug et statistiques
    void setDebugMode(bool enabled) { debugMode = enabled; }
    int getNodesEvaluated() const { return nodesEvaluated; }
    int getCutoffsCount() const { return cutoffsCount; }

    // Setters pour tests
    void resetCounters() { nodesEvaluated = 0; cutoffsCount = 0; }
    
protected:
    // Core minimax algorithm
    int minimax(const Board& board, int depth, bool maximizing);

    // Alpha-Beta optimized algorithm (uses make/unmake for performance)
    int alphabeta(Board& board, int depth, int alpha, int beta, bool maximizing);

    // Move generation (fastMode=true skips expensive validation for inner search nodes)
    std::vector<Position> generateMoves(const Board& board, bool fastMode = false);

    // Board evaluation
    int evaluateBoard(const Board& board);

private:
    // Debug helpers
    std::string getIndent(int depth) const;
    void logNode(int depth, const Position& move, int value, bool maximizing) const;
};

#endif // MINMAXAI_HPP