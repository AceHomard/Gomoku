/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DebugUI.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DEBUGUI_HPP
#define DEBUGUI_HPP

#include <SFML/Graphics.hpp>
#include "../Game/Board.hpp"
#include "../Game/Player.hpp"
#include "../AI/MinMaxAI.hpp"
#include "Timer.hpp"
#include <vector>
#include <string>
#include <map>

// Debug information structure for AI analysis
struct DebugInfo {
    std::vector<Position> principalVariation;
    std::vector<Position> bestMoves;
    std::vector<std::pair<Position, int>> moveScores;
    int searchDepth;
    int nodesEvaluated;
    int ttHitRate;
    double evaluationScore;
    std::string searchStatus;
    bool isSearching;
    
    DebugInfo() : searchDepth(0), nodesEvaluated(0), ttHitRate(0), 
                  evaluationScore(0.0), isSearching(false) {}
};

class DebugUI {
private:
    sf::Font font;
    bool fontLoaded;
    bool visible;
    bool enabled;
    
    // Display properties
    sf::Vector2f position;
    sf::Vector2f size;
    sf::Color backgroundColor;
    sf::Color textColor;
    sf::Color highlightColor;
    unsigned int fontSize;
    
    // Debug panels
    sf::RectangleShape backgroundPanel;
    sf::RectangleShape headerPanel;
    
    // Text objects
    sf::Text headerText;
    sf::Text debugInfoText;
    sf::Text principalVariationText;
    sf::Text performanceText;
    sf::Text heuristicText;
    
    // Debug data
    DebugInfo currentDebugInfo;
    std::vector<std::string> logMessages;
    size_t maxLogMessages;
    
    // Board visualization overlay
    std::vector<sf::CircleShape> moveHighlights;
    std::vector<sf::Text> moveNumbers;
    std::vector<sf::Text> scoreTexts;
    
public:
    DebugUI();
    ~DebugUI();
    
    // Initialization
    bool loadFont(const std::string& fontPath = "");
    void setPosition(float x, float y);
    void setSize(float width, float height);
    
    // Visibility control
    void show();
    void hide();
    void toggle();
    bool isVisible() const { return visible; }
    void setEnabled(bool enable) { enabled = enable; }
    bool isEnabled() const { return enabled; }
    
    // Debug data updates
    void updateDebugInfo(const DebugInfo& info);
    void setAIPlayer(const MinMaxAI* aiPlayer);
    void updateFromAIPlayer(const MinMaxAI* aiPlayer);
    
    // Principal variation display
    void setPrincipalVariation(const std::vector<Position>& pv);
    void setMoveScores(const std::vector<std::pair<Position, int>>& scores);
    void setSearchStatus(const std::string& status, bool searching = false);
    void setPerformanceMetrics(int nodes, int ttHits, double time);
    
    // Logging system
    void addLogMessage(const std::string& message);
    void clearLog();
    
    // Board overlay
    void highlightPosition(const Position& pos, sf::Color color = sf::Color::Yellow);
    void highlightMoves(const std::vector<Position>& moves, sf::Color color = sf::Color::Blue);
    void showMoveNumbers(const std::vector<Position>& moves);
    void showMoveScores(const std::vector<std::pair<Position, int>>& moveScores);
    void clearHighlights();
    
    // Rendering
    void draw(sf::RenderWindow& window);
    void drawOverlay(sf::RenderWindow& window, const sf::Vector2f& boardOffset, float cellSize);
    void drawMainPanel(sf::RenderWindow& window);
    void drawLogPanel(sf::RenderWindow& window);
    
    // Event handling
    bool handleMouseClick(int mouseX, int mouseY);
    void handleKeyPress(sf::Keyboard::Key key);
    
private:
    void setupPanels();
    void setupTexts();
    void updateTexts();
    void updateMainDebugInfo();
    void updatePerformanceInfo();
    void updatePrincipalVariationDisplay();
    void updateLogDisplay();
    
    // Text formatting helpers
    std::string formatMoveSequence(const std::vector<Position>& moves) const;
    std::string formatPosition(const Position& pos) const;
    std::string formatScore(int score) const;
    std::string formatPerformance(int nodes, double time) const;
    
    // Color helpers
    sf::Color getScoreColor(int score) const;
    sf::Color getMoveColor(int moveIndex, int totalMoves) const;
    
    // Layout helpers
    float getLineHeight() const;
    sf::Vector2f getNextLinePosition(const sf::Vector2f& currentPos) const;
};

// Debug overlay for board visualization
class BoardDebugOverlay {
private:
    std::vector<sf::CircleShape> highlights;
    std::vector<sf::Text> labels;
    sf::Font font;
    bool fontLoaded;
    
public:
    BoardDebugOverlay();
    ~BoardDebugOverlay();
    
    bool loadFont(const sf::Font& debugFont);
    
    // Highlight methods
    void addHighlight(const Position& pos, sf::Color color, const sf::Vector2f& boardOffset, float cellSize);
    void addLabel(const Position& pos, const std::string& text, sf::Color color, 
                  const sf::Vector2f& boardOffset, float cellSize);
    void clear();
    
    // Drawing
    void draw(sf::RenderWindow& window);
    
    // Quick visualization methods
    void visualizePrincipalVariation(const std::vector<Position>& pv, 
                                   const sf::Vector2f& boardOffset, float cellSize);
    void visualizeMoveScores(const std::vector<std::pair<Position, int>>& moveScores,
                            const sf::Vector2f& boardOffset, float cellSize);
    void visualizeThreats(const std::vector<Position>& threats,
                         const sf::Vector2f& boardOffset, float cellSize);
};

// Quick debug macros for development
#ifdef DEBUG_BUILD
    #define DEBUG_LOG(debugUI, message) do { if (debugUI) debugUI->addLogMessage(message); } while(0)
    #define DEBUG_HIGHLIGHT(debugUI, pos, color) do { if (debugUI) debugUI->highlightPosition(pos, color); } while(0)
    #define DEBUG_UPDATE_AI(debugUI, aiPlayer) do { if (debugUI) debugUI->updateFromAIPlayer(aiPlayer); } while(0)
#else
    #define DEBUG_LOG(debugUI, message) do { } while(0)
    #define DEBUG_HIGHLIGHT(debugUI, pos, color) do { } while(0)
    #define DEBUG_UPDATE_AI(debugUI, aiPlayer) do { } while(0)
#endif

#endif // DEBUGUI_HPP