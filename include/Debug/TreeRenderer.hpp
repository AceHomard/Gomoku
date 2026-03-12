/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TreeRenderer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/30 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TREERENDERER_HPP
#define TREERENDERER_HPP

#ifdef DEBUG_VISU

#include <SFML/Graphics.hpp>
#include "SearchDataCollector.hpp"
#include <map>
#include <set>
#include <tuple>

class TreeRenderer {
public:
    TreeRenderer();

    void render(sf::RenderWindow& window, sf::Font& font,
                const DebugTreeNode* root, float areaWidth, float areaHeight);

    const DebugTreeNode* getNodeAtPosition(sf::Vector2f mousePos) const;
    bool getNodeBounds(const DebugTreeNode* node, sf::FloatRect& outBounds) const;

    void setHoveredNode(const DebugTreeNode* node) { hoveredNode = node; }
    void setSelectedNode(const DebugTreeNode* node) { selectedNode = node; }
    void toggleCollapse(const DebugTreeNode* node);
    bool isCollapsed(const DebugTreeNode* node) const;
    void autoCollapseForBestPath(const DebugTreeNode* root);

private:
    // Layout constants
    static constexpr float NODE_W = 110.0f;
    static constexpr float NODE_H = 55.0f;
    static constexpr float H_SPACING = 25.0f;
    static constexpr float V_SPACING = 70.0f;
    static constexpr float TOP_MARGIN = 50.0f;
    static constexpr int MAX_RENDER_DEPTH = 20;

    // Node positions cache for hit-testing
    std::map<const DebugTreeNode*, sf::FloatRect> nodeBounds;

    const DebugTreeNode* hoveredNode = nullptr;
    const DebugTreeNode* selectedNode = nullptr;

    // Collapse state keyed by (x, y, depth) to distinguish same move at different depths
    std::set<std::tuple<int,int,int>> collapsedPositions;

    // Best decision path
    std::set<const DebugTreeNode*> bestPath;
    void computeBestPath(const DebugTreeNode* root);
    void collapseNonBestPath(const DebugTreeNode* node);

    // Layout
    float computeSubtreeWidth(const DebugTreeNode* node, int depth) const;

    // Recursive render
    void renderSubtree(sf::RenderWindow& window, sf::Font& font,
                       const DebugTreeNode* node,
                       float x, float y, float scale, int depth);
    void renderNode(sf::RenderWindow& window, sf::Font& font,
                    const DebugTreeNode* node, float x, float y);
    void renderConnection(sf::RenderWindow& window,
                          float parentX, float parentY,
                          float childX, float childY,
                          bool isBest, bool isOnBestPath);

    sf::Color getNodeColor(const DebugTreeNode* node) const;

    // Filter children to keep only the most relevant ones for display
    static constexpr int MAX_VISIBLE_CHILDREN = 8;
    std::vector<const DebugTreeNode*> getVisibleChildren(const DebugTreeNode* node) const;

    sf::Clock animClock;

    // Last computed total tree width (for auto-fit zoom)
    float lastTotalWidth = 0.0f;
    float lastTotalHeight = 0.0f;

public:
    float getLastTotalWidth() const { return lastTotalWidth; }
    float getLastTotalHeight() const { return lastTotalHeight; }
};

#endif // DEBUG_VISU
#endif // TREERENDERER_HPP
