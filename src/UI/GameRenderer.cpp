#include "UI/GameRenderer.hpp"
#include "Game/Game.hpp"
#include "Game/IPlayer.hpp"
#include "Game/Rules.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>


void GameRenderer::renderHoverPreview() {
    if (!window || !gameRef || !boardRef) return;
    if (gameRef->getGameState() != PLAYING) return;
    if (!gameRef->getCurrentPlayer() || gameRef->getCurrentPlayer()->getType() != HUMAN) return;

    // Get mouse position relative to the render window
    sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
    int mx = mousePos.x;
    int my = mousePos.y;

    // Ensure cursor is on the board
    if (!isPositionOnBoard(mx, my)) return;

    // Compute board coordinates and skip if occupied
    Position pos = getBoardPosition(mx, my);
    if (boardRef->getCell(pos.x, pos.y) != EMPTY) return;

    // Determine validity of the move
    CellState player = gameRef->getCurrentPlayer()->getColor();
    bool valid = Rules::isValidMove(*boardRef, pos.x, pos.y, player);

    // Prepare ghost stone
    sf::CircleShape ghost;
    ghost.setRadius(cellSize * 0.4f);
    ghost.setOrigin(sf::Vector2f(cellSize * 0.4f, cellSize * 0.4f));
    ghost.setPosition(sf::Vector2f(boardOffset.x + pos.x * cellSize,
                                   boardOffset.y + pos.y * cellSize));

    if (valid) {
        // Semi-transparent stone in player's color
        sf::Color fill = (player == BLACK) ? currentTheme.blackStone : currentTheme.whiteStone;
        fill.a = 120; // transparency
        ghost.setFillColor(fill);
        ghost.setOutlineColor((player == BLACK) ? currentTheme.blackStoneOutline : currentTheme.whiteStoneOutline);
        ghost.setOutlineThickness(1.0f);
    } else {
        // Invalid move: red semi-transparent overlay
        sf::Color invalid(220, 50, 50, 140);
        ghost.setFillColor(invalid);
        ghost.setOutlineColor(sf::Color(160, 30, 30, 200));
        ghost.setOutlineThickness(2.0f);
    }

    window->draw(ghost);
}

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


UITheme UITheme::createDefaultTheme() {
    UITheme theme;
    
    // Board colors
    theme.boardBackground = sf::Color(210, 180, 140);
    theme.gridLines = sf::Color(101, 67, 33);  
    theme.boardBorder = sf::Color(139, 69, 19);  
    
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



// GameRenderer implementation
GameRenderer::GameRenderer()
    : fontsLoaded(false)
    , window(nullptr)
    , cellSize(30.0f)
    , colorSelectTitle(getDummyFont(), "", 14)
    , blackButtonLabel(getDummyFont(), "", 12)
    , whiteButtonLabel(getDummyFont(), "", 12)
    , gameTitle(getDummyFont(), "", 18)
    , currentPlayerText(getDummyFont(), "", 16)
    , statusText(getDummyFont(), "", 14)
    , captureCountText(getDummyFont(), "", 14)
    , turnCountText(getDummyFont(), "", 14)
    , gameInfoText(getDummyFont(), "", 12)
    , controlsHelpText(getDummyFont(), "", 10)
    , gameRef(nullptr)
    , boardRef(nullptr)
{
    currentTheme = UITheme::createDefaultTheme();

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
        if (!monoLoaded && mainLoaded) {
            monoFont = mainFont;
            monoLoaded = true;
        }
        if (!mainLoaded && monoLoaded) {
            mainFont = monoFont;
            mainLoaded = true;
        }
        
        setupTexts();
    }
    
    return fontsLoaded;
}


void GameRenderer::setGame(const Game* game) {
    gameRef = game;
}

void GameRenderer::setBoard(const Board* board) {
    boardRef = board;
    createStonePieces();
}

