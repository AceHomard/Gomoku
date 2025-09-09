/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MinMaxAI.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AI/MinMaxAI.hpp"
#include <algorithm>
#include <limits>
#include <iostream>
#include <iomanip>

const int INFINITY_VAL = 100000;
const int WIN_VALUE = 50000;
const int LOSE_VALUE = -50000;
const int MAX_QUIESCENCE_DEPTH = 4;

MinMaxAI::MinMaxAI(CellState playerColor, int depth, double timeLimitSec, const std::string& playerName)
    : Player(playerColor, AI, playerName), searchDepth(std::max(1, depth)), timeLimit(timeLimitSec),
      heuristic(std::make_unique<Heuristic>()), gameTree(std::make_unique<GameTree>(heuristic.get())),
      nodesEvaluated(0), ttHits(0), ttMisses(0) {
    
    // Initialize killer moves table
    killerMoves.resize(searchDepth + 5);
    for (auto& level : killerMoves) {
        level.reserve(2); // Usually 2 killer moves per level
    }
    
    // Configure game tree
    gameTree->setMaxDepth(searchDepth);
    gameTree->enableAlphaBetaPruning(true);
    gameTree->enableMoveOrderingFlag(true);
    gameTree->setMoveGenerationStrategy(TACTICAL_ONLY);
    gameTree->setTacticalRadius(2);
    gameTree->setMaxMovesPerLevel(25);
    
    std::cout << "[MinMaxAI] Initialized with depth=" << searchDepth 
              << ", time limit=" << timeLimit << "s" << std::endl;
}

MinMaxAI::~MinMaxAI() {
    clearTranspositionTable();
}

Position MinMaxAI::makeMove(const Board& board) {
    // Reset performance counters
    nodesEvaluated = 0;
    ttHits = 0;
    ttMisses = 0;
    principalVariation.clear();
    startTime = std::chrono::steady_clock::now();
    
    std::cout << "[MinMaxAI] Thinking..." << std::endl;
    
    // Check for immediate win or required defense
    std::vector<Position> winningMoves = heuristic->findWinningMoves(board, color);
    if (!winningMoves.empty()) {
        std::cout << "[MinMaxAI] Found immediate winning move!" << std::endl;
        return winningMoves[0];
    }
    
    std::vector<Position> defensiveMoves = heuristic->findDefensiveMoves(board, color);
    if (!defensiveMoves.empty()) {
        std::cout << "[MinMaxAI] Must defend!" << std::endl;
        // Still use minimax but prioritize defensive moves
    }
    
    Position bestMove;
    if (isOpeningPhase(board)) {
        bestMove = getOpeningMove(board);
        if (bestMove.x != -1 && bestMove.y != -1) {
            std::cout << "[MinMaxAI] Using opening move" << std::endl;
            return bestMove;
        }
    }
    
    // Use iterative deepening for better time management
    bestMove = iterativeDeepening(board);
    
    double elapsedTime = getElapsedTime();
    std::cout << "[MinMaxAI] Move selected in " << std::fixed << std::setprecision(3) 
              << elapsedTime << "s" << std::endl;
    
    printSearchStatistics();
    
    if (bestMove.x == -1 || bestMove.y == -1) {
        // Fallback to simple move generation
        std::vector<Position> moves = generateBasicMoves(board);
        if (!moves.empty()) {
            bestMove = moves[0];
            std::cout << "[MinMaxAI] Using fallback move" << std::endl;
        }
    }
    
    return bestMove;
}

