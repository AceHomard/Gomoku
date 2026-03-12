/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DebugVisualizer.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/30 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifdef DEBUG_VISU

#include "Debug/DebugVisualizer.hpp"
#include <iostream>
#include <algorithm>

DebugVisualizer::DebugVisualizer()
    : fontLoaded(false), hoveredNode(nullptr), selectedNode(nullptr),
      fullscreen(false), dragging(false), zoomLevel(1.0f), lastClickedNode(nullptr) {
}

DebugVisualizer::~DebugVisualizer() {
    close();
}

bool DebugVisualizer::initialize() {
    std::cout << "[DebugVisu] Creating debug window..." << std::endl;

    window.create(
        sf::VideoMode(sf::Vector2u(DEFAULT_W, DEFAULT_H)),
        "Gomoku - AI Debug Visualizer",
        sf::Style::Titlebar | sf::Style::Close
    );
    window.setVerticalSyncEnabled(true);
    window.setPosition(sf::Vector2i(50, 50));

    std::cout << "[DebugVisu] Window created (" << window.getSize().x << "x" << window.getSize().y << ")" << std::endl;

    fontLoaded = loadFont();
    if (!fontLoaded) {
        std::cerr << "[DebugVisu] Warning: could not load font" << std::endl;
    }

    treeRenderer = std::make_unique<TreeRenderer>();
    statsPanel = std::make_unique<StatsPanel>();

    resetTreeView();

    std::cout << "[DebugVisu] Initialized | F=Fullscreen | Wheel=Zoom | Pan=Drag | R=Reset | DblClick=Collapse" << std::endl;
    return true;
}

void DebugVisualizer::resetTreeView() {
    // Use known dimensions rather than window.getSize() which may return 0 on WSL2
    int w = getWidth();
    int h = getHeight();
    if (w <= STATS_PANEL_W || h <= 0) {
        w = fullscreen ? 1920 : DEFAULT_W;
        h = fullscreen ? 1080 : DEFAULT_H;
    }
    float treeW = static_cast<float>(w - STATS_PANEL_W);
    float treeH = static_cast<float>(h);
    treeView.setSize(sf::Vector2f(treeW, treeH));
    treeView.setCenter(sf::Vector2f(treeW / 2.0f, treeH / 2.0f));

    // Set viewport so mapPixelToCoords works correctly in handleEvents
    float viewportRight = treeW / static_cast<float>(w);
    treeView.setViewport(sf::FloatRect(
        sf::Vector2f(0.0f, 0.0f),
        sf::Vector2f(viewportRight, 1.0f)));

    zoomLevel = 1.0f;
}

int DebugVisualizer::getWidth() const {
    return static_cast<int>(window.getSize().x);
}

int DebugVisualizer::getHeight() const {
    return static_cast<int>(window.getSize().y);
}

void DebugVisualizer::toggleFullscreen() {
    fullscreen = !fullscreen;
    if (fullscreen) {
        window.create(
            sf::VideoMode::getDesktopMode(),
            "Gomoku - AI Debug Visualizer",
            sf::Style::None
        );
    } else {
        window.create(
            sf::VideoMode(sf::Vector2u(DEFAULT_W, DEFAULT_H)),
            "Gomoku - AI Debug Visualizer",
            sf::Style::Titlebar | sf::Style::Close
        );
        window.setPosition(sf::Vector2i(50, 50));
    }
    window.setVerticalSyncEnabled(true);
    resetTreeView();
    std::cout << "[DebugVisu] " << (fullscreen ? "Fullscreen" : "Windowed") << " mode" << std::endl;
}

bool DebugVisualizer::loadFont() {
    const char* fontPaths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        nullptr
    };

    for (int i = 0; fontPaths[i]; i++) {
        if (font.openFromFile(fontPaths[i])) {
            std::cout << "[DebugVisu] Font loaded: " << fontPaths[i] << std::endl;
            return true;
        }
    }
    return false;
}

void DebugVisualizer::refreshSnapshot() {
    auto& collector = SearchDataCollector::instance();
    if (collector.hasNewSnapshot()) {
        hoveredNode = nullptr;
        selectedNode = nullptr;
        lastClickedNode = nullptr;
        if (treeRenderer) {
            treeRenderer->setHoveredNode(nullptr);
            treeRenderer->setSelectedNode(nullptr);
        }

        cachedTree = collector.cloneSnapshot();
        cachedStats = collector.getStats();
        collector.markSnapshotRead();
        autoFitPending = true;

        // Auto-collapse all branches except the best decision path
        if (treeRenderer && cachedTree) {
            treeRenderer->autoCollapseForBestPath(cachedTree.get());
        }
    }
}

