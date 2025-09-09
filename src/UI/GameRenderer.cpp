/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GameRenderer.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "UI/GameRenderer.hpp"
#include "Game/Game.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

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

// StoneAnimation implementation
StoneAnimation::StoneAnimation(Type type, const Position& pos, float dur)
    : animationType(type)
    , position(pos)
    , duration(dur)
    , active(false)
    , originalColor(sf::Color::White)
    , targetColor(sf::Color::Black)
{
    animatedStone.setRadius(10.0f);
    animatedStone.setOrigin(sf::Vector2f(10.0f, 10.0f));
}

StoneAnimation::~StoneAnimation() {
}

void StoneAnimation::start() {
    active = true;
    animationClock.restart();
}

void StoneAnimation::update() {
    if (!active) return;
    
    float elapsed = animationClock.getElapsedTime().asSeconds();
    if (elapsed >= duration) {
        active = false;
        return;
    }
    
    float progress = elapsed / duration;
    
    switch (animationType) {
        case PLACE: {
            // Scale animation for stone placement
            float scale = UIUtils::easeOut(progress);
            animatedStone.setScale(sf::Vector2f(scale, scale));
            
            // Color transition
            sf::Color currentColor = UIUtils::blendColors(originalColor, targetColor, progress);
            animatedStone.setFillColor(currentColor);
            break;
        }
        
        case CAPTURE: {
            // Shrink and fade animation for captures
            float scale = 1.0f - UIUtils::easeIn(progress);
            animatedStone.setScale(sf::Vector2f(scale, scale));
            
            sf::Color fadeColor = targetColor;
            fadeColor.a = static_cast<unsigned char>(255 * (1.0f - progress));
            animatedStone.setFillColor(fadeColor);
            break;
        }
        
        case HIGHLIGHT: {
            // Pulsing highlight effect
            float alpha = 0.5f + 0.5f * std::sin(progress * 6.28f * 2.0f); // 2 cycles
            sf::Color highlightColor = originalColor;
            highlightColor.a = static_cast<unsigned char>(255 * alpha);
            animatedStone.setFillColor(highlightColor);
            break;
        }
        
        case FADE_IN: {
            sf::Color fadeColor = targetColor;
            fadeColor.a = static_cast<unsigned char>(255 * UIUtils::easeOut(progress));
            animatedStone.setFillColor(fadeColor);
            break;
        }
        
        case FADE_OUT: {
            sf::Color fadeColor = originalColor;
            fadeColor.a = static_cast<unsigned char>(255 * (1.0f - UIUtils::easeIn(progress)));
            animatedStone.setFillColor(fadeColor);
            break;
        }
    }
}

bool StoneAnimation::isComplete() const {
    return !active;
}

void StoneAnimation::setColors(sf::Color original, sf::Color target) {
    originalColor = original;
    targetColor = target;
}

void StoneAnimation::draw(sf::RenderWindow& window, const sf::Vector2f& boardOffset, float cellSize) {
    if (!active) return;
    
    sf::Vector2f screenPos(boardOffset.x + position.x * cellSize + cellSize/2,
                          boardOffset.y + position.y * cellSize + cellSize/2);
    
    animatedStone.setRadius(cellSize * 0.4f);
    animatedStone.setOrigin(sf::Vector2f(cellSize * 0.4f, cellSize * 0.4f));
    animatedStone.setPosition(screenPos);
    
    window.draw(animatedStone);
}

