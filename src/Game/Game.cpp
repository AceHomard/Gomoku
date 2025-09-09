/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Game.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Game/Game.hpp"
#include "AI/MinMaxAI.hpp"
#include <iostream>

// Constants
const float Game::MIN_CELL_SIZE = 20.0f;
const float Game::MAX_CELL_SIZE = 40.0f;
const int Game::UI_PANEL_WIDTH = 250;
const int Game::BOARD_MARGIN = 50;

Game::Game() 
    : window(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH + UI_PANEL_WIDTH, WINDOW_HEIGHT)), "Gomoku - Enhanced with AI Timer")
    , aiTimer(std::make_unique<AITimer>("AI Player"))
    , debugUI(std::make_unique<DebugUI>())
    , renderer(std::make_unique<GameRenderer>())
    , debugMode(false)
    , board(BOARD_SIZE)
    , player1(nullptr)
    , player2(nullptr)
    , currentPlayer(nullptr)
    , state(MENU)
    , mode(HUMAN_VS_HUMAN)
    , running(true)
    , gameWon(false)
    , winner(EMPTY)
    , statusMessage("Welcome to Gomoku! Press F1 for Debug Mode")
    , cellSize(CELL_SIZE)
    , boardOffset(BOARD_MARGIN, BOARD_MARGIN)
{
    if (!initializeWindow()) {
        running = false;
    }
    
    if (!loadResources()) {
        std::cerr << "Warning: Could not load font resources" << std::endl;
    }
    
    // Initialize enhanced UI components
    renderer->initialize(&window);
    renderer->setGame(this);
    renderer->setBoard(&board);
    
    // Load fonts for UI components
    aiTimer->loadFont();
    debugUI->loadFont();
    
    std::cout << "Enhanced UI initialized with mandatory AI Timer" << std::endl;
    
    calculateBoardDimensions();
    setupPlayers(HUMAN_VS_HUMAN);
}

Game::~Game() {
}

bool Game::initializeWindow() {
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    return true;
}

bool Game::loadResources() {
    // Try to load a system font - this is optional
    // The game will work without fonts, just won't display text
    return true; // Return true for now since font is optional
}

void Game::calculateBoardDimensions() {
    // Calculate optimal cell size based on window size
    float availableWidth = WINDOW_WIDTH - 2 * BOARD_MARGIN;
    float availableHeight = WINDOW_HEIGHT - 2 * BOARD_MARGIN;
    
    float optimalCellSize = std::min(availableWidth / BOARD_SIZE, availableHeight / BOARD_SIZE);
    cellSize = std::max(MIN_CELL_SIZE, std::min(MAX_CELL_SIZE, optimalCellSize));
    
    // Center the board
    float boardWidth = BOARD_SIZE * cellSize;
    float boardHeight = BOARD_SIZE * cellSize;
    
    boardOffset.x = (WINDOW_WIDTH - boardWidth) / 2.0f;
    boardOffset.y = (WINDOW_HEIGHT - boardHeight) / 2.0f;
}

void Game::run() {
    while (running && window.isOpen()) {
        handleEvents();
        update();
        render();
    }
}

void Game::handleEvents() {
    while (auto event = window.pollEvent()) {
        handleWindowEvents(*event);
        
        if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left && state == PLAYING) {
                handleMouseClick(static_cast<int>(mousePressed->position.x), static_cast<int>(mousePressed->position.y));
            }
        } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            handleKeyPress(keyPressed->code);
        }
    }
}

void Game::handleWindowEvents(const sf::Event& event) {
    if (event.is<sf::Event::Closed>()) {
        running = false;
        window.close();
    } else if (event.is<sf::Event::Resized>()) {
        // Handle window resize
        calculateBoardDimensions();
    }
}