void DebugVisualizer::handleEvents() {
    if (!window.isOpen()) return;

    float treeAreaW = static_cast<float>(getWidth() - STATS_PANEL_W);

    while (auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            return;
        }

        // Keyboard
        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::F) {
                toggleFullscreen();
                return;
            }
            if (keyPressed->code == sf::Keyboard::Key::R) {
                resetTreeView();
            }
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                window.close();
                return;
            }
        }

        // Mouse move — hover detection + pan drag
        if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
            sf::Vector2i pixelPos = mouseMoved->position;

            // Pan dragging
            if (dragging) {
                sf::Vector2i delta = pixelPos - lastMousePos;
                treeView.move(sf::Vector2f(
                    static_cast<float>(-delta.x) * zoomLevel,
                    static_cast<float>(-delta.y) * zoomLevel));
                lastMousePos = pixelPos;
            }

            // Hover detection (only in tree area)
            if (treeRenderer && static_cast<float>(pixelPos.x) < treeAreaW) {
                sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, treeView);
                hoveredNode = treeRenderer->getNodeAtPosition(worldPos);
                treeRenderer->setHoveredNode(hoveredNode);
            } else {
                hoveredNode = nullptr;
                if (treeRenderer) treeRenderer->setHoveredNode(nullptr);
            }
        }

        // Mouse wheel — zoom
        if (const auto* scrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
            if (static_cast<float>(scrolled->position.x) < treeAreaW) {
                float factor = (scrolled->delta > 0) ? 0.85f : 1.15f;
                // Zoom towards mouse position
                sf::Vector2f beforeZoom = window.mapPixelToCoords(scrolled->position, treeView);
                treeView.zoom(factor);
                zoomLevel *= factor;
                sf::Vector2f afterZoom = window.mapPixelToCoords(scrolled->position, treeView);
                treeView.move(beforeZoom - afterZoom);
            }
        }

        // Mouse button pressed
        if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            // Middle click — start pan
            if (mousePressed->button == sf::Mouse::Button::Middle) {
                dragging = true;
                lastMousePos = mousePressed->position;
            }
            // Left click
            if (mousePressed->button == sf::Mouse::Button::Left) {
                if (hoveredNode) {
                    // Double-click detection for collapse toggle
                    if (lastClickedNode == hoveredNode &&
                        doubleClickClock.getElapsedTime().asMilliseconds() < 400) {
                        // Double click — toggle collapse
                        // Save anchor position before toggle to stabilize view
                        sf::FloatRect bounds;
                        if (treeRenderer->getNodeBounds(hoveredNode, bounds)) {
                            anchorNode = hoveredNode;
                            sf::Vector2f worldCenter(bounds.position.x + bounds.size.x / 2.0f,
                                                      bounds.position.y + bounds.size.y / 2.0f);
                            anchorScreenPos = sf::Vector2f(window.mapCoordsToPixel(worldCenter, treeView));
                        }
                        treeRenderer->toggleCollapse(hoveredNode);
                        lastClickedNode = nullptr;
                    } else {
                        // Single click — select
                        selectedNode = hoveredNode;
                        treeRenderer->setSelectedNode(selectedNode);
                        lastClickedNode = hoveredNode;
                        doubleClickClock.restart();
                    }
                } else if (static_cast<float>(mousePressed->position.x) < treeAreaW) {
                    // Left click on empty tree area — start pan too
                    dragging = true;
                    lastMousePos = mousePressed->position;
                }
            }
        }

        // Mouse button released
        if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseReleased->button == sf::Mouse::Button::Middle ||
                mouseReleased->button == sf::Mouse::Button::Left) {
                dragging = false;
            }
        }
    }
}

