/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DebugUI.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "UI/DebugUI.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

// Helper function to create default font for SFML 3 compatibility
static sf::Font& getDummyFont() {
    static sf::Font dummyFont;
    static bool initialized = false;
    if (!initialized) {
        // Try to load any system font, but don't fail if none available
        (void)dummyFont.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
        initialized = true;
    }
    return dummyFont;
}

DebugUI::DebugUI()
    : fontLoaded(false)
    , visible(false)
    , enabled(true)
    , position(650.0f, 50.0f)
    , size(350.0f, 500.0f)
    , backgroundColor(sf::Color(30, 30, 40, 230))
    , textColor(sf::Color::White)
    , highlightColor(sf::Color::Yellow)
    , fontSize(12)
    , headerText(getDummyFont(), "", 14)
    , debugInfoText(getDummyFont(), "", 12)
    , principalVariationText(getDummyFont(), "", 11)
    , performanceText(getDummyFont(), "", 11)
    , heuristicText(getDummyFont(), "", 11)
    , maxLogMessages(20)
{
    setupPanels();
    setupTexts();
    
    // Try to load a default font
    loadFont();
}

DebugUI::~DebugUI() {
}

bool DebugUI::loadFont(const std::string& fontPath) {
    std::vector<std::string> fontPaths;
    
    if (!fontPath.empty()) {
        fontPaths.push_back(fontPath);
    }
    
    // Add default font paths
    fontPaths.insert(fontPaths.end(), {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/System/Library/Fonts/Menlo.ttc",
        "C:/Windows/Fonts/consola.ttf",
        "assets/fonts/mono.ttf"
    });
    
    for (const auto& path : fontPaths) {
        if (font.openFromFile(path)) {
            fontLoaded = true;
            setupTexts();
            return true;
        }
    }
    
    fontLoaded = false;
    return false;
}

void DebugUI::setPosition(float x, float y) {
    position.x = x;
    position.y = y;
    setupPanels();
}

void DebugUI::setSize(float width, float height) {
    size.x = width;
    size.y = height;
    setupPanels();
}

void DebugUI::show() {
    visible = true;
}

void DebugUI::hide() {
    visible = false;
}

void DebugUI::toggle() {
    visible = !visible;
}

void DebugUI::setupPanels() {
    // Main background panel
    backgroundPanel.setSize(size);
    backgroundPanel.setPosition(position);
    backgroundPanel.setFillColor(backgroundColor);
    backgroundPanel.setOutlineColor(sf::Color(100, 100, 120));
    backgroundPanel.setOutlineThickness(2);
    
    // Header panel
    headerPanel.setSize(sf::Vector2f(size.x, 30));
    headerPanel.setPosition(position);
    headerPanel.setFillColor(sf::Color(50, 50, 70, 200));
    headerPanel.setOutlineColor(sf::Color(150, 150, 180));
    headerPanel.setOutlineThickness(1);
}

void DebugUI::setupTexts() {
    if (!fontLoaded) return;
    
    // Header text
    headerText = sf::Text(font, "AI Debug Panel [F1 to toggle]", fontSize + 2);
    headerText.setFillColor(sf::Color::White);
    headerText.setPosition(sf::Vector2f(position.x + 10, position.y + 5));
    
    // Main debug info
    debugInfoText = sf::Text(font, "", fontSize);
    debugInfoText.setFillColor(textColor);
    debugInfoText.setPosition(sf::Vector2f(position.x + 10, position.y + 40));
    
    // Principal variation
    principalVariationText = sf::Text(font, "", fontSize - 1);
    principalVariationText.setFillColor(sf::Color(200, 255, 200));
    
    // Performance metrics
    performanceText = sf::Text(font, "", fontSize - 1);
    performanceText.setFillColor(sf::Color(255, 200, 200));
    
    // Heuristic information
    heuristicText = sf::Text(font, "", fontSize - 1);
    heuristicText.setFillColor(sf::Color(200, 200, 255));
    
    updateTexts();
}

