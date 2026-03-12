#include "UI/Timer.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>

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
    fontLoaded = false;
    
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
        
        moveHistory.push_back(currentMoveTime);
        moveCount++;
        
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
    
    if (moveCount > 0) {
        averageText.setString("Average: " + getFormattedAverageTime() + " (" + std::to_string(moveCount) + " moves)");
    } else {
        averageText.setString("No moves yet");
    }
    
    if (totalTime > 0.0) {
        statusText.setString("Total: " + getFormattedTotalTime());
    } else {
        statusText.setString("");
    }
}

std::string Timer::formatTime(double seconds) const {
    std::stringstream ss;
    
    if (seconds < 1.0) {
        ss << std::fixed << std::setprecision(0) << (seconds * 1000) << "ms";
    } else if (seconds < 10.0) {
        ss << std::fixed << std::setprecision(2) << seconds << "s";
    } else if (seconds < 60.0) {
        ss << std::fixed << std::setprecision(1) << seconds << "s";
    } else {
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
    
    timerText.setPosition(sf::Vector2f(x, y));
    window.draw(timerText);
}

void Timer::drawDetailed(sf::RenderWindow& window, float x, float y, float width, float height) {
    sf::RectangleShape background(sf::Vector2f(width, height));
    background.setPosition(sf::Vector2f(x - 5, y - 5));
    background.setFillColor(sf::Color(250, 250, 250, 200));
    background.setOutlineColor(sf::Color(100, 100, 100));
    background.setOutlineThickness(1);
    window.draw(background);
    
    if (!fontLoaded) {
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