void GameRenderer::calculateLayout(int windowWidth, int windowHeight) {
    const int PANEL_WIDTH = 250;
    const int STATUS_HEIGHT = 60;
    const int MARGIN = 20;
    
    int boardAreaWidth = windowWidth - PANEL_WIDTH - 2 * MARGIN;
    int boardAreaHeight = windowHeight - STATUS_HEIGHT - 2 * MARGIN;
    
    float optimalCellSize = std::min(
        static_cast<float>(boardAreaWidth) / BOARD_SIZE,
        static_cast<float>(boardAreaHeight) / BOARD_SIZE
    );
    
    cellSize = std::max(20.0f, std::min(40.0f, optimalCellSize));
    
    float boardWidth = BOARD_SIZE * cellSize;
    float boardHeight = BOARD_SIZE * cellSize;
    
    boardOffset.x = (boardAreaWidth - boardWidth) / 2 + MARGIN;
    boardOffset.y = (boardAreaHeight - boardHeight) / 2 + MARGIN;
    
    leftPanel.setSize(sf::Vector2f(PANEL_WIDTH, windowHeight));
    leftPanel.setPosition(sf::Vector2f(windowWidth - PANEL_WIDTH, 0));
    
    statusPanel.setSize(sf::Vector2f(windowWidth, STATUS_HEIGHT));
    statusPanel.setPosition(sf::Vector2f(0, windowHeight - STATUS_HEIGHT));
    
}

void GameRenderer::setBoardParameters(const sf::Vector2f& offset, float size) {
    boardOffset = offset;
    cellSize = size;
    setupBoard();
}

void GameRenderer::render() {
    if (!window) return;
    
    renderBoard();
    renderStones();
    
    if (gameRef && gameRef->isGameOver() && !gameRef->getWinningAlignment().empty()) {
        for (const sf::CircleShape& highlight : winningAlignmentHighlights) {
            window->draw(highlight);
        }
    }
    
    if (moveHighlight.getOutlineThickness() > 0 && moveHighlight.getOutlineColor().a > 0) {
        window->draw(moveHighlight);
    }
    renderHoverPreview();
    renderUI();
    
    updateAnimations();
    for (auto& animation : activeAnimations) {
        animation->draw(*window, boardOffset, cellSize);
    }
}

void GameRenderer::renderBoard() {
    float boardWidth = (BOARD_SIZE - 1) * cellSize + 2 * currentTheme.panelBorderThickness;
    float boardHeight = (BOARD_SIZE - 1) * cellSize + 2 * currentTheme.panelBorderThickness;
    
    boardBackground.setSize(sf::Vector2f(boardWidth, boardHeight));
    boardBackground.setPosition(sf::Vector2f(
        boardOffset.x - currentTheme.panelBorderThickness,
        boardOffset.y - currentTheme.panelBorderThickness
    ));
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
    drawPanel(leftPanel);
    drawPanel(statusPanel);
    
    renderGameInfo();
    renderStatusBar();
    renderControls();

    if (gameRef && gameRef->getGameMode() == HUMAN_VS_AI && fontsLoaded) {
        float panelWidth = 220.0f;
        float panelHeight = 120.0f;
        float panelX = window->getSize().x - panelWidth - 20.0f;
        float panelY = 320.0f;

        colorSelectPanel.setSize(sf::Vector2f(panelWidth, panelHeight));
        colorSelectPanel.setPosition(sf::Vector2f(panelX, panelY));
        colorSelectPanel.setFillColor(UIUtils::adjustAlpha(currentTheme.panelBackground, 235));
        colorSelectPanel.setOutlineColor(currentTheme.panelBorder);
        colorSelectPanel.setOutlineThickness(1.0f);
        window->draw(colorSelectPanel);

        colorSelectTitle = sf::Text(mainFont, "Choose your color", 14);
        colorSelectTitle.setFillColor(currentTheme.primaryText);
        colorSelectTitle.setPosition(sf::Vector2f(panelX + 12, panelY + 10));
        window->draw(colorSelectTitle);

        float btnWidth = (panelWidth - 30.0f) / 2.0f;
        float btnHeight = 34.0f;
        float btnY = panelY + 60.0f;
        float btnX1 = panelX + 10.0f;
        float btnX2 = btnX1 + btnWidth + 10.0f;

        sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
        sf::Vector2f mpos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        blackButtonShape.setSize(sf::Vector2f(btnWidth, btnHeight));
        blackButtonShape.setPosition(sf::Vector2f(btnX1, btnY));
        bool hoverBlack = sf::FloatRect(blackButtonShape.getPosition(), blackButtonShape.getSize()).contains(mpos);
        blackButtonShape.setFillColor(hoverBlack ? sf::Color(45, 45, 45) : sf::Color(30, 30, 30));
        blackButtonShape.setOutlineColor(currentTheme.panelBorder);
        blackButtonShape.setOutlineThickness(hoverBlack ? 2.0f : 1.0f);
        window->draw(blackButtonShape);

        whiteButtonShape.setSize(sf::Vector2f(btnWidth, btnHeight));
        whiteButtonShape.setPosition(sf::Vector2f(btnX2, btnY));
        bool hoverWhite = sf::FloatRect(whiteButtonShape.getPosition(), whiteButtonShape.getSize()).contains(mpos);
        whiteButtonShape.setFillColor(hoverWhite ? sf::Color(245, 245, 245) : sf::Color(230, 230, 230));
        whiteButtonShape.setOutlineColor(currentTheme.panelBorder);
        whiteButtonShape.setOutlineThickness(hoverWhite ? 2.0f : 1.0f);
        window->draw(whiteButtonShape);

        blackButtonLabel = sf::Text(mainFont, "Black", 13);
        blackButtonLabel.setStyle(sf::Text::Bold);
        blackButtonLabel.setFillColor(sf::Color(240, 240, 240));
        UIUtils::setTextCentered(blackButtonLabel, btnX1 + btnWidth / 2.0f, btnY + btnHeight / 2.0f);
        {
            sf::Vector2f tp = blackButtonLabel.getPosition();
            blackButtonLabel.setPosition(sf::Vector2f(std::round(tp.x), std::round(tp.y)));
        }
        window->draw(blackButtonLabel);

        whiteButtonLabel = sf::Text(mainFont, "White", 13);
        whiteButtonLabel.setStyle(sf::Text::Bold);
        whiteButtonLabel.setFillColor(sf::Color(30, 30, 30));
        UIUtils::setTextCentered(whiteButtonLabel, btnX2 + btnWidth / 2.0f, btnY + btnHeight / 2.0f);
        {
            sf::Vector2f tp = whiteButtonLabel.getPosition();
            whiteButtonLabel.setPosition(sf::Vector2f(std::round(tp.x), std::round(tp.y)));
        }
        window->draw(whiteButtonLabel);

        blackButtonRect = sf::FloatRect(blackButtonShape.getPosition(), blackButtonShape.getSize());
        whiteButtonRect = sf::FloatRect(whiteButtonShape.getPosition(), whiteButtonShape.getSize());
    }
}