// UITheme implementations
UITheme UITheme::createDefaultTheme() {
    UITheme theme;
    
    // Board colors
    theme.boardBackground = sf::Color(210, 180, 140);  // Tan
    theme.gridLines = sf::Color(101, 67, 33);          // Dark brown
    theme.boardBorder = sf::Color(139, 69, 19);        // Saddle brown
    
    // Stone colors
    theme.blackStone = sf::Color(20, 20, 20);
    theme.whiteStone = sf::Color(245, 245, 245);
    theme.blackStoneOutline = sf::Color(0, 0, 0);
    theme.whiteStoneOutline = sf::Color(100, 100, 100);
    
    // UI colors
    theme.panelBackground = sf::Color(240, 240, 240);
    theme.panelBorder = sf::Color(150, 150, 150);
    theme.headerBackground = sf::Color(200, 200, 200);
    
    // Text colors
    theme.primaryText = sf::Color(20, 20, 20);
    theme.secondaryText = sf::Color(100, 100, 100);
    theme.highlightText = sf::Color(0, 100, 200);
    theme.warningText = sf::Color(200, 50, 50);
    
    // Highlight colors
    theme.lastMoveHighlight = sf::Color(255, 100, 100, 150);
    theme.validMoveHint = sf::Color(100, 255, 100, 100);
    theme.threatHighlight = sf::Color(255, 0, 0, 200);
    
    // Sizes
    theme.boardLineThickness = 1.0f;
    theme.stoneOutlineThickness = 1.5f;
    theme.panelBorderThickness = 2.0f;
    
    return theme;
}

UITheme UITheme::createDarkTheme() {
    UITheme theme = createDefaultTheme();
    
    // Dark theme adjustments
    theme.boardBackground = sf::Color(40, 40, 40);
    theme.gridLines = sf::Color(120, 120, 120);
    theme.panelBackground = sf::Color(30, 30, 30);
    theme.primaryText = sf::Color(220, 220, 220);
    theme.secondaryText = sf::Color(160, 160, 160);
    
    return theme;
}

UITheme UITheme::createClassicTheme() {
    UITheme theme = createDefaultTheme();
    
    // Classic go board appearance
    theme.boardBackground = sf::Color(255, 206, 84);   // Traditional yellow
    theme.gridLines = sf::Color(0, 0, 0);
    theme.boardLineThickness = 1.5f;
    
    return theme;
}

// GameRenderer implementation
GameRenderer::GameRenderer()
    : fontsLoaded(false)
    , window(nullptr)
    , cellSize(30.0f)
    , gameTitle(getDummyFont(), "", 18)
    , currentPlayerText(getDummyFont(), "", 16)
    , statusText(getDummyFont(), "", 14)
    , captureCountText(getDummyFont(), "", 14)
    , gameInfoText(getDummyFont(), "", 12)
    , controlsHelpText(getDummyFont(), "", 10)
    , gameRef(nullptr)
    , boardRef(nullptr)
{
    currentTheme = UITheme::createDefaultTheme();
    
    aiTimer = std::make_unique<AITimer>("AI");
    debugUI = std::make_unique<DebugUI>();
    
    setupDefaultFonts();
}

GameRenderer::~GameRenderer() {
}

bool GameRenderer::initialize(sf::RenderWindow* renderWindow) {
    window = renderWindow;
    if (!window) return false;
    
    // Load fonts
    loadFonts();
    
    // Setup initial layout
    sf::Vector2u windowSize = window->getSize();
    calculateLayout(windowSize.x, windowSize.y);
    
    setupBoard();
    setupUI();
    setupTexts();
    
    return true;
}

bool GameRenderer::loadFonts(const std::string& mainFontPath, const std::string& monoFontPath) {
    std::vector<std::string> mainFontPaths;
    std::vector<std::string> monoFontPaths;
    
    if (!mainFontPath.empty()) mainFontPaths.push_back(mainFontPath);
    if (!monoFontPath.empty()) monoFontPaths.push_back(monoFontPath);
    
    // Add system font paths
    mainFontPaths.insert(mainFontPaths.end(), {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "C:/Windows/Fonts/arial.ttf"
    });
    
    monoFontPaths.insert(monoFontPaths.end(), {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/System/Library/Fonts/Menlo.ttc",
        "C:/Windows/Fonts/consola.ttf"
    });
    
    bool mainLoaded = false, monoLoaded = false;
    
    for (const auto& path : mainFontPaths) {
        if (mainFont.openFromFile(path)) {
            mainLoaded = true;
            break;
        }
    }
    
    for (const auto& path : monoFontPaths) {
        if (monoFont.openFromFile(path)) {
            monoLoaded = true;
            break;
        }
    }
    
    fontsLoaded = mainLoaded || monoLoaded;
    
    if (fontsLoaded) {
        // Use main font as fallback for mono if needed
        if (!monoLoaded && mainLoaded) {
            monoFont = mainFont;
            monoLoaded = true;
        }
        if (!mainLoaded && monoLoaded) {
            mainFont = monoFont;
            mainLoaded = true;
        }
        
        setupTexts();
        aiTimer->loadFont();
        debugUI->loadFont();
    }
    
    return fontsLoaded;
}

