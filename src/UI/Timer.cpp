/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Timer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "UI/Timer.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>

// Helper function for SFML 3 compatibility
static sf::Font& getDummyFont() {
    static sf::Font dummyFont;
    static bool initialized = false;
    if (!initialized) {
        (void)dummyFont.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
        initialized = true;
    }
    return dummyFont;
}

Timer::Timer() 
    : timerText(getDummyFont(), "", 20)
    , averageText(getDummyFont(), "", 16)
    , statusText(getDummyFont(), "", 14)
    , isRunning(false)
    , fontLoaded(false)
    , position(10.0f, 10.0f)
    , textColor(sf::Color::Black)
    , fontSize(16)
    , currentMoveTime(0.0)
    , averageMoveTime(0.0)
    , totalTime(0.0)
    , moveCount(0)
{
    setupDefaultFont();
    
    // Setup text objects will be done when font is loaded
    
    timerText.setCharacterSize(fontSize + 4);
    averageText.setCharacterSize(fontSize);
    statusText.setCharacterSize(fontSize - 2);
    
    timerText.setFillColor(textColor);
    averageText.setFillColor(sf::Color(100, 100, 100));
    statusText.setFillColor(sf::Color(150, 150, 150));
    
    updateTexts();
}

Timer::~Timer() {
}

void Timer::setupDefaultFont() {
    // Try to create a basic bitmap font or use system defaults
    fontLoaded = false;
    
    // On most systems, we can try some common font paths
    std::vector<std::string> fontPaths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/System/Library/Fonts/Arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "assets/fonts/arial.ttf"
    };
    
    for (const auto& path : fontPaths) {
        if (font.openFromFile(path)) {
            fontLoaded = true;
            break;
        }
    }
    
    // If no font loaded, the timer will still work but without text rendering
    if (!fontLoaded) {
        std::cerr << "Warning: Could not load any fonts. Timer will work but text won't be displayed." << std::endl;
    }
}

bool Timer::loadFont(const std::string& fontPath) {
    if (fontPath.empty()) {
        setupDefaultFont();
        return fontLoaded;
    }
    
    fontLoaded = font.openFromFile(fontPath);
    if (fontLoaded) {
        timerText = sf::Text(font, "", fontSize + 4);
        averageText = sf::Text(font, "", fontSize);
        statusText = sf::Text(font, "", fontSize - 2);
        updateTexts();
    }
    
    return fontLoaded;
}

void Timer::startMoveTimer() {
    moveStartTime = std::chrono::steady_clock::now();
    isRunning = true;
    clock.restart();
}

void Timer::stopMoveTimer() {
    if (isRunning) {
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double>(endTime - moveStartTime);
        currentMoveTime = duration.count();
        
        // Add to history
        moveHistory.push_back(currentMoveTime);
        moveCount++;
        
        // Calculate statistics
        calculateStatistics();
        
        isRunning = false;
        updateTexts();
    }
}

void Timer::reset() {
    isRunning = false;
    currentMoveTime = 0.0;
    averageMoveTime = 0.0;
    totalTime = 0.0;
    moveCount = 0;
    moveHistory.clear();
    
    updateTexts();
}

void Timer::pause() {
    if (isRunning) {
        // Calculate current elapsed time and store it
        auto currentTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double>(currentTime - moveStartTime);
        currentMoveTime = duration.count();
        isRunning = false;
    }
}

void Timer::resume() {
    if (!isRunning && currentMoveTime > 0.0) {
        // Restart timer with existing time offset
        moveStartTime = std::chrono::steady_clock::now() - std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(currentMoveTime));
        isRunning = true;
    }
}

void Timer::setPosition(float x, float y) {
    position.x = x;
    position.y = y;
}

void Timer::setTextColor(sf::Color color) {
    textColor = color;
    timerText.setFillColor(color);
    averageText.setFillColor(sf::Color(color.r * 0.7f, color.g * 0.7f, color.b * 0.7f));
    statusText.setFillColor(sf::Color(color.r * 0.5f, color.g * 0.5f, color.b * 0.5f));
}

void Timer::setFontSize(unsigned int size) {
    fontSize = size;
    timerText.setCharacterSize(fontSize + 4);
    averageText.setCharacterSize(fontSize);
    statusText.setCharacterSize(fontSize - 2);
}

double Timer::getCurrentMoveTime() const {
    if (isRunning) {
        auto currentTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double>(currentTime - moveStartTime);
        return duration.count();
    }
    return currentMoveTime;
}

double Timer::getAverageMoveTime() const {
    return averageMoveTime;
}

double Timer::getTotalTime() const {
    return totalTime;
}

int Timer::getMoveCount() const {
    return moveCount;
}

std::vector<double> Timer::getMoveHistory() const {
    return moveHistory;
}

