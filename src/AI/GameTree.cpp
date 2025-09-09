/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GameTree.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AI/GameTree.hpp"
#include "AI/Heuristic.hpp"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <queue>
#include <functional>
#include <set>

const int INFINITY_VAL = 100000;
const int WIN_VALUE = 50000;

GameTree::GameTree(Heuristic* heuristicEvaluator) 
    : heuristic(heuristicEvaluator), maxDepth(10), nodesGenerated(0), nodesPruned(0),
      maxNodesLimit(100000), strategy(TACTICAL_ONLY), tacticalRadius(2), maxMovesPerLevel(20),
      enableAlphaBeta(true), enableMoveOrdering(true), enableNullMovePruning(false),
      nullMoveDepthReduction(2) {
}

GameTree::~GameTree() {
    clearTree();
}

void GameTree::buildTree(const Board& rootBoard, CellState startingPlayer, int depth) {
    clearTree();
    maxDepth = depth;
    nodesGenerated = 0;
    nodesPruned = 0;
    visitedPositions.clear();
    
    // Create root node
    root = std::make_unique<TreeNode>(rootBoard, Position(-1, -1), startingPlayer, 0);
    nodesGenerated++;
    
    // Add root position to visited set
    addVisitedPosition(rootBoard);
    
    // Start building the tree
    buildTreeRecursive(root.get(), maxDepth);
    
    std::cout << "[GameTree] Built tree with " << nodesGenerated << " nodes, "
              << nodesPruned << " pruned" << std::endl;
}

void GameTree::buildTreeRecursive(TreeNode* node, int remainingDepth) {
    if (remainingDepth <= 0 || !shouldExpandNode(node, remainingDepth)) {
        node->isTerminal = true;
        node->evaluation = heuristic->evaluatePosition(node->boardState, node->player);
        return;
    }
    
    // Check for terminal positions (wins/losses)
    if (isTerminalPosition(node->boardState, node->player)) {
        node->isTerminal = true;
        if (node->boardState.checkWin(node->player) || node->boardState.checkCaptureWin(node->player)) {
            node->evaluation = WIN_VALUE - node->depth; // Prefer shorter wins
        } else {
            node->evaluation = -WIN_VALUE + node->depth; // Delay losses
        }
        return;
    }
    
    // Generate moves for the current position
    std::vector<Position> moves = generateMoves(node->boardState, strategy);
    
    if (enableMoveOrdering) {
        CellState nextPlayer = (node->player == BLACK) ? WHITE : BLACK;
        moves = orderMoves(node->boardState, moves, nextPlayer, node->depth);
    }
    
    // Limit the number of moves to consider
    if (static_cast<int>(moves.size()) > maxMovesPerLevel) {
        moves.resize(maxMovesPerLevel);
    }
    
    // Create child nodes
    for (const Position& move : moves) {
        if (nodesGenerated >= maxNodesLimit) {
            break; // Memory limit reached
        }
        
        auto child = createChild(node, move);
        if (child) {
            node->children.push_back(std::move(child));
            nodesGenerated++;
            
            // Recursively build subtree
            buildTreeRecursive(node->children.back().get(), remainingDepth - 1);
        }
    }
    
    // If no children were created, this is a terminal node
    if (node->children.empty()) {
        node->isTerminal = true;
        node->evaluation = heuristic->evaluatePosition(node->boardState, node->player);
    }
}

std::unique_ptr<TreeNode> GameTree::createChild(TreeNode* parent, const Position& move) {
    Board childBoard = parent->boardState;
    CellState nextPlayer = (parent->player == BLACK) ? WHITE : BLACK;
    
    // Make the move
    if (!childBoard.placePiece(move.x, move.y, nextPlayer)) {
        return nullptr; // Invalid move
    }
    
    // Handle captures
    std::vector<Position> captures = childBoard.checkCaptures(move.x, move.y, nextPlayer);
    childBoard.executeCaptures(captures);
    
    // Check for repeated positions
    if (isRepeatedPosition(childBoard)) {
        return nullptr; // Skip repeated positions
    }
    
    // Create child node
    auto child = std::make_unique<TreeNode>(childBoard, move, nextPlayer, parent->depth + 1, parent);
    
    // Add to visited positions
    addVisitedPosition(childBoard);
    
    return child;
}