void GameRenderer::setTheme(const UITheme& theme) {
    currentTheme = theme;
    applyThemeToBoard();
    applyThemeToUI();
    applyThemeToText();
}

void GameRenderer::setGame(const Game* game) {
    gameRef = game;
}

void GameRenderer::setBoard(const Board* board) {
    boardRef = board;
    createStonePieces(); // Recreate stone sprites when board changes
}

void GameRenderer::calculateLayout(int windowWidth, int windowHeight) {
    // Calculate optimal board layout
    const int PANEL_WIDTH = 250;
    const int STATUS_HEIGHT = 60;
    const int MARGIN = 20;
    
    // Available space for board
    int boardAreaWidth = windowWidth - PANEL_WIDTH - 2 * MARGIN;
    int boardAreaHeight = windowHeight - STATUS_HEIGHT - 2 * MARGIN;
    
    // Calculate cell size to fit board
    float optimalCellSize = std::min(
        static_cast<float>(boardAreaWidth) / BOARD_SIZE,
        static_cast<float>(boardAreaHeight) / BOARD_SIZE
    );
    
    cellSize = std::max(20.0f, std::min(40.0f, optimalCellSize));
    
    // Center the board
    float boardWidth = BOARD_SIZE * cellSize;
    float boardHeight = BOARD_SIZE * cellSize;
    
    boardOffset.x = (boardAreaWidth - boardWidth) / 2 + MARGIN;
    boardOffset.y = (boardAreaHeight - boardHeight) / 2 + MARGIN;
    
    // Position UI panels
    leftPanel.setSize(sf::Vector2f(PANEL_WIDTH, windowHeight));
    leftPanel.setPosition(sf::Vector2f(windowWidth - PANEL_WIDTH, 0));
    
    statusPanel.setSize(sf::Vector2f(windowWidth, STATUS_HEIGHT));
    statusPanel.setPosition(sf::Vector2f(0, windowHeight - STATUS_HEIGHT));
    
    // Update AI timer and debug UI positions
    aiTimer->setPosition(windowWidth - PANEL_WIDTH + 10, 10);
    debugUI->setPosition(windowWidth - PANEL_WIDTH + 10, 150);
    debugUI->setSize(PANEL_WIDTH - 20, 300);
}

void GameRenderer::setBoardParameters(const sf::Vector2f& offset, float size) {
    boardOffset = offset;
    cellSize = size;
    setupBoard();
}

void GameRenderer::render() {
    if (!window) return;
    
    window->clear(sf::Color(250, 250, 250));
    
    renderBoard();
    renderStones();
    renderUI();
    renderLastMoveHighlight();
    
    // Update and render animations
    updateAnimations();
    for (auto& animation : activeAnimations) {
        animation->draw(*window, boardOffset, cellSize);
    }
    
    // Render debug overlay if visible
    if (debugUI->isVisible()) {
        debugUI->drawOverlay(*window, boardOffset, cellSize);
        debugUI->draw(*window);
    }
    
    // Render AI timer
    aiTimer->drawAIStatus(*window, window->getSize().x - 240, 10, 230, 120);
    
    window->display();
}