void DebugUI::updateDebugInfo(const DebugInfo& info) {
    currentDebugInfo = info;
    updateTexts();
}

void DebugUI::updateFromAIPlayer(const MinMaxAI* aiPlayer) {
    if (!aiPlayer) return;
    
    currentDebugInfo.searchDepth = aiPlayer->getSearchDepth();
    currentDebugInfo.nodesEvaluated = aiPlayer->getNodesEvaluated();
    currentDebugInfo.ttHitRate = aiPlayer->getTTHitRate();
    currentDebugInfo.principalVariation = aiPlayer->getPrincipalVariation();
    
    updateTexts();
}

void DebugUI::setPrincipalVariation(const std::vector<Position>& pv) {
    currentDebugInfo.principalVariation = pv;
    updatePrincipalVariationDisplay();
}

void DebugUI::setMoveScores(const std::vector<std::pair<Position, int>>& scores) {
    currentDebugInfo.moveScores = scores;
    updateTexts();
}

void DebugUI::setSearchStatus(const std::string& status, bool searching) {
    currentDebugInfo.searchStatus = status;
    currentDebugInfo.isSearching = searching;
    updateTexts();
}

void DebugUI::setPerformanceMetrics(int nodes, int ttHits, double time) {
    currentDebugInfo.nodesEvaluated = nodes;
    currentDebugInfo.ttHitRate = ttHits;
    (void)time;  // Suppress unused parameter warning
    updatePerformanceInfo();
}

void DebugUI::addLogMessage(const std::string& message) {
    logMessages.push_back(message);
    
    // Maintain maximum log size
    while (logMessages.size() > maxLogMessages) {
        logMessages.erase(logMessages.begin());
    }
    
    updateLogDisplay();
}

void DebugUI::clearLog() {
    logMessages.clear();
    updateLogDisplay();
}

void DebugUI::highlightPosition(const Position& pos, sf::Color color) {
    // This will be used in board overlay rendering
    // Implementation depends on board rendering context
    (void)pos;  // Suppress unused parameter warning
    (void)color;
}

void DebugUI::highlightMoves(const std::vector<Position>& moves, sf::Color color) {
    // Store highlights for overlay rendering
    moveHighlights.clear();
    for (size_t i = 0; i < moves.size(); ++i) {
        sf::CircleShape highlight;
        highlight.setRadius(8);
        highlight.setFillColor(sf::Color::Transparent);
        highlight.setOutlineColor(color);
        highlight.setOutlineThickness(2);
        moveHighlights.push_back(highlight);
    }
}

void DebugUI::showMoveNumbers(const std::vector<Position>& moves) {
    moveNumbers.clear();
    if (!fontLoaded) return;
    
    for (size_t i = 0; i < moves.size(); ++i) {
        sf::Text numberText(font, std::to_string(i + 1), 10);
        numberText.setFillColor(sf::Color::White);
        moveNumbers.push_back(numberText);
    }
}

void DebugUI::showMoveScores(const std::vector<std::pair<Position, int>>& moveScores) {
    scoreTexts.clear();
    if (!fontLoaded) return;
    
    for (const auto& moveScore : moveScores) {
        sf::Text scoreText(font, formatScore(moveScore.second), 9);
        scoreText.setFillColor(getScoreColor(moveScore.second));
        scoreTexts.push_back(scoreText);
    }
}

void DebugUI::clearHighlights() {
    moveHighlights.clear();
    moveNumbers.clear();
    scoreTexts.clear();
}

void DebugUI::updateTexts() {
    if (!fontLoaded) return;
    
    updateMainDebugInfo();
    updatePerformanceInfo();
    updatePrincipalVariationDisplay();
}

