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
#include "UI/GameRenderer.hpp"
#include "Game/Rules.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

// Constants
const float Game::MIN_CELL_SIZE = 20.0f;
const float Game::MAX_CELL_SIZE = 40.0f;
const int Game::UI_PANEL_WIDTH = 250;
const int Game::BOARD_MARGIN = 50;

Game::Game() 
    : window(
        sf::VideoMode(sf::Vector2u(WINDOW_WIDTH + UI_PANEL_WIDTH, WINDOW_HEIGHT)),
        "Gomoku",
        sf::Style::Titlebar | sf::Style::Close
    )
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
    , statusMessage("Welcome to Gomoku!")
    , currentSuggestion(-1, -1)
    , suggestionActive(false)
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

    calculateBoardDimensions();
    startNewGame(HUMAN_VS_HUMAN);  // Start in PLAYING state instead of MENU
}

Game::~Game() {
}

bool Game::initializeWindow() {
    // Use VSync instead of framerate limit to prevent conflicting settings
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
    }
}

void Game::handleMouseClick(int mouseX, int mouseY) {
    if (state != PLAYING || !currentPlayer || currentPlayer->getType() != HUMAN) {
        return;
    }
    
    Position boardPos = getMouseBoardPosition(mouseX, mouseY);
    
    if (boardPos.x >= 0 && boardPos.x < BOARD_SIZE && 
        boardPos.y >= 0 && boardPos.y < BOARD_SIZE) {
        
        // Check if the move is valid (apply full Rules including double-three)
        if (!Rules::isValidMove(board, boardPos.x, boardPos.y, currentPlayer->getColor())) {
            return;
        }
        
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
            // ESC now quits the game
            running = false;
            window.close();
            break;
            
        case sf::Keyboard::Key::R:
            if (state == PLAYING) {
                pause();
            } else if (state == GAME_OVER || state == PAUSED) {
                reset();
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
        case sf::Keyboard::Key::P:
        case sf::Keyboard::Key::Space:
            if (state == PLAYING) {
                pause();
            } else if (state == PAUSED) {
                resume();
            }
            break;
        case sf::Keyboard::Key::S:
            // Show move suggestion in Human vs Human mode
            if (mode == HUMAN_VS_HUMAN && state == PLAYING) {
                if (suggestionActive) {
                    clearMoveSuggestion();
                } else {
                    showMoveSuggestion();
                }
            }
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
        std::cout << "[Draw] Board full and no wins detected" << std::endl;
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
        std::cout << "[Move] Rejected move by " << player->getName() << " at (" << move.x << "," << move.y << ")" << std::endl;
        return false;
    }
    
    // Place the piece
    std::cout << "[Move] " << player->getName() << " plays (" << move.x << "," << move.y << ") as "
              << (player->getColor() == BLACK ? "BLACK" : "WHITE") << std::endl;
    if (!board.placePiece(move.x, move.y, player->getColor())) {
        std::cout << "[Move] Board::placePiece rejected after validation (unexpected)" << std::endl;
        return false;
    }
    
    // Check for captures
    std::vector<Position> captures = board.checkCaptures(move.x, move.y, player->getColor());
    if (!captures.empty()) {
        executeCaptures(captures, player);
        std::cout << "[Move] Captures executed: " << captures.size() << "; totals -> BLACK="
                  << board.getCaptureCount(BLACK) << ", WHITE=" << board.getCaptureCount(WHITE) << std::endl;
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
        std::cout << "[WinCheck] Game won for "
                  << (player->getColor() == BLACK ? "BLACK" : "WHITE") << std::endl;
        endGame(player->getColor());
    }
}

void Game::switchPlayer() {
    if (currentPlayer == player1.get()) {
        currentPlayer = player2.get();
    } else {
        currentPlayer = player1.get();
    }
    if (currentPlayer) {
        if (mode == AI_VS_AI && player1 && player2) {
            // Ne pas écraser le message de simulation en AI vs AI
            statusMessage = player1->getName() + std::string(" vs ") + player2->getName() + std::string(" | simulation en cours");
            return;
        }
        std::string colorName = (currentPlayer->getColor() == BLACK) ? "Black" : "White";
        statusMessage = currentPlayer->getName() + " (" + colorName + ")'s turn";
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
    if (mode == AI_VS_AI && player1 && player2) {
        // Message spécifique pour AI vs AI
        statusMessage = player1->getName() + std::string(" vs ") + player2->getName() + std::string(" | simulation en cours");
    } else {
        // Include color in status
        std::string colorName = (currentPlayer->getColor() == BLACK) ? "Black" : "White";
        statusMessage = currentPlayer->getName() + " (" + colorName + ")'s turn";
    }
}

void Game::reset() {
    board.clear();

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
        statusMessage = "Paused - Press R to reset, or P/Space to resume";
    }
}

void Game::resume() {
    if (state == PAUSED) {
        state = PLAYING;
        if (currentPlayer) {
            if (mode == AI_VS_AI && player1 && player2) {
                // Conserver le message AI vs AI
                statusMessage = player1->getName() + std::string(" vs ") + player2->getName() + std::string(" | simulation en cours");
                return;
            }
            std::string colorName = (currentPlayer->getColor() == BLACK) ? "Black" : "White";
            statusMessage = currentPlayer->getName() + " (" + colorName + ")'s turn";
        }
    }
}

void Game::endGame(CellState winnerColor) {
    gameWon = true;
    winner = winnerColor;
    state = GAME_OVER;
    
    if (winner == EMPTY) {
        statusMessage = "Game drawn!";
        std::cout << "[End] Draw" << std::endl;
    } else {
        std::string winnerName = getPlayerName(winner);
        std::string colorName = (winner == BLACK) ? "Black" : "White";
        
        // Determine win type
        if (Rules::hasCaptureWin(board, winner)) {
            statusMessage = winnerName + " (" + colorName + ") wins by capture!";
            std::cout << "[End] " << winnerName << " wins by capture (BLACK="
                      << board.getCaptureCount(BLACK) << ", WHITE=" << board.getCaptureCount(WHITE) << ")" << std::endl;
        } else {
            statusMessage = winnerName + " (" + colorName + ") wins by alignment!";
            std::cout << "[End] " << winnerName << " wins by alignment" << std::endl;
        }
    }
    
    // Notify players
    if (player1) player1->onGameEnd(player1->getColor() == winner);
    if (player2) player2->onGameEnd(player2->getColor() == winner);
}

// Rendering methods
void Game::render() {
    window.clear(sf::Color(240, 240, 240));
    
    // Use enhanced renderer if available, otherwise fallback
    if (renderer)
        renderer->render();
    // Always render enhanced UI components
    renderEnhancedUI();
    
    window.display();
}

Position Game::getMouseBoardPosition(int mouseX, int mouseY) const {
    // Always delegate to renderer so input uses the same boardOffset/cellSize as rendering    
    if (renderer) {
        Position p = renderer->getBoardPosition(mouseX, mouseY);
        return p;
    }
    
    // Fallback (should not happen): approximate using local offsets/sizes
    float relativeX = static_cast<float>(mouseX) - boardOffset.x;
    float relativeY = static_cast<float>(mouseY) - boardOffset.y;
    float normX = relativeX / cellSize;
    float normY = relativeY / cellSize;
    int boardX = static_cast<int>(std::round(normX));
    int boardY = static_cast<int>(std::round(normY));
    boardX = std::max(0, std::min(BOARD_SIZE - 1, boardX));
    boardY = std::max(0, std::min(BOARD_SIZE - 1, boardY));
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
        float timerX = window.getSize().x - 240;
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

// Move suggestion implementation
void Game::showMoveSuggestion() {
    if (!currentPlayer || mode != HUMAN_VS_HUMAN) {
        return;
    }
    
    Position suggestion = getAISuggestion(currentPlayer->getColor());
    if (suggestion.x >= 0 && suggestion.y >= 0) {
        currentSuggestion = suggestion;
        suggestionActive = true;
        
        // Highlight the suggestion using GameRenderer
        if (renderer) {
            renderer->highlightMove(suggestion, sf::Color(255, 255, 0, 128)); // Semi-transparent yellow
        }
        
        statusMessage = "Suggestion: " + std::to_string(suggestion.x + 1) + "," + std::to_string(suggestion.y + 1) + " (Press S to clear)";
    }
}

void Game::clearMoveSuggestion() {
    suggestionActive = false;
    currentSuggestion = Position(-1, -1);
    
    if (renderer) {
        renderer->clearHighlights();
    }
    
    if (currentPlayer) {
        statusMessage = currentPlayer->getName() + "'s turn";
    }
}

Position Game::getAISuggestion(CellState player) {
    // Create a temporary AI with shallow depth for quick suggestion
    MinMaxAI tempAI(player, 4, 0.2f); // Depth 4, 0.2s time limit for quick response
    
    // Get AI suggestion
    Position suggestion = tempAI.makeMove(board);
    
    return suggestion;
}