void GameRenderer::renderBoard() {
    // Draw board background
    boardBackground.setSize(sf::Vector2f(BOARD_SIZE * cellSize, BOARD_SIZE * cellSize));
    boardBackground.setPosition(boardOffset);
    boardBackground.setFillColor(currentTheme.boardBackground);
    boardBackground.setOutlineColor(currentTheme.boardBorder);
    boardBackground.setOutlineThickness(currentTheme.panelBorderThickness);
    window->draw(boardBackground);
    
    drawBoardGrid();
    drawCoordinates();
}

void GameRenderer::renderStones() {
    if (!boardRef) return;
    
    for (int x = 0; x < BOARD_SIZE; ++x) {
        for (int y = 0; y < BOARD_SIZE; ++y) {
            CellState cell = boardRef->getCell(x, y);
            if (cell != EMPTY) {
                drawStone(Position(x, y), cell);
            }
        }
    }
}

void GameRenderer::renderUI() {
    // Draw panels
    drawPanel(leftPanel);
    drawPanel(statusPanel);
    
    renderGameInfo();
    renderStatusBar();
    renderControls();
}

void GameRenderer::renderGameInfo() {
    if (!gameRef || !fontsLoaded) return;
    
    updateGameInfoText();
    updateCurrentPlayerText();
    updateCaptureText();
    
    float yPos = 180;
    const float lineHeight = 25;
    
    gameTitle.setPosition(sf::Vector2f(window->getSize().x - 240, yPos));
    window->draw(gameTitle);
    yPos += lineHeight;
    
    currentPlayerText.setPosition(sf::Vector2f(window->getSize().x - 240, yPos));
    window->draw(currentPlayerText);
    yPos += lineHeight;
    
    captureCountText.setPosition(sf::Vector2f(window->getSize().x - 240, yPos));
    window->draw(captureCountText);
    yPos += lineHeight;
    
    gameInfoText.setPosition(sf::Vector2f(window->getSize().x - 240, yPos));
    window->draw(gameInfoText);
}

void GameRenderer::renderStatusBar() {
    if (!fontsLoaded) return;
    
    updateStatusText();
    statusText.setPosition(sf::Vector2f(10, window->getSize().y - 45));
    window->draw(statusText);
}

void GameRenderer::renderControls() {
    if (!fontsLoaded) return;
    
    std::string controls = "Controls:\n"
                          "F1 - Toggle Debug\n"
                          "R - Restart Game\n"
                          "U - Undo Move\n"
                          "1/2/3 - Game Modes\n"
                          "ESC - Pause";
    
    controlsHelpText.setString(controls);
    controlsHelpText.setPosition(sf::Vector2f(window->getSize().x - 240, window->getSize().y - 200));
    window->draw(controlsHelpText);
}

void GameRenderer::renderLastMoveHighlight() {
    if (!gameRef) return;
    
    // This would be implemented with access to game's move history
    // For now, just show a placeholder implementation
}

void GameRenderer::addAnimation(StoneAnimation::Type type, const Position& pos, float duration) {
    auto animation = std::make_unique<StoneAnimation>(type, pos, duration);
    
    // Set appropriate colors based on animation type and position
    if (boardRef) {
        CellState cell = boardRef->getCell(pos.x, pos.y);
        if (cell == BLACK) {
            animation->setColors(currentTheme.whiteStone, currentTheme.blackStone);
        } else if (cell == WHITE) {
            animation->setColors(currentTheme.blackStone, currentTheme.whiteStone);
        }
    }
    
    animation->start();
    activeAnimations.push_back(std::move(animation));
}

void GameRenderer::updateAnimations() {
    // Update all active animations
    for (auto& animation : activeAnimations) {
        animation->update();
    }
    
    // Remove completed animations
    activeAnimations.erase(
        std::remove_if(activeAnimations.begin(), activeAnimations.end(),
                      [](const std::unique_ptr<StoneAnimation>& anim) {
                          return anim->isComplete();
                      }),
        activeAnimations.end()
    );
}

void GameRenderer::clearAnimations() {
    activeAnimations.clear();
}

