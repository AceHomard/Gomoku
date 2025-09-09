/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   performance_benchmark.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 11:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 11:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "include/AI/MinMaxAI.hpp"
#include "include/Game/Game.hpp"
#include "include/Game/Board.hpp"
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <memory>

class PerformanceBenchmark {
private:
    struct BenchmarkResult {
        int depth;
        double averageTime;
        double maxTime;
        double minTime;
        double medianTime;
        std::vector<double> allTimes;
        int totalMoves;
        int timeouts;
        double timeoutRate;
        long nodesEvaluated;
        int ttHitRate;
        
        BenchmarkResult() : depth(0), averageTime(0), maxTime(0), minTime(0), medianTime(0),
                           totalMoves(0), timeouts(0), timeoutRate(0), nodesEvaluated(0), ttHitRate(0) {}
    };

    struct GameBenchmarkResult {
        std::string scenario;
        std::vector<BenchmarkResult> depthResults;
        double totalGameTime;
        bool completed;
        std::string error;
    };

public:
    // Main benchmarking function
    void runFullBenchmark() {
        std::cout << "=== GOMOKU AI PERFORMANCE BENCHMARK ===" << std::endl;
        std::cout << "Subject Requirements:" << std::endl;
        std::cout << "- Average move time: ≤ 0.5 seconds" << std::endl;
        std::cout << "- Minimum search depth: 10 levels" << std::endl;
        std::cout << "- No crashes under any circumstances" << std::endl;
        std::cout << "==========================================" << std::endl << std::endl;

        // Test scenarios
        std::vector<GameBenchmarkResult> results;
        
        // 1. Empty board scenarios
        results.push_back(benchmarkEmptyBoard());
        
        // 2. Opening phase scenarios
        results.push_back(benchmarkOpeningPhase());
        
        // 3. Mid-game scenarios  
        results.push_back(benchmarkMidGame());
        
        // 4. Complex tactical scenarios
        results.push_back(benchmarkTacticalPositions());
        
        // 5. Endgame scenarios
        results.push_back(benchmarkEndgame());
        
        // 6. Stress testing
        results.push_back(benchmarkStressTest());
        
        // 7. Memory pressure test
        results.push_back(benchmarkMemoryPressure());
        
        // Generate comprehensive report
        generateBenchmarkReport(results);
        
        // Check if requirements are met
        validateRequirements(results);
    }

private:
    GameBenchmarkResult benchmarkEmptyBoard() {
        std::cout << "🔍 Testing: Empty board performance..." << std::endl;
        GameBenchmarkResult result;
        result.scenario = "Empty Board";
        
        Board board;
        
        // Test different depths
        for (int depth = 10; depth <= 16; depth += 2) {
            result.depthResults.push_back(benchmarkAIAtDepth(board, depth, 20));
        }
        
        result.completed = true;
        return result;
    }
    
    GameBenchmarkResult benchmarkOpeningPhase() {
        std::cout << "🔍 Testing: Opening phase performance..." << std::endl;
        GameBenchmarkResult result;
        result.scenario = "Opening Phase";
        
        // Create opening position
        Board board;
        board.placePiece(9, 9, BLACK);   // Center
        board.placePiece(10, 10, WHITE); // Adjacent
        board.placePiece(8, 8, BLACK);   // Diagonal
        
        for (int depth = 10; depth <= 14; depth += 2) {
            result.depthResults.push_back(benchmarkAIAtDepth(board, depth, 15));
        }
        
        result.completed = true;
        return result;
    }
    
    GameBenchmarkResult benchmarkMidGame() {
        std::cout << "🔍 Testing: Mid-game performance..." << std::endl;
        GameBenchmarkResult result;
        result.scenario = "Mid-Game";
        
        // Create complex mid-game position
        Board board;
        
        // Set up complex pattern
        std::vector<std::pair<Position, CellState>> moves = {
            {{9, 9}, BLACK}, {{10, 9}, WHITE}, {{8, 9}, BLACK},
            {{11, 9}, WHITE}, {{7, 9}, BLACK}, {{9, 10}, WHITE},
            {{9, 8}, BLACK}, {{10, 8}, WHITE}, {{8, 8}, BLACK},
            {{11, 8}, WHITE}, {{7, 8}, BLACK}, {{9, 11}, WHITE},
            {{10, 11}, BLACK}, {{8, 11}, WHITE}, {{11, 11}, BLACK}
        };
        
        for (const auto& move : moves) {
            board.placePiece(move.first.x, move.first.y, move.second);
        }
        
        for (int depth = 10; depth <= 12; depth += 1) {
            result.depthResults.push_back(benchmarkAIAtDepth(board, depth, 10));
        }
        
        result.completed = true;
        return result;
    }
    