void Game::handleMouseClick(int mouseX, int mouseY) {
    if (state != PLAYING || !currentPlayer || currentPlayer->getType() != HUMAN) {
        return;
    }
    
    Position boardPos = getMouseBoardPosition(mouseX, mouseY);
    
    if (boardPos.x >= 0 && boardPos.x < BOARD_SIZE && 
        boardPos.y >= 0 && boardPos.y < BOARD_SIZE) {
        
        // Set the pending move for human player
        HumanPlayer* humanPlayer = dynamic_cast<HumanPlayer*>(currentPlayer);
        if (humanPlayer) {
            humanPlayer->setPendingMove(boardPos.x, boardPos.y);
        }
    }
}

void Game::handleKeyPress(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Key::Escape:
            if (state == PLAYING) {
                pause();
            } else if (state == PAUSED) {
                resume();
            }
            break;
            
        case sf::Keyboard::Key::R:
            if (state == GAME_OVER || state == PAUSED) {
                reset();
            }
            break;
            
        case sf::Keyboard::Key::N:
            startNewGame(mode);
            break;
            
        case sf::Keyboard::Key::U:
            if (state == PLAYING) {
                undoLastMove();
            }
            break;
            
        case sf::Keyboard::Key::Num1:
            startNewGame(HUMAN_VS_HUMAN);
            break;
            
        case sf::Keyboard::Key::Num2:
            startNewGame(HUMAN_VS_AI);
            break;
            
        case sf::Keyboard::Key::Num3:
            startNewGame(AI_VS_AI);
            break;
            
        case sf::Keyboard::Key::F1:
            toggleDebugMode();
            break;
            
        default:
            handleDebugKeyPress(key);
            break;
    }
}

void Game::update() {
    // Update AI timer continuously (MANDATORY display requirement)
    updateAITimer();
    
    // Update debug information
    updateDebugInfo();
    
    if (state != PLAYING || !currentPlayer) {
        return;
    }
    
    // Handle AI moves
    if (currentPlayer->getType() == AI) {
        handleAIMove();
    } else {
        // Handle human moves
        HumanPlayer* humanPlayer = dynamic_cast<HumanPlayer*>(currentPlayer);
        if (humanPlayer && humanPlayer->hasPendingMoveAvailable()) {
            Position move = humanPlayer->makeMove(board);
            if (move.x >= 0 && move.y >= 0) {
                processMove(move, currentPlayer);
                
                // Add placement animation
                if (renderer) {
                    renderer->addAnimation(StoneAnimation::PLACE, move, 0.2f);
                }
            }
        }
    }
    
    updateGameState();
}

void Game::handleAIMove() {
    // Start AI timer (MANDATORY requirement)
    startAITimer();
    
    // Update AI timer with current AI player info
    if (currentPlayer && currentPlayer->getType() == AI) {
        AIPlayer* aiPlayer = dynamic_cast<AIPlayer*>(currentPlayer);
        if (aiPlayer) {
            aiTimer->setAIName(currentPlayer->getName());
            aiTimer->setSearchDepth(aiPlayer->getSearchDepth());
        }
    }
    
    Position move = currentPlayer->makeMove(board);
    
    // Stop AI timer and record the move time
    stopAITimer();
    
    if (move.x >= 0 && move.y >= 0) {
        processMove(move, currentPlayer);
        
        // Add placement animation
        if (renderer) {
            renderer->addAnimation(StoneAnimation::PLACE, move, 0.3f);
        }
    }
}

void Game::updateGameState() {
    if (gameWon || state == GAME_OVER) {
        return;
    }
    
    // Check for win conditions
    checkWinCondition(currentPlayer);
    
    // Check for draw
    if (Rules::isGameDrawn(board)) {
        endGame(EMPTY); // Draw
    }
}

bool Game::makeMove(int x, int y) {
    if (state != PLAYING || !currentPlayer) {
        return false;
    }
    
    return processMove(Position(x, y), currentPlayer);
}