std::string Timer::getFormattedCurrentTime() const {
    return formatTime(getCurrentMoveTime());
}

std::string Timer::getFormattedAverageTime() const {
    return formatTime(averageMoveTime);
}

std::string Timer::getFormattedTotalTime() const {
    return formatTime(totalTime);
}

void Timer::update() {
    if (isRunning) {
        currentMoveTime = getCurrentMoveTime();
        updateTexts();
    }
}

void Timer::updateTexts() {
    if (!fontLoaded) return;
    
    // Current move timer (most important - MANDATORY display)
    std::string currentTimeStr;
    if (isRunning) {
        currentTimeStr = "AI thinking: " + getFormattedCurrentTime();
        timerText.setFillColor(getTimerColor(getCurrentMoveTime()));
    } else if (currentMoveTime > 0.0) {
        currentTimeStr = "Last move: " + getFormattedCurrentTime();
        timerText.setFillColor(textColor);
    } else {
        currentTimeStr = "Ready";
        timerText.setFillColor(textColor);
    }
    
    timerText.setString(currentTimeStr);
    
    // Average time
    if (moveCount > 0) {
        averageText.setString("Average: " + getFormattedAverageTime() + " (" + std::to_string(moveCount) + " moves)");
    } else {
        averageText.setString("No moves yet");
    }
    
    // Status information
    if (totalTime > 0.0) {
        statusText.setString("Total: " + getFormattedTotalTime());
    } else {
        statusText.setString("");
    }
}

std::string Timer::formatTime(double seconds) const {
    std::stringstream ss;
    
    if (seconds < 1.0) {
        // Show milliseconds for sub-second times
        ss << std::fixed << std::setprecision(0) << (seconds * 1000) << "ms";
    } else if (seconds < 10.0) {
        // Show two decimal places for times under 10 seconds
        ss << std::fixed << std::setprecision(2) << seconds << "s";
    } else if (seconds < 60.0) {
        // Show one decimal place for times under 1 minute
        ss << std::fixed << std::setprecision(1) << seconds << "s";
    } else {
        // Show minutes:seconds for longer times
        int minutes = static_cast<int>(seconds) / 60;
        int secs = static_cast<int>(seconds) % 60;
        ss << minutes << ":" << std::setfill('0') << std::setw(2) << secs;
    }
    
    return ss.str();
}

void Timer::calculateStatistics() {
    if (moveHistory.empty()) {
        averageMoveTime = 0.0;
        totalTime = 0.0;
        return;
    }
    
    totalTime = std::accumulate(moveHistory.begin(), moveHistory.end(), 0.0);
    averageMoveTime = totalTime / moveHistory.size();
}

sf::Color Timer::getTimerColor(double time) const {
    // Color coding: Green (fast) -> Yellow (medium) -> Red (slow)
    if (time < 0.5) {
        return sf::Color(0, 200, 0);  // Green - very fast
    } else if (time < 1.0) {
        return sf::Color(100, 200, 0);  // Yellow-green - fast
    } else if (time < 3.0) {
        return sf::Color(200, 200, 0);  // Yellow - medium
    } else if (time < 10.0) {
        return sf::Color(200, 100, 0);  // Orange - slow
    } else {
        return sf::Color(200, 0, 0);  // Red - very slow
    }
}

void Timer::draw(sf::RenderWindow& window) {
    drawDetailed(window, position.x, position.y, 200, 80);
}

void Timer::drawCompact(sf::RenderWindow& window, float x, float y) {
    if (!fontLoaded) {
        // Draw a simple colored rectangle to indicate timer status
        sf::RectangleShape indicator(sf::Vector2f(100, 20));
        indicator.setPosition(sf::Vector2f(x, y));
        if (isRunning) {
            indicator.setFillColor(sf::Color(200, 200, 0, 100));
        } else {
            indicator.setFillColor(sf::Color(100, 100, 100, 100));
        }
        window.draw(indicator);
        return;
    }
    
    // Just show current timer
    timerText.setPosition(sf::Vector2f(x, y));
    window.draw(timerText);
}

