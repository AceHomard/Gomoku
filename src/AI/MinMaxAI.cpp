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
#include "Game/Rules.hpp"
#include "Game/Constants.hpp"
#include "Debug.hpp"
#include <algorithm>
#include <limits>
#include <iostream>
#include <chrono>

using namespace EvalScores;

MinMaxAI::MinMaxAI(CellState playerColor, int depth, const std::string& playerName)
    : IPlayer(playerColor, AI, playerName), searchDepth(depth),
      timeLimitMs(AIConfig::DEFAULT_TIME_LIMIT_MS),
      heuristic(std::make_unique<Heuristic>()),
      timeExpired(false), inGuaranteedDepth(false),
      ttHits(0), nodesEvaluated(0), cutoffsCount(0), depthReached(0), maxDepthEverReached(0), debugMode(false) {

    // Initialize killer moves table (2 slots per depth level)
    killerMoves.resize(searchDepth + 1, {Position(-1, -1), Position(-1, -1)});

    LOG_INFO("MinMaxAI initialized with depth=" << searchDepth
             << ", timeLimit=" << timeLimitMs << "ms");
}

MinMaxAI::~MinMaxAI() {
}

Position MinMaxAI::makeMove(const Board& board) {
    LOG_INFO("MinMaxAI thinking (iterative deepening)...");

    // Reset counters
    nodesEvaluated = 0;
    cutoffsCount = 0;
    depthReached = 0;
    ttHits = 0;
    timeExpired = false;
    searchStartTime = std::chrono::high_resolution_clock::now();

    // Reset killer moves for this search
    for (auto& slot : killerMoves) {
        slot[0] = Position(-1, -1);
        slot[1] = Position(-1, -1);
    }

    std::vector<Position> moves = generateMoves(board);
    if (moves.empty()) {
        return Position(-1, -1);
    }

    // Validate moves
    std::vector<Position> validMoves;
    for (const Position& move : moves) {
        if (Rules::isValidMove(board, move.x, move.y, color)) {
            validMoves.push_back(move);
        }
    }

    if (validMoves.empty()) {
        LOG_ERROR("No valid moves found - game should be over!");
        return Position(-1, -1);
    }

    // If only one valid move, return it immediately
    if (validMoves.size() == 1) {
        return validMoves[0];
    }

    Position bestMove = validMoves[0];
    int bestValue = std::numeric_limits<int>::min();
    inGuaranteedDepth = false;

    // Iterative deepening: search depth 1, 2, 3, ... up to searchDepth
    for (int currentDepth = 1; currentDepth <= searchDepth; currentDepth++) {
        if (isTimeUp()) {
            break;
        }

        // Put the best move from previous iteration first (key ID optimization)
        if (currentDepth > 1 && bestMove.x >= 0) {
            for (size_t i = 1; i < validMoves.size(); i++) {
                if (validMoves[i] == bestMove) {
                    std::swap(validMoves[0], validMoves[i]);
                    break;
                }
            }
        }

        Position currentBestMove = validMoves[0];
        int currentBestValue = std::numeric_limits<int>::min();
        bool completedIteration = true;

        for (const Position& move : validMoves) {
            Board tempBoard = board;
            if (tempBoard.placePiece(move.x, move.y, color)) {
                int value = alphabeta(tempBoard, currentDepth - 1,
                                    std::numeric_limits<int>::min(),
                                    std::numeric_limits<int>::max(),
                                    false);

                // If time expired during search, this result is unreliable
                if (timeExpired) {
                    completedIteration = false;
                    break;
                }

                if (debugMode) {
                    logNode(currentDepth, move, value, true);
                }

                if (value > currentBestValue) {
                    currentBestValue = value;
                    currentBestMove = move;
                }
            }
        }

        // Only use results from fully completed iterations
        if (completedIteration) {
            bestMove = currentBestMove;
            bestValue = currentBestValue;
            depthReached = currentDepth;

            if (debugMode) {
                LOG_DEBUG("Depth " << currentDepth << " complete: best=("
                          << bestMove.x << "," << bestMove.y << ") value=" << bestValue);
            }

            // Early exit if we found a winning move
            if (bestValue >= WIN_VALUE) {
                break;
            }
        }
    }

    // Track max depth across entire game
    if (depthReached > maxDepthEverReached) {
        maxDepthEverReached = depthReached;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - searchStartTime);

    LOG_INFO("MinMaxAI selected move: (" << bestMove.x << ", " << bestMove.y
              << ") value=" << bestValue << " depth=" << depthReached);
    LOG_PERF("Nodes evaluated: " << nodesEvaluated
              << ", Cutoffs: " << cutoffsCount
              << ", TT hits: " << ttHits
              << ", TT size: " << transpositionTable.size()
              << ", Depth reached: " << depthReached << "/" << searchDepth
              << ", Time: " << duration.count() << "ms");

    return bestMove;
}