bool Game::processMove(Position move, Player* player) {
    if (!validateMove(move, player)) {
        return false;
    }
    
    // Place the piece
    if (!board.placePiece(move.x, move.y, player->getColor())) {
        return false;
    }
    
    // Record the move
    moveHistory.push_back(move);
    
    // Check for captures
    std::vector<Position> captures = board.checkCaptures(move.x, move.y, player->getColor());
    if (!captures.empty()) {
        captureHistory.push_back(captures);
        executeCaptures(captures, player);
    } else {
        captureHistory.push_back(std::vector<Position>());
    }
    
    // Check win condition
    checkWinCondition(player);
    
    if (!gameWon) {
        switchPlayer();
    }
    
    return true;
}

bool Game::validateMove(Position move, Player* player) {
    return Rules::isValidMove(board, move.x, move.y, player->getColor());
}

void Game::executeCaptures(const std::vector<Position>& captures, Player* player) {
    int captureCount = captures.size();
    player->addCapture(captureCount);
    board.setCaptureCount(player->getColor(), player->getCaptureCount());
    
    player->onCapture(captureCount);
    
    statusMessage = player->getName() + " captured " + std::to_string(captureCount) + " stones!";
}

void Game::checkWinCondition(Player* player) {
    if (Rules::isGameWon(board, player->getColor())) {
        endGame(player->getColor());
    }
}

void Game::switchPlayer() {
    if (currentPlayer == player1.get()) {
        currentPlayer = player2.get();
    } else {
        currentPlayer = player1.get();
    }
}

void Game::setupPlayers(GameMode gameMode) {
    this->mode = gameMode;
    
    switch (gameMode) {
        case HUMAN_VS_HUMAN:
            player1 = std::make_unique<HumanPlayer>(BLACK, "Player 1");
            player2 = std::make_unique<HumanPlayer>(WHITE, "Player 2");
            break;
            
        case HUMAN_VS_AI:
            player1 = std::make_unique<HumanPlayer>(BLACK, "Human");
            player2 = std::make_unique<AIPlayer>(WHITE, 2, "AI");
            break;
            
        case AI_VS_AI:
            player1 = std::make_unique<AIPlayer>(BLACK, 1, "AI 1");
            player2 = std::make_unique<AIPlayer>(WHITE, 2, "AI 2");
            break;
    }
    
    currentPlayer = player1.get();
}

Player* Game::getOpponent() const {
    if (currentPlayer == player1.get()) {
        return player2.get();
    }
    return player1.get();
}

void Game::startNewGame(GameMode gameMode) {
    reset();
    setupPlayers(gameMode);
    state = PLAYING;
    statusMessage = currentPlayer->getName() + "'s turn";
}

void Game::reset() {
    board.clear();
    moveHistory.clear();
    captureHistory.clear();
    
    if (player1) player1->setCaptureCount(0);
    if (player2) player2->setCaptureCount(0);
    
    currentPlayer = player1.get();
    gameWon = false;
    winner = EMPTY;
    state = PLAYING;
    statusMessage = "Game reset!";
}

void Game::pause() {
    if (state == PLAYING) {
        state = PAUSED;
        statusMessage = "Game paused - Press ESC to resume";
    }
}

void Game::resume() {
    if (state == PAUSED) {
        state = PLAYING;
        if (currentPlayer) {
            statusMessage = currentPlayer->getName() + "'s turn";
        }
    }
}

void Game::endGame(CellState winnerColor) {
    gameWon = true;
    winner = winnerColor;
    state = GAME_OVER;
    
    if (winner == EMPTY) {
        statusMessage = "Game drawn!";
    } else {
        std::string winnerName = getPlayerName(winner);
        
        // Determine win type
        if (Rules::hasCaptureWin(board, winner)) {
            statusMessage = winnerName + " wins by capture!";
        } else {
            statusMessage = winnerName + " wins by alignment!";
        }
    }
    
    // Notify players
    if (player1) player1->onGameEnd(player1->getColor() == winner);
    if (player2) player2->onGameEnd(player2->getColor() == winner);
}

