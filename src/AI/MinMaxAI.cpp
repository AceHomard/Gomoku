#include "AI/MinMaxAI.hpp"
#include "Game/Rules.hpp"
#include "Game/Constants.hpp"
#include "Debug.hpp"
#ifdef DEBUG_VISU
#include "Debug/SearchDataCollector.hpp"
#endif
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
#ifdef DEBUG_VISU
    shouldStop = false;
#endif
    auto startTime = std::chrono::high_resolution_clock::now();

    // OPENING BOOK: first 3-5 moves use pre-computed positions (instant)
    {
        int stoneCount = 0;
        int center = board.getSize() / 2;
        for (int x = 0; x < board.getSize(); x++)
            for (int y = 0; y < board.getSize(); y++)
                if (board.getCell(x, y) != EMPTY) stoneCount++;

        if (stoneCount <= 4) {
            Position bookMove(-1, -1);

            if (stoneCount == 0) {
                // First move (Black): always center
                bookMove = Position(center, center);
            } else if (stoneCount == 1) {
                // Second move (White): offset from center, 2-3 cells away
                // Try several L-shape offsets, pick first valid
                int offsets[][2] = {{2, 1}, {1, 2}, {-2, 1}, {-1, 2}, {2, -1}, {-2, -1}};
                for (auto& off : offsets) {
                    int ox = center + off[0], oy = center + off[1];
                    if (board.isValidMove(ox, oy)) {
                        bookMove = Position(ox, oy);
                        break;
                    }
                }
            } else if (stoneCount == 2) {
                // Third move (Black): extend L-shape from own stone, gap preferred (X_X)
                for (int x = 0; x < board.getSize(); x++) {
                    for (int y = 0; y < board.getSize(); y++) {
                        if (board.getCell(x, y) == color) {
                            // Try gap positions (distance 2) for X_X shape
                            int offsets[][2] = {{2, 1}, {1, 2}, {-2, 1}, {-1, 2},
                                                {2, -1}, {-2, -1}, {2, 0}, {0, 2}, {-2, 0}, {0, -2}};
                            for (auto& off : offsets) {
                                int ox = x + off[0], oy = y + off[1];
                                if (board.isValidMove(ox, oy) &&
                                    Rules::isValidMove(board, ox, oy, color)) {
                                    bookMove = Position(ox, oy);
                                    goto bookDone;
                                }
                            }
                        }
                    }
                }
            }

            bookDone:
            if (bookMove.x >= 0 && bookMove.y >= 0 &&
                Rules::isValidMove(board, bookMove.x, bookMove.y, color)) {
                LOG_INFO("Opening book move: (" << bookMove.x << ", " << bookMove.y << ")");
                return bookMove;
            }
        }
    }

    VISU_HOOK(SearchDataCollector::instance().onSearchStart(searchDepth));

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

    // Use a mutable copy for make/unmake throughout the search tree
    Board searchBoard = board;

    for (const Position& move : moves) {
        // Double-check move validity WITH Rules validation (includes mandatory moves check!)
        if (!Rules::isValidMove(searchBoard, move.x, move.y, color)) {
            LOG_ERROR("Move (" << move.x << "," << move.y
                      << ") in candidate list but Rules says NOT valid! Cell state: "
                      << (int)searchBoard.getCell(move.x, move.y));
            continue;
        }

        foundValidMove = true;

        VISU_HOOK(SearchDataCollector::instance().onNodeEnter(
            move, searchDepth,
            std::numeric_limits<int>::min(),
            std::numeric_limits<int>::max(), true));

        MoveUndo undo = searchBoard.makeMove(move.x, move.y, color);

        int value;
        // LOCAL win check at root: if this move wins, no need to search deeper
        if (searchBoard.checkCaptureWin(color) || searchBoard.checkAlignment(move.x, move.y, color)) {
            value = WIN_VALUE + searchDepth;
        } else {
            value = alphabeta(searchBoard, searchDepth - 1,
                            std::numeric_limits<int>::min(),
                            std::numeric_limits<int>::max(),
                            false);
        }

        searchBoard.unmakeMove(undo);

        VISU_HOOK(SearchDataCollector::instance().onNodeExit(value, false));

        if (debugMode) {
            logNode(searchDepth, move, value, true);
        }

        if (value > bestValue) {
            bestValue = value;
            bestMove = move;
            VISU_HOOK(SearchDataCollector::instance().onBestMoveUpdate(bestMove, bestValue));
        }
    }

    // Safety check: if no valid move found, game should be over
    if (!foundValidMove) {
        LOG_ERROR("No valid moves found - game should be over!");
        return Position(-1, -1);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    VISU_HOOK(SearchDataCollector::instance().onSearchComplete(
        bestMove, bestValue, nodesEvaluated, cutoffsCount,
        static_cast<float>(duration.count())));

    LOG_INFO("MinMaxAI selected move: (" << bestMove.x << ", " << bestMove.y
              << ") value=" << bestValue);
    LOG_PERF("Nodes evaluated: " << nodesEvaluated
              << ", Cutoffs: " << cutoffsCount
              << ", Time: " << duration.count() << "ms");

    return bestMove;
}