void GameRenderer::highlightMove(const Position& pos, sf::Color color) {
    moveHighlight.setRadius(cellSize * 0.45f);
    moveHighlight.setOrigin(sf::Vector2f(cellSize * 0.45f, cellSize * 0.45f));
    moveHighlight.setFillColor(sf::Color::Transparent);
    moveHighlight.setOutlineColor(color);
    moveHighlight.setOutlineThickness(3);
    
    sf::Vector2f screenPos(boardOffset.x + pos.x * cellSize + cellSize/2,
                          boardOffset.y + pos.y * cellSize + cellSize/2);
    moveHighlight.setPosition(screenPos);
}

bool GameRenderer::isPositionOnBoard(int mouseX, int mouseY) const {
    sf::FloatRect boardBounds(sf::Vector2f(boardOffset.x, boardOffset.y),
                             sf::Vector2f(BOARD_SIZE * cellSize, BOARD_SIZE * cellSize));
    return boardBounds.contains(sf::Vector2f(static_cast<float>(mouseX), static_cast<float>(mouseY)));
}

Position GameRenderer::getBoardPosition(int mouseX, int mouseY) const {
    int boardX = static_cast<int>((mouseX - boardOffset.x) / cellSize);
    int boardY = static_cast<int>((mouseY - boardOffset.y) / cellSize);
    
    return Position(std::max(0, std::min(BOARD_SIZE - 1, boardX)),
                   std::max(0, std::min(BOARD_SIZE - 1, boardY)));
}

// Private method implementations
void GameRenderer::setupBoard() {
    createGridLines();
    moveHighlight.setRadius(cellSize * 0.45f);
    moveHighlight.setOrigin(sf::Vector2f(cellSize * 0.45f, cellSize * 0.45f));
}

void GameRenderer::setupUI() {
    applyThemeToUI();
}

void GameRenderer::setupTexts() {
    if (!fontsLoaded) return;
    
    gameTitle = sf::Text(mainFont, "Gomoku", 18);
    gameTitle.setFillColor(currentTheme.primaryText);
    
    currentPlayerText = sf::Text(mainFont, "", 16);
    currentPlayerText.setFillColor(currentTheme.highlightText);
    
    statusText = sf::Text(mainFont, "", 14);
    statusText.setFillColor(currentTheme.primaryText);
    
    captureCountText = sf::Text(mainFont, "", 14);
    captureCountText.setFillColor(currentTheme.secondaryText);
    
    gameInfoText = sf::Text(mainFont, "", 12);
    gameInfoText.setFillColor(currentTheme.secondaryText);
    
    controlsHelpText = sf::Text(monoFont, "", 10);
    controlsHelpText.setFillColor(currentTheme.secondaryText);
}

void GameRenderer::createGridLines() {
    gridLines.clear();
    
    // Horizontal lines
    for (int i = 0; i < BOARD_SIZE; ++i) {
        sf::RectangleShape line;
        line.setSize(sf::Vector2f(BOARD_SIZE * cellSize, currentTheme.boardLineThickness));
        line.setPosition(sf::Vector2f(boardOffset.x, boardOffset.y + i * cellSize + cellSize/2));
        line.setFillColor(currentTheme.gridLines);
        gridLines.push_back(line);
    }
    
    // Vertical lines
    for (int i = 0; i < BOARD_SIZE; ++i) {
        sf::RectangleShape line;
        line.setSize(sf::Vector2f(currentTheme.boardLineThickness, BOARD_SIZE * cellSize));
        line.setPosition(sf::Vector2f(boardOffset.x + i * cellSize + cellSize/2, boardOffset.y));
        line.setFillColor(currentTheme.gridLines);
        gridLines.push_back(line);
    }
}

void GameRenderer::createStonePieces() {
    templateStone.setRadius(cellSize * 0.4f);
    templateStone.setOrigin(sf::Vector2f(cellSize * 0.4f, cellSize * 0.4f));
    templateStone.setOutlineThickness(currentTheme.stoneOutlineThickness);
}

