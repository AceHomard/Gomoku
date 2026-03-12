#ifdef DEBUG_VISU

#include "Debug/TreeRenderer.hpp"
#include <sstream>
#include <cmath>
#include <algorithm>
#include <limits>

TreeRenderer::TreeRenderer() {
}

void TreeRenderer::toggleCollapse(const DebugTreeNode* node) {
    if (!node || node->children.empty()) return;
    auto key = std::make_tuple(node->move.x, node->move.y, node->depth);
    if (collapsedPositions.count(key)) {
        collapsedPositions.erase(key);
    } else {
        collapsedPositions.insert(key);
    }
}

bool TreeRenderer::isCollapsed(const DebugTreeNode* node) const {
    if (!node) return false;
    return collapsedPositions.count(std::make_tuple(node->move.x, node->move.y, node->depth)) > 0;
}

void TreeRenderer::autoCollapseForBestPath(const DebugTreeNode* root) {
    // Compute best path first, then collapse everything else
    computeBestPath(root);
    collapsedPositions.clear();
    collapseNonBestPath(root);
}

void TreeRenderer::computeBestPath(const DebugTreeNode* root) {
    bestPath.clear();
    if (!root) return;

    bestPath.insert(root);
    const DebugTreeNode* current = root;

    while (!current->children.empty()) {
        // First look for the flagged best move child
        const DebugTreeNode* next = nullptr;
        for (auto& child : current->children) {
            if (child->isBestMove) {
                next = child.get();
                break;
            }
        }

        // If no flagged best move, follow optimal score
        // Use CHILDREN's isMaximizing to decide: if children are MAX, pick highest;
        // if children are MIN, pick lowest (children's flag = who made the decision)
        if (!next) {
            bool childrenAreMax = !current->children.empty()
                && current->children[0]->isMaximizing;
            if (childrenAreMax) {
                int best = std::numeric_limits<int>::min();
                for (auto& child : current->children) {
                    if (!child->isPruned && child->score > best) {
                        best = child->score;
                        next = child.get();
                    }
                }
            } else {
                int best = std::numeric_limits<int>::max();
                for (auto& child : current->children) {
                    if (!child->isPruned && child->score < best) {
                        best = child->score;
                        next = child.get();
                    }
                }
            }
        }

        if (!next) break;
        bestPath.insert(next);
        current = next;
    }

}

void TreeRenderer::collapseNonBestPath(const DebugTreeNode* node) {
    if (!node || node->children.empty()) return;

    for (auto& child : node->children) {
        if (!child->children.empty()) {
            auto key = std::make_tuple(child->move.x, child->move.y, child->depth);
            if (bestPath.count(child.get())) {
                // On best path: keep expanded, recurse into children
                collapsedPositions.erase(key);
                collapseNonBestPath(child.get());
            } else {
                // Not on best path: collapse
                collapsedPositions.insert(key);
            }
        }
    }
}

