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
    
    // Font and display
    bool loadFont(const std::string& fontPath = "");
    
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


#endif // TIMER_HPP