void DebugUI::updateMainDebugInfo() {
    std::ostringstream oss;
    
    // Search status
    if (currentDebugInfo.isSearching) {
        oss << "Status: " << currentDebugInfo.searchStatus << "\n";
    } else {
        oss << "Status: Ready\n";
    }
    
    // Search parameters
    if (currentDebugInfo.searchDepth > 0) {
        oss << "Search Depth: " << currentDebugInfo.searchDepth << "\n";
    }
    
    // Evaluation score
    if (currentDebugInfo.evaluationScore != 0.0) {
        oss << "Position Score: " << std::fixed << std::setprecision(2) 
            << currentDebugInfo.evaluationScore << "\n";
    }
    
    // Move scores summary
    if (!currentDebugInfo.moveScores.empty()) {
        oss << "Moves Analyzed: " << currentDebugInfo.moveScores.size() << "\n";
        
        // Show top 3 moves
        auto sortedMoves = currentDebugInfo.moveScores;
        std::sort(sortedMoves.begin(), sortedMoves.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        oss << "Top Moves:\n";
        for (size_t i = 0; i < std::min(static_cast<size_t>(3), sortedMoves.size()); ++i) {
            oss << "  " << formatPosition(sortedMoves[i].first) 
                << ": " << formatScore(sortedMoves[i].second) << "\n";
        }
    }
    
    debugInfoText.setString(oss.str());
}

void DebugUI::updatePerformanceInfo() {
    std::ostringstream oss;
    
    if (currentDebugInfo.nodesEvaluated > 0) {
        oss << "Performance:\n";
        oss << "  Nodes: " << currentDebugInfo.nodesEvaluated << "\n";
        oss << "  TT Hit Rate: " << currentDebugInfo.ttHitRate << "%\n";
    }
    
    performanceText.setString(oss.str());
    
    // Position performance text
    float yPos = position.y + 200;
    performanceText.setPosition(sf::Vector2f(position.x + 10, yPos));
}

void DebugUI::updatePrincipalVariationDisplay() {
    std::ostringstream oss;
    
    if (!currentDebugInfo.principalVariation.empty()) {
        oss << "Principal Variation:\n";
        oss << "  " << formatMoveSequence(currentDebugInfo.principalVariation) << "\n";
    } else {
        oss << "Principal Variation: None\n";
    }
    
    principalVariationText.setString(oss.str());
    
    // Position PV text
    float yPos = position.y + 140;
    principalVariationText.setPosition(sf::Vector2f(position.x + 10, yPos));
}

void DebugUI::updateLogDisplay() {
    // Update log display - this could be a scrollable area
}

void DebugUI::draw(sf::RenderWindow& window) {
    if (!visible || !enabled) return;
    
    drawMainPanel(window);
}

void DebugUI::drawOverlay(sf::RenderWindow& window, const sf::Vector2f& boardOffset, float cellSize) {
    if (!visible || !enabled) return;
    
    // Draw move highlights on the board
    for (size_t i = 0; i < moveHighlights.size() && i < currentDebugInfo.moveScores.size(); ++i) {
        Position pos = currentDebugInfo.moveScores[i].first;
        sf::Vector2f screenPos(boardOffset.x + pos.x * cellSize + cellSize/2,
                              boardOffset.y + pos.y * cellSize + cellSize/2);
        
        moveHighlights[i].setPosition(sf::Vector2f(screenPos.x - moveHighlights[i].getRadius(),
                                                  screenPos.y - moveHighlights[i].getRadius()));
        window.draw(moveHighlights[i]);
    }
    
    // Draw move numbers
    for (size_t i = 0; i < moveNumbers.size() && i < currentDebugInfo.principalVariation.size(); ++i) {
        Position pos = currentDebugInfo.principalVariation[i];
        sf::Vector2f screenPos(boardOffset.x + pos.x * cellSize + cellSize/2,
                              boardOffset.y + pos.y * cellSize + cellSize/2);
        
        moveNumbers[i].setPosition(sf::Vector2f(screenPos.x - 5, screenPos.y - 8));
        window.draw(moveNumbers[i]);
    }
    
    // Draw score texts
    for (size_t i = 0; i < scoreTexts.size() && i < currentDebugInfo.moveScores.size(); ++i) {
        Position pos = currentDebugInfo.moveScores[i].first;
        sf::Vector2f screenPos(boardOffset.x + pos.x * cellSize + cellSize + 2,
                              boardOffset.y + pos.y * cellSize + cellSize/2);
        
        scoreTexts[i].setPosition(sf::Vector2f(screenPos.x, screenPos.y - 6));
        window.draw(scoreTexts[i]);
    }
}

void DebugUI::drawMainPanel(sf::RenderWindow& window) {
    // Draw background
    window.draw(backgroundPanel);
    window.draw(headerPanel);
    
    if (!fontLoaded) {
        // Fallback: draw simple status indicators
        sf::RectangleShape statusIndicator(sf::Vector2f(size.x - 20, 20));
        statusIndicator.setPosition(sf::Vector2f(position.x + 10, position.y + 40));
        if (currentDebugInfo.isSearching) {
            statusIndicator.setFillColor(sf::Color(200, 200, 0, 100));
        } else {
            statusIndicator.setFillColor(sf::Color(100, 100, 100, 100));
        }
        window.draw(statusIndicator);
        return;
    }
    
    // Draw text elements
    window.draw(headerText);
    window.draw(debugInfoText);
    window.draw(principalVariationText);
    window.draw(performanceText);
    window.draw(heuristicText);
}

void DebugUI::drawLogPanel(sf::RenderWindow& window) {
    // Draw log messages at the bottom of the panel
    if (!fontLoaded || logMessages.empty()) return;
    
    float yPos = position.y + size.y - 100;
    float lineHeight = fontSize + 2;
    
    for (size_t i = logMessages.size(); i > 0 && yPos > position.y + 250; --i) {
        sf::Text logText(font, logMessages[i-1], fontSize - 2);
        logText.setFillColor(sf::Color(180, 180, 180));
        logText.setPosition(sf::Vector2f(position.x + 10, yPos));
        
        window.draw(logText);
        yPos -= lineHeight;
    }
}

bool DebugUI::handleMouseClick(int mouseX, int mouseY) {
    if (!visible) return false;
    
    // Check if click is within debug panel
    sf::FloatRect bounds = backgroundPanel.getGlobalBounds();
    return bounds.contains(sf::Vector2f(static_cast<float>(mouseX), static_cast<float>(mouseY)));
}

void DebugUI::handleKeyPress(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Key::F1:
            toggle();
            break;
        case sf::Keyboard::Key::F2:
            clearLog();
            break;
        case sf::Keyboard::Key::F3:
            clearHighlights();
            break;
        default:
            break;
    }
}

