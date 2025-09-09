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
#include <unordered_set>
#include <functional>
#include <unordered_map>

// Forward declaration
class Heuristic;

// Tree node structure
struct TreeNode {
    Board boardState;
    Position move;          // The move that led to this state
    CellState player;       // Player who made the move
    int depth;              // Depth in the tree
    int evaluation;         // Heuristic evaluation
    bool isTerminal;        // Whether this is a terminal node
    
    // Tree structure
    std::vector<std::unique_ptr<TreeNode>> children;
    TreeNode* parent;
    
    // Search information
    int alpha;
    int beta;
    bool isMaximizing;
    bool isPruned;
    
    // Statistics
    int visitCount;
    
    TreeNode() : move(-1, -1), player(EMPTY), depth(0), evaluation(0), 
                 isTerminal(false), parent(nullptr), alpha(-10000), 
                 beta(10000), isMaximizing(true), isPruned(false), visitCount(0) {}
                 
    TreeNode(const Board& board, Position m, CellState p, int d, TreeNode* par = nullptr)
        : boardState(board), move(m), player(p), depth(d), evaluation(0), 
          isTerminal(false), parent(par), alpha(-10000), beta(10000), 
          isMaximizing(d % 2 == 0), isPruned(false), visitCount(0) {}
};

// Move generation strategy
enum MoveGenerationStrategy {
    ALL_MOVES,          // Generate all valid moves
    TACTICAL_ONLY,      // Only moves near existing stones
    THREAT_RESPONSE,    // Focus on threats and responses
    OPENING_BOOK,       // Use opening book moves
    ENDGAME_FOCUSED     // Focus on winning/defensive moves
};

class GameTree {
private:
    std::unique_ptr<TreeNode> root;
    Heuristic* heuristic;           // Reference to heuristic evaluator
    int maxDepth;
    int nodesGenerated;
    int nodesPruned;
    int maxNodesLimit;              // Memory limit
    
    // Move generation parameters
    MoveGenerationStrategy strategy;
    int tacticalRadius;             // Radius for tactical move generation
    int maxMovesPerLevel;           // Limit moves per level for performance
    
    // Pruning and optimization
    bool enableAlphaBeta;
    bool enableMoveOrdering;
    bool enableNullMovePruning;
    int nullMoveDepthReduction;
    
    // Hash set for position detection (prevent infinite loops)
    std::unordered_set<uint64_t> visitedPositions;

public:
    GameTree(Heuristic* heuristicEvaluator);
    ~GameTree();
    
    // Tree operations
    void buildTree(const Board& rootBoard, CellState startingPlayer, int depth);
    void expandNode(TreeNode* node, int remainingDepth);
    std::unique_ptr<TreeNode> createChild(TreeNode* parent, const Position& move);
    
    // Tree search algorithms
    int minimax(TreeNode* node, int depth, bool maximizing);
    int alphaBeta(TreeNode* node, int depth, int alpha, int beta, bool maximizing);
    int negamax(TreeNode* node, int depth, int alpha, int beta, int color);
    
    // Move generation
    std::vector<Position> generateMoves(const Board& board, MoveGenerationStrategy strategy);
    std::vector<Position> generateAllValidMoves(const Board& board);
    std::vector<Position> generateTacticalMoves(const Board& board);
    std::vector<Position> generateThreatMoves(const Board& board, CellState player);
    std::vector<Position> generateOpeningMoves(const Board& board);
    std::vector<Position> generateEndgameMoves(const Board& board, CellState player);
    
    // Move ordering for alpha-beta optimization
    std::vector<Position> orderMoves(const Board& board, const std::vector<Position>& moves, 
                                     CellState player, int depth);
    int getMoveOrderingScore(const Board& board, const Position& move, CellState player);
    
    // Tree analysis
    TreeNode* findBestLeaf();
    std::vector<Position> extractPrincipalVariation();
    int getTreeSize() const;
    int getMaxDepthReached() const;
    double getBranchingFactor() const;
    
