#ifndef AITIMER_HPP
#define AITIMER_HPP

#include "Timer.hpp"
#include <string>

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
    
    // Enhanced AI display
    void drawAIStatus(sf::RenderWindow& window, float x, float y, float width, float height);
    
    // Performance metrics
    double getNodesPerSecond() const;
    std::string getPerformanceString() const;
};

#endif // AITIMER_HPP