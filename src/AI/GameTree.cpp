/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GameTree.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AI/GameTree.hpp"
#include "AI/Heuristic.hpp"

GameTree::GameTree(Heuristic* heuristicEvaluator) 
    : heuristic(heuristicEvaluator), maxDepth(4) {
}

GameTree::~GameTree() {
}

void GameTree::buildTree(const Board& rootBoard, CellState startingPlayer, int depth) {
    // Simplified stub - no actual tree building
    (void)rootBoard;
    (void)startingPlayer;
    (void)depth;
}

std::vector<Position> GameTree::generateMoves(const Board& board, MoveGenerationStrategy strategy) {
    // Simplified move generation - delegate to heuristic or return basic moves
    (void)strategy;
    if (heuristic) {
        return heuristic->getRelevantMoves(board);
    }
    
    // Fallback: return center if empty board
    std::vector<Position> moves;
    int center = board.getSize() / 2;
    if (board.isValidMove(center, center)) {
        moves.push_back(Position(center, center));
    }
    return moves;
}

std::vector<Position> GameTree::generateTacticalMoves(const Board& board) {
    // Simplified tactical moves - delegate to regular move generation
    return generateMoves(board, TACTICAL_ONLY);
}

std::vector<Position> GameTree::generateThreatMoves(const Board& board, CellState player) {
    // Simplified threat moves - delegate to heuristic or return empty
    if (heuristic) {
        return heuristic->findWinningMoves(board, player);
    }
    return std::vector<Position>();
}

int GameTree::getMoveOrderingScore(const Board& board, const Position& move, CellState player) {
    // Simplified move scoring - delegate to heuristic or return neutral
    if (heuristic) {
        return heuristic->evaluateMove(board, move, player);
    }
    return 0;
}