void Timer::drawDetailed(sf::RenderWindow& window, float x, float y, float width, float height) {
    // Draw background panel
    sf::RectangleShape background(sf::Vector2f(width, height));
    background.setPosition(sf::Vector2f(x - 5, y - 5));
    background.setFillColor(sf::Color(250, 250, 250, 200));
    background.setOutlineColor(sf::Color(100, 100, 100));
    background.setOutlineThickness(1);
    window.draw(background);
    
    if (!fontLoaded) {
        // Fallback: just draw status rectangles
        sf::RectangleShape statusRect(sf::Vector2f(width - 10, 15));
        statusRect.setPosition(sf::Vector2f(x, y));
        if (isRunning) {
            statusRect.setFillColor(getTimerColor(getCurrentMoveTime()));
        } else {
            statusRect.setFillColor(sf::Color(100, 100, 100));
        }
        window.draw(statusRect);
        return;
    }
    
    // Draw text elements
    float yOffset = y;
    
    timerText.setPosition(sf::Vector2f(x, yOffset));
    window.draw(timerText);
    yOffset += fontSize + 8;
    
    if (moveCount > 0) {
        averageText.setPosition(sf::Vector2f(x, yOffset));
        window.draw(averageText);
        yOffset += fontSize + 4;
        
        if (!statusText.getString().isEmpty()) {
            statusText.setPosition(sf::Vector2f(x, yOffset));
            window.draw(statusText);
        }
    }
}

// AITimer implementation
AITimer::AITimer(const std::string& name)
    : Timer()
    , aiName(name)
    , searchDepth(0)
    , nodesEvaluated(0)
    , showDebugInfo(false)
{
}

AITimer::~AITimer() {
}

void AITimer::setAIName(const std::string& name) {
    aiName = name;
}

void AITimer::setSearchDepth(int depth) {
    searchDepth = depth;
}

void AITimer::setNodesEvaluated(int nodes) {
    nodesEvaluated = nodes;
}

void AITimer::setShowDebugInfo(bool show) {
    showDebugInfo = show;
}

void AITimer::drawAIStatus(sf::RenderWindow& window, float x, float y, float width, float height) {
    // Draw enhanced background
    sf::RectangleShape background(sf::Vector2f(width, height));
    background.setPosition(sf::Vector2f(x - 5, y - 5));
    background.setFillColor(sf::Color(240, 245, 255, 220));
    background.setOutlineColor(sf::Color(100, 150, 200));
    background.setOutlineThickness(2);
    window.draw(background);
    
    if (!fontLoaded) {
        drawDetailed(window, x, y, width, height);
        return;
    }
    
    float yOffset = y;
    const float lineHeight = fontSize + 4;
    
    // AI name and current status
    sf::Text aiNameText(font, aiName + " Status", fontSize + 2);
    aiNameText.setFillColor(sf::Color(50, 50, 150));
    aiNameText.setPosition(sf::Vector2f(x, yOffset));
    window.draw(aiNameText);
    yOffset += lineHeight + 4;
    
    // Current timer (MANDATORY - most prominent)
    timerText.setPosition(sf::Vector2f(x, yOffset));
    window.draw(timerText);
    yOffset += lineHeight + 2;
    
    if (showDebugInfo && isRunning) {
        // Search depth
        if (searchDepth > 0) {
            sf::Text depthText(font, "Search depth: " + std::to_string(searchDepth), fontSize - 2);
            depthText.setFillColor(sf::Color(100, 100, 100));
            depthText.setPosition(sf::Vector2f(x + 10, yOffset));
            window.draw(depthText);
            yOffset += lineHeight - 2;
        }
        
        // Nodes evaluated
        if (nodesEvaluated > 0) {
            sf::Text nodesText(font, "Nodes: " + std::to_string(nodesEvaluated) + " (" + getPerformanceString() + ")", fontSize - 2);
            nodesText.setFillColor(sf::Color(100, 100, 100));
            nodesText.setPosition(sf::Vector2f(x + 10, yOffset));
            window.draw(nodesText);
            yOffset += lineHeight - 2;
        }
    }
    
    // Average and statistics
    if (moveCount > 0) {
        averageText.setPosition(sf::Vector2f(x, yOffset));
        window.draw(averageText);
        yOffset += lineHeight;
        
        if (!statusText.getString().isEmpty()) {
            statusText.setPosition(sf::Vector2f(x, yOffset));
            window.draw(statusText);
        }
    }
}

std::string AITimer::getAIStatusString() const {
    std::string status = aiName + ": ";
    if (isTimerRunning()) {
        status += "thinking (" + getFormattedCurrentTime() + ")";
    } else if (getCurrentMoveTime() > 0.0) {
        status += "completed in " + getFormattedCurrentTime();
    } else {
        status += "ready";
    }
    
    return status;
}

double AITimer::getNodesPerSecond() const {
    double currentTime = getCurrentMoveTime();
    if (currentTime > 0.0 && nodesEvaluated > 0) {
        return nodesEvaluated / currentTime;
    }
    return 0.0;
}

std::string AITimer::getPerformanceString() const {
    double nps = getNodesPerSecond();
    if (nps < 1000) {
        return std::to_string(static_cast<int>(nps)) + " nps";
    } else if (nps < 1000000) {
        return std::to_string(static_cast<int>(nps / 1000)) + "K nps";
    } else {
        return std::to_string(static_cast<int>(nps / 1000000)) + "M nps";
    }
}