void GameRenderer::renderGameInfo() {
    if (!gameRef || !fontsLoaded) return;

    updateGameInfoText();
    updateCurrentPlayerText();
    updateCaptureText();
    updateTurnCountText();

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

    turnCountText.setPosition(sf::Vector2f(window->getSize().x - 240, yPos));
    window->draw(turnCountText);
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
                          "R - Restart Game\n"
                          "1 - Human vs Human\n"
                          "2 - Human vs AI\n"
                          "3 - AI vs AI\n"
                          "P/Space - Pause/Resume\n"
                          "S - Move Suggestion (HvH)\n"
                          "Left/Z - Undo Move\n"
                          "Right/Y - Redo Move\n"
                          "Home/End - First/Last Move\n"
                          "Enter - Exit Replay\n"
                          "ESC - Quit";
    
    controlsHelpText.setString(controls);

    float controlsY = (gameRef && gameRef->getGameMode() == HUMAN_VS_AI) ? 460.0f : 320.0f;
    controlsHelpText.setPosition(sf::Vector2f(window->getSize().x - 240, controlsY));
    window->draw(controlsHelpText);
}

int GameRenderer::hitTestColorSelection(int mouseX, int mouseY) const {
    if (!window) return 0;
    if (!gameRef || gameRef->getGameMode() != HUMAN_VS_AI) return 0;

    sf::FloatRect blackRect = blackButtonRect;
    sf::FloatRect whiteRect = whiteButtonRect;

    auto isValid = [](const sf::FloatRect& r) {
        return r.size.x > 0.f && r.size.y > 0.f;
    };

    if (!isValid(blackRect) || !isValid(whiteRect)) {
        float panelWidth = 220.0f;
        // float panelHeight = 120.0f;
        float panelX = window->getSize().x - panelWidth - 20.0f;
        float panelY = 320.0f; // keep in sync with renderUI
        float btnWidth = (panelWidth - 30.0f) / 2.0f;
        float btnHeight = 34.0f;
        float btnY = panelY + 60.0f;
        float btnX1 = panelX + 10.0f;
        float btnX2 = btnX1 + btnWidth + 10.0f;
        blackRect = sf::FloatRect(sf::Vector2f(btnX1, btnY), sf::Vector2f(btnWidth, btnHeight));
        whiteRect = sf::FloatRect(sf::Vector2f(btnX2, btnY), sf::Vector2f(btnWidth, btnHeight));
    }

    auto inflate = [](sf::FloatRect r, float pad) {
        r.position.x -= pad; r.position.y -= pad;
        r.size.x += pad * 2.f; r.size.y += pad * 2.f;
        return r;
    };
    blackRect = inflate(blackRect, 3.f);
    whiteRect = inflate(whiteRect, 3.f);

    sf::Vector2f p(static_cast<float>(mouseX), static_cast<float>(mouseY));
    if (blackRect.contains(p)) return 1;
    if (whiteRect.contains(p)) return 2;
    return 0;
}