Position MinMaxAI::iterativeDeepening(const Board& board) {
    Position bestMove(-1, -1);
    
    for (int depth = 1; depth <= searchDepth && !isTimeUp(); depth++) {
        Position currentBestMove(-1, -1);
        uint64_t boardHash = computeBoardHash(board);
        
        int value = minimax(board, depth, -INFINITY_VAL, INFINITY_VAL, true, currentBestMove, boardHash);
        
        if (!isTimeUp() && currentBestMove.x != -1 && currentBestMove.y != -1) {
            bestMove = currentBestMove;
            
            // Update principal variation
            if (principalVariation.empty() || principalVariation[0] != bestMove) {
                principalVariation.clear();
                principalVariation.push_back(bestMove);
            }
            
            std::cout << "[MinMaxAI] Depth " << depth << " completed, best move: (" 
                      << bestMove.x << "," << bestMove.y << "), value: " << value << std::endl;
        }
        
        // If we found a definitive win, no need to search deeper
        if (value >= WIN_VALUE * 0.9) {
            std::cout << "[MinMaxAI] Found winning line at depth " << depth << std::endl;
            break;
        }
        
        // If time is running out, break
        if (getElapsedTime() > timeLimit * 0.8) {
            std::cout << "[MinMaxAI] Time limit approaching, stopping search" << std::endl;
            break;
        }
    }
    
    return bestMove;
}

int MinMaxAI::minimax(const Board& board, int depth, int alpha, int beta, bool maximizing, 
                      Position& bestMove, uint64_t boardHash) {
    
    nodesEvaluated++;
    
    // Check time limit
    if (nodesEvaluated % 1000 == 0 && isTimeUp()) {
        return maximizing ? alpha : beta;
    }
    
    // Check transposition table
    Position ttBestMove(-1, -1);
    int ttValue;
    if (probeTranspositionTable(boardHash, depth, alpha, beta, ttValue, ttBestMove)) {
        bestMove = ttBestMove;
        return ttValue;
    }
    
    // Terminal node check
    if (isTerminalNode(board, depth)) {
        int value = evaluateTerminalNode(board);
        storeTranspositionTable(boardHash, depth, value, 0, Position(-1, -1)); // Exact value
        return value;
    }
    
    // Generate and order moves
    std::vector<ScoredMove> orderedMoves = generateOrderedMoves(board, depth);
    if (orderedMoves.empty()) {
        int value = heuristic->evaluatePosition(board, maximizing ? color : getOpponentColor());
        storeTranspositionTable(boardHash, depth, value, 0, Position(-1, -1));
        return value;
    }
    
    Position currentBestMove(-1, -1);
    int bestValue = maximizing ? -INFINITY_VAL : INFINITY_VAL;
    int originalAlpha = alpha;
    int originalBeta = beta;
    
    for (const auto& scoredMove : orderedMoves) {
        if (isTimeUp()) break;
        
        Position move = scoredMove.move;
        
        // Make the move on a copy of the board
        Board nextBoard = board;
        if (!nextBoard.placePiece(move.x, move.y, maximizing ? color : getOpponentColor())) {
            continue;
        }
        
        // Handle captures
        std::vector<Position> captures = nextBoard.checkCaptures(move.x, move.y, maximizing ? color : getOpponentColor());
        nextBoard.executeCaptures(captures);
        
        uint64_t nextHash = computeBoardHash(nextBoard);
        Position nextBestMove(-1, -1);
        
        int value = minimax(nextBoard, depth - 1, alpha, beta, !maximizing, nextBestMove, nextHash);
        
        if (maximizing) {
            if (value > bestValue) {
                bestValue = value;
                currentBestMove = move;
            }
            alpha = std::max(alpha, value);
        } else {
            if (value < bestValue) {
                bestValue = value;
                currentBestMove = move;
            }
            beta = std::min(beta, value);
        }
        
        // Alpha-beta pruning
        if (beta <= alpha) {
            // Update killer move
            updateKillerMove(move, depth);
            break;
        }
    }
    
    bestMove = currentBestMove;
    
    // Store in transposition table
    int flag;
    if (bestValue <= originalAlpha) {
        flag = 2; // Upper bound
    } else if (bestValue >= originalBeta) {
        flag = 1; // Lower bound  
    } else {
        flag = 0; // Exact value
    }
    
    storeTranspositionTable(boardHash, depth, bestValue, flag, currentBestMove);
    
    return bestValue;
}

