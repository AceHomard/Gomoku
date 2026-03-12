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
#include "Game/HumanPlayer.hpp"
#include "Game/AIPlayer.hpp"
#include "AI/MinMaxAI.hpp"
#include "UI/GameRenderer.hpp"
#include "Game/Rules.hpp"
#ifdef DEBUG_VISU
#include "Debug/DebugVisualizer.hpp"
#endif
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
    , currentMoveIndex(-1)
    , replayMode(false)
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

    calculateBoardDimensions();

#ifdef DEBUG_VISU
    debugVisualizer = std::make_unique<DebugVisualizer>();
    debugVisualizer->initialize();
#endif

    startNewGame(HUMAN_VS_HUMAN);  // Default: Human vs Human (press 2 for AI mode)
}

Game::~Game() {
#ifdef DEBUG_VISU
    // Signal AI thread to stop and wait
    if (aiSearchRunning) {
        // Try to stop the AI
        if (currentPlayer) {
            AIPlayer* aiPlayer = dynamic_cast<AIPlayer*>(currentPlayer);
            if (aiPlayer) {
                MinMaxAI* minmax = dynamic_cast<MinMaxAI*>(currentPlayer);
                if (minmax) minmax->requestStop();
            }
        }
    }
    if (aiThread.joinable()) {
        aiThread.join();
    }
    if (debugVisualizer) {
        debugVisualizer->close();
    }
#endif
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
        try {
            handleEvents();
#ifdef DEBUG_VISU
            if (debugVisualizer && debugVisualizer->isOpen()) {
                debugVisualizer->handleEvents();
            }
#endif
            update();
            render();
#ifdef DEBUG_VISU
            if (debugVisualizer && debugVisualizer->isOpen()) {
                debugVisualizer->render();
            }
#endif
        } catch (const std::bad_alloc&) {
            // Enter a degraded mode to avoid quitting unexpectedly
            std::cerr << "[Game] bad_alloc caught - pausing game to recover" << std::endl;
            statusMessage = "Low memory detected - game paused (press P/Space to resume)";
            if (state == PLAYING) {
                state = PAUSED;
            }
            // Continue loop to keep window responsive
        } catch (const std::exception& e) {
            std::cerr << "[Game] Exception recovered: " << e.what() << std::endl;
            statusMessage = std::string("Recovered from error: ") + e.what();
            // Keep going to avoid unexpected quit
        } catch (...) {
            std::cerr << "[Game] Unknown exception recovered" << std::endl;
            statusMessage = "Recovered from unknown error";
        }
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
#ifdef DEBUG_VISU
        if (debugVisualizer) debugVisualizer->close();
#endif
    }
}

void Game::handleMouseClick(int mouseX, int mouseY) {
    // Handle color selection panel clicks first (always active in Human vs AI mode)
    if (renderer && mode == HUMAN_VS_AI) {
        int hit = renderer->hitTestColorSelection(mouseX, mouseY);
        if (hit == 1) {
            startNewGame(HUMAN_VS_AI, BLACK);
            return;
        } else if (hit == 2) {
            startNewGame(HUMAN_VS_AI, WHITE);
            return;
        }
    }

    if (state != PLAYING || !currentPlayer || currentPlayer->getType() != HUMAN) {
        return;
    }
    
    // Ignore clicks outside of the board area to avoid accidental placements when clicking UI panels
    if (renderer && !renderer->isPositionOnBoard(mouseX, mouseY)) {
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
            // Enter Human vs AI mode with default Black for human; panel remains visible to change color
            startNewGame(HUMAN_VS_AI, BLACK);
            break;
            
        case sf::Keyboard::Key::Num3:
            startNewGame(AI_VS_AI);
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
                showMoveSuggestion();
            }
            break;

        // Replay controls
        case sf::Keyboard::Key::Left:
        case sf::Keyboard::Key::Z:
            // Undo move / previous move
            if (!moveHistory.empty()) {
                undoMove();
            }
            break;

        case sf::Keyboard::Key::Right:
        case sf::Keyboard::Key::Y:
            // Redo move / next move
            if (!moveHistory.empty()) {
                redoMove();
            }
            break;

        case sf::Keyboard::Key::Home:
            // Go to first move
            if (!moveHistory.empty()) {
                goToMove(0);
            }
            break;

        case sf::Keyboard::Key::End:
            // Go to last move
            if (!moveHistory.empty()) {
                goToMove(moveHistory.size() - 1);
            }
            break;

        case sf::Keyboard::Key::Enter:
            // Exit replay mode
            if (replayMode) {
                exitReplayMode();
            }
            break;

        default:
            break;
    }
}