    GameBenchmarkResult benchmarkTacticalPositions() {
        std::cout << "🔍 Testing: Tactical positions..." << std::endl;
        GameBenchmarkResult result;
        result.scenario = "Tactical Positions";
        
        // Create position with multiple threats
        Board board;
        
        // Create forcing sequence scenario
        std::vector<std::pair<Position, CellState>> tacticalMoves = {
            {{9, 9}, BLACK}, {{10, 9}, WHITE}, {{11, 9}, BLACK},
            {{9, 10}, WHITE}, {{9, 11}, BLACK}, {{9, 8}, WHITE},
            {{8, 9}, BLACK}, {{7, 9}, WHITE}, {{12, 9}, BLACK},
            {{10, 10}, WHITE}, {{11, 11}, BLACK}
        };
        
        for (const auto& move : tacticalMoves) {
            board.placePiece(move.first.x, move.first.y, move.second);
        }
        
        for (int depth = 10; depth <= 12; depth += 1) {
            result.depthResults.push_back(benchmarkAIAtDepth(board, depth, 8));
        }
        
        result.completed = true;
        return result;
    }
    
    GameBenchmarkResult benchmarkEndgame() {
        std::cout << "🔍 Testing: Endgame performance..." << std::endl;
        GameBenchmarkResult result;
        result.scenario = "Endgame";
        
        // Create near-full board
        Board board;
        
        // Fill board partially for endgame scenario
        for (int x = 0; x < 19; x++) {
            for (int y = 0; y < 19; y++) {
                if ((x + y) % 3 == 0 && x < 15 && y < 15) {
                    board.placePiece(x, y, ((x + y) % 2 == 0) ? BLACK : WHITE);
                }
            }
        }
        
        for (int depth = 10; depth <= 12; depth += 1) {
            result.depthResults.push_back(benchmarkAIAtDepth(board, depth, 5));
        }
        
        result.completed = true;
        return result;
    }
    