std::vector<ScoredMove> MinMaxAI::generateOrderedMoves(const Board& board, int depth) {
    std::vector<Position> moves = generateBasicMoves(board);
    std::vector<ScoredMove> scoredMoves;
    scoredMoves.reserve(moves.size());
    
    for (const Position& move : moves) {
        int score = scoreMoveOrdering(board, move, depth);
        scoredMoves.emplace_back(move, score);
    }
    
    // Sort moves by score (highest first)
    std::sort(scoredMoves.begin(), scoredMoves.end());
    
    // Limit number of moves to consider
    int maxMoves = std::min(static_cast<int>(scoredMoves.size()), 
                           depth >= 6 ? 15 : (depth >= 4 ? 20 : 25));
    if (static_cast<int>(scoredMoves.size()) > maxMoves) {
        scoredMoves.resize(maxMoves);
    }
    
    return scoredMoves;
}

std::vector<Position> MinMaxAI::generateBasicMoves(const Board& board) {
    return heuristic->getRelevantMoves(board);
}

int MinMaxAI::scoreMoveOrdering(const Board& board, const Position& move, int depth) {
    int score = 0;
    
    // Killer moves get highest priority after hash moves
    if (isKillerMove(move, depth)) {
        score += 9000;
    }
    
    // Evaluate the move using heuristic
    score += heuristic->evaluateMove(board, move, color);
    
    // Tactical moves get priority
    if (isTacticalMove(board, move)) {
        score += 1000;
    }
    
    // Center preference for early game
    if (isOpeningPhase(board)) {
        int centerDist = abs(move.x - BOARD_SIZE/2) + abs(move.y - BOARD_SIZE/2);
        score += (BOARD_SIZE - centerDist) * 10;
    }
    
    return score;
}

bool MinMaxAI::isTimeUp() const {
    return getElapsedTime() >= timeLimit;
}

double MinMaxAI::getElapsedTime() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return duration.count() / 1000.0;
}

uint64_t MinMaxAI::computeBoardHash(const Board& board) {
    uint64_t hash = 0;
    // Simple hash function - in production, use proper Zobrist hashing
    
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

bool MinMaxAI::probeTranspositionTable(uint64_t hash, int depth, int alpha, int beta, 
                                       int& value, Position& bestMove) {
    auto it = transpositionTable.find(hash);
    if (it == transpositionTable.end()) {
        ttMisses++;
        return false;
    }
    
    const TTEntry& entry = it->second;
    if (entry.depth < depth) {
        ttMisses++;
        return false;
    }
    
    ttHits++;
    
    // Check if we can use this value
    if (entry.flag == 0) { // Exact value
        value = entry.value;
        bestMove = entry.bestMove;
        return true;
    } else if (entry.flag == 1 && entry.value >= beta) { // Lower bound
        value = entry.value;
        bestMove = entry.bestMove;
        return true;
    } else if (entry.flag == 2 && entry.value <= alpha) { // Upper bound
        value = entry.value;
        bestMove = entry.bestMove;
        return true;
    }
    
    // Can't use value, but can use best move for ordering
    if (entry.bestMove.x != -1 && entry.bestMove.y != -1) {
        bestMove = entry.bestMove;
    }
    
    return false;
}

void MinMaxAI::storeTranspositionTable(uint64_t hash, int depth, int value, int flag, const Position& bestMove) {
    TTEntry& entry = transpositionTable[hash];
    
    // Replace if this is a deeper search or same depth
    if (entry.depth <= depth) {
        entry.depth = depth;
        entry.value = value;
        entry.flag = flag;
        entry.bestMove = bestMove;
    }
}

void MinMaxAI::clearTranspositionTable() {
    transpositionTable.clear();
    ttHits = 0;
    ttMisses = 0;
}

void MinMaxAI::updateKillerMove(const Position& move, int depth) {
    if (depth >= 0 && depth < static_cast<int>(killerMoves.size())) {
        auto& level = killerMoves[depth];
        
        // Don't add duplicates
        auto it = std::find(level.begin(), level.end(), move);
        if (it == level.end()) {
            level.insert(level.begin(), move);
            // Keep only 2 killer moves per level
            if (level.size() > 2) {
                level.pop_back();
            }
        }
    }
}

bool MinMaxAI::isKillerMove(const Position& move, int depth) {
    if (depth < 0 || depth >= static_cast<int>(killerMoves.size())) return false;
    
    const auto& level = killerMoves[depth];
    return std::find(level.begin(), level.end(), move) != level.end();
}

bool MinMaxAI::isTerminalNode(const Board& board, int depth) {
    if (depth <= 0) return true;
    
    // Check for wins
    if (board.checkWin(color) || board.checkWin(getOpponentColor())) {
        return true;
    }
    
    // Check for capture wins
    if (board.checkCaptureWin(color) || board.checkCaptureWin(getOpponentColor())) {
        return true;
    }
    
    return false;
}

int MinMaxAI::evaluateTerminalNode(const Board& board) {
    if (board.checkWin(color) || board.checkCaptureWin(color)) {
        return WIN_VALUE;
    }
    if (board.checkWin(getOpponentColor()) || board.checkCaptureWin(getOpponentColor())) {
        return LOSE_VALUE;
    }
    
    return heuristic->evaluatePosition(board, color);
}

bool MinMaxAI::isTacticalMove(const Board& board, const Position& move) {
    // Check if move creates or responds to threats
    Board tempBoard = board;
    if (!tempBoard.placePiece(move.x, move.y, color)) {
        return false;
    }
    
    // Check for immediate wins
    if (tempBoard.checkWin(color)) return true;
    
    // Check for captures
    std::vector<Position> captures = tempBoard.checkCaptures(move.x, move.y, color);
    if (!captures.empty()) return true;
    
    // Check for threat creation/response
    std::vector<Threat> threats = heuristic->findImmediateThreats(tempBoard, color);
    if (!threats.empty()) return true;
    
    return false;
}

Position MinMaxAI::getOpeningMove(const Board& board) {
    // Simple opening book
    int moveCount = 0;
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) != EMPTY) {
                moveCount++;
            }
        }
    }
    
    int center = BOARD_SIZE / 2;
    
    if (moveCount == 0) {
        // First move - take center
        return Position(center, center);
    } else if (moveCount == 1) {
        // Second move - play near center
        if (board.getCell(center, center) == EMPTY) {
            return Position(center, center);
        } else {
            // Play adjacent to center
            std::vector<Position> adjacentMoves = {
                Position(center-1, center-1), Position(center-1, center), Position(center-1, center+1),
                Position(center, center-1), Position(center, center+1),
                Position(center+1, center-1), Position(center+1, center), Position(center+1, center+1)
            };
            
            for (const Position& move : adjacentMoves) {
                if (board.isValidMove(move.x, move.y)) {
                    return move;
                }
            }
        }
    }
    
    return Position(-1, -1); // No opening move found
}

