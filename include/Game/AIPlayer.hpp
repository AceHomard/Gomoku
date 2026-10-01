#ifndef AIPLAYER_HPP
#define AIPLAYER_HPP

#include "IPlayer.hpp"
#include <memory>

// Forward declaration
class MinMaxAI;

class AIPlayer : public IPlayer {
private:
    int difficulty;
    int searchDepth;
    std::unique_ptr<MinMaxAI> minMaxEngine;

public:
    AIPlayer(CellState playerColor, int difficultyLevel = 1, const std::string& playerName = "AI");
    virtual ~AIPlayer();
    
    virtual Position makeMove(const Board& board) override;
    virtual void onGameStart() override;
    virtual void onGameEnd(bool won) override;
    
    // AI-specific configuration
    void setDifficulty(int level);
    int getDifficulty() const { return difficulty; }
    void setSearchDepth(int depth);
    int getSearchDepth() const;
    MinMaxAI* getMinMaxEngine() const { return minMaxEngine.get(); }
    int getMaxDepthEverReached() const;
};

#endif // AIPLAYER_HPP