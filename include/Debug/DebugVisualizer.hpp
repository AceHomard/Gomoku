#ifndef DEBUGVISUALIZER_HPP
#define DEBUGVISUALIZER_HPP

#ifdef DEBUG_VISU

#include <SFML/Graphics.hpp>
#include "SearchDataCollector.hpp"
#include "TreeRenderer.hpp"
#include "StatsPanel.hpp"
#include <memory>

class DebugVisualizer {
public:
    DebugVisualizer();
    ~DebugVisualizer();

    bool initialize();
    void handleEvents();
    void render();
    bool isOpen() const;
    void close();
    void toggleFullscreen();

private:
    sf::RenderWindow window;
    sf::Font font;
    bool fontLoaded;

    std::unique_ptr<TreeRenderer> treeRenderer;
    std::unique_ptr<StatsPanel> statsPanel;

    // Owned copy of the tree
    std::unique_ptr<DebugTreeNode> cachedTree;
    SearchStats cachedStats;

    // Interaction state
    const DebugTreeNode* hoveredNode;
    const DebugTreeNode* selectedNode;

    static constexpr int DEFAULT_W = 1800;
    static constexpr int DEFAULT_H = 1200;
    static constexpr int STATS_PANEL_W = 300;

    bool fullscreen;

    // Zoom/Pan
    sf::View treeView;
    bool dragging;
    sf::Vector2i lastMousePos;
    float zoomLevel;

    // Double-click detection
    sf::Clock doubleClickClock;
    const DebugTreeNode* lastClickedNode;

    // Auto-fit zoom after new snapshot
    bool autoFitPending = false;

    // Anchor for view stability on collapse/expand
    const DebugTreeNode* anchorNode = nullptr;
    sf::Vector2f anchorScreenPos;

    int getWidth() const;
    int getHeight() const;

    bool loadFont();
    void refreshSnapshot();
    void resetTreeView();
};

#endif // DEBUG_VISU
#endif // DEBUGVISUALIZER_HPP
