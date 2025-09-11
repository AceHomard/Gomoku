/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GameTree.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GAMETREE_HPP
#define GAMETREE_HPP

#include "../Game/Board.hpp"
#include <vector>
#include <memory>

// Forward declaration
class Heuristic;

// Simplified tree node structure
struct TreeNode {
    Board boardState;
    Position move;
    CellState player;
    int depth;
    int evaluation;
    
    TreeNode() : move(-1, -1), player(EMPTY), depth(0), evaluation(0) {}
    TreeNode(const Board& board, Position m, CellState p, int d)
        : boardState(board), move(m), player(p), depth(d), evaluation(0) {}
};

// Simplified move generation strategy
enum MoveGenerationStrategy {
    ALL_MOVES,
    TACTICAL_ONLY,
    THREAT_RESPONSE,
    OPENING_BOOK,
    ENDGAME_FOCUSED
};

class GameTree {
private:
    Heuristic* heuristic;
    int maxDepth;

public:
    GameTree(Heuristic* heuristicEvaluator);
    ~GameTree();
    
    // Basic tree operations - simplified implementations
    void buildTree(const Board& rootBoard, CellState startingPlayer, int depth);
    std::vector<Position> generateMoves(const Board& board, MoveGenerationStrategy strategy);
    std::vector<Position> generateTacticalMoves(const Board& board);
    std::vector<Position> generateThreatMoves(const Board& board, CellState player);
    int getMoveOrderingScore(const Board& board, const Position& move, CellState player);
    
    // Configuration
    void setMaxDepth(int depth) { maxDepth = depth; }
    void enableAlphaBetaPruning(bool enable) { (void)enable; } // Stub
    void enableMoveOrderingFlag(bool enable) { (void)enable; } // Stub
    void setMoveGenerationStrategy(MoveGenerationStrategy strat) { (void)strat; } // Stub
    void setTacticalRadius(int radius) { (void)radius; } // Stub
    void setMaxMovesPerLevel(int maxMoves) { (void)maxMoves; } // Stub
};

#endif // GAMETREE_HPP