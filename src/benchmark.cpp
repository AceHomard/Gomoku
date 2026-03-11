/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   benchmark.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Game/Board.hpp"
#include "Game/Rules.hpp"
#include "Game/AIPlayer.hpp"
#include "AI/MinMaxAI.hpp"
#include "Game/Constants.hpp"
#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iomanip>

struct GameResult {
    CellState winner;       // BLACK, WHITE, or EMPTY (draw)
    int totalMoves;
    int durationMs;
    int maxDepthBlack;
    int maxDepthWhite;
    std::vector<int> moveTimes;   // ms per move
    std::vector<int> moveDepths;  // depth reached per move
};

static GameResult playOneGame(int noiseRange) {
    Board board;
    MinMaxAI ai1(BLACK, AIConfig::DEFAULT_DEPTH, "AI-Black");
    MinMaxAI ai2(WHITE, AIConfig::DEFAULT_DEPTH, "AI-White");

    ai1.setRandomNoise(noiseRange);
    ai2.setRandomNoise(noiseRange);
    ai1.onGameStart();
    ai2.onGameStart();

    GameResult result;
    result.winner = EMPTY;
    result.totalMoves = 0;
    result.maxDepthBlack = 0;
    result.maxDepthWhite = 0;

    MinMaxAI* currentAI = &ai1;
    CellState currentColor = BLACK;
    int maxMoves = 19 * 19;

    auto gameStart = std::chrono::high_resolution_clock::now();

    for (int move = 0; move < maxMoves; move++) {
        auto moveStart = std::chrono::high_resolution_clock::now();
        Position pos = currentAI->makeMove(board);
        auto moveEnd = std::chrono::high_resolution_clock::now();

        int moveMs = (int)std::chrono::duration_cast<std::chrono::milliseconds>(moveEnd - moveStart).count();
        result.moveTimes.push_back(moveMs);
        result.moveDepths.push_back(currentAI->getDepthReached());

        if (pos.x < 0 || pos.y < 0) {
            break;  // No valid move
        }

        board.placePiece(pos.x, pos.y, currentColor);
        result.totalMoves++;

        // Check win
        if (Rules::isGameWon(board, currentColor)) {
            result.winner = currentColor;
            break;
        }

        // Check draw
        if (Rules::isGameDrawn(board)) {
            break;
        }

        // Switch player
        if (currentAI == &ai1) {
            currentAI = &ai2;
            currentColor = WHITE;
        } else {
            currentAI = &ai1;
            currentColor = BLACK;
        }
    }

    auto gameEnd = std::chrono::high_resolution_clock::now();
    result.durationMs = (int)std::chrono::duration_cast<std::chrono::milliseconds>(gameEnd - gameStart).count();
    result.maxDepthBlack = ai1.getMaxDepthEverReached();
    result.maxDepthWhite = ai2.getMaxDepthEverReached();

    return result;
}