void GameRenderer::addAnimation(StoneAnimation::Type type, const Position& pos, float duration) {
    auto animation = std::make_unique<StoneAnimation>(type, pos, duration);
    
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
    for (auto& animation : activeAnimations) {
        animation->update();
    }
    
    activeAnimations.erase(
        std::remove_if(activeAnimations.begin(), activeAnimations.end(),
                      [](const std::unique_ptr<StoneAnimation>& anim) {
                          return anim->isComplete();
                      }),
        activeAnimations.end()
    );
}

void GameRenderer::highlightMove(const Position& pos, sf::Color color) {
    moveHighlight.setRadius(cellSize * 0.45f);
    moveHighlight.setOrigin(sf::Vector2f(cellSize * 0.45f, cellSize * 0.45f));
    moveHighlight.setFillColor(sf::Color::Transparent);
    moveHighlight.setOutlineColor(color);
    moveHighlight.setOutlineThickness(3);
    
    sf::Vector2f screenPos(boardOffset.x + pos.x * cellSize,
                          boardOffset.y + pos.y * cellSize);
    moveHighlight.setPosition(screenPos);
}

void GameRenderer::highlightWinningAlignment(const std::vector<Position>& alignment, sf::Color color) {
    winningAlignmentHighlights.clear();
    
    for (const Position& pos : alignment) {
        sf::CircleShape highlight;
        highlight.setRadius(cellSize * 0.5f);
        highlight.setOrigin(sf::Vector2f(cellSize * 0.5f, cellSize * 0.5f));
        highlight.setFillColor(sf::Color(color.r, color.g, color.b, 100));
        highlight.setOutlineThickness(4.0f);
        
        sf::Vector2f screenPos(boardOffset.x + pos.x * cellSize,
                              boardOffset.y + pos.y * cellSize);
        highlight.setPosition(screenPos);
        
        winningAlignmentHighlights.push_back(highlight);
    }
}

bool GameRenderer::isPositionOnBoard(int mouseX, int mouseY) const {
    sf::FloatRect boardBounds(sf::Vector2f(boardOffset.x - cellSize / 2, boardOffset.y - cellSize / 2),
                             sf::Vector2f((BOARD_SIZE) * cellSize, (BOARD_SIZE) * cellSize));
    return boardBounds.contains(sf::Vector2f(static_cast<float>(mouseX), static_cast<float>(mouseY)));
}

Position GameRenderer::getBoardPosition(int mouseX, int mouseY) const {
    float relX = static_cast<float>(mouseX) - boardOffset.x;
    float relY = static_cast<float>(mouseY) - boardOffset.y;
    float normX = relX / cellSize;
    float normY = relY / cellSize;

    int boardX = static_cast<int>(std::floor(normX + 0.5f));
    int boardY = static_cast<int>(std::floor(normY + 0.5f));

    boardX = std::max(0, std::min(BOARD_SIZE - 1, boardX));
    boardY = std::max(0, std::min(BOARD_SIZE - 1, boardY));

    return Position(boardX, boardY);
}

void GameRenderer::clearHighlights() {
    moveHighlight.setOutlineThickness(0);
    winningAlignmentHighlights.clear();
}

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

    currentPlayerText = sf::Text(mainFont, "", 16);
    currentPlayerText.setFillColor(currentTheme.highlightText);

    statusText = sf::Text(mainFont, "", 14);
    statusText.setFillColor(currentTheme.primaryText);

    captureCountText = sf::Text(mainFont, "", 14);
    captureCountText.setFillColor(currentTheme.secondaryText);

    turnCountText = sf::Text(mainFont, "", 14);
    turnCountText.setFillColor(currentTheme.secondaryText);

    gameInfoText = sf::Text(mainFont, "", 12);
    gameInfoText.setFillColor(currentTheme.secondaryText);

    controlsHelpText = sf::Text(monoFont, "", 10);
    controlsHelpText.setFillColor(currentTheme.secondaryText);
}

