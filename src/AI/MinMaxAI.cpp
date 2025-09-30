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
      heuristic(std::make_unique<Heuristic>()), nodesEvaluated(0), cutoffsCount(0), debugMode(false) {

    LOG_INFO("MinMaxAI initialized with depth=" << searchDepth);
}

MinMaxAI::~MinMaxAI() {
}

Position MinMaxAI::makeMove(const Board& board) {
    LOG_INFO("MinMaxAI thinking...");

    // Reset counters
    nodesEvaluated = 0;
    cutoffsCount = 0;
    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<Position> moves = generateMoves(board);
    if (moves.empty()) {
        return Position(-1, -1);
    }

    Position bestMove(-1, -1);  // Initialize to invalid
    int bestValue = std::numeric_limits<int>::min();
    bool foundValidMove = false;

    if (debugMode) {
        LOG_DEBUG("Evaluating " << moves.size() << " moves at depth " << searchDepth);
    }

    for (const Position& move : moves) {
        // Double-check move validity WITH Rules validation (includes mandatory moves check!)
        if (!Rules::isValidMove(board, move.x, move.y, color)) {
            LOG_ERROR("Move (" << move.x << "," << move.y
                      << ") in candidate list but Rules says NOT valid! Cell state: "
                      << (int)board.getCell(move.x, move.y));
            continue;
        }

        Board tempBoard = board;
        if (tempBoard.placePiece(move.x, move.y, color)) {
            foundValidMove = true;

            // Handle captures
            std::vector<Position> captures = tempBoard.checkCaptures(move.x, move.y, color);
            tempBoard.executeCaptures(captures);

            int value = alphabeta(tempBoard, searchDepth - 1,
                                std::numeric_limits<int>::min(),
                                std::numeric_limits<int>::max(),
                                false);

            if (debugMode) {
                logNode(searchDepth, move, value, true);
            }

            if (value > bestValue) {
                bestValue = value;
                bestMove = move;
            }
        } else {
            LOG_ERROR("placePiece failed for (" << move.x << "," << move.y << ")!");
        }
    }

    // Safety check: if no valid move found, return error
    if (!foundValidMove) {
        LOG_ERROR("No valid moves found! This should never happen!");
        return Position(-1, -1);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    LOG_INFO("MinMaxAI selected move: (" << bestMove.x << ", " << bestMove.y
              << ") value=" << bestValue);
    LOG_PERF("Nodes evaluated: " << nodesEvaluated
              << ", Cutoffs: " << cutoffsCount
              << ", Time: " << duration.count() << "ms");

    return bestMove;
}

std::vector<Position> MinMaxAI::generateMoves(const Board& board) {
    return heuristic->getRelevantMoves(board, color);
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

int MinMaxAI::evaluateBoard(const Board& board) {
    if (board.checkWin(color) || board.checkCaptureWin(color)) {
        return WIN_VALUE;
    }
    if (board.checkWin(getOpponentColor()) || board.checkCaptureWin(getOpponentColor())) {
        return LOSE_VALUE;
    }
    
    return heuristic->evaluatePosition(board, color);
}

void MinMaxAI::onGameStart() {
    LOG_INFO("MinMaxAI game started, AI is ready");
}

void MinMaxAI::onGameEnd(bool won) {
    (void)won; // Suppress unused parameter warning
    LOG_INFO("MinMaxAI game ended - " << (won ? "Victory!" : "Defeat"));
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

int MinMaxAI::alphabeta(const Board& board, int depth, int alpha, int beta, bool maximizing) {
    nodesEvaluated++;

    if (isTerminalNode(board, depth)) {
        int value = evaluateBoard(board);
        if (debugMode && depth > searchDepth - 3) {
            LOG_DEBUG(getIndent(searchDepth - depth) << "Terminal: value=" << value);
        }
        return value;
    }

    std::vector<Position> moves = generateMoves(board);
    if (moves.empty()) {
        return evaluateBoard(board);
    }

    if (maximizing) {
        int maxValue = std::numeric_limits<int>::min();
        for (const Position& move : moves) {
            Board tempBoard = board;
            if (tempBoard.placePiece(move.x, move.y, color)) {
                std::vector<Position> captures = tempBoard.checkCaptures(move.x, move.y, color);
                tempBoard.executeCaptures(captures);

                int value = alphabeta(tempBoard, depth - 1, alpha, beta, false);

                if (debugMode && depth > searchDepth - 3) {
                    logNode(depth, move, value, maximizing);
                }

                maxValue = std::max(maxValue, value);
                alpha = std::max(alpha, value);

                // Alpha-Beta coupure
                if (beta <= alpha) {
                    cutoffsCount++;
                    if (debugMode && depth > searchDepth - 3) {
                        LOG_DEBUG(getIndent(searchDepth - depth) << "CUTOFF: beta=" << beta << " <= alpha=" << alpha);
                    }
                    break;
                }
            }
        }
        return maxValue;
    } else {
        int minValue = std::numeric_limits<int>::max();
        CellState opponentColor = (color == BLACK) ? WHITE : BLACK;
        for (const Position& move : moves) {
            Board tempBoard = board;
            if (tempBoard.placePiece(move.x, move.y, opponentColor)) {
                std::vector<Position> captures = tempBoard.checkCaptures(move.x, move.y, opponentColor);
                tempBoard.executeCaptures(captures);

                int value = alphabeta(tempBoard, depth - 1, alpha, beta, true);

                if (debugMode && depth > searchDepth - 3) {
                    logNode(depth, move, value, maximizing);
                }

                minValue = std::min(minValue, value);
                beta = std::min(beta, value);

                // Alpha-Beta coupure
                if (beta <= alpha) {
                    cutoffsCount++;
                    if (debugMode && depth > searchDepth - 3) {
                        LOG_DEBUG(getIndent(searchDepth - depth) << "CUTOFF: beta=" << beta << " <= alpha=" << alpha);
                    }
                    break;
                }
            }
        }
        return minValue;
    }
}

