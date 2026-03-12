/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SearchDataCollector.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/30 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SEARCHDATACOLLECTOR_HPP
#define SEARCHDATACOLLECTOR_HPP

#ifdef DEBUG_VISU

#include "../Game/Board.hpp"
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <stack>

struct DebugTreeNode {
    Position move;
    int score;
    int alpha;
    int beta;
    int depth;
    bool isMaximizing;
    bool isPruned;
    bool isBestMove;
    bool isCurrentlyEvaluating;
    std::vector<std::unique_ptr<DebugTreeNode>> children;

    DebugTreeNode() : move(-1, -1), score(0), alpha(0), beta(0),
                      depth(0), isMaximizing(true), isPruned(false),
                      isBestMove(false), isCurrentlyEvaluating(false) {}
};

struct SearchStats {
    Position bestMove{-1, -1};
    int bestScore = 0;
    int nodesEvaluated = 0;
    int cutoffsCount = 0;
    float searchTimeMs = 0.0f;
    int currentAlpha = 0;
    int currentBeta = 0;
};

class SearchDataCollector {
public:
    static SearchDataCollector& instance();

    // Called from AI thread (writes to live tree)
    void onSearchStart(int searchDepth);
    void onNodeEnter(Position move, int depth, int alpha, int beta, bool isMax);
    void onNodeExit(int score, bool pruned);
    void onBestMoveUpdate(Position move, int score);
    void onSearchComplete(Position bestMove, int score, int nodes, int cuts, float ms);

    // Called from main thread — returns owned copies (no race)
    std::unique_ptr<DebugTreeNode> cloneSnapshot() const;
    SearchStats getStats() const;
    bool isSearchInProgress() const;
    bool hasNewSnapshot() const;
    void markSnapshotRead();

    void setMaxCollectDepth(int d) { maxCollectDepth = d; }

private:
    SearchDataCollector() = default;
    SearchDataCollector(const SearchDataCollector&) = delete;
    SearchDataCollector& operator=(const SearchDataCollector&) = delete;

    // Live tree: only AI thread touches this (no mutex needed for writes)
    std::unique_ptr<DebugTreeNode> liveRoot;
    std::stack<DebugTreeNode*> nodeStack;
    int maxCollectDepth = 20;
    int totalSearchDepth = 0;
    int skippedEnters = 0;  // Track non-collected enters to avoid stack corruption

    // Snapshot: swapped from live at search end, read by main thread
    mutable std::mutex snapshotMutex;
    std::unique_ptr<DebugTreeNode> snapshotRoot;
    SearchStats snapshotStats;

    std::atomic<bool> searching{false};
    std::atomic<bool> newSnapshotAvailable{false};

    bool shouldCollect(int depth) const;
    static std::unique_ptr<DebugTreeNode> cloneTree(const DebugTreeNode* node);
};

#endif // DEBUG_VISU
#endif // SEARCHDATACOLLECTOR_HPP