void Game::update() {
    // Update AI timer continuously (MANDATORY display requirement)
    updateAITimer();
    
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
#ifdef DEBUG_VISU
    // Async mode: launch AI in separate thread so game loop keeps rendering
    if (!aiSearchRunning) {
        aiSearchRunning = true;
        aiSearchDone = false;
        startAITimer();

        if (currentPlayer && currentPlayer->getType() == AI) {
            AIPlayer* aiPlayer = dynamic_cast<AIPlayer*>(currentPlayer);
            if (aiPlayer) {
                aiTimer->setAIName(currentPlayer->getName());
                aiTimer->setSearchDepth(aiPlayer->getSearchDepth());
            }
        }

        // Copy the board for the AI thread (make/unmake mutates it)
        Board searchBoardCopy = board;
        IPlayer* aiPlayer = currentPlayer;

        if (aiThread.joinable()) {
            aiThread.join();
        }

        aiThread = std::thread([this, searchBoardCopy, aiPlayer]() mutable {
            aiResult = aiPlayer->makeMove(searchBoardCopy);
            aiSearchDone = true;
        });
    }

    if (aiSearchDone) {
        if (aiThread.joinable()) {
            aiThread.join();
        }
        stopAITimer();

        Position move = aiResult;
        aiSearchRunning = false;
        aiSearchDone = false;

        if (move.x >= 0 && move.y >= 0) {
            processMove(move, currentPlayer);
            if (renderer) {
                renderer->addAnimation(StoneAnimation::PLACE, move, 0.3f);
            }
        } else {
            std::cout << "[Game] AI cannot find valid move - ending game as draw" << std::endl;
            endGame(EMPTY);
        }
    }
#else
    // Synchronous mode (original behavior)
    startAITimer();

    if (currentPlayer && currentPlayer->getType() == AI) {
        AIPlayer* aiPlayer = dynamic_cast<AIPlayer*>(currentPlayer);
        if (aiPlayer) {
            aiTimer->setAIName(currentPlayer->getName());
            aiTimer->setSearchDepth(aiPlayer->getSearchDepth());
        }
    }

    Position move = currentPlayer->makeMove(board);
    stopAITimer();

    if (move.x >= 0 && move.y >= 0) {
        processMove(move, currentPlayer);
        if (renderer) {
            renderer->addAnimation(StoneAnimation::PLACE, move, 0.3f);
        }
    } else {
        std::cout << "[Game] AI cannot find valid move - ending game as draw" << std::endl;
        endGame(EMPTY);
    }
#endif
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

bool Game::processMove(Position move, IPlayer* player) {
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

    // Save move to history (only if not in replay mode)
    if (!replayMode) {
        // If replaying from middle and making new move, erase future history
        if (currentMoveIndex < static_cast<int>(moveHistory.size()) - 1) {
            moveHistory.erase(moveHistory.begin() + currentMoveIndex + 1, moveHistory.end());
        }

        // Add new move record
        MoveRecord record(move, player->getColor(), captures,
                         board.getCaptureCount(BLACK), board.getCaptureCount(WHITE));
        moveHistory.push_back(record);
        currentMoveIndex = static_cast<int>(moveHistory.size()) - 1;
    }

    // Check win condition
    checkWinCondition(player);

    if (!gameWon) {
        switchPlayer();
    }

    return true;
}

bool Game::validateMove(Position move, IPlayer* player) {
    return Rules::isValidMove(board, move.x, move.y, player->getColor());
}

void Game::executeCaptures(const std::vector<Position>& captures, IPlayer* player) {
    int captureCount = captures.size();
    player->addCapture(captureCount);
    board.setCaptureCount(player->getColor(), player->getCaptureCount());
    
    
    statusMessage = player->getName() + " captured " + std::to_string(captureCount) + " stones!";
}

void Game::checkWinCondition(IPlayer* player) {
    if (Rules::isGameWon(board, player->getColor())) {
        std::cout << "[WinCheck] Game won for "
                  << (player->getColor() == BLACK ? "BLACK" : "WHITE") << std::endl;
        
        // Capture winning alignment if it's an alignment win
        if (Rules::hasAlignmentWin(board, player->getColor())) {
            winningAlignment = Rules::getWinningAlignment(board, player->getColor());
            std::cout << "[WinCheck] Winning alignment captured: " << winningAlignment.size() << " stones" << std::endl;
        }
        
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
            statusMessage = player1->getName() + std::string(" vs ") + player2->getName() + std::string(" | simulation in progress");
            return;
        }
        std::string colorName = (currentPlayer->getColor() == BLACK) ? "Black" : "White";
        statusMessage = currentPlayer->getName() + " (" + colorName + ")'s turn";
    }
}

