"""
AI Player implementation using Minimax algorithm
"""

import time
from game.player import Player
from .minimax import MinimaxAI

class AIPlayer(Player):
    """AI player using Minimax algorithm"""
    
    def __init__(self, color, name="AI", difficulty="normal"):
        super().__init__(color, name)
        self.difficulty = difficulty
        self.ai_engine = self.create_ai_engine()
        self.thinking_time = 0.0
        self.last_move_stats = {}
    
    def create_ai_engine(self):
        """Create AI engine based on difficulty"""
        if self.difficulty == "easy":
            return MinimaxAI(max_depth=6, time_limit=0.3)
        elif self.difficulty == "normal":
            return MinimaxAI(max_depth=10, time_limit=0.45)
        elif self.difficulty == "hard":
            return MinimaxAI(max_depth=12, time_limit=0.45)
        else:
            return MinimaxAI(max_depth=10, time_limit=0.45)
    
    def get_move(self, board):
        """Get the best move from AI"""
        start_time = time.time()
        
        try:
            move = self.ai_engine.get_best_move(board, self.color)
            self.thinking_time = time.time() - start_time
            self.last_move_stats = self.ai_engine.get_search_stats()
            
            return move
            
        except Exception as e:
            print(f"AI Error: {e}")
            # Fallback to simple move selection
            return self.get_fallback_move(board)
    
    def get_fallback_move(self, board):
        """Simple fallback move selection"""
        # Try center first
        center = board.BOARD_SIZE // 2
        if board.is_empty(center, center):
            return (center, center)
        
        # Find any empty adjacent to existing stones
        for row in range(board.BOARD_SIZE):
            for col in range(board.BOARD_SIZE):
                if board.get_stone(row, col) != board.EMPTY:
                    # Check adjacent positions
                    for dr in [-1, 0, 1]:
                        for dc in [-1, 0, 1]:
                            if dr == 0 and dc == 0:
                                continue
                            nr, nc = row + dr, col + dc
                            if board.is_valid_position(nr, nc) and board.is_empty(nr, nc):
                                return (nr, nc)
        
        # Last resort: any empty position
        for row in range(board.BOARD_SIZE):
            for col in range(board.BOARD_SIZE):
                if board.is_empty(row, col):
                    return (row, col)
        
        return None
    
    def get_thinking_time(self):
        """Get the time taken for the last move"""
        return self.thinking_time
    
    def get_last_stats(self):
        """Get statistics from the last move"""
        return self.last_move_stats
    
    def set_difficulty(self, difficulty):
        """Change AI difficulty"""
        self.difficulty = difficulty
        self.ai_engine = self.create_ai_engine()

class SuggestiveAI:
    """AI that provides move suggestions for human players"""
    
    def __init__(self, difficulty="normal"):
        self.ai_engine = MinimaxAI(max_depth=8, time_limit=0.2)  # Faster for suggestions
    
    def suggest_move(self, board, player):
        """Suggest the best move for a human player"""
        try:
            move = self.ai_engine.get_best_move(board, player)
            return move
        except:
            return None
    
    def get_top_moves(self, board, player, count=3):
        """Get top N move suggestions"""
        legal_moves = []
        for row in range(board.BOARD_SIZE):
            for col in range(board.BOARD_SIZE):
                if board.is_empty(row, col):
                    legal_moves.append((row, col))
        
        if not legal_moves:
            return []
        
        # Use heuristic to evaluate moves quickly
        heuristic = self.ai_engine.heuristic
        move_scores = []
        
        for move in legal_moves:
            row, col = move
            urgency = heuristic.get_move_urgency(board, row, col, player)
            move_scores.append((urgency, move))
        
        # Sort and return top moves
        move_scores.sort(reverse=True)
        return [move for _, move in move_scores[:count]]