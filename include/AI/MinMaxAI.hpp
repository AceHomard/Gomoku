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
#include <chrono>
#include <array>
#include <unordered_map>

enum TTFlag { TT_EXACT, TT_LOWERBOUND, TT_UPPERBOUND };

struct TTEntry {
    uint64_t hash;       // Full hash for collision detection
    int depth;
    int value;
    TTFlag flag;
    Position bestMove;
};

class MinMaxAI : public IPlayer {
private:
    int searchDepth;
    int timeLimitMs;
    std::unique_ptr<Heuristic> heuristic;

    // Iterative deepening
    mutable std::chrono::high_resolution_clock::time_point searchStartTime;
    mutable bool timeExpired;
    mutable bool inGuaranteedDepth;  // Skip time checks during MIN_DEPTH iterations

    // Killer move heuristic (2 slots per depth level)
    static constexpr int MAX_KILLER_SLOTS = 2;
    mutable std::vector<std::array<Position, 2>> killerMoves;

    // Transposition table
    static constexpr size_t MAX_TT_SIZE = 1 << 20; // ~1M entries
    mutable std::unordered_map<uint64_t, TTEntry> transpositionTable;
    mutable int ttHits;

    // Debug et performance
    mutable int nodesEvaluated;
    mutable int cutoffsCount;
    mutable int depthReached;
    mutable int maxDepthEverReached;  // Max depth across entire game
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
    int getDepthReached() const { return depthReached; }
    int getMaxDepthEverReached() const { return maxDepthEverReached; }

    int getTTHits() const { return ttHits; }

    // Setters pour tests
    void resetCounters() { nodesEvaluated = 0; cutoffsCount = 0; depthReached = 0; ttHits = 0; }
    
protected:
    // Core minimax algorithm
    int minimax(const Board& board, int depth, bool maximizing);

    // Alpha-Beta optimized algorithm
    int alphabeta(const Board& board, int depth, int alpha, int beta, bool maximizing);

    // Move generation
    std::vector<Position> generateMoves(const Board& board);

    // Board evaluation
    int evaluateBoard(const Board& board);

private:
    // Time management
    bool isTimeUp() const;

    // Debug helpers
    std::string getIndent(int depth) const;
    void logNode(int depth, const Position& move, int value, bool maximizing) const;
};

#endif // MINMAXAI_HPP