// Helper methods
std::string DebugUI::formatMoveSequence(const std::vector<Position>& moves) const {
    std::ostringstream oss;
    for (size_t i = 0; i < moves.size(); ++i) {
        if (i > 0) oss << " ";
        oss << formatPosition(moves[i]);
        if (i >= 10) {  // Limit display length
            oss << "...";
            break;
        }
    }
    return oss.str();
}

std::string DebugUI::formatPosition(const Position& pos) const {
    char col = 'A' + pos.x;
    int row = pos.y + 1;
    return std::string(1, col) + std::to_string(row);
}

std::string DebugUI::formatScore(int score) const {
    if (score > 10000) return "+WIN";
    if (score < -10000) return "-WIN";
    if (score >= 0) return "+" + std::to_string(score);
    return std::to_string(score);
}

sf::Color DebugUI::getScoreColor(int score) const {
    if (score > 1000) return sf::Color::Green;
    if (score > 100) return sf::Color(150, 255, 150);
    if (score > 0) return sf::Color(200, 255, 200);
    if (score == 0) return sf::Color::White;
    if (score > -100) return sf::Color(255, 200, 200);
    if (score > -1000) return sf::Color(255, 150, 150);
    return sf::Color::Red;
}

sf::Color DebugUI::getMoveColor(int moveIndex, int totalMoves) const {
    if (totalMoves == 0) return sf::Color(128, 128, 0, 150);
    
    float ratio = static_cast<float>(moveIndex) / totalMoves;
    unsigned char red = static_cast<unsigned char>(255 * (1.0f - ratio));
    unsigned char green = static_cast<unsigned char>(255 * ratio);
    return sf::Color(red, green, 0, 150);
}