std::vector<const DebugTreeNode*> TreeRenderer::getVisibleChildren(const DebugTreeNode* node) const {
    if (!node || node->children.empty()) return {};

    // If few enough children, show all
    if (static_cast<int>(node->children.size()) <= MAX_VISIBLE_CHILDREN) {
        std::vector<const DebugTreeNode*> all;
        for (auto& child : node->children) {
            all.push_back(child.get());
        }
        return all;
    }

    // Collect pointers with their score magnitude for sorting
    std::vector<std::pair<int, const DebugTreeNode*>> scored;
    scored.reserve(node->children.size());
    for (auto& child : node->children) {
        scored.emplace_back(std::abs(child->score), child.get());
    }

    // Sort by absolute score descending (most interesting first)
    std::sort(scored.begin(), scored.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // Always include bestPath nodes and bestMove nodes
    std::set<const DebugTreeNode*> mustShow;
    for (auto& child : node->children) {
        if (bestPath.count(child.get()) || child->isBestMove) {
            mustShow.insert(child.get());
        }
    }

    // Fill up to MAX_VISIBLE_CHILDREN, prioritizing mustShow + top scores
    std::set<const DebugTreeNode*> selected(mustShow.begin(), mustShow.end());
    for (auto& [score, child] : scored) {
        if (static_cast<int>(selected.size()) >= MAX_VISIBLE_CHILDREN) break;
        selected.insert(child);
    }

    // Return in original order (preserves tree structure)
    std::vector<const DebugTreeNode*> result;
    for (auto& child : node->children) {
        if (selected.count(child.get())) {
            result.push_back(child.get());
        }
    }
    return result;
}

float TreeRenderer::computeSubtreeWidth(const DebugTreeNode* node, int depth) const {
    if (!node || node->children.empty() || depth >= MAX_RENDER_DEPTH || isCollapsed(node)) {
        return NODE_W + H_SPACING;
    }
    auto visible = getVisibleChildren(node);
    float totalWidth = 0;
    // Add space for "...+N" indicator if children were hidden
    if (visible.size() < node->children.size()) {
        totalWidth += 60.0f; // small width for the ellipsis indicator
    }
    for (auto* child : visible) {
        totalWidth += computeSubtreeWidth(child, depth + 1);
    }
    return std::max(totalWidth, NODE_W + H_SPACING);
}

void TreeRenderer::render(sf::RenderWindow& window, sf::Font& font,
                          const DebugTreeNode* root, float areaWidth, float areaHeight) {
    (void)areaHeight;
    nodeBounds.clear();

    if (!root || root->children.empty()) {
        sf::Text placeholder(font, "Waiting for AI search...", 16);
        placeholder.setFillColor(sf::Color(120, 120, 140));
        placeholder.setPosition(sf::Vector2f(areaWidth / 2.0f - 100.0f, 200.0f));
        window.draw(placeholder);
        return;
    }

    // Compute best decision path
    computeBestPath(root);

    // Compute total width needed (no scaling — zoom/pan handles overflow)
    float totalWidth = computeSubtreeWidth(root, 0);
    lastTotalWidth = totalWidth;
    lastTotalHeight = TOP_MARGIN + (MAX_RENDER_DEPTH + 1) * (NODE_H + V_SPACING);

    // Start position: centered if fits, otherwise left-aligned with margin
    float startX = (areaWidth - totalWidth) / 2.0f;
    if (startX < 10.0f) startX = 10.0f;

    // Render full tree recursively at 1:1 scale
    renderSubtree(window, font, root, startX, TOP_MARGIN, 1.0f, 0);
}

void TreeRenderer::renderSubtree(sf::RenderWindow& window, sf::Font& font,
                                  const DebugTreeNode* node,
                                  float x, float y, float scale, int depth) {
    if (!node) return;

    // Compute this node's center X
    float subtreeW = computeSubtreeWidth(node, depth) * scale;
    float nodeCenterX = x + subtreeW / 2.0f - NODE_W / 2.0f;

    // Draw this node
    renderNode(window, font, node, nodeCenterX, y);

    // Stop recursion if collapsed, max depth, or no children
    if (node->children.empty() || depth >= MAX_RENDER_DEPTH || isCollapsed(node)) {
        return;
    }

    // Get filtered children (top N by score + bestPath)
    auto visible = getVisibleChildren(node);
    int hiddenCount = static_cast<int>(node->children.size()) - static_cast<int>(visible.size());

    // Draw children recursively
    float childX = x;
    float childY = y + NODE_H + V_SPACING;

    for (auto* child : visible) {
        float childSubtreeW = computeSubtreeWidth(child, depth + 1) * scale;
        float childCenterX = childX + childSubtreeW / 2.0f - NODE_W / 2.0f;

        // Connection
        bool isBest = child->isBestMove;
        bool onPath = bestPath.count(node) && bestPath.count(child);
        renderConnection(window,
                         nodeCenterX + NODE_W / 2.0f, y + NODE_H,
                         childCenterX + NODE_W / 2.0f, childY,
                         isBest, onPath);

        // Recurse
        renderSubtree(window, font, child, childX, childY, scale, depth + 1);

        childX += childSubtreeW;
    }

    // Draw "...+N" indicator for hidden children
    if (hiddenCount > 0) {
        float indicatorX = childX + 5.0f;
        float indicatorY = childY + NODE_H / 2.0f - 8.0f;

        // Dotted connection line
        renderConnection(window,
                         nodeCenterX + NODE_W / 2.0f, y + NODE_H,
                         indicatorX + 20.0f, childY,
                         false, false);

        // Ellipsis badge
        std::ostringstream ellipsis;
        ellipsis << "...+" << hiddenCount;
        sf::Text ellipsisText(font, ellipsis.str(), 11);
        ellipsisText.setFillColor(sf::Color(140, 140, 160));
        ellipsisText.setStyle(sf::Text::Italic);
        ellipsisText.setPosition(sf::Vector2f(indicatorX, indicatorY));
        window.draw(ellipsisText);
    }
}

void TreeRenderer::renderNode(sf::RenderWindow& window, sf::Font& font,
                               const DebugTreeNode* node, float x, float y) {
    if (!node) return;

    bool onPath = bestPath.count(node) > 0;
    sf::Color bgColor = getNodeColor(node);

    // Node rectangle
    sf::RectangleShape rect(sf::Vector2f(NODE_W, NODE_H));
    rect.setPosition(sf::Vector2f(x, y));
    rect.setFillColor(bgColor);
    rect.setOutlineThickness(2.0f);

    // Outline color based on state
    if (node == selectedNode) {
        rect.setOutlineColor(sf::Color(0, 220, 255));
        rect.setOutlineThickness(3.0f);
    } else if (node == hoveredNode) {
        rect.setOutlineColor(sf::Color(255, 255, 0));
        rect.setOutlineThickness(3.0f);
    } else if (onPath) {
        // Animated gold glow for best path
        float pulse = std::sin(animClock.getElapsedTime().asSeconds() * 3.0f) * 0.3f + 0.7f;
        std::uint8_t glowAlpha = static_cast<std::uint8_t>(255 * pulse);
        rect.setOutlineColor(sf::Color(255, 215, 0, glowAlpha));
        rect.setOutlineThickness(3.0f);
    } else if (node->isBestMove) {
        rect.setOutlineColor(sf::Color(255, 215, 0));
        rect.setOutlineThickness(3.0f);
    } else {
        rect.setOutlineColor(sf::Color(60, 60, 80));
    }

    window.draw(rect);

    // Best path glow background
    if (onPath && node != selectedNode && node != hoveredNode) {
        sf::RectangleShape glow(sf::Vector2f(NODE_W + 6.0f, NODE_H + 6.0f));
        glow.setPosition(sf::Vector2f(x - 3.0f, y - 3.0f));
        glow.setFillColor(sf::Color(255, 215, 0, 30));
        glow.setOutlineColor(sf::Color::Transparent);
        window.draw(glow);
    }

    // Store bounds for hit-testing
    nodeBounds[node] = sf::FloatRect(sf::Vector2f(x, y), sf::Vector2f(NODE_W, NODE_H));

    // Text: move coordinates
    std::ostringstream coordStr;
    if (node->move.x >= 0) {
        coordStr << "(" << node->move.x << "," << node->move.y << ")";
    } else {
        coordStr << "ROOT";
    }
    sf::Text coordText(font, coordStr.str(), 11);
    coordText.setFillColor(sf::Color::White);
    coordText.setPosition(sf::Vector2f(x + 5.0f, y + 3.0f));
    window.draw(coordText);

    // Text: score
    std::ostringstream scoreStr;
    scoreStr << (node->score >= 0 ? "+" : "") << node->score;
    sf::Text scoreText(font, scoreStr.str(), 14);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setStyle(sf::Text::Bold);
    scoreText.setPosition(sf::Vector2f(x + 5.0f, y + 18.0f));
    window.draw(scoreText);

    // Text: MAX/MIN
    std::string typeStr = node->isMaximizing ? "MAX" : "MIN";
    sf::Text typeText(font, typeStr, 10);
    typeText.setFillColor(node->isMaximizing ? sf::Color(100, 200, 255) : sf::Color(255, 150, 100));
    typeText.setPosition(sf::Vector2f(x + 5.0f, y + 38.0f));
    window.draw(typeText);

    // Depth indicator
    std::ostringstream depthStr;
    depthStr << "d" << node->depth;
    sf::Text depthText(font, depthStr.str(), 10);
    depthText.setFillColor(sf::Color(150, 150, 170));
    depthText.setPosition(sf::Vector2f(x + NODE_W - 25.0f, y + 38.0f));
    window.draw(depthText);

    // PRUNED overlay
    if (node->isPruned) {
        sf::Text prunedText(font, "CUT", 10);
        prunedText.setFillColor(sf::Color(255, 100, 100, 200));
        prunedText.setStyle(sf::Text::Bold);
        prunedText.setPosition(sf::Vector2f(x + NODE_W - 35.0f, y + 3.0f));
        window.draw(prunedText);
    }

    // Collapse indicator: [+] with child count or [-]
    if (!node->children.empty()) {
        bool collapsed = isCollapsed(node);
        std::ostringstream badge;
        if (collapsed) {
            badge << "[+] " << node->children.size();
        } else {
            badge << "[-]";
        }
        sf::Text collapseText(font, badge.str(), 9);
        collapseText.setFillColor(sf::Color(200, 200, 220));
        collapseText.setPosition(sf::Vector2f(x + NODE_W - 55.0f, y + NODE_H - 14.0f));
        window.draw(collapseText);
    }
}

void TreeRenderer::renderConnection(sf::RenderWindow& window,
                                     float parentX, float parentY,
                                     float childX, float childY,
                                     bool isBest, bool isOnBestPath) {
    if (isOnBestPath) {
        // Thick gold glow for best path
        // Draw glow rectangle behind the line
        float dx = childX - parentX;
        float dy = childY - parentY;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len > 0) {
            float angle = std::atan2(dy, dx) * 180.0f / 3.14159265f;
            sf::RectangleShape glowLine(sf::Vector2f(len, 4.0f));
            glowLine.setPosition(sf::Vector2f(parentX, parentY));
            glowLine.setRotation(sf::degrees(angle));
            glowLine.setFillColor(sf::Color(255, 215, 0, 180));
            window.draw(glowLine);
        }
    } else {
        sf::Color lineColor = isBest ? sf::Color(255, 215, 0, 200) : sf::Color(100, 100, 120, 150);
        sf::Vertex line[] = {
            sf::Vertex{sf::Vector2f(parentX, parentY), lineColor},
            sf::Vertex{sf::Vector2f(childX, childY), lineColor}
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }
}

sf::Color TreeRenderer::getNodeColor(const DebugTreeNode* node) const {
    if (node->isPruned) {
        return sf::Color(80, 80, 90);
    }
    if (node->score > 5000) {
        return sf::Color(46, 125, 50);
    }
    if (node->score > 0) {
        return sf::Color(56, 142, 60);
    }
    if (node->score < -5000) {
        return sf::Color(183, 28, 28);
    }
    if (node->score < 0) {
        return sf::Color(211, 47, 47);
    }
    return sf::Color(66, 66, 80);
}

const DebugTreeNode* TreeRenderer::getNodeAtPosition(sf::Vector2f mousePos) const {
    for (auto& pair : nodeBounds) {
        if (pair.second.contains(mousePos)) {
            return pair.first;
        }
    }
    return nullptr;
}

bool TreeRenderer::getNodeBounds(const DebugTreeNode* node, sf::FloatRect& outBounds) const {
    auto it = nodeBounds.find(node);
    if (it != nodeBounds.end()) {
        outBounds = it->second;
        return true;
    }
    return false;
}

#endif // DEBUG_VISU
