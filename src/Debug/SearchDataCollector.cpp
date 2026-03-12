#ifdef DEBUG_VISU

#include "Debug/SearchDataCollector.hpp"

SearchDataCollector& SearchDataCollector::instance() {
    static SearchDataCollector inst;
    return inst;
}

bool SearchDataCollector::shouldCollect(int depth) const {
    return depth >= totalSearchDepth - maxCollectDepth;
}

// --- AI thread methods (no snapshot mutex needed) ---

void SearchDataCollector::onSearchStart(int searchDepth) {
    liveRoot = std::make_unique<DebugTreeNode>();
    liveRoot->move = Position(-1, -1);
    liveRoot->depth = searchDepth;
    liveRoot->isMaximizing = true;
    totalSearchDepth = searchDepth;
    while (!nodeStack.empty()) nodeStack.pop();
    nodeStack.push(liveRoot.get());
    skippedEnters = 0;
    searching = true;
}

void SearchDataCollector::onNodeEnter(Position move, int depth, int alpha, int beta, bool isMax) {
    if (!shouldCollect(depth)) {
        skippedEnters++;
        return;
    }
    if (nodeStack.empty()) return;

    auto node = std::make_unique<DebugTreeNode>();
    node->move = move;
    node->depth = depth;
    node->alpha = alpha;
    node->beta = beta;
    node->isMaximizing = isMax;
    node->isCurrentlyEvaluating = true;

    DebugTreeNode* parent = nodeStack.top();
    DebugTreeNode* raw = node.get();
    parent->children.push_back(std::move(node));
    nodeStack.push(raw);
}

void SearchDataCollector::onNodeExit(int score, bool pruned) {
    // If this exit corresponds to a skipped enter, just decrement counter
    if (skippedEnters > 0) {
        skippedEnters--;
        return;
    }

    if (nodeStack.empty()) return;

    DebugTreeNode* node = nodeStack.top();
    if (nodeStack.size() <= 1) return; // Don't pop root

    node->score = score;
    node->isPruned = pruned;
    node->isCurrentlyEvaluating = false;
    nodeStack.pop();
}

void SearchDataCollector::onBestMoveUpdate(Position move, int score) {
    if (!liveRoot) return;
    for (auto& child : liveRoot->children) {
        child->isBestMove = (child->move == move);
    }
    (void)score;
}

void SearchDataCollector::onSearchComplete(Position bestMove, int score, int nodes, int cuts, float ms) {
    if (liveRoot) {
        liveRoot->score = score;
        liveRoot->isCurrentlyEvaluating = false;
    }

    // Create snapshot under lock — main thread reads this safely
    SearchStats newStats;
    newStats.bestMove = bestMove;
    newStats.bestScore = score;
    newStats.nodesEvaluated = nodes;
    newStats.cutoffsCount = cuts;
    newStats.searchTimeMs = ms;

    auto newSnapshot = cloneTree(liveRoot.get());

    {
        std::lock_guard<std::mutex> lock(snapshotMutex);
        snapshotRoot = std::move(newSnapshot);
        snapshotStats = newStats;
    }

    newSnapshotAvailable = true;
    searching = false;
}

// --- Main thread methods ---

std::unique_ptr<DebugTreeNode> SearchDataCollector::cloneSnapshot() const {
    std::lock_guard<std::mutex> lock(snapshotMutex);
    return cloneTree(snapshotRoot.get());
}

SearchStats SearchDataCollector::getStats() const {
    std::lock_guard<std::mutex> lock(snapshotMutex);
    return snapshotStats;
}

bool SearchDataCollector::isSearchInProgress() const {
    return searching.load();
}

bool SearchDataCollector::hasNewSnapshot() const {
    return newSnapshotAvailable.load();
}

void SearchDataCollector::markSnapshotRead() {
    newSnapshotAvailable = false;
}

// --- Helper: deep clone tree ---

std::unique_ptr<DebugTreeNode> SearchDataCollector::cloneTree(const DebugTreeNode* node) {
    if (!node) return nullptr;

    auto clone = std::make_unique<DebugTreeNode>();
    clone->move = node->move;
    clone->score = node->score;
    clone->alpha = node->alpha;
    clone->beta = node->beta;
    clone->depth = node->depth;
    clone->isMaximizing = node->isMaximizing;
    clone->isPruned = node->isPruned;
    clone->isBestMove = node->isBestMove;
    clone->isCurrentlyEvaluating = false;

    for (auto& child : node->children) {
        clone->children.push_back(cloneTree(child.get()));
    }

    return clone;
}

#endif // DEBUG_VISU
