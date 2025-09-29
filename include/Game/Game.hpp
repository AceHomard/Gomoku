/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Game.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GAME_HPP
#define GAME_HPP

#include "../Gomoku.hpp"
#include "Board.hpp"
#include "IPlayer.hpp"
#include "Rules.hpp"
#include "../UI/Timer.hpp"
#include "../UI/GameRenderer.hpp"
#include <memory>
#include <vector>

// Forward declarations
class HumanPlayer;
class AIPlayer;

enum GameMode {
    HUMAN_VS_HUMAN = 0,
    HUMAN_VS_AI = 1,
    AI_VS_AI = 2
};

enum GameState {
    MENU = 0,
    PLAYING = 1,
    GAME_OVER = 2,
    PAUSED = 3
};

class Game {
private:
    // SFML components
    sf::RenderWindow window;
    sf::Font font;
    sf::Clock gameClock;
    
    // Enhanced UI components (MANDATORY TIMER)
    std::unique_ptr<AITimer> aiTimer;
    std::unique_ptr<GameRenderer> renderer;
    bool debugMode;
    
    // Game components
    Board board;
    std::unique_ptr<IPlayer> player1;
    std::unique_ptr<IPlayer> player2;
    IPlayer* currentPlayer;
    
    // Game state
    GameState state;
    GameMode mode;
    bool running;
    bool gameWon;
    CellState winner;
    std::string statusMessage;
   
    // Move suggestion system
    Position currentSuggestion;
    bool suggestionActive;
    
    // UI elements
    float cellSize;
    sf::Vector2f boardOffset;

public:
    Game();
    ~Game();
    
    // Main game loop
    void run();
    void handleEvents();
    void update();
    void render();
    
    // Game management
    void startNewGame(GameMode gameMode, CellState humanPreferredColor = BLACK);
    void reset();
    void pause();
    void resume();
    void endGame(CellState winnerColor);
    
    // Move processing
    bool makeMove(int x, int y);
    bool processMove(Position move, IPlayer* player);
    
    // Player management
    void switchPlayer();
    void setupPlayers(GameMode gameMode, CellState humanPreferredColor = BLACK);
    IPlayer* getCurrentPlayer() const { return currentPlayer; }
    IPlayer* getOpponent() const;
    
    // Game state queries
    bool isGameRunning() const { return running && state == PLAYING; }
    bool isGameOver() const { return state == GAME_OVER; }
    GameState getGameState() const { return state; }
    GameMode getGameMode() const { return mode; }
    CellState getWinner() const { return winner; }
    const std::string& getStatusMessage() const { return statusMessage; }
    
    // Move suggestion for hotseat mode
    void showMoveSuggestion();
    Position getAISuggestion(CellState player);
    
    // UI and rendering (enhanced)

    void renderEnhancedUI(); // New enhanced rendering
    Position getMouseBoardPosition(int mouseX, int mouseY) const;
    
    // Timer management (MANDATORY)
    void startAITimer();
    void stopAITimer();
    void updateAITimer();
    
    // Event handling (enhanced)
    void handleMouseClick(int x, int y);
    void handleKeyPress(sf::Keyboard::Key key);
    void handleWindowEvents(const sf::Event& event);
    
private:
    // Initialization
    bool initializeWindow();
    bool loadResources();
    void calculateBoardDimensions();
    
    // Game logic helpers
    bool validateMove(Position move, IPlayer* player);
    void executeCaptures(const std::vector<Position>& captures, IPlayer* player);
    void checkWinCondition(IPlayer* player);
    void updateGameState();
    
    // AI handling
    void handleAIMove();
    bool waitForAIMove();
    
    // UI helpers
    sf::Vector2f getBoardPosition(int x, int y) const;
    sf::Color getPlayerColor(CellState player) const;
    std::string getPlayerName(CellState player) const;
    
    // Constants
    static const float MIN_CELL_SIZE;
    static const float MAX_CELL_SIZE;
    static const int UI_PANEL_WIDTH;
    static const int BOARD_MARGIN;
};

#endif // GAME_HPP