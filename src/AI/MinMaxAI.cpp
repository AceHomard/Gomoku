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

    // Use a mutable copy for make/unmake throughout the search tree
    Board searchBoard = board;

    // IMMEDIATE WIN CHECK: before entering search, check if any move wins instantly
    for (const Position& move : validMoves) {
        MoveUndo undo = searchBoard.makeMove(move.x, move.y, color);
        bool wins = searchBoard.checkCaptureWin(color) || searchBoard.checkAlignment(move.x, move.y, color);
        searchBoard.unmakeMove(undo);
        if (wins) {
            LOG_INFO("Immediate win detected at (" << move.x << "," << move.y << ")");
            return move;
        }
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
            MoveUndo undo = searchBoard.makeMove(move.x, move.y, color);

            int value;
            // LOCAL win check at root: if this move wins, no need to search deeper
            if (searchBoard.checkCaptureWin(color) || searchBoard.checkAlignment(move.x, move.y, color)) {
                value = WIN_VALUE + currentDepth;
            } else {
                value = alphabeta(searchBoard, currentDepth - 1,
                                std::numeric_limits<int>::min(),
                                std::numeric_limits<int>::max(),
                                false);
            }

            searchBoard.unmakeMove(undo);

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

std::vector<Position> MinMaxAI::generateMoves(const Board& board, bool fastMode) {
    return heuristic->getRelevantMoves(board, color, fastMode);
}

int MinMaxAI::evaluateBoard(const Board& board) {
    // Win/loss detection is handled locally after each makeMove in alphabeta().
    // This function just returns heuristic evaluation.
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
    LOG_INFO("AI Stats: Max depth reached during game: " << maxDepthEverReached
              << "/" << searchDepth);
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

int MinMaxAI::alphabeta(Board& board, int depth, int alpha, int beta, bool maximizing) {
    nodesEvaluated++;

    // Time check
    if (isTimeUp()) {
        return heuristic->evaluatePosition(board, color);
    }

    // Leaf node: heuristic only (wins already caught by parent's post-makeMove check)
    if (depth <= 0) {
        return heuristic->evaluatePosition(board, color);
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

    std::vector<Position> moves = generateMoves(board, true);
    if (moves.empty()) {
        return heuristic->evaluatePosition(board, color);
    }

    // TT best move: put it at front if available
    if (ttBestMove.x >= 0) {
        for (size_t i = 1; i < moves.size(); i++) {
            if (moves[i] == ttBestMove) {
                std::swap(moves[0], moves[i]);
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
            MoveUndo undo = board.makeMove(move.x, move.y, color);

            // LOCAL win check: only check around last move + capture counter
            int value;
            if (board.checkCaptureWin(color) || board.checkAlignment(move.x, move.y, color)) {
                value = WIN_VALUE + depth; // prefer faster wins
            } else {
                value = alphabeta(board, depth - 1, alpha, beta, false);
            }

            board.unmakeMove(undo);

            if (value > maxValue) {
                maxValue = value;
                bestMove = move;
            }
            alpha = std::max(alpha, value);

            if (beta <= alpha) {
                cutoffsCount++;
                // Store killer move
                if (depth < (int)killerMoves.size() && !(move == killerMoves[depth][0])) {
                    killerMoves[depth][1] = killerMoves[depth][0];
                    killerMoves[depth][0] = move;
                }
                break;
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
            MoveUndo undo = board.makeMove(move.x, move.y, opponentColor);

            // LOCAL win check for opponent
            int value;
            if (board.checkCaptureWin(opponentColor) || board.checkAlignment(move.x, move.y, opponentColor)) {
                value = LOSE_VALUE - depth; // penalize faster losses
            } else {
                value = alphabeta(board, depth - 1, alpha, beta, true);
            }

            board.unmakeMove(undo);

            if (value < minValue) {
                minValue = value;
                bestMove = move;
            }
            beta = std::min(beta, value);

            if (beta <= alpha) {
                cutoffsCount++;
                // Store killer move
                if (depth < (int)killerMoves.size() && !(move == killerMoves[depth][0])) {
                    killerMoves[depth][1] = killerMoves[depth][0];
                    killerMoves[depth][0] = move;
                }
                break;
            }
        }

        // --- Store in Transposition Table ---
        TTFlag flag;
        if (minValue >= beta) flag = TT_LOWERBOUND;
        else if (minValue <= originalAlpha) flag = TT_UPPERBOUND;
        else flag = TT_EXACT;

        if (transpositionTable.size() < MAX_TT_SIZE) {
            transpositionTable[hash] = {hash, depth, minValue, flag, bestMove};
        }

        return minValue;
    }
}