int GameTree::alphaBeta(TreeNode* node, int depth, int alpha, int beta, bool maximizing) {
    node->visitCount++;
    
    if (depth <= 0 || node->isTerminal) {
        node->evaluation = heuristic->evaluatePosition(node->boardState, 
                                                      maximizing ? node->player : 
                                                      (node->player == BLACK ? WHITE : BLACK));
        return node->evaluation;
    }
    
    // Check for immediate wins/losses
    CellState currentPlayer = maximizing ? node->player : (node->player == BLACK ? WHITE : BLACK);
    if (node->boardState.checkWin(currentPlayer) || node->boardState.checkCaptureWin(currentPlayer)) {
        node->evaluation = maximizing ? WIN_VALUE - node->depth : -WIN_VALUE + node->depth;
        return node->evaluation;
    }
    
    int bestValue = maximizing ? -INFINITY_VAL : INFINITY_VAL;
    
    for (auto& child : node->children) {
        int value = alphaBeta(child.get(), depth - 1, alpha, beta, !maximizing);
        
        if (maximizing) {
            bestValue = std::max(bestValue, value);
            alpha = std::max(alpha, value);
        } else {
            bestValue = std::min(bestValue, value);
            beta = std::min(beta, value);
        }
        
        // Alpha-beta pruning
        if (beta <= alpha) {
            nodesPruned++;
            break;
        }
    }
    
    node->evaluation = bestValue;
    return bestValue;
}

int GameTree::minimax(TreeNode* node, int depth, bool maximizing) {
    return alphaBeta(node, depth, -INFINITY_VAL, INFINITY_VAL, maximizing);
}

std::vector<Position> GameTree::generateMoves(const Board& board, MoveGenerationStrategy strategy) {
    switch (strategy) {
        case ALL_MOVES:
            return generateAllValidMoves(board);
        case TACTICAL_ONLY:
            return generateTacticalMoves(board);
        case THREAT_RESPONSE:
            return generateThreatMoves(board, BLACK); // TODO: Pass correct player
        case OPENING_BOOK:
            return generateOpeningMoves(board);
        case ENDGAME_FOCUSED:
            return generateEndgameMoves(board, BLACK); // TODO: Pass correct player
        default:
            return generateTacticalMoves(board);
    }
}

std::vector<Position> GameTree::generateAllValidMoves(const Board& board) {
    std::vector<Position> moves;
    
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.isValidMove(x, y)) {
                moves.push_back(Position(x, y));
            }
        }
    }
    
    return moves;
}

std::vector<Position> GameTree::generateTacticalMoves(const Board& board) {
    std::vector<Position> moves;
    std::set<std::pair<int,int>> moveSet;
    
    // Find all moves within tacticalRadius of existing stones
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) != EMPTY) {
                // Add all valid moves in radius around this stone
                for (int dx = -tacticalRadius; dx <= tacticalRadius; dx++) {
                    for (int dy = -tacticalRadius; dy <= tacticalRadius; dy++) {
                        int nx = x + dx, ny = y + dy;
                        if (board.isValidMove(nx, ny)) {
                            moveSet.insert({nx, ny});
                        }
                    }
                }
            }
        }
    }
    
    // Convert set to vector
    for (const auto& move : moveSet) {
        moves.push_back(Position(move.first, move.second));
    }
    
    // If no tactical moves (empty board), play in center
    if (moves.empty()) {
        int center = BOARD_SIZE / 2;
        if (board.isValidMove(center, center)) {
            moves.push_back(Position(center, center));
        }
    }
    
    return moves;
}