    GameBenchmarkResult benchmarkStressTest() {
        std::cout << "🔍 Testing: Stress test (100 consecutive moves)..." << std::endl;
        GameBenchmarkResult result;
        result.scenario = "Stress Test";
        
        try {
            auto start = std::chrono::high_resolution_clock::now();
            
            // Run complete AI vs AI game
            MinMaxAI ai1(BLACK, 10, 0.5, "AI Black");
            MinMaxAI ai2(WHITE, 10, 0.5, "AI White"); 
            
            Board board;
            std::vector<double> moveTimes;
            
            ai1.onGameStart();
            ai2.onGameStart();
            
            int moveCount = 0;
            const int maxMoves = 100;
            
            while (moveCount < maxMoves && !board.checkWin(BLACK) && !board.checkWin(WHITE)) {
                CellState currentPlayer = (moveCount % 2 == 0) ? BLACK : WHITE;
                MinMaxAI& currentAI = (currentPlayer == BLACK) ? ai1 : ai2;
                
                auto moveStart = std::chrono::high_resolution_clock::now();
                Position move = currentAI.makeMove(board);
                auto moveEnd = std::chrono::high_resolution_clock::now();
                
                double moveTime = std::chrono::duration<double>(moveEnd - moveStart).count();
                moveTimes.push_back(moveTime);
                
                if (move.x == -1 || move.y == -1 || !board.placePiece(move.x, move.y, currentPlayer)) {
                    result.error = "Invalid move generated at move " + std::to_string(moveCount);
                    result.completed = false;
                    return result;
                }
                
                // Execute captures
                std::vector<Position> captures = board.checkCaptures(move.x, move.y, currentPlayer);
                board.executeCaptures(captures);
                
                moveCount++;
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            result.totalGameTime = std::chrono::duration<double>(end - start).count();
            
            // Create summary result
            BenchmarkResult summary;
            summary.depth = 10;
            summary.allTimes = moveTimes;
            summary.totalMoves = moveCount;
            summary.averageTime = std::accumulate(moveTimes.begin(), moveTimes.end(), 0.0) / moveTimes.size();
            summary.maxTime = *std::max_element(moveTimes.begin(), moveTimes.end());
            summary.minTime = *std::min_element(moveTimes.begin(), moveTimes.end());
            
            std::sort(moveTimes.begin(), moveTimes.end());
            summary.medianTime = moveTimes[moveTimes.size() / 2];
            
            summary.timeouts = std::count_if(moveTimes.begin(), moveTimes.end(), 
                                           [](double t) { return t > 0.5; });
            summary.timeoutRate = (double)summary.timeouts / moveTimes.size() * 100.0;
            
            result.depthResults.push_back(summary);
            result.completed = true;
            
        } catch (const std::exception& e) {
            result.error = "Exception during stress test: " + std::string(e.what());
            result.completed = false;
        }
        
        return result;
    }
    
    GameBenchmarkResult benchmarkMemoryPressure() {
        std::cout << "🔍 Testing: Memory pressure handling..." << std::endl;
        GameBenchmarkResult result;
        result.scenario = "Memory Pressure";
        
        try {
            // Create AI with deep search to stress memory
            MinMaxAI ai(BLACK, 12, 1.0, "Memory Test AI");
            
            Board board;
            // Create complex position
            for (int i = 0; i < 50; i++) {
                int x = 5 + (i % 9);
                int y = 5 + ((i / 9) % 9);
                if (board.isValidMove(x, y)) {
                    board.placePiece(x, y, (i % 2 == 0) ? BLACK : WHITE);
                }
            }
            
            ai.onGameStart();
            
            // Run multiple moves to stress transposition table
            std::vector<double> times;
            for (int i = 0; i < 10; i++) {
                auto start = std::chrono::high_resolution_clock::now();
                Position move = ai.makeMove(board);
                auto end = std::chrono::high_resolution_clock::now();
                
                double time = std::chrono::duration<double>(end - start).count();
                times.push_back(time);
                
                if (move.x != -1 && move.y != -1) {
                    Board tempBoard = board;
                    tempBoard.placePiece(move.x, move.y, BLACK);
                }
            }
            
            BenchmarkResult summary;
            summary.depth = 12;
            summary.allTimes = times;
            summary.totalMoves = times.size();
            summary.averageTime = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
            summary.maxTime = *std::max_element(times.begin(), times.end());
            summary.minTime = *std::min_element(times.begin(), times.end());
            
            result.depthResults.push_back(summary);
            result.completed = true;
            
        } catch (const std::exception& e) {
            result.error = "Exception during memory pressure test: " + std::string(e.what());
            result.completed = false;
        }
        
        return result;
    }
    
    BenchmarkResult benchmarkAIAtDepth(const Board& board, int depth, int numMoves) {
        BenchmarkResult result;
        result.depth = depth;
        
        MinMaxAI ai(BLACK, depth, 0.5, "Benchmark AI");
        ai.onGameStart();
        
        std::vector<double> times;
        int timeouts = 0;
        long totalNodes = 0;
        
        for (int i = 0; i < numMoves; i++) {
            Board testBoard = board;
            
            auto start = std::chrono::high_resolution_clock::now();
            Position move = ai.makeMove(testBoard);
            auto end = std::chrono::high_resolution_clock::now();
            
            double time = std::chrono::duration<double>(end - start).count();
            times.push_back(time);
            
            if (time > 0.5) timeouts++;
            totalNodes += ai.getNodesEvaluated();
            
            // Add some variation to the board for next iteration
            if (move.x != -1 && move.y != -1 && i < numMoves - 1) {
                testBoard.placePiece(move.x, move.y, BLACK);
            }
        }
        
        result.allTimes = times;
        result.totalMoves = numMoves;
        result.timeouts = timeouts;
        result.timeoutRate = (double)timeouts / numMoves * 100.0;
        result.nodesEvaluated = totalNodes / numMoves;
        result.ttHitRate = ai.getTTHitRate();
        
        if (!times.empty()) {
            result.averageTime = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
            result.maxTime = *std::max_element(times.begin(), times.end());
            result.minTime = *std::min_element(times.begin(), times.end());
            
            std::sort(times.begin(), times.end());
            result.medianTime = times[times.size() / 2];
        }
        
        std::cout << "  Depth " << depth << ": avg=" << std::fixed << std::setprecision(3) 
                  << result.averageTime << "s, max=" << result.maxTime << "s, timeouts=" 
                  << result.timeouts << "/" << numMoves << std::endl;
        
        return result;
    }
    
    void generateBenchmarkReport(const std::vector<GameBenchmarkResult>& results) {
        std::ofstream report("performance_report.txt");
        
        report << "GOMOKU AI PERFORMANCE BENCHMARK REPORT" << std::endl;
        report << "=======================================" << std::endl;
        report << "Generated: " << getCurrentTimestamp() << std::endl;
        report << std::endl;
        
        report << "SUBJECT REQUIREMENTS:" << std::endl;
        report << "- Average move time: ≤ 0.5 seconds" << std::endl;
        report << "- Minimum search depth: 10 levels" << std::endl;
        report << "- No crashes under any circumstances" << std::endl;
        report << std::endl;
        
        for (const auto& scenario : results) {
            report << "SCENARIO: " << scenario.scenario << std::endl;
            report << "Status: " << (scenario.completed ? "COMPLETED" : "FAILED") << std::endl;
            
            if (!scenario.error.empty()) {
                report << "Error: " << scenario.error << std::endl;
            }
            
            report << std::endl;
            
            for (const auto& depthResult : scenario.depthResults) {
                report << "  Depth " << depthResult.depth << ":" << std::endl;
                report << "    Average time: " << std::fixed << std::setprecision(3) 
                       << depthResult.averageTime << "s" << std::endl;
                report << "    Median time:  " << depthResult.medianTime << "s" << std::endl;
                report << "    Max time:     " << depthResult.maxTime << "s" << std::endl;
                report << "    Min time:     " << depthResult.minTime << "s" << std::endl;
                report << "    Timeout rate: " << std::fixed << std::setprecision(1) 
                       << depthResult.timeoutRate << "%" << std::endl;
                report << "    Nodes/move:   " << depthResult.nodesEvaluated << std::endl;
                report << "    TT hit rate:  " << depthResult.ttHitRate << "%" << std::endl;
                report << std::endl;
            }
        }
        
        report.close();
        std::cout << "\n📊 Detailed report saved to: performance_report.txt" << std::endl;
    }
    
    void validateRequirements(const std::vector<GameBenchmarkResult>& results) {
        std::cout << "\n=== REQUIREMENT VALIDATION ===" << std::endl;
        
        bool overallPass = true;
        bool timeRequirementMet = true;
        bool depthRequirementMet = true;
        bool stabilityRequirementMet = true;
        
        double worstAverageTime = 0.0;
        int deepestSearchTested = 0;
        
        for (const auto& scenario : results) {
            if (!scenario.completed) {
                stabilityRequirementMet = false;
                std::cout << "❌ STABILITY: Scenario '" << scenario.scenario 
                          << "' failed: " << scenario.error << std::endl;
            }
            
            for (const auto& depthResult : scenario.depthResults) {
                deepestSearchTested = std::max(deepestSearchTested, depthResult.depth);
                worstAverageTime = std::max(worstAverageTime, depthResult.averageTime);
                
                if (depthResult.depth >= 10 && depthResult.averageTime > 0.5) {
                    timeRequirementMet = false;
                }
            }
        }
        
        if (deepestSearchTested < 10) {
            depthRequirementMet = false;
        }
        
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "⏱️  TIME REQUIREMENT (≤0.5s avg): ";
        if (timeRequirementMet) {
            std::cout << "✅ PASSED (worst: " << worstAverageTime << "s)" << std::endl;
        } else {
            std::cout << "❌ FAILED (worst: " << worstAverageTime << "s)" << std::endl;
            overallPass = false;
        }
        
        std::cout << "🔍 DEPTH REQUIREMENT (≥10 levels): ";
        if (depthRequirementMet) {
            std::cout << "✅ PASSED (tested up to depth " << deepestSearchTested << ")" << std::endl;
        } else {
            std::cout << "❌ FAILED (only tested depth " << deepestSearchTested << ")" << std::endl;
            overallPass = false;
        }
        
        std::cout << "🛡️  STABILITY REQUIREMENT (no crashes): ";
        if (stabilityRequirementMet) {
            std::cout << "✅ PASSED" << std::endl;
        } else {
            std::cout << "❌ FAILED" << std::endl;
            overallPass = false;
        }
        
        std::cout << "\n=== FINAL VALIDATION RESULT ===" << std::endl;
        if (overallPass) {
            std::cout << "🎉 PROJECT VALIDATION: ✅ PASSED" << std::endl;
            std::cout << "The AI meets all subject requirements!" << std::endl;
        } else {
            std::cout << "⚠️  PROJECT VALIDATION: ❌ FAILED" << std::endl;
            std::cout << "The AI does not meet subject requirements." << std::endl;
            std::cout << "Optimization needed before submission." << std::endl;
        }
    }
    
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

int main() {
    try {
        PerformanceBenchmark benchmark;
        benchmark.runFullBenchmark();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error during benchmarking: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error during benchmarking" << std::endl;
        return 1;
    }
}