bool MinMaxAI::isOpeningPhase(const Board& board) const {
    int moveCount = 0;
    for (int x = 0; x < board.getSize(); x++) {
        for (int y = 0; y < board.getSize(); y++) {
            if (board.getCell(x, y) != EMPTY) {
                moveCount++;
            }
        }
    }
    return moveCount < 8;
}

void MinMaxAI::onGameStart() {
    clearTranspositionTable();
    
    // Clear killer moves
    for (auto& level : killerMoves) {
        level.clear();
    }
    
    principalVariation.clear();
    nodesEvaluated = 0;
    ttHits = 0;
    ttMisses = 0;
    
    std::cout << "[MinMaxAI] Game started, AI is ready" << std::endl;
}

void MinMaxAI::onGameEnd(bool won) {
    std::cout << "[MinMaxAI] Game ended - " << (won ? "Victory!" : "Defeat") << std::endl;
    printSearchStatistics();
}

void MinMaxAI::printSearchStatistics() const {
    std::cout << "[MinMaxAI] Search Statistics:" << std::endl;
    std::cout << "  - Nodes evaluated: " << nodesEvaluated << std::endl;
    std::cout << "  - TT hit rate: " << getTTHitRate() << "%" << std::endl;
    std::cout << "  - TT entries: " << transpositionTable.size() << std::endl;
    
    if (!principalVariation.empty()) {
        std::cout << "  - Principal variation: ";
        for (size_t i = 0; i < std::min(principalVariation.size(), size_t(5)); i++) {
            std::cout << "(" << principalVariation[i].x << "," << principalVariation[i].y << ") ";
        }
        std::cout << std::endl;
    }
}