std::vector<Position> GameTree::generateThreatMoves(const Board& board, CellState player) {
    std::vector<Position> moves;
    
    // Find immediate winning moves
    std::vector<Position> winningMoves = heuristic->findWinningMoves(board, player);
    moves.insert(moves.end(), winningMoves.begin(), winningMoves.end());
    
    // Find defensive moves
    std::vector<Position> defensiveMoves = heuristic->findDefensiveMoves(board, player);
    moves.insert(moves.end(), defensiveMoves.begin(), defensiveMoves.end());
    
    // Find threat-creating moves
    std::vector<Threat> threats = heuristic->findThreats(board, player);
    for (const Threat& threat : threats) {
        if (threat.type == OPEN_FOUR || threat.type == OPEN_THREE) {
            moves.push_back(threat.position);
        }
    }
    
    // Remove duplicates
    std::sort(moves.begin(), moves.end(), [](const Position& a, const Position& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });
    moves.erase(std::unique(moves.begin(), moves.end()), moves.end());
    
    // If no threat moves, fall back to tactical moves
    if (moves.empty()) {
        return generateTacticalMoves(board);
    }
    
    return moves;
}

std::vector<Position> GameTree::generateOpeningMoves(const Board& board) {
    std::vector<Position> moves;
    int center = BOARD_SIZE / 2;
    
    // Count existing stones
    int stoneCount = 0;
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) != EMPTY) {
                stoneCount++;
            }
        }
    }
    
    if (stoneCount == 0) {
        // First move: center
        moves.push_back(Position(center, center));
    } else if (stoneCount <= 4) {
        // Early opening: play near center
        for (int r = 1; r <= 2; r++) {
            for (int dx = -r; dx <= r; dx++) {
                for (int dy = -r; dy <= r; dy++) {
                    if (abs(dx) + abs(dy) <= r) { // Diamond pattern
                        int x = center + dx, y = center + dy;
                        if (board.isValidMove(x, y)) {
                            moves.push_back(Position(x, y));
                        }
                    }
                }
            }
        }
    } else {
        // Transition to tactical play
        return generateTacticalMoves(board);
    }
    
    return moves;
}

std::vector<Position> GameTree::generateEndgameMoves(const Board& board, CellState player) {
    // Focus on forcing moves and critical defensive moves
    return generateThreatMoves(board, player);
}

std::vector<Position> GameTree::orderMoves(const Board& board, const std::vector<Position>& moves, 
                                          CellState player, int /* depth */) {
    std::vector<std::pair<Position, int>> scoredMoves;
    
    for (const Position& move : moves) {
        int score = getMoveOrderingScore(board, move, player);
        scoredMoves.emplace_back(move, score);
    }
    
    // Sort by score (higher first)
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<Position> orderedMoves;
    for (const auto& scored : scoredMoves) {
        orderedMoves.push_back(scored.first);
    }
    
    return orderedMoves;
}

int GameTree::getMoveOrderingScore(const Board& board, const Position& move, CellState player) {
    // Use heuristic to evaluate the move
    return heuristic->evaluateMove(board, move, player);
}

bool GameTree::shouldExpandNode(const TreeNode* node, int remainingDepth) {
    // Don't expand terminal nodes
    if (node->isTerminal) return false;
    
    // Don't expand beyond depth limit
    if (remainingDepth <= 0) return false;
    
    // Don't expand if we've hit the node limit
    if (nodesGenerated >= maxNodesLimit) return false;
    
    // Don't expand if this is a repeated position
    if (isRepeatedPosition(node->boardState)) return false;
    
    return true;
}

bool GameTree::isTerminalPosition(const Board& board, CellState /* player */) {
    // Check for win conditions
    if (board.checkWin(BLACK) || board.checkWin(WHITE)) return true;
    if (board.checkCaptureWin(BLACK) || board.checkCaptureWin(WHITE)) return true;
    
    // Check if board is full (shouldn't happen in Gomoku)
    bool hasEmptyCell = false;
    for (int x = 0; x < board.getSize() && !hasEmptyCell; x++) {
        for (int y = 0; y < board.getSize() && !hasEmptyCell; y++) {
            if (board.getCell(x, y) == EMPTY) {
                hasEmptyCell = true;
            }
        }
    }
    
    return !hasEmptyCell;
}