int main(int argc, char** argv) {
    int numGames = 5;
    int noiseRange = 15;
    bool quiet = false;

    // Parse args
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "-n" || arg == "--games") && i + 1 < argc) {
            numGames = std::atoi(argv[++i]);
        } else if ((arg == "--noise") && i + 1 < argc) {
            noiseRange = std::atoi(argv[++i]);
        } else if (arg == "-q" || arg == "--quiet") {
            quiet = true;
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: benchmark [options]\n"
                      << "  -n, --games N    Number of games to play (default: 5)\n"
                      << "  --noise N        Random noise range for move scoring (default: 15)\n"
                      << "  -q, --quiet      Only show final results\n"
                      << "  -h, --help       Show this help\n";
            return 0;
        }
    }

    if (!quiet) {
        std::cout << "=== Gomoku AI Benchmark ===" << std::endl;
        std::cout << "Games: " << numGames << " | Noise: ±" << noiseRange
                  << " | Depth: " << AIConfig::DEFAULT_DEPTH
                  << " | Time limit: " << AIConfig::DEFAULT_TIME_LIMIT_MS << "ms" << std::endl;
        std::cout << std::string(60, '-') << std::endl;
    }

    int blackWins = 0, whiteWins = 0, draws = 0;
    std::vector<int> allMoveTimes;
    std::vector<int> allMoveDepths;
    std::vector<int> gameDurations;
    std::vector<int> allDepthsBlack, allDepthsWhite;
    std::vector<int> moveCounts;

    for (int g = 0; g < numGames; g++) {
        if (!quiet)
            std::cout << "Game " << (g + 1) << "/" << numGames << "... " << std::flush;

        GameResult result = playOneGame(noiseRange);

        // Collect stats
        if (result.winner == BLACK) { blackWins++; if (!quiet) std::cout << "BLACK wins"; }
        else if (result.winner == WHITE) { whiteWins++; if (!quiet) std::cout << "WHITE wins"; }
        else { draws++; if (!quiet) std::cout << "DRAW"; }

        if (!quiet)
            std::cout << " (" << result.totalMoves << " moves, "
                      << result.durationMs << "ms, depth B:"
                      << result.maxDepthBlack << " W:" << result.maxDepthWhite << ")" << std::endl;

        allMoveTimes.insert(allMoveTimes.end(), result.moveTimes.begin(), result.moveTimes.end());
        allMoveDepths.insert(allMoveDepths.end(), result.moveDepths.begin(), result.moveDepths.end());
        gameDurations.push_back(result.durationMs);
        allDepthsBlack.push_back(result.maxDepthBlack);
        allDepthsWhite.push_back(result.maxDepthWhite);
        moveCounts.push_back(result.totalMoves);
    }

    // Summary
    std::cout << std::string(60, '-') << std::endl;
    std::cout << "=== RESULTS ===" << std::endl;
    std::cout << "BLACK wins: " << blackWins << " | WHITE wins: " << whiteWins
              << " | Draws: " << draws << std::endl;

    if (!allMoveTimes.empty()) {
        std::sort(allMoveTimes.begin(), allMoveTimes.end());
        int totalTime = std::accumulate(allMoveTimes.begin(), allMoveTimes.end(), 0);
        int avgTime = totalTime / (int)allMoveTimes.size();
        int maxTime = allMoveTimes.back();
        int medianTime = allMoveTimes[allMoveTimes.size() / 2];
        int over500 = 0;
        for (int t : allMoveTimes) if (t > 500) over500++;

        std::cout << "\n=== MOVE TIME STATS ===" << std::endl;
        std::cout << "  Avg:    " << avgTime << "ms" << std::endl;
        std::cout << "  Median: " << medianTime << "ms" << std::endl;
        std::cout << "  Max:    " << maxTime << "ms" << std::endl;
        std::cout << "  >500ms: " << over500 << "/" << allMoveTimes.size()
                  << " (" << std::fixed << std::setprecision(1)
                  << (100.0 * over500 / allMoveTimes.size()) << "%)" << std::endl;
    }

    if (!allMoveDepths.empty()) {
        std::sort(allMoveDepths.begin(), allMoveDepths.end());
        int totalDepth = std::accumulate(allMoveDepths.begin(), allMoveDepths.end(), 0);
        int avgDepth = totalDepth / (int)allMoveDepths.size();
        int minDepth = allMoveDepths.front();
        int maxDepth = allMoveDepths.back();
        int medianDepth = allMoveDepths[allMoveDepths.size() / 2];
        int avgMoves = std::accumulate(moveCounts.begin(), moveCounts.end(), 0) / (int)moveCounts.size();

        std::cout << "\n=== DEPTH STATS (per move) ===" << std::endl;
        std::cout << "  Avg:    " << avgDepth << "/" << AIConfig::DEFAULT_DEPTH << std::endl;
        std::cout << "  Median: " << medianDepth << "/" << AIConfig::DEFAULT_DEPTH << std::endl;
        std::cout << "  Min:    " << minDepth << "/" << AIConfig::DEFAULT_DEPTH << std::endl;
        std::cout << "  Max:    " << maxDepth << "/" << AIConfig::DEFAULT_DEPTH << std::endl;
        std::cout << "  Avg moves per game:  " << avgMoves << std::endl;
    }

    return 0;
}
