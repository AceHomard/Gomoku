/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GameRenderer.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GAMERENDERER_HPP
#define GAMERENDERER_HPP

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "../Game/Board.hpp"
#include "Timer.hpp"
#include "DebugUI.hpp"
#include <vector>
#include <memory>

// Forward declarations to avoid circular includes
class Game;
class Player;

// Animation system for smooth stone placement and captures
class StoneAnimation {
public:
    enum Type {
        PLACE,      // Stone placement animation
        CAPTURE,    // Stone capture animation
        HIGHLIGHT,  // Move highlighting
        FADE_IN,    // Fade in effect
        FADE_OUT    // Fade out effect
    };

private:
    Type animationType;
    Position position;
    sf::Clock animationClock;
    float duration;
    bool active;
    sf::CircleShape animatedStone;
    sf::Color originalColor;
    sf::Color targetColor;
    
public:
    StoneAnimation(Type type, const Position& pos, float dur = 0.5f);
    ~StoneAnimation();
    
    void start();
    void update();
    bool isActive() const { return active; }
    bool isComplete() const;
    
    void setColors(sf::Color original, sf::Color target);
    void draw(sf::RenderWindow& window, const sf::Vector2f& boardOffset, float cellSize);
    
    Type getType() const { return animationType; }
    Position getPosition() const { return position; }
};

// UI Theme system for consistent styling
struct UITheme {
    // Board colors
    sf::Color boardBackground;
    sf::Color gridLines;
    sf::Color boardBorder;
    
    // Stone colors
    sf::Color blackStone;
    sf::Color whiteStone;
    sf::Color blackStoneOutline;
    sf::Color whiteStoneOutline;
    
    // UI panel colors
    sf::Color panelBackground;
    sf::Color panelBorder;
    sf::Color headerBackground;
    
    // Text colors
    sf::Color primaryText;
    sf::Color secondaryText;
    sf::Color highlightText;
    sf::Color warningText;
    
    // Highlight colors
    sf::Color lastMoveHighlight;
    sf::Color validMoveHint;
    sf::Color threatHighlight;
    
    // Sizes
    float boardLineThickness;
    float stoneOutlineThickness;
    float panelBorderThickness;
    
    static UITheme createDefaultTheme();
    static UITheme createDarkTheme();
    static UITheme createClassicTheme();
};

class GameRenderer {
private:
    sf::Font mainFont;
    sf::Font monoFont;
    bool fontsLoaded;
    UITheme currentTheme;
    
    // Render targets
    sf::RenderWindow* window;
    
    // Board rendering
    sf::Vector2f boardOffset;
    float cellSize;
    sf::RectangleShape boardBackground;
    std::vector<sf::RectangleShape> gridLines;
    std::vector<sf::CircleShape> hoshiPoints;
    
    // Stone rendering
    std::vector<std::vector<sf::CircleShape>> stoneSprites;
    sf::CircleShape templateStone;
    
    // UI panels
    sf::RectangleShape leftPanel;
    sf::RectangleShape rightPanel;
    sf::RectangleShape statusPanel;

    // Color selection panel (for Human vs AI)
    sf::RectangleShape colorSelectPanel;
    sf::RectangleShape blackButtonShape;
    sf::RectangleShape whiteButtonShape;
    sf::Text colorSelectTitle;
    sf::Text blackButtonLabel;
    sf::Text whiteButtonLabel;
    sf::FloatRect blackButtonRect; // cached for hit testing
    sf::FloatRect whiteButtonRect; // cached for hit testing
    
    // Text objects
    sf::Text gameTitle;
    sf::Text currentPlayerText;
    sf::Text statusText;
    sf::Text captureCountText;
    sf::Text gameInfoText;
    sf::Text controlsHelpText;
    
    // Animation system
    std::vector<std::unique_ptr<StoneAnimation>> activeAnimations;
    
    std::unique_ptr<DebugUI> debugUI;
    
    // Highlighting system
    std::vector<Position> highlightedMoves;
    sf::CircleShape moveHighlight;
    sf::RectangleShape captureHighlight;
    