std::vector<Position> MinMaxAI::generateMoves(const Board& board) {
    return heuristic->getRelevantMoves(board, color);
}

int MinMaxAI::evaluateBoard(const Board& board) {
    // This function handles ALL evaluations (terminal and non-terminal)
    // Called from: 1) alphabeta when depth <= 0
    //              2) alphabeta when game is won/lost
    //              3) alphabeta when moves.empty()

    // Check terminal states (win/loss) - MUST be done here
    // Use checkWinFast() which only checks from lastMove position (much faster than full board scan)
    if (board.checkWinFast(color)) {
        return WIN_VALUE;
    }
    if (board.checkWinFast(getOpponentColor())) {
        return LOSE_VALUE;
    }

    // Non-terminal: use heuristic (no longer checks wins - we already did)
    return heuristic->evaluatePosition(board, color);
}

void MinMaxAI::onGameStart() {
    transpositionTable.clear();
    maxDepthEverReached = 0;
    LOG_INFO("MinMaxAI game started, TT cleared, AI is ready");
}

void MinMaxAI::onGameEnd(bool won) {
    (void)won;
    LOG_INFO("MinMaxAI game ended - " << (won ? "Victory!" : "Defeat"));
    std::cout << "[AI Stats] Max depth reached during game: " << maxDepthEverReached
              << "/" << searchDepth << std::endl;
}

std::string MinMaxAI::getIndent(int depth) const {
    return std::string(depth * 2, ' ');
}

void MinMaxAI::logNode(int depth, const Position& move, int value, bool maximizing) const {
    (void)depth; (void)move; (void)value; (void)maximizing; // Suppress warnings when DEBUG_MODE is off
    LOG_DEBUG(getIndent(searchDepth - depth)
              << "Depth " << depth << ": (" << move.x << "," << move.y
              << ") " << (maximizing ? "MAX" : "MIN")
              << " = " << value);
}

bool MinMaxAI::isTimeUp() const {
    // Never interrupt guaranteed depth iterations
    if (inGuaranteedDepth) return false;

    if (timeExpired) return true;

    // Check time periodically (every TIME_CHECK_INTERVAL nodes)
    if (nodesEvaluated % AIConfig::TIME_CHECK_INTERVAL == 0) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - searchStartTime);
        if (elapsed.count() >= timeLimitMs) {
            timeExpired = true;
            return true;
        }
    }
    return false;
}