float DebugUI::getLineHeight() const {
    return fontSize + 4;
}

sf::Vector2f DebugUI::getNextLinePosition(const sf::Vector2f& currentPos) const {
    return sf::Vector2f(currentPos.x, currentPos.y + getLineHeight());
}

// BoardDebugOverlay implementation
BoardDebugOverlay::BoardDebugOverlay() : fontLoaded(false) {
}

BoardDebugOverlay::~BoardDebugOverlay() {
}

bool BoardDebugOverlay::loadFont(const sf::Font& debugFont) {
    font = debugFont;
    fontLoaded = true;
    return true;
}

void BoardDebugOverlay::addHighlight(const Position& pos, sf::Color color, 
                                    const sf::Vector2f& boardOffset, float cellSize) {
    sf::CircleShape highlight;
    highlight.setRadius(cellSize * 0.4f);
    highlight.setFillColor(sf::Color::Transparent);
    highlight.setOutlineColor(color);
    highlight.setOutlineThickness(2);
    
    sf::Vector2f screenPos(boardOffset.x + pos.x * cellSize + cellSize/2,
                          boardOffset.y + pos.y * cellSize + cellSize/2);
    highlight.setPosition(sf::Vector2f(screenPos.x - highlight.getRadius(),
                                      screenPos.y - highlight.getRadius()));
    
    highlights.push_back(highlight);
}

void BoardDebugOverlay::addLabel(const Position& pos, const std::string& text, sf::Color color,
                                const sf::Vector2f& boardOffset, float cellSize) {
    if (!fontLoaded) return;
    
    if (!fontLoaded) return;
    
    sf::Text label(font, text, 10);
    label.setFillColor(color);
    
    sf::Vector2f screenPos(boardOffset.x + pos.x * cellSize + cellSize + 2,
                          boardOffset.y + pos.y * cellSize + cellSize/2 - 5);
    label.setPosition(screenPos);
    
    labels.push_back(label);
}

void BoardDebugOverlay::clear() {
    highlights.clear();
    labels.clear();
}

void BoardDebugOverlay::draw(sf::RenderWindow& window) {
    for (const auto& highlight : highlights) {
        window.draw(highlight);
    }
    
    for (const auto& label : labels) {
        window.draw(label);
    }
}

void BoardDebugOverlay::visualizePrincipalVariation(const std::vector<Position>& pv,
                                                   const sf::Vector2f& boardOffset, float cellSize) {
    clear();
    
    for (size_t i = 0; i < pv.size(); ++i) {
        sf::Color color = sf::Color(255 - i * 20, 255 - i * 20, 0, 200);
        addHighlight(pv[i], color, boardOffset, cellSize);
        addLabel(pv[i], std::to_string(i + 1), sf::Color::White, boardOffset, cellSize);
    }
}

void BoardDebugOverlay::visualizeMoveScores(const std::vector<std::pair<Position, int>>& moveScores,
                                          const sf::Vector2f& boardOffset, float cellSize) {
    clear();
    
    for (const auto& moveScore : moveScores) {
        sf::Color color;
        if (moveScore.second > 0) {
            color = sf::Color(0, 255, 0, 150);
        } else if (moveScore.second < 0) {
            color = sf::Color(255, 0, 0, 150);
        } else {
            color = sf::Color(128, 128, 128, 150);
        }
        
        addHighlight(moveScore.first, color, boardOffset, cellSize);
        addLabel(moveScore.first, std::to_string(moveScore.second), color, boardOffset, cellSize);
    }
}

void BoardDebugOverlay::visualizeThreats(const std::vector<Position>& threats,
                                        const sf::Vector2f& boardOffset, float cellSize) {
    clear();
    
    for (const auto& threat : threats) {
        addHighlight(threat, sf::Color::Red, boardOffset, cellSize);
        addLabel(threat, "!", sf::Color::Red, boardOffset, cellSize);
    }
}