bool Game::undoLastMove() {
    if (moveHistory.empty() || state != PLAYING) {
        return false;
    }
    
    // Remove last move
    Position lastMove = moveHistory.back();
    moveHistory.pop_back();
    
    // Restore captured pieces if any
    if (!captureHistory.empty()) {
        std::vector<Position> lastCaptures = captureHistory.back();
        captureHistory.pop_back();
        
        // Restore captured stones
        CellState opponentColor = board.getOpponent(board.getCell(lastMove.x, lastMove.y));
        for (const Position& capturePos : lastCaptures) {
            const_cast<Board&>(board).placePiece(capturePos.x, capturePos.y, opponentColor);
        }
        
        // Adjust capture counts
        if (currentPlayer) {
            currentPlayer->setCaptureCount(currentPlayer->getCaptureCount() - lastCaptures.size());
            board.setCaptureCount(currentPlayer->getColor(), currentPlayer->getCaptureCount());
        }
    }
    
    // Remove the piece
    const_cast<Board&>(board).placePiece(lastMove.x, lastMove.y, EMPTY);
    
    // Switch back to previous player
    switchPlayer();
    
    statusMessage = "Move undone - " + currentPlayer->getName() + "'s turn";
    return true;
}

// Rendering methods
void Game::render() {
    window.clear(sf::Color(240, 240, 240));
    
    // Use enhanced renderer if available, otherwise fallback
    if (renderer) {
        renderer->render();
    } else {
        // Fallback rendering
        renderBoard();
        renderUI();
        renderStatusMessage();
    }
    
    // Always render enhanced UI components
    renderEnhancedUI();
    
    window.display();
}

void Game::renderBoard() {
    board.draw(window);
    
    // Highlight last move if available
    if (!moveHistory.empty()) {
        Position lastMove = moveHistory.back();
        sf::CircleShape highlight(cellSize / 2 - 1);
        highlight.setPosition(sf::Vector2f(boardOffset.x + lastMove.x * cellSize + 1, 
                                          boardOffset.y + lastMove.y * cellSize + 1));
        highlight.setFillColor(sf::Color::Transparent);
        highlight.setOutlineColor(sf::Color::Red);
        highlight.setOutlineThickness(2);
        window.draw(highlight);
    }
}

void Game::renderUI() {
    renderGameInfo();
    
    // Draw UI panel background
    sf::RectangleShape uiPanel(sf::Vector2f(UI_PANEL_WIDTH, WINDOW_HEIGHT));
    uiPanel.setPosition(sf::Vector2f(WINDOW_WIDTH, 0));
    uiPanel.setFillColor(sf::Color(220, 220, 220));
    window.draw(uiPanel);
    
    // Draw capture counts if font is available
    if (font.getInfo().family != "") {
        board.drawCaptureCount(window, font);
    }
}

void Game::renderGameInfo() {
    // Game info rendering (without font for now)
    // This can be enhanced when font loading is implemented
}

void Game::renderStatusMessage() {
    // Status message rendering (without font for now)
    // This can be enhanced when font loading is implemented
}

Position Game::getMouseBoardPosition(int mouseX, int mouseY) const {
    int boardX = (mouseX - boardOffset.x) / cellSize;
    int boardY = (mouseY - boardOffset.y) / cellSize;
    
    return Position(boardX, boardY);
}

sf::Vector2f Game::getBoardPosition(int x, int y) const {
    return sf::Vector2f(boardOffset.x + x * cellSize, boardOffset.y + y * cellSize);
}

sf::Color Game::getPlayerColor(CellState player) const {
    switch (player) {
        case BLACK: return sf::Color::Black;
        case WHITE: return sf::Color::White;
        default: return sf::Color::Transparent;
    }
}

std::string Game::getPlayerName(CellState player) const {
    if (player1 && player1->getColor() == player) {
        return player1->getName();
    } else if (player2 && player2->getColor() == player) {
        return player2->getName();
    }
    return (player == BLACK) ? "Black" : "White";
}

// Enhanced UI Methods (MANDATORY TIMER IMPLEMENTATION)
void Game::startAITimer() {
    if (aiTimer) {
        aiTimer->startMoveTimer();
        
        // Update debug UI if AI is thinking
        if (debugUI && currentPlayer && currentPlayer->getType() == AI) {
            debugUI->setSearchStatus("AI searching for best move...", true);
        }
    }
}