void DebugVisualizer::render() {
    if (!window.isOpen()) return;

    refreshSnapshot();

    window.clear(sf::Color(30, 30, 40));

    bool searching = SearchDataCollector::instance().isSearchInProgress();
    float treeAreaW = static_cast<float>(getWidth() - STATS_PANEL_W);
    float treeAreaH = static_cast<float>(getHeight());

    // --- Tree area with custom view (zoom/pan) ---
    // Viewport is already set on treeView (in resetTreeView)
    window.setView(treeView);

    if (treeRenderer && cachedTree && fontLoaded) {
        treeRenderer->render(window, font, cachedTree.get(), treeView.getSize().x, treeView.getSize().y);

        // Stabilize view after collapse/expand: keep anchor node at same screen position
        if (anchorNode) {
            sf::FloatRect bounds;
            if (treeRenderer->getNodeBounds(anchorNode, bounds)) {
                sf::Vector2f newWorldCenter(bounds.position.x + bounds.size.x / 2.0f,
                                             bounds.position.y + bounds.size.y / 2.0f);
                sf::Vector2f currentPixel = sf::Vector2f(window.mapCoordsToPixel(newWorldCenter, treeView));
                sf::Vector2f pixelDelta = anchorScreenPos - currentPixel;
                treeView.move(sf::Vector2f(-pixelDelta.x * zoomLevel, -pixelDelta.y * zoomLevel));
            }
            anchorNode = nullptr;
        }

        // Auto-fit zoom after new snapshot so the tree fits in view
        if (autoFitPending && treeRenderer->getLastTotalWidth() > 0) {
            float totalW = treeRenderer->getLastTotalWidth();
            float totalH = treeRenderer->getLastTotalHeight();
            float viewW = treeView.getSize().x;
            float viewH = treeView.getSize().y;
            float scaleX = totalW / viewW;
            float scaleY = totalH / viewH;
            float neededZoom = std::max(scaleX, scaleY);
            if (neededZoom > 1.0f) {
                // Tree is bigger than view: zoom out to fit
                treeView.setSize(sf::Vector2f(viewW * neededZoom, viewH * neededZoom));
                treeView.setCenter(sf::Vector2f(totalW / 2.0f, totalH / 2.0f));
                zoomLevel = neededZoom;
            } else {
                resetTreeView();
            }
            autoFitPending = false;
        }
    } else if (fontLoaded) {
        sf::Text placeholder(font,
            "En attente de la recherche IA...\n\n"
            "Appuyez sur 2 pour le mode Humain vs IA\n"
            "puis jouez un coup pour declencher l'IA", 16);
        placeholder.setFillColor(sf::Color(120, 120, 140));
        placeholder.setPosition(sf::Vector2f(treeAreaW / 2.0f - 180.0f, 200.0f));
        window.draw(placeholder);
    }

    // --- Switch to default view for UI elements (stats panel, title, separator) ---
    window.setView(window.getDefaultView());

    // Title bar
    if (fontLoaded) {
        sf::RectangleShape titleBg(sf::Vector2f(treeAreaW, 35.0f));
        titleBg.setPosition(sf::Vector2f(0.0f, 0.0f));
        titleBg.setFillColor(sf::Color(30, 30, 40, 220));
        window.draw(titleBg);

        sf::Text title(font, "MINIMAX TREE - AI Debug Visualizer", 16);
        title.setFillColor(sf::Color(200, 200, 220));
        title.setPosition(sf::Vector2f(15.0f, 8.0f));
        window.draw(title);

        // Zoom indicator
        std::string zoomStr = "Zoom: " + std::to_string(static_cast<int>(100.0f / zoomLevel)) + "%";
        sf::Text zoomText(font, zoomStr, 11);
        zoomText.setFillColor(sf::Color(140, 140, 160));
        zoomText.setPosition(sf::Vector2f(treeAreaW - 100.0f, 12.0f));
        window.draw(zoomText);
    }

    // Separator
    sf::RectangleShape separator(sf::Vector2f(2.0f, treeAreaH));
    separator.setFillColor(sf::Color(80, 80, 100));
    separator.setPosition(sf::Vector2f(treeAreaW, 0.0f));
    window.draw(separator);

    // --- Stats panel (fixed, default view) ---
    if (statsPanel && fontLoaded) {
        float panelX = treeAreaW + 10.0f;
        float panelY = 15.0f;
        float panelW = static_cast<float>(STATS_PANEL_W - 20);

        statsPanel->render(window, font, panelX, panelY, panelW, 280.0f, cachedStats, searching);

        // Node details
        const DebugTreeNode* detailNode = selectedNode;
        if (!detailNode && cachedTree) {
            for (auto& child : cachedTree->children) {
                if (child->isBestMove) {
                    detailNode = child.get();
                    break;
                }
            }
        }
        float legendY;
        if (detailNode) {
            float detailsHeight = statsPanel->renderNodeDetails(window, font, panelX, panelY + 300.0f, panelW, detailNode);
            legendY = panelY + 300.0f + detailsHeight + 15.0f;
        } else {
            legendY = panelY + 540.0f;
        }

        // Legend (only if it fits in window)
        if (legendY + 240.0f <= treeAreaH) {
            statsPanel->renderLegend(window, font, panelX, legendY, panelW);
        }
    }

    window.display();
}

bool DebugVisualizer::isOpen() const {
    return window.isOpen();
}

void DebugVisualizer::close() {
    if (window.isOpen()) {
        window.close();
    }
}

#endif // DEBUG_VISU
