/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Heuristic.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEURISTIC_HPP
#define HEURISTIC_HPP

#include "../Game/Board.hpp"
#include <vector>
#include <array>

// Pattern types for recognition
enum PatternType {
    FIVE = 0,           // Five in a row (win)
    OPEN_FOUR = 1,      // Four with both ends open
    CLOSED_FOUR = 2,    // Four with one end blocked
    OPEN_THREE = 3,     // Three with both ends open
    CLOSED_THREE = 4,   // Three with one end blocked
    OPEN_TWO = 5,       // Two with both ends open
    CLOSED_TWO = 6,     // Two with one end blocked
    SINGLE = 7,         // Single stone
    INVALID = 8         // Invalid pattern
};

// Pattern evaluation structure
struct Pattern {
    PatternType type;
    int length;
    bool openStart;
    bool openEnd;
    int value;
    
    Pattern() : type(INVALID), length(0), openStart(false), openEnd(false), value(0) {}
    Pattern(PatternType t, int len, bool start, bool end, int val) 
        : type(t), length(len), openStart(start), openEnd(end), value(val) {}
};

// Threat analysis structure
struct Threat {
    Position position;
    PatternType type;
    int priority;
    int direction; // 0=horizontal, 1=vertical, 2=diagonal1, 3=diagonal2
    
    Threat(Position pos, PatternType t, int prio, int dir) 
        : position(pos), type(t), priority(prio), direction(dir) {}
};

class Heuristic {
private:
    // Pattern value tables
    static const std::array<int, 9> PATTERN_VALUES;
    static const std::array<int, 4> DIRECTION_VECTORS_X;
    static const std::array<int, 4> DIRECTION_VECTORS_Y;
    
    // Evaluation weights
    static const int CAPTURE_VALUE;
    static const int WIN_VALUE;
    static const int POSITION_WEIGHT;
    static const int THREAT_MULTIPLIER;
    static const int MOBILITY_WEIGHT;

public:
    Heuristic();
    ~Heuristic();
    
    // Main evaluation function
    int evaluatePosition(const Board& board, CellState player);
    
    // Individual evaluation components
    int evaluatePatterns(const Board& board, CellState player);
    int evaluateCaptures(const Board& board, CellState player);
    int evaluateThreats(const Board& board, CellState player);
    int evaluatePositional(const Board& board, CellState player);
    int evaluateMobility(const Board& board, CellState player);
    
    // Pattern recognition
    Pattern analyzePattern(const Board& board, int x, int y, int direction, CellState player);
    std::vector<Pattern> findAllPatterns(const Board& board, CellState player);
    PatternType classifyPattern(int length, bool openStart, bool openEnd);
    
    // Threat analysis
    std::vector<Threat> findThreats(const Board& board, CellState player);
    std::vector<Threat> findImmediateThreats(const Board& board, CellState player);
    bool isForcedMove(const Board& board, const Position& move, CellState player);
    int getThreatLevel(const Board& board, CellState player);
    
    // Tactical evaluation
    bool isWinning(const Board& board, CellState player);
    bool isLosing(const Board& board, CellState player);
    std::vector<Position> findWinningMoves(const Board& board, CellState player);
    std::vector<Position> findDefensiveMoves(const Board& board, CellState player);
    
    // Move evaluation
    int evaluateMove(const Board& board, const Position& move, CellState player);
    int evaluateMoveCaptures(const Board& board, const Position& move, CellState player);
    int evaluateMoveThreats(const Board& board, const Position& move, CellState player);
    
    // Pattern utilities
    bool isOpenEnded(const Board& board, int x, int y, int dx, int dy, int length, CellState player);
    int countConsecutiveStones(const Board& board, int x, int y, int dx, int dy, CellState player);
    int getPatternValue(PatternType type);
    
    // Board analysis utilities
    bool hasAdjacentStone(const Board& board, int x, int y, int radius = 1);
    std::vector<Position> getRelevantMoves(const Board& board);
    double getBoardComplexity(const Board& board);
    
    // Opening and endgame evaluation
    int evaluateOpening(const Board& board, CellState player);
    int evaluateEndgame(const Board& board, CellState player);
    bool isOpeningPhase(const Board& board);
    bool isEndgamePhase(const Board& board);
    
    // Debug and analysis
    void printPatternAnalysis(const Board& board, CellState player);
    void printThreatAnalysis(const Board& board, CellState player);
    std::string patternTypeToString(PatternType type);

private:
    // Helper functions for pattern analysis
    int scanDirection(const Board& board, int x, int y, int dx, int dy, CellState player, bool& openStart, bool& openEnd);
    bool isValidAndEmpty(const Board& board, int x, int y);
    bool isValidAndPlayer(const Board& board, int x, int y, CellState player);
    bool isValidAndOpponent(const Board& board, int x, int y, CellState player);
    
    // Positional evaluation helpers
    int getPositionalScore(int x, int y);
    int getCenterDistance(int x, int y);
    int getCornerDistance(int x, int y);
    
    // Capture analysis helpers
    int evaluateCaptureOpportunities(const Board& board, const Position& move, CellState player);
    int evaluateCaptureThreats(const Board& board, const Position& move, CellState player);
    
    // Advanced pattern matching
    bool matchesPattern(const Board& board, int x, int y, int direction, 
                       const std::vector<int>& pattern, CellState player);
    int getPatternScore(const std::vector<int>& pattern);
    
    // Threat computation
    int computeThreatPriority(PatternType type, bool isImmediate);
    bool canBlockThreat(const Board& board, const Threat& threat, CellState player);
    
    // Performance optimizations
    mutable std::vector<std::vector<int>> evaluationCache;
    void invalidateCache();
    bool isCacheValid(int x, int y) const;
    void updateCache(int x, int y, int value);
};

#endif // HEURISTIC_HPP