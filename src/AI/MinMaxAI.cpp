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
      heuristic(std::make_unique<Heuristic>()),
      nodesEvaluated(0), cutoffsCount(0), debugMode(false) {

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
            // NOTE: placePiece() already handles captures internally (Board.cpp:101-109)

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

    // Safety check: if no valid move found in candidates, do fallback scan
    if (!foundValidMove) {
        LOG_ERROR("No valid moves in candidate list - trying fallback full board scan");

        // FALLBACK: Scan entire board for ANY valid move
        // This is slower but prevents infinite loops when heuristic fails
        for (int x = 0; x < board.getSize(); x++) {
            for (int y = 0; y < board.getSize(); y++) {
                if (Rules::isValidMove(board, x, y, color)) {
                    LOG_INFO("Fallback found valid move at (" << x << "," << y << ")");
                    return Position(x, y);
                }
            }
        }

        // Truly no valid moves exist (game should be over)
        LOG_ERROR("No valid moves found even after full board scan!");
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

int MinMaxAI::evaluateBoard(const Board& board) {
    // This function handles ALL evaluations (terminal and non-terminal)
    // Called from: 1) alphabeta when depth <= 0
    //              2) alphabeta when game is won/lost
    //              3) alphabeta when moves.empty()

    // Check terminal states (win/loss) - MUST be done here
    if (board.checkWin(color) || board.checkCaptureWin(color)) {
        return WIN_VALUE;
    }
    if (board.checkWin(getOpponentColor()) || board.checkCaptureWin(getOpponentColor())) {
        return LOSE_VALUE;
    }

    // Non-terminal: use heuristic (no longer checks wins - we already did)
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

    // Check depth limit first (cheapest check)
    if (depth <= 0) {
        return evaluateBoard(board);
    }

    // Check for terminal game states (win/loss)
    // NOTE: evaluateBoard() will re-check these, but we need to know NOW to avoid generateMoves()
    if (board.checkWin(color) || board.checkCaptureWin(color) ||
        board.checkWin(getOpponentColor()) || board.checkCaptureWin(getOpponentColor())) {
        int value = evaluateBoard(board);
        if (debugMode && depth > searchDepth - 3) {
            LOG_DEBUG(getIndent(searchDepth - depth) << "Terminal win/loss: value=" << value);
        }
        return value;
    }

    std::vector<Position> moves = generateMoves(board);
    if (moves.empty()) {
        return evaluateBoard(board);  // Draw or no legal moves
    }

    if (maximizing) {
        int maxValue = std::numeric_limits<int>::min();

        for (const Position& move : moves) {
            Board tempBoard = board;
            if (tempBoard.placePiece(move.x, move.y, color)) {
                // NOTE: placePiece() already handles captures internally

                int value = alphabeta(tempBoard, depth - 1, alpha, beta, false);

                if (debugMode && depth > searchDepth - 3) {
                    logNode(depth, move, value, maximizing);
                }

                if (value > maxValue) {
                    maxValue = value;
                }
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
                // NOTE: placePiece() already handles captures internally

                int value = alphabeta(tempBoard, depth - 1, alpha, beta, true);

                if (debugMode && depth > searchDepth - 3) {
                    logNode(depth, move, value, maximizing);
                }

                if (value < minValue) {
                    minValue = value;
                }
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