    // Game state references (non-owning)
    const Game* gameRef;
    const Board* boardRef;
    
public:
    GameRenderer();
    ~GameRenderer();
    
    // Initialization
    bool initialize(sf::RenderWindow* renderWindow);
    bool loadFonts(const std::string& mainFontPath = "", const std::string& monoFontPath = "");
    void setTheme(const UITheme& theme);
    
    // Game state binding
    void setGame(const Game* game);
    void setBoard(const Board* board);
    
    // Layout and sizing
    void calculateLayout(int windowWidth, int windowHeight);
    void setBoardParameters(const sf::Vector2f& offset, float size);
    
    // Main rendering
    void render();
    void renderBoard();
    void renderStones();
    void renderUI();
    void renderGameInfo();
    void renderStatusBar();
    void renderControls();
    
    // Enhanced features
    void renderHoverPreview();
    
    // Animation system
    void addAnimation(StoneAnimation::Type type, const Position& pos, float duration = 0.5f);
    void updateAnimations();
    void clearAnimations();
    
    // Highlighting
    void highlightMove(const Position& pos, sf::Color color = sf::Color::Yellow);
    void clearHighlights();
    
    // UI component access
    DebugUI* getDebugUI() { return debugUI.get(); }
    // Hit testing for color selection buttons (returns 1 for Black, 2 for White, 0 for none)
    int hitTestColorSelection(int mouseX, int mouseY) const;
    
    // Event handling support
    bool isPositionOnBoard(int mouseX, int mouseY) const;
    Position getBoardPosition(int mouseX, int mouseY) const;
    bool isPositionInPanel(int mouseX, int mouseY) const;
    
    // Theme and appearance
    void setStoneStyle(bool enableShadows, bool enable3D = false);
    void setGridStyle(bool enableCoordinates, bool thickBorder = true);
    void setAnimationSpeed(float speed);
    
    // Debug and development
    void showGrid(bool show);
    void showCoordinates(bool show);
    void showMoveNumbers(bool show);
    private:
    // Setup methods
    void setupBoard();
    void setupUI();
    void setupTexts();
    void createGridLines();
    void createStonePieces();
    
    // Rendering helpers
    void drawBoardGrid();
    void drawCoordinates();
    void drawStone(const Position& pos, CellState state);
    void drawPanel(sf::RectangleShape& panel);
    
    // Text rendering
    void updateGameInfoText();
    void updateStatusText();
    void updateCaptureText();
    void updateCurrentPlayerText();
    
    // Theme helpers
    void applyThemeToBoard();
    void applyThemeToUI();
    void applyThemeToText();
    
    // Layout calculations
    sf::Vector2f calculateOptimalBoardSize(int windowWidth, int windowHeight);
    sf::Vector2f calculateBoardOffset(const sf::Vector2f& boardSize, int windowWidth, int windowHeight);
    float calculateOptimalCellSize(const sf::Vector2f& availableSize);
    
    // Font fallbacks
    void setupDefaultFonts();
    bool loadSystemFonts();
};

// Utility functions for UI elements
namespace UIUtils {
    // Color utilities
    sf::Color blendColors(const sf::Color& color1, const sf::Color& color2, float ratio);
    sf::Color adjustBrightness(const sf::Color& color, float factor);
    sf::Color adjustAlpha(const sf::Color& color, unsigned char alpha);
    
    // Text utilities
    sf::Vector2f centerText(const sf::Text& text, const sf::FloatRect& bounds);
    void setTextCentered(sf::Text& text, float x, float y);
    std::string wrapText(const std::string& text, const sf::Font& font, unsigned int fontSize, float maxWidth);
    
    // Animation easing functions
    float easeInOut(float t);
    float easeIn(float t);
    float easeOut(float t);
    float bounce(float t);
    
    // Geometric helpers
    bool isPointInCircle(const sf::Vector2f& point, const sf::Vector2f& center, float radius);
    bool isPointInRect(const sf::Vector2f& point, const sf::FloatRect& rect);
    sf::Vector2f rotatePoint(const sf::Vector2f& point, const sf::Vector2f& center, float angle);
}

#endif // GAMERENDERER_HPP