void Game::stopAITimer() {
    if (aiTimer) {
        aiTimer->stopMoveTimer();
        
        // Update performance metrics for debug
        if (debugUI && currentPlayer && currentPlayer->getType() == AI) {
            AIPlayer* aiPlayer = dynamic_cast<AIPlayer*>(currentPlayer);
            if (aiPlayer) {
                aiTimer->setNodesEvaluated(aiPlayer->getNodesEvaluated());
                debugUI->setPerformanceMetrics(
                    aiPlayer->getNodesEvaluated(),
                    aiPlayer->getTTHitRate(),
                    aiTimer->getCurrentMoveTime()
                );
            }
            debugUI->setSearchStatus("Search completed", false);
        }
        
        std::cout << "AI move completed in " << aiTimer->getFormattedCurrentTime() << std::endl;
    }
}

void Game::updateAITimer() {
    if (aiTimer) {
        aiTimer->update();
    }
}

void Game::toggleDebugMode() {
    debugMode = !debugMode;
    if (debugUI) {
        if (debugMode) {
            debugUI->show();
            statusMessage = "Debug mode enabled - F1 to toggle";
        } else {
            debugUI->hide();
            statusMessage = "Debug mode disabled";
        }
    }
    std::cout << "Debug mode " << (debugMode ? "enabled" : "disabled") << std::endl;
}

void Game::updateDebugInfo() {
    if (debugUI && debugMode && currentPlayer && currentPlayer->getType() == AI) {
        AIPlayer* aiPlayer = dynamic_cast<AIPlayer*>(currentPlayer);
        if (aiPlayer) {
            // Create debug info structure
            DebugInfo info;
            info.searchDepth = aiPlayer->getSearchDepth();
            info.nodesEvaluated = aiPlayer->getNodesEvaluated();
            info.ttHitRate = aiPlayer->getTTHitRate();
            info.principalVariation = aiPlayer->getPrincipalVariation();
            info.isSearching = aiTimer->isTimerRunning();
            
            if (info.isSearching) {
                info.searchStatus = "Searching at depth " + std::to_string(info.searchDepth);
            } else {
                info.searchStatus = "Ready";
            }
            
            debugUI->updateDebugInfo(info);
        }
    }
}

void Game::renderEnhancedUI() {
    if (!window.isOpen()) return;
    
    // Always render AI timer (MANDATORY requirement)
    if (aiTimer) {
        float timerX = window.getSize().x - 250;
        float timerY = 10;
        aiTimer->drawAIStatus(window, timerX, timerY, 240, 100);
    }
    
    // Render debug UI if enabled
    if (debugUI && debugMode) {
        debugUI->draw(window);
        
        // Render debug overlay on board
        if (renderer) {
            debugUI->drawOverlay(window, boardOffset, cellSize);
        }
    }
}

void Game::handleDebugKeyPress(sf::Keyboard::Key key) {
    if (debugUI) {
        debugUI->handleKeyPress(key);
    }
    
    // Additional debug key handlers
    switch (key) {
        case sf::Keyboard::Key::F2:
            if (debugUI) {
                debugUI->clearLog();
                debugUI->addLogMessage("Debug log cleared");
            }
            break;
            
        case sf::Keyboard::Key::F3:
            if (debugUI) {
                debugUI->clearHighlights();
            }
            break;
            
        case sf::Keyboard::Key::T:
            if (aiTimer) {
                std::cout << "AI Timer Stats:" << std::endl;
                std::cout << "Current: " << aiTimer->getFormattedCurrentTime() << std::endl;
                std::cout << "Average: " << aiTimer->getFormattedAverageTime() << std::endl;
                std::cout << "Total: " << aiTimer->getFormattedTotalTime() << std::endl;
                std::cout << "Moves: " << aiTimer->getMoveCount() << std::endl;
            }
            break;
            
        default:
            break;
    }
}