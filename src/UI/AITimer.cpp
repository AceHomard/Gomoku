#include "UI/AITimer.hpp"

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