int MinMaxAI::alphabeta(const Board& board, int depth, int alpha, int beta, bool maximizing) {
    nodesEvaluated++;

    // Time check
    if (isTimeUp()) {
        return evaluateBoard(board);
    }

    // Check depth limit first (cheapest check)
    if (depth <= 0) {
        return evaluateBoard(board);
    }

    // Check for terminal game states (win/loss)
    // Use checkWinFast() - only checks from last move position instead of scanning entire board
    if (board.checkWinFast(color) || board.checkWinFast(getOpponentColor())) {
        int value = evaluateBoard(board);
        if (debugMode && depth > searchDepth - 3) {
            LOG_DEBUG(getIndent(searchDepth - depth) << "Terminal win/loss: value=" << value);
        }
        return value;
    }

    // --- Transposition Table Lookup ---
    int originalAlpha = alpha;
    uint64_t hash = board.getZobristHash();
    Position ttBestMove(-1, -1);

    auto ttIt = transpositionTable.find(hash);
    if (ttIt != transpositionTable.end()) {
        const TTEntry& entry = ttIt->second;
        if (entry.hash == hash) { // Collision check
            ttBestMove = entry.bestMove;
            if (entry.depth >= depth) {
                ttHits++;
                if (entry.flag == TT_EXACT) {
                    return entry.value;
                } else if (entry.flag == TT_LOWERBOUND) {
                    alpha = std::max(alpha, entry.value);
                } else if (entry.flag == TT_UPPERBOUND) {
                    beta = std::min(beta, entry.value);
                }
                if (alpha >= beta) {
                    return entry.value;
                }
            }
        }
    }

    std::vector<Position> moves = generateMoves(board);
    if (moves.empty()) {
        return evaluateBoard(board);  // Draw or no legal moves
    }

    // TT best move: put it at front if available
    if (ttBestMove.x >= 0) {
        for (size_t i = 1; i < moves.size(); i++) {
            if (moves[i] == ttBestMove) {
                Position tmp = moves[i];
                moves.erase(moves.begin() + i);
                moves.insert(moves.begin(), tmp);
                break;
            }
        }
    }

    // Killer move heuristic: put killer moves at the front (after TT move)
    if (depth < (int)killerMoves.size()) {
        size_t insertPos = (ttBestMove.x >= 0 && !moves.empty() && moves[0] == ttBestMove) ? 1 : 0;
        for (int k = MAX_KILLER_SLOTS - 1; k >= 0; k--) {
            const Position& killer = killerMoves[depth][k];
            if (killer.x < 0 || killer == ttBestMove) continue;
            for (size_t i = insertPos + 1; i < moves.size(); i++) {
                if (moves[i] == killer) {
                    Position tmp = moves[i];
                    moves.erase(moves.begin() + i);
                    moves.insert(moves.begin() + insertPos, tmp);
                    break;
                }
            }
        }
    }

    Position bestMove = moves[0];

    if (maximizing) {
        int maxValue = std::numeric_limits<int>::min();

        for (const Position& move : moves) {
            Board tempBoard = board;
            if (tempBoard.placePiece(move.x, move.y, color)) {
                int value = alphabeta(tempBoard, depth - 1, alpha, beta, false);

                if (debugMode && depth > searchDepth - 3) {
                    logNode(depth, move, value, maximizing);
                }

                if (value > maxValue) {
                    maxValue = value;
                    bestMove = move;
                }
                alpha = std::max(alpha, value);

                // Alpha-Beta coupure
                if (beta <= alpha) {
                    cutoffsCount++;
                    // Store killer move
                    if (depth < (int)killerMoves.size() && !(move == killerMoves[depth][0])) {
                        killerMoves[depth][1] = killerMoves[depth][0];
                        killerMoves[depth][0] = move;
                    }
                    if (debugMode && depth > searchDepth - 3) {
                        LOG_DEBUG(getIndent(searchDepth - depth) << "CUTOFF: beta=" << beta << " <= alpha=" << alpha);
                    }
                    break;
                }
            }
        }

        // --- Store in Transposition Table ---
        TTFlag flag;
        if (maxValue <= originalAlpha) flag = TT_UPPERBOUND;
        else if (maxValue >= beta) flag = TT_LOWERBOUND;
        else flag = TT_EXACT;

        if (transpositionTable.size() < MAX_TT_SIZE) {
            transpositionTable[hash] = {hash, depth, maxValue, flag, bestMove};
        }

        return maxValue;
    } else {
        int minValue = std::numeric_limits<int>::max();
        CellState opponentColor = (color == BLACK) ? WHITE : BLACK;

        for (const Position& move : moves) {
            Board tempBoard = board;
            if (tempBoard.placePiece(move.x, move.y, opponentColor)) {
                int value = alphabeta(tempBoard, depth - 1, alpha, beta, true);

                if (debugMode && depth > searchDepth - 3) {
                    logNode(depth, move, value, maximizing);
                }

                if (value < minValue) {
                    minValue = value;
                    bestMove = move;
                }
                beta = std::min(beta, value);

                // Alpha-Beta coupure
                if (beta <= alpha) {
                    cutoffsCount++;
                    // Store killer move
                    if (depth < (int)killerMoves.size() && !(move == killerMoves[depth][0])) {
                        killerMoves[depth][1] = killerMoves[depth][0];
                        killerMoves[depth][0] = move;
                    }
                    if (debugMode && depth > searchDepth - 3) {
                        LOG_DEBUG(getIndent(searchDepth - depth) << "CUTOFF: beta=" << beta << " <= alpha=" << alpha);
                    }
                    break;
                }
            }
        }

        // --- Store in Transposition Table ---
        TTFlag flag;
        if (minValue >= beta) flag = TT_LOWERBOUND;      // Failed high (from min's perspective: didn't get below beta → lower bound for parent)
        else if (minValue <= originalAlpha) flag = TT_UPPERBOUND;  // Failed low
        else flag = TT_EXACT;

        if (transpositionTable.size() < MAX_TT_SIZE) {
            transpositionTable[hash] = {hash, depth, minValue, flag, bestMove};
        }

        return minValue;
    }
}