void GameRenderer::drawBoardGrid() {
    for (const auto& line : gridLines) {
        window->draw(line);
    }
}

void GameRenderer::drawCoordinates() {
    if (!fontsLoaded) return;
    
    sf::Text coordText(monoFont, "", 10);
    coordText.setFillColor(currentTheme.secondaryText);
    
    // Column labels (A-S)
    for (int i = 0; i < BOARD_SIZE; ++i) {
        char label = 'A' + i;
        coordText.setString(std::string(1, label));
        coordText.setPosition(sf::Vector2f(boardOffset.x + i * cellSize + cellSize/2 - 5, boardOffset.y - 20));
        window->draw(coordText);
    }
    
    // Row labels (1-19)
    for (int i = 0; i < BOARD_SIZE; ++i) {
        coordText.setString(std::to_string(i + 1));
        coordText.setPosition(sf::Vector2f(boardOffset.x - 20, boardOffset.y + i * cellSize + cellSize/2 - 8));
        window->draw(coordText);
    }
}

void GameRenderer::drawStone(const Position& pos, CellState state) {
    if (state == EMPTY) return;
    
    sf::Vector2f screenPos(boardOffset.x + pos.x * cellSize + cellSize/2,
                          boardOffset.y + pos.y * cellSize + cellSize/2);
    
    templateStone.setPosition(screenPos);
    
    if (state == BLACK) {
        templateStone.setFillColor(currentTheme.blackStone);
        templateStone.setOutlineColor(currentTheme.blackStoneOutline);
    } else if (state == WHITE) {
        templateStone.setFillColor(currentTheme.whiteStone);
        templateStone.setOutlineColor(currentTheme.whiteStoneOutline);
    }
    
    window->draw(templateStone);
}

void GameRenderer::drawPanel(sf::RectangleShape& panel) {
    window->draw(panel);
}

void GameRenderer::updateGameInfoText() {
    if (!gameRef) return;
    
    std::ostringstream oss;
    oss << "Mode: ";
    
    switch (gameRef->getGameMode()) {
        case HUMAN_VS_HUMAN:
            oss << "Human vs Human";
            break;
        case HUMAN_VS_AI:
            oss << "Human vs AI";
            break;
        case AI_VS_AI:
            oss << "AI vs AI";
            break;
    }
    
    gameInfoText.setString(oss.str());
}

void GameRenderer::updateStatusText() {
    if (!gameRef) {
        statusText.setString("Ready");
        return;
    }
    
    std::string status;
    switch (gameRef->getGameState()) {
        case PLAYING:
            if (gameRef->getCurrentPlayer()) {
                status = gameRef->getCurrentPlayer()->getName() + "'s turn";
            } else {
                status = "Playing";
            }
            break;
        case GAME_OVER:
            status = "Game Over";
            break;
        case PAUSED:
            status = "Paused";
            break;
        case MENU:
            status = "Menu";
            break;
    }
    
    statusText.setString(status);
}

void GameRenderer::updateCaptureText() {
    if (!gameRef) return;
    
    std::ostringstream oss;
    // This would need access to capture counts from players
    oss << "Captures: Player 1: 0, Player 2: 0";
    captureCountText.setString(oss.str());
}

void GameRenderer::updateCurrentPlayerText() {
    if (!gameRef || !gameRef->getCurrentPlayer()) {
        currentPlayerText.setString("No active player");
        return;
    }
    
    Player* current = gameRef->getCurrentPlayer();
    std::string playerInfo = current->getName();
    
    if (current->getType() == AI) {
        playerInfo += " (AI)";
    }
    
    currentPlayerText.setString("Current: " + playerInfo);
}

void GameRenderer::applyThemeToBoard() {
    createGridLines();
    createStonePieces();
}