void Game::setupPlayers(GameMode gameMode, CellState humanPreferredColor) {
    this->mode = gameMode;
    
    switch (gameMode) {
        case HUMAN_VS_HUMAN:
            player1 = std::make_unique<HumanPlayer>(BLACK, "Player 1");
            player2 = std::make_unique<HumanPlayer>(WHITE, "Player 2");
            break;
            
        case HUMAN_VS_AI:
            if (humanPreferredColor == BLACK) {
                // Human wants to be Black: keep Black as player1
                player1 = std::make_unique<HumanPlayer>(BLACK, "Human");
                player2 = std::make_unique<AIPlayer>(WHITE, 2, "AI");
            } else {
                // Human wants to be White: player1 must remain Black (AI), human is player2 as White
                player1 = std::make_unique<AIPlayer>(BLACK, 2, "AI");
                player2 = std::make_unique<HumanPlayer>(WHITE, "Human");
            }
            break;
            
        case AI_VS_AI:
            player1 = std::make_unique<AIPlayer>(BLACK, 1, "AI 1");
            player2 = std::make_unique<AIPlayer>(WHITE, 2, "AI 2");
            break;
    }
    
    currentPlayer = player1.get();
}

IPlayer* Game::getOpponent() const {
    if (currentPlayer == player1.get()) {
        return player2.get();
    }
    return player1.get();
}

void Game::startNewGame(GameMode gameMode, CellState humanPreferredColor) {
    reset();
    clearHistory();  // Clear move history for new game
    setupPlayers(gameMode, humanPreferredColor);
    state = PLAYING;
    if (mode == AI_VS_AI && player1 && player2) {
        // Message spécifique pour AI vs AI
        statusMessage = player1->getName() + std::string(" vs ") + player2->getName() + std::string(" | simulation in progress");
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
    winningAlignment.clear();  // Clear winning alignment
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
                statusMessage = player1->getName() + std::string(" vs ") + player2->getName() + std::string(" | simulation in progress");
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
            
            // Highlight winning alignment if renderer is available
            if (renderer && !winningAlignment.empty()) {
                sf::Color highlightColor = (winner == BLACK) ? sf::Color(255, 215, 0, 200) : sf::Color(255, 255, 255, 200); // Gold for black, bright white for white
                renderer->highlightWinningAlignment(winningAlignment, highlightColor);
                std::cout << "[Highlight] Winning alignment highlighted with " << winningAlignment.size() << " stones" << std::endl;
            }
        }
    }
    
    // Notify players
    if (player1) player1->onGameEnd(player1->getColor() == winner);
    if (player2) player2->onGameEnd(player2->getColor() == winner);
}

// Rendering methods
void Game::render() {
    try {
        window.clear(sf::Color(240, 240, 240));
        
        // Use enhanced renderer if available, otherwise fallback
        if (renderer)
            renderer->render();
        // Always render enhanced UI components
        renderEnhancedUI();
        
        window.display();
    } catch (const std::exception& e) {
        std::cerr << "[Render] Exception: " << e.what() << std::endl;
        // Try to keep the window alive with a minimal frame
        try {
            window.display();
        } catch (...) {
            // As last resort, close window but keep process alive
            running = false;
        }
    }
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
    }
}

void Game::stopAITimer() {
    if (aiTimer) {
        aiTimer->stopMoveTimer();
        
        std::cout << "AI move completed in " << aiTimer->getFormattedCurrentTime() << std::endl;
    }
}