void GameRenderer::createGridLines() {
    gridLines.clear();
    hoshiPoints.clear();
    
    float lineLength = (BOARD_SIZE - 1) * cellSize;
    
    for (int i = 0; i < BOARD_SIZE; ++i) {
        sf::RectangleShape line;
        line.setSize(sf::Vector2f(lineLength, currentTheme.boardLineThickness));
        line.setPosition(sf::Vector2f(boardOffset.x, boardOffset.y + i * cellSize));
        line.setFillColor(currentTheme.gridLines);
        gridLines.push_back(line);
    }
    
    for (int i = 0; i < BOARD_SIZE; ++i) {
        sf::RectangleShape line;
        line.setSize(sf::Vector2f(currentTheme.boardLineThickness, lineLength));
        line.setPosition(sf::Vector2f(boardOffset.x + i * cellSize, boardOffset.y));
        line.setFillColor(currentTheme.gridLines);
        gridLines.push_back(line);
    }
    
    if (BOARD_SIZE == 19) {
        int hoshiPositions[9][2] = {
            {3, 3}, {3, 9}, {3, 15},
            {9, 3}, {9, 9}, {9, 15},
            {15, 3}, {15, 9}, {15, 15}
        };
        
        for (int i = 0; i < 9; ++i) {
            sf::CircleShape hoshi(3.0f);
            hoshi.setFillColor(currentTheme.gridLines);
            hoshi.setOrigin(sf::Vector2f(3.0f, 3.0f));
            hoshi.setPosition(sf::Vector2f(
                boardOffset.x + hoshiPositions[i][0] * cellSize,
                boardOffset.y + hoshiPositions[i][1] * cellSize
            ));
            hoshiPoints.push_back(hoshi);
        }
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
    
    for (const auto& hoshi : hoshiPoints) {
        window->draw(hoshi);
    }
}

void GameRenderer::drawCoordinates() {
    if (!fontsLoaded) return;
    
    sf::Text coordText(monoFont, "", 10);
    coordText.setFillColor(currentTheme.secondaryText);
    
    for (int i = 0; i < BOARD_SIZE; ++i) {
        coordText.setString(std::to_string(i));
        coordText.setPosition(sf::Vector2f(boardOffset.x + i * cellSize - 5, boardOffset.y - 20));
        window->draw(coordText);
    }

    for (int i = 0; i < BOARD_SIZE; ++i) {
        coordText.setString(std::to_string(i));
        coordText.setPosition(sf::Vector2f(boardOffset.x - 20, boardOffset.y + i * cellSize - 8));
        window->draw(coordText);
    }
}

void GameRenderer::drawStone(const Position& pos, CellState state) {
    if (state == EMPTY) return;
    
    sf::Vector2f screenPos(boardOffset.x + pos.x * cellSize,
                          boardOffset.y + pos.y * cellSize);
    
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
    statusText.setString(gameRef->getStatusMessage());
}

void GameRenderer::updateCaptureText() {
    if (!gameRef || !boardRef) return;

    int blackCaptures = boardRef->getCaptureCount(BLACK);
    int whiteCaptures = boardRef->getCaptureCount(WHITE);

    std::ostringstream oss;
    oss << "Captures: Black=" << blackCaptures << ", White=" << whiteCaptures;
    captureCountText.setString(oss.str());
}

void GameRenderer::updateTurnCountText() {
    if (!gameRef) return;

    int totalMoves = gameRef->getTotalMoves();
    int turnNumber = (totalMoves / 2) + 1; 

    std::ostringstream oss;
    oss << "Turn: " << turnNumber << " (Move: " << totalMoves << ")";
    turnCountText.setString(oss.str());
}

void GameRenderer::updateCurrentPlayerText() {
    if (!gameRef || !gameRef->getCurrentPlayer()) {
        currentPlayerText.setString("No active player");
        return;
    }

    IPlayer* current = gameRef->getCurrentPlayer();
    std::string playerInfo = current->getName();

    if (current->getType() == AI) {
        playerInfo += " (AI)";
    }

    std::string colorName = (current->getColor() == BLACK) ? "Black" : "White";
    playerInfo += " - " + colorName;

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
    fontsLoaded = false;
}

