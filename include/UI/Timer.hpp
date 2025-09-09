/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Timer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TIMER_HPP
#define TIMER_HPP

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <chrono>
#include <vector>
#include <string>

class Timer {
protected:
    sf::Clock clock;
    sf::Font font;
    sf::Text timerText;
    sf::Text averageText;
    sf::Text statusText;
    
    // Timing data
    std::chrono::steady_clock::time_point moveStartTime;
    std::vector<double> moveHistory;
    bool isRunning;
    bool fontLoaded;
    
    // Display properties
    sf::Vector2f position;
    sf::Color textColor;
    unsigned int fontSize;
    
    // Statistics
    double currentMoveTime;
    double averageMoveTime;
    double totalTime;
    int moveCount;
    
public:
    Timer();
    ~Timer();
    
    // Timer control
    void startMoveTimer();
    void stopMoveTimer();
    void reset();
    void pause();
    void resume();
    
    // Font and display
    bool loadFont(const std::string& fontPath = "");
    void setPosition(float x, float y);
    void setTextColor(sf::Color color);
    void setFontSize(unsigned int size);
    
    // Time tracking
    double getCurrentMoveTime() const;
    double getAverageMoveTime() const;
    double getTotalTime() const;
    int getMoveCount() const;
    std::vector<double> getMoveHistory() const;
    
    // Status
    bool isTimerRunning() const { return isRunning; }
    std::string getFormattedCurrentTime() const;
    std::string getFormattedAverageTime() const;
    std::string getFormattedTotalTime() const;
    
    // Rendering
    void update();
    void draw(sf::RenderWindow& window);
    void drawCompact(sf::RenderWindow& window, float x, float y);
    void drawDetailed(sf::RenderWindow& window, float x, float y, float width, float height);
    
private:
    void updateTexts();
    std::string formatTime(double seconds) const;
    void calculateStatistics();
    sf::Color getTimerColor(double time) const;
    void setupDefaultFont();
};

// AI Timer wrapper for specific AI move timing
class AITimer : public Timer {
private:
    std::string aiName;
    int searchDepth;
    int nodesEvaluated;
    bool showDebugInfo;
    
public:
    AITimer(const std::string& name = "AI");
    ~AITimer();
    
    // AI-specific methods
    void setAIName(const std::string& name);
    void setSearchDepth(int depth);
    void setNodesEvaluated(int nodes);
    void setShowDebugInfo(bool show);
    
    // Enhanced AI display
    void drawAIStatus(sf::RenderWindow& window, float x, float y, float width, float height);
    std::string getAIStatusString() const;
    
    // Performance metrics
    double getNodesPerSecond() const;
    std::string getPerformanceString() const;
};

#endif // TIMER_HPP