    // Tree navigation
    TreeNode* getRoot() { return root.get(); }
    const TreeNode* getRoot() const { return root.get(); }
    TreeNode* findNode(const std::vector<Position>& movePath);
    std::vector<Position> getPathToRoot(const TreeNode* node);
    
    // Configuration
    void setMaxDepth(int depth) { maxDepth = depth; }
    int getMaxDepth() const { return maxDepth; }
    void setMaxNodesLimit(int limit) { maxNodesLimit = limit; }
    void setMoveGenerationStrategy(MoveGenerationStrategy strat) { strategy = strat; }
    void setTacticalRadius(int radius) { tacticalRadius = radius; }
    void setMaxMovesPerLevel(int maxMoves) { maxMovesPerLevel = maxMoves; }
    
    // Pruning configuration
    void enableAlphaBetaPruning(bool enable) { enableAlphaBeta = enable; }
    void enableMoveOrderingFlag(bool enable) { enableMoveOrdering = enable; }
    void enableNullMovePruningFlag(bool enable) { enableNullMovePruning = enable; }
    void setNullMoveReduction(int reduction) { nullMoveDepthReduction = reduction; }
    
    // Statistics
    int getNodesGenerated() const { return nodesGenerated; }
    int getNodesPruned() const { return nodesPruned; }
    double getPruningEfficiency() const;
    
    // Memory management
    void clearTree();
    void pruneTree(int maxNodes);
    size_t getMemoryUsage() const;
    
    // Debug and visualization
    void printTree(int maxDepth = -1) const;
    void printNode(const TreeNode* node, int indent = 0) const;
    void printStatistics() const;
    std::string getTreeSummary() const;
    
    // Position utilities
    bool isRepeatedPosition(const Board& board);
    uint64_t computePositionHash(const Board& board);
    void addVisitedPosition(const Board& board);
    void clearVisitedPositions();

private:
    // Internal tree building helpers
    void buildTreeRecursive(TreeNode* node, int remainingDepth);
    bool shouldExpandNode(const TreeNode* node, int remainingDepth);
    bool isTerminalPosition(const Board& board, CellState player);
    
    // Move generation helpers
    bool isMoveRelevant(const Board& board, const Position& move, MoveGenerationStrategy strategy);
    bool hasNearbyStones(const Board& board, int x, int y, int radius);
    int countEmptyNeighbors(const Board& board, int x, int y);
    
    // Pruning helpers
    bool canPruneNode(const TreeNode* node, int alpha, int beta);
    void updateAlphaBeta(TreeNode* node, int value, bool maximizing);
    bool shouldTryNullMove(const TreeNode* node);
    
    // Memory management helpers
    void pruneSubtree(TreeNode* node);
    void deleteSubtree(TreeNode* node);
    size_t computeNodeSize(const TreeNode* node) const;
    
    // Evaluation caching
    std::unordered_map<uint64_t, int> evaluationCache;
    bool getCachedEvaluation(const Board& board, int& evaluation);
    void cacheEvaluation(const Board& board, int evaluation);
    void clearEvaluationCache();
    
    // Tree traversal utilities
    void traverseDepthFirst(TreeNode* node, std::function<void(TreeNode*)> visitor);
    void traverseBreadthFirst(TreeNode* node, std::function<void(TreeNode*)> visitor);
    
    // Node comparison for sorting
    static bool compareNodesByEvaluation(const std::unique_ptr<TreeNode>& a, 
                                        const std::unique_ptr<TreeNode>& b);
};

// Tree analysis utilities (standalone functions)
namespace TreeAnalysis {
    std::vector<Position> extractBestLine(const TreeNode* root);
    int computeTreeValue(const TreeNode* node);
    bool isPositionDrawn(const TreeNode* node);
    double computeWinProbability(const TreeNode* node, CellState player);
    std::string positionToString(const Position& pos);
    std::vector<std::string> generateTreeDescription(const TreeNode* root);
}

#endif // GAMETREE_HPP