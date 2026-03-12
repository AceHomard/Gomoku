#ifndef GAMERENDERER_HPP
#define GAMERENDERER_HPP

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "../Game/Board.hpp"
#include "Timer.hpp"
#include "StoneAnimation.hpp"
#include "UIUtils.hpp"
#include <vector>
#include <memory>

// Forward declarations to avoid circular includes
class Game;
class Player;


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
    sf::Text turnCountText;
    sf::Text gameInfoText;
    sf::Text controlsHelpText;
    
    // Animation system
    std::vector<std::unique_ptr<StoneAnimation>> activeAnimations;
    
    // Highlighting system
    sf::CircleShape moveHighlight;
    std::vector<sf::CircleShape> winningAlignmentHighlights;
    
    // Game state references (non-owning)
    const Game* gameRef;
    const Board* boardRef;
    
public:
    GameRenderer();
    ~GameRenderer();
    
    // Initialization
    bool initialize(sf::RenderWindow* renderWindow);
    bool loadFonts(const std::string& mainFontPath = "", const std::string& monoFontPath = "");
    
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
    
    // Highlighting
    void highlightMove(const Position& pos, sf::Color color = sf::Color::Yellow);
    void highlightWinningAlignment(const std::vector<Position>& alignment, sf::Color color = sf::Color::Green);
    void clearHighlights();
    
    // Hit testing for color selection buttons (returns 1 for Black, 2 for White, 0 for none)
    int hitTestColorSelection(int mouseX, int mouseY) const;
    
    // Event handling support
    bool isPositionOnBoard(int mouseX, int mouseY) const;
    Position getBoardPosition(int mouseX, int mouseY) const;
    
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
    void updateTurnCountText();
    void updateCurrentPlayerText();
    
    // Theme helpers
    void applyThemeToBoard();
    void applyThemeToUI();
    void applyThemeToText();
    
    // Layout calculations
    
    // Font fallbacks
    void setupDefaultFonts();
};


#endif // GAMERENDERER_HPP