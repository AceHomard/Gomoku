/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MinMaxAI.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINMAXAI_HPP
#define MINMAXAI_HPP

#include "../Game/Player.hpp"
#include "../Game/Board.hpp"
#include "Heuristic.hpp"
#include <memory>

class MinMaxAI : public Player {
private:
    int searchDepth;
    std::unique_ptr<Heuristic> heuristic;

public:
    MinMaxAI(CellState playerColor, int depth = 4, 
             const std::string& playerName = "MinMax AI");
    virtual ~MinMaxAI();
    
    // Override Player methods
    virtual Position makeMove(const Board& board) override;
    virtual void onGameStart() override;
    virtual void onGameEnd(bool won) override;
    
    // AI-specific configuration
    void setSearchDepth(int depth) { searchDepth = std::max(1, depth); }
    int getSearchDepth() const { return searchDepth; }
    
private:
    // Core minimax algorithm
    int minimax(const Board& board, int depth, bool maximizing);
    
    // Move generation
    std::vector<Position> generateMoves(const Board& board);
    
    // Terminal node evaluation
    bool isTerminalNode(const Board& board, int depth);
    int evaluateBoard(const Board& board);
};

#endif // MINMAXAI_HPP