void GameRenderer::applyThemeToUI() {
    leftPanel.setFillColor(currentTheme.panelBackground);
    leftPanel.setOutlineColor(currentTheme.panelBorder);
    leftPanel.setOutlineThickness(currentTheme.panelBorderThickness);
    
    statusPanel.setFillColor(currentTheme.headerBackground);
    statusPanel.setOutlineColor(currentTheme.panelBorder);
    statusPanel.setOutlineThickness(1);
}

void GameRenderer::applyThemeToText() {
    if (!fontsLoaded) return;
    
    gameTitle.setFillColor(currentTheme.primaryText);
    currentPlayerText.setFillColor(currentTheme.highlightText);
    statusText.setFillColor(currentTheme.primaryText);
    captureCountText.setFillColor(currentTheme.secondaryText);
    gameInfoText.setFillColor(currentTheme.secondaryText);
    controlsHelpText.setFillColor(currentTheme.secondaryText);
}

void GameRenderer::setupDefaultFonts() {
    // This method would set up fallback rendering when no fonts are available
    // For now, just mark fonts as not loaded
    fontsLoaded = false;
}

// UIUtils namespace implementation
namespace UIUtils {

sf::Color blendColors(const sf::Color& color1, const sf::Color& color2, float ratio) {
    ratio = std::max(0.0f, std::min(1.0f, ratio));
    
    unsigned char r = static_cast<unsigned char>(color1.r * (1.0f - ratio) + color2.r * ratio);
    unsigned char g = static_cast<unsigned char>(color1.g * (1.0f - ratio) + color2.g * ratio);
    unsigned char b = static_cast<unsigned char>(color1.b * (1.0f - ratio) + color2.b * ratio);
    unsigned char a = static_cast<unsigned char>(color1.a * (1.0f - ratio) + color2.a * ratio);
    
    return sf::Color(r, g, b, a);
}

sf::Color adjustBrightness(const sf::Color& color, float factor) {
    factor = std::max(0.0f, factor);
    
    unsigned char r = static_cast<unsigned char>(std::min(255.0f, color.r * factor));
    unsigned char g = static_cast<unsigned char>(std::min(255.0f, color.g * factor));
    unsigned char b = static_cast<unsigned char>(std::min(255.0f, color.b * factor));
    
    return sf::Color(r, g, b, color.a);
}

sf::Color adjustAlpha(const sf::Color& color, unsigned char alpha) {
    return sf::Color(color.r, color.g, color.b, alpha);
}

sf::Vector2f centerText(const sf::Text& text, const sf::FloatRect& bounds) {
    sf::FloatRect textBounds = text.getLocalBounds();
    
    float x = bounds.position.x + (bounds.size.x - textBounds.size.x) / 2.0f;
    float y = bounds.position.y + (bounds.size.y - textBounds.size.y) / 2.0f;
    
    return sf::Vector2f(x, y);
}

void setTextCentered(sf::Text& text, float x, float y) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setPosition(sf::Vector2f(x - bounds.size.x / 2.0f, y - bounds.size.y / 2.0f));
}

float easeInOut(float t) {
    return t * t * (3.0f - 2.0f * t);
}

float easeIn(float t) {
    return t * t;
}

float easeOut(float t) {
    return 1.0f - (1.0f - t) * (1.0f - t);
}

float bounce(float t) {
    if (t < 0.5f) {
        return 2.0f * t * t;
    } else {
        return -1.0f + 4.0f * t - 2.0f * t * t;
    }
}

bool isPointInCircle(const sf::Vector2f& point, const sf::Vector2f& center, float radius) {
    float dx = point.x - center.x;
    float dy = point.y - center.y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

bool isPointInRect(const sf::Vector2f& point, const sf::FloatRect& rect) {
    return rect.contains(point);
}

sf::Vector2f rotatePoint(const sf::Vector2f& point, const sf::Vector2f& center, float angle) {
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);
    
    float dx = point.x - center.x;
    float dy = point.y - center.y;
    
    float x = center.x + dx * cos_a - dy * sin_a;
    float y = center.y + dx * sin_a + dy * cos_a;
    
    return sf::Vector2f(x, y);
}

} // namespace UIUtils