void Game::updateAITimer() {
    if (aiTimer) {
        aiTimer->update();
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
            std::cout << "Suggestion: " << suggestion.x << "," << suggestion.y << std::endl;
            renderer->highlightMove(suggestion, sf::Color(255, 255, 0, 128)); // Semi-transparent yellow
        }
        
        statusMessage = "Suggestion: " + std::to_string(suggestion.x) + "," + std::to_string(suggestion.y);
    }
}

Position Game::getAISuggestion(CellState player) {
    // Create a temporary AI with shallow depth for quick suggestion
    MinMaxAI tempAI(player, 10);

    // Get AI suggestion
    Position suggestion = tempAI.makeMove(board);

    return suggestion;
}

// ============================================================================
// Move History and Replay System
// ============================================================================

void Game::reconstructBoardAtMove(int moveIndex) {
    // Clear the board
    board.clear();
    board.setCaptureCount(BLACK, 0);
    board.setCaptureCount(WHITE, 0);

    // Replay all moves up to moveIndex
    for (int i = 0; i <= moveIndex && i < static_cast<int>(moveHistory.size()); i++) {
        const MoveRecord& record = moveHistory[i];

        // Place the piece
        board.placePiece(record.position.x, record.position.y, record.player);

        // Execute captures
        if (!record.captures.empty()) {
            board.executeCaptures(record.captures);
        }
    }

    // Restore capture counts to the state at moveIndex
    if (moveIndex >= 0 && moveIndex < static_cast<int>(moveHistory.size())) {
        const MoveRecord& record = moveHistory[moveIndex];
        board.setCaptureCount(BLACK, record.blackCaptures);
        board.setCaptureCount(WHITE, record.whiteCaptures);
    }
}

void Game::undoMove() {
    if (!canUndo()) {
        std::cout << "[Replay] Cannot undo - at beginning of history" << std::endl;
        return;
    }

    replayMode = true;
    currentMoveIndex--;

    // Reconstruct board at previous move
    if (currentMoveIndex >= 0) {
        reconstructBoardAtMove(currentMoveIndex);
    } else {
        // Back to empty board
        board.clear();
        board.setCaptureCount(BLACK, 0);
        board.setCaptureCount(WHITE, 0);
    }

    statusMessage = "Replay: Move " + std::to_string(currentMoveIndex + 1) +
                    "/" + std::to_string(moveHistory.size());
    std::cout << "[Replay] " << statusMessage << std::endl;
}

void Game::redoMove() {
    if (!canRedo()) {
        std::cout << "[Replay] Cannot redo - at end of history" << std::endl;
        return;
    }

    replayMode = true;
    currentMoveIndex++;

    // Reconstruct board at next move
    reconstructBoardAtMove(currentMoveIndex);

    statusMessage = "Replay: Move " + std::to_string(currentMoveIndex + 1) +
                    "/" + std::to_string(moveHistory.size());
    std::cout << "[Replay] " << statusMessage << std::endl;
}

void Game::goToMove(int index) {
    if (index < 0 || index >= static_cast<int>(moveHistory.size())) {
        std::cout << "[Replay] Invalid move index: " << index << std::endl;
        return;
    }

    replayMode = true;
    currentMoveIndex = index;

    // Reconstruct board at target move
    reconstructBoardAtMove(currentMoveIndex);

    statusMessage = "Replay: Move " + std::to_string(currentMoveIndex + 1) +
                    "/" + std::to_string(moveHistory.size());
    std::cout << "[Replay] Jumped to " << statusMessage << std::endl;
}

void Game::exitReplayMode() {
    if (!replayMode) {
        std::cout << "[Replay] Not in replay mode" << std::endl;
        return;
    }

    replayMode = false;

    // Go to last move in history
    if (!moveHistory.empty()) {
        currentMoveIndex = static_cast<int>(moveHistory.size()) - 1;
        reconstructBoardAtMove(currentMoveIndex);
    }

    statusMessage = "Exited replay mode - Game resumed";
    std::cout << "[Replay] " << statusMessage << std::endl;
}

void Game::clearHistory() {
    moveHistory.clear();
    currentMoveIndex = -1;
    replayMode = false;
    std::cout << "[Replay] History cleared" << std::endl;
}