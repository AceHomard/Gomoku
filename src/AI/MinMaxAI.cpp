/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MinMaxAI.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AI/MinMaxAI.hpp"
#include "Game/Rules.hpp"
#include <algorithm>
#include <limits>
#include <iostream>

const int WIN_VALUE = 10000;
const int LOSE_VALUE = -10000;

MinMaxAI::MinMaxAI(CellState playerColor, int depth, const std::string& playerName)
    : Player(playerColor, AI, playerName), searchDepth(std::max(1, depth)),
      heuristic(std::make_unique<Heuristic>()) {
    
    std::cout << "[MinMaxAI] Initialized with depth=" << searchDepth << std::endl;
}

MinMaxAI::~MinMaxAI() {
}

Position MinMaxAI::makeMove(const Board& board) {
    std::cout << "[MinMaxAI] Thinking..." << std::endl;
    
    std::vector<Position> moves = generateMoves(board);
    if (moves.empty()) {
        return Position(-1, -1);
    }
    
    Position bestMove = moves[0];
    int bestValue = std::numeric_limits<int>::min();
    
    for (const Position& move : moves) {
        Board tempBoard = board;
        if (tempBoard.placePiece(move.x, move.y, color)) {
            // Handle captures
            std::vector<Position> captures = tempBoard.checkCaptures(move.x, move.y, color);
            tempBoard.executeCaptures(captures);
            
            int value = minimax(tempBoard, searchDepth - 1, false);
            
            if (value > bestValue) {
                bestValue = value;
                bestMove = move;
            }
        }
    }
    
    std::cout << "[MinMaxAI] Selected move: (" << bestMove.x << ", " << bestMove.y << ")" << std::endl;
    return bestMove;
}

int MinMaxAI::minimax(const Board& board, int depth, bool maximizing) {
    if (isTerminalNode(board, depth)) {
        return evaluateBoard(board);
    }
    
    std::vector<Position> moves = generateMoves(board);
    if (moves.empty()) {
        return evaluateBoard(board);
    }
    
    if (maximizing) {
        int maxValue = std::numeric_limits<int>::min();
        for (const Position& move : moves) {
            Board tempBoard = board;
            if (tempBoard.placePiece(move.x, move.y, color)) {
                std::vector<Position> captures = tempBoard.checkCaptures(move.x, move.y, color);
                tempBoard.executeCaptures(captures);
                
                int value = minimax(tempBoard, depth - 1, false);
                maxValue = std::max(maxValue, value);
            }
        }
        return maxValue;
    } else {
        int minValue = std::numeric_limits<int>::max();
        CellState opponentColor = (color == BLACK) ? WHITE : BLACK;
        for (const Position& move : moves) {
            Board tempBoard = board;
            if (tempBoard.placePiece(move.x, move.y, opponentColor)) {
                std::vector<Position> captures = tempBoard.checkCaptures(move.x, move.y, opponentColor);
                tempBoard.executeCaptures(captures);
                
                int value = minimax(tempBoard, depth - 1, true);
                minValue = std::min(minValue, value);
            }
        }
        return minValue;
    }
}

std::vector<Position> MinMaxAI::generateMoves(const Board& board) {
    return heuristic->getRelevantMoves(board);
}

bool MinMaxAI::isTerminalNode(const Board& board, int depth) {
    if (depth <= 0) return true;
    
    // Check for wins
    if (board.checkWin(color) || board.checkWin(getOpponentColor())) {
        return true;
    }
    
    // Check for capture wins
    if (board.checkCaptureWin(color) || board.checkCaptureWin(getOpponentColor())) {
        return true;
    }
    
    return false;
}

int MinMaxAI::evaluateBoard(const Board& board) {
    if (board.checkWin(color) || board.checkCaptureWin(color)) {
        return WIN_VALUE;
    }
    if (board.checkWin(getOpponentColor()) || board.checkCaptureWin(getOpponentColor())) {
        return LOSE_VALUE;
    }
    
    return heuristic->evaluatePosition(board, color);
}

void MinMaxAI::onGameStart() {
    std::cout << "[MinMaxAI] Game started, AI is ready" << std::endl;
}

void MinMaxAI::onGameEnd(bool won) {
    std::cout << "[MinMaxAI] Game ended - " << (won ? "Victory!" : "Defeat") << std::endl;
}

