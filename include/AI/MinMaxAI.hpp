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

#include "../Game/Player.hpp"
#include "../Game/Board.hpp"
#include "Heuristic.hpp"
#include "GameTree.hpp"
#include <chrono>
#include <unordered_map>
#include <memory>

// Transposition table entry
struct TTEntry {
    int depth;
    int value;
    int flag;  // 0=exact, 1=lower bound, 2=upper bound
    Position bestMove;
    
    TTEntry() : depth(0), value(0), flag(0), bestMove(-1, -1) {}
    TTEntry(int d, int v, int f, Position move) : depth(d), value(v), flag(f), bestMove(move) {}
};

// Move scoring for move ordering
struct ScoredMove {
    Position move;
    int score;
    
    ScoredMove() : move(-1, -1), score(0) {}
    ScoredMove(Position pos, int s) : move(pos), score(s) {}
    
    bool operator<(const ScoredMove& other) const {
        return score > other.score; // Higher scores first
    }
};

class MinMaxAI : public Player {
private:
    int searchDepth;
    double timeLimit;  // Time limit in seconds
    std::unique_ptr<Heuristic> heuristic;
    std::unique_ptr<GameTree> gameTree;
    
    // Transposition table for memoization
    std::unordered_map<uint64_t, TTEntry> transpositionTable;
    
    // Performance tracking
    int nodesEvaluated;
    int ttHits;
    int ttMisses;
    std::chrono::steady_clock::time_point startTime;
    
    // Killer moves heuristic
    std::vector<std::vector<Position>> killerMoves;  // [depth][move]
    
    // Principal variation
    std::vector<Position> principalVariation;

public:
    MinMaxAI(CellState playerColor, int depth = 10, double timeLimitSec = 0.5, 
             const std::string& playerName = "MinMax AI");
    virtual ~MinMaxAI();
    
    // Override Player methods
    virtual Position makeMove(const Board& board) override;
    virtual void onGameStart() override;
    virtual void onGameEnd(bool won) override;
    
    // AI-specific configuration
    void setSearchDepth(int depth) { searchDepth = std::max(1, depth); }
    int getSearchDepth() const { return searchDepth; }
    void setTimeLimit(double seconds) { timeLimit = seconds; }
    double getTimeLimit() const { return timeLimit; }
    
    // Performance statistics
    int getNodesEvaluated() const { return nodesEvaluated; }
    int getTTHitRate() const { return ttHits + ttMisses > 0 ? (ttHits * 100) / (ttHits + ttMisses) : 0; }
    std::vector<Position> getPrincipalVariation() const { return principalVariation; }
    
private:
    // Core minimax algorithm
    int minimax(const Board& board, int depth, int alpha, int beta, bool maximizing, 
                Position& bestMove, uint64_t boardHash);
    
    // Iterative deepening
    Position iterativeDeepening(const Board& board);
    
    // Move generation and ordering
    std::vector<ScoredMove> generateOrderedMoves(const Board& board, int depth);
    std::vector<Position> generateBasicMoves(const Board& board);
    int scoreMoveOrdering(const Board& board, const Position& move, int depth);
    
    // Time management
    bool isTimeUp() const;
    double getElapsedTime() const;
    
    // Transposition table operations
    uint64_t computeBoardHash(const Board& board);
    bool probeTranspositionTable(uint64_t hash, int depth, int alpha, int beta, int& value, Position& bestMove);
    void storeTranspositionTable(uint64_t hash, int depth, int value, int flag, const Position& bestMove);
    void clearTranspositionTable();
    
    // Killer moves heuristic
    void updateKillerMove(const Position& move, int depth);
    bool isKillerMove(const Position& move, int depth);
    
    // Quiescence search (for handling tactical complications)
    int quiescenceSearch(const Board& board, int alpha, int beta, bool maximizing, int qDepth = 0);
    
    // Terminal node evaluation
    bool isTerminalNode(const Board& board, int depth);
    int evaluateTerminalNode(const Board& board);
    
    // Debug and analysis
    void printPrincipalVariation() const;
    void printSearchStatistics() const;
    
    // Threat detection for move ordering and quiescence
    bool isTacticalMove(const Board& board, const Position& move);
    std::vector<Position> findThreatMoves(const Board& board, CellState player);
    
    // Opening book (simple)
    Position getOpeningMove(const Board& board);
    bool isOpeningPhase(const Board& board) const;
};

#endif // MINMAXAI_HPP