std::vector<Position> MinMaxAI::generateMoves(const Board& board, bool fastMode) {
    return heuristic->getRelevantMoves(board, color, fastMode);
}

int MinMaxAI::evaluateBoard(const Board& board) {
    // Win/loss detection is now handled locally after each makeMove in alphabeta().
    // This function is only used at root level for initial evaluation.
    // Leaf nodes call heuristic->evaluatePosition() directly.
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

int MinMaxAI::alphabeta(Board& board, int depth, int alpha, int beta, bool maximizing) {
    nodesEvaluated++;

#ifdef DEBUG_VISU
    if (shouldStop) return 0;
#endif

    // Leaf node: heuristic only (wins already caught by parent's post-makeMove check)
    if (depth <= 0) {
        return heuristic->evaluatePosition(board, color);
    }

    std::vector<Position> moves = generateMoves(board, true);
    if (moves.empty()) {
        return heuristic->evaluatePosition(board, color);
    }

    if (maximizing) {
        int maxValue = std::numeric_limits<int>::min();

        for (const Position& move : moves) {
            VISU_HOOK(SearchDataCollector::instance().onNodeEnter(move, depth, alpha, beta, true));

            MoveUndo undo = board.makeMove(move.x, move.y, color);

            // LOCAL win check: only check around last move + capture counter (O(1) + O(40))
            int value;
            if (board.checkCaptureWin(color) || board.checkAlignment(move.x, move.y, color)) {
                value = WIN_VALUE + depth; // prefer faster wins
            } else {
                value = alphabeta(board, depth - 1, alpha, beta, false);
            }

            board.unmakeMove(undo);

            bool pruned = false;
            if (value > maxValue) {
                maxValue = value;
            }
            alpha = std::max(alpha, value);

            if (beta <= alpha) {
                cutoffsCount++;
                pruned = true;
                VISU_HOOK(SearchDataCollector::instance().onNodeExit(value, true));
                break;
            }
            VISU_HOOK(SearchDataCollector::instance().onNodeExit(value, false));
            (void)pruned;
        }

        return maxValue;
    } else {
        int minValue = std::numeric_limits<int>::max();
        CellState opponentColor = (color == BLACK) ? WHITE : BLACK;

        for (const Position& move : moves) {
            VISU_HOOK(SearchDataCollector::instance().onNodeEnter(move, depth, alpha, beta, false));

            MoveUndo undo = board.makeMove(move.x, move.y, opponentColor);

            // LOCAL win check for opponent
            int value;
            if (board.checkCaptureWin(opponentColor) || board.checkAlignment(move.x, move.y, opponentColor)) {
                value = LOSE_VALUE - depth; // penalize faster losses
            } else {
                value = alphabeta(board, depth - 1, alpha, beta, true);
            }

            board.unmakeMove(undo);

            bool pruned = false;
            if (value < minValue) {
                minValue = value;
            }
            beta = std::min(beta, value);

            if (beta <= alpha) {
                cutoffsCount++;
                pruned = true;
                VISU_HOOK(SearchDataCollector::instance().onNodeExit(value, true));
                break;
            }
            VISU_HOOK(SearchDataCollector::instance().onNodeExit(value, false));
            (void)pruned;
        }

        return minValue;
    }
}