std::vector<Position> GameTree::extractPrincipalVariation() {
    std::vector<Position> pv;
    TreeNode* current = root.get();
    
    while (current && !current->children.empty()) {
        // Find the best child
        TreeNode* bestChild = nullptr;
        int bestValue = -INFINITY_VAL;
        
        for (auto& child : current->children) {
            if (child->evaluation > bestValue) {
                bestValue = child->evaluation;
                bestChild = child.get();
            }
        }
        
        if (bestChild) {
            pv.push_back(bestChild->move);
            current = bestChild;
        } else {
            break;
        }
    }
    
    return pv;
}

int GameTree::getTreeSize() const {
    return nodesGenerated;
}

double GameTree::getPruningEfficiency() const {
    return nodesGenerated > 0 ? (double)nodesPruned / nodesGenerated : 0.0;
}

void GameTree::clearTree() {
    root.reset();
    evaluationCache.clear();
    visitedPositions.clear();
    nodesGenerated = 0;
    nodesPruned = 0;
}

bool GameTree::isRepeatedPosition(const Board& board) {
    uint64_t hash = computePositionHash(board);
    return visitedPositions.find(hash) != visitedPositions.end();
}

uint64_t GameTree::computePositionHash(const Board& board) {
    uint64_t hash = 0;
    
    // Simple hash function - in production, use proper Zobrist hashing
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            CellState cell = board.getCell(x, y);
            if (cell != EMPTY) {
                hash ^= (uint64_t(cell) << ((x * board.getSize() + y) % 60)) ^ (x * 31 + y * 37);
            }
        }
    }
    
    return hash;
}

void GameTree::addVisitedPosition(const Board& board) {
    uint64_t hash = computePositionHash(board);
    visitedPositions.insert(hash);
}

void GameTree::clearVisitedPositions() {
    visitedPositions.clear();
}

void GameTree::printTree(int /* maxDepth */) const {
    if (!root) {
        std::cout << "[GameTree] Tree is empty" << std::endl;
        return;
    }
    
    std::cout << "[GameTree] Tree structure:" << std::endl;
    printNode(root.get(), 0);
}

void GameTree::printNode(const TreeNode* node, int indent) const {
    if (!node) return;
    
    std::string indentStr(indent * 2, ' ');
    std::cout << indentStr << "Move: (" << node->move.x << "," << node->move.y 
              << ") Player: " << (node->player == BLACK ? "BLACK" : "WHITE")
              << " Depth: " << node->depth << " Eval: " << node->evaluation
              << " Children: " << node->children.size() << std::endl;
    
    // Limit depth to avoid too much output
    if (indent < 3) {
        for (const auto& child : node->children) {
            printNode(child.get(), indent + 1);
        }
    }
}

void GameTree::printStatistics() const {
    std::cout << "[GameTree] Statistics:" << std::endl;
    std::cout << "  - Nodes generated: " << nodesGenerated << std::endl;
    std::cout << "  - Nodes pruned: " << nodesPruned << std::endl;
    std::cout << "  - Pruning efficiency: " << std::fixed << std::setprecision(2) 
              << getPruningEfficiency() * 100 << "%" << std::endl;
    std::cout << "  - Max depth: " << maxDepth << std::endl;
    std::cout << "  - Cache entries: " << evaluationCache.size() << std::endl;
}

// TreeAnalysis namespace functions
namespace TreeAnalysis {
    std::vector<Position> extractBestLine(const TreeNode* root) {
        std::vector<Position> line;
        const TreeNode* current = root;
        
        while (current && !current->children.empty()) {
            const TreeNode* bestChild = nullptr;
            int bestValue = -INFINITY_VAL;
            
            for (const auto& child : current->children) {
                if (child->evaluation > bestValue) {
                    bestValue = child->evaluation;
                    bestChild = child.get();
                }
            }
            
            if (bestChild) {
                line.push_back(bestChild->move);
                current = bestChild;
            } else {
                break;
            }
        }
        
        return line;
    }
    
    std::string positionToString(const Position& pos) {
        return "(" + std::to_string(pos.x) + "," + std::to_string(pos.y) + ")";
    }
}