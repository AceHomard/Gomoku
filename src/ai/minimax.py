"""
Minimax algorithm with Alpha-Beta pruning for Gomoku AI
"""

import time
import math
from game.board import Board
from game.rules import GameRules
from .heuristic import GomokuHeuristic

class MinimaxAI:
    """Minimax AI with Alpha-Beta pruning"""
    
    def __init__(self, max_depth=10, time_limit=0.45):
        self.max_depth = max_depth
        self.time_limit = time_limit  # Slightly under 0.5s to be safe
        self.heuristic = GomokuHeuristic()
        self.nodes_evaluated = 0
        self.start_time = 0
        self.transposition_table = {}
        self.killer_moves = {}
    
    def get_best_move(self, board, player):
        """Get the best move using iterative deepening minimax"""
        self.start_time = time.time()
        self.nodes_evaluated = 0
        self.transposition_table.clear()
        self.killer_moves.clear()
        
        legal_moves = self.get_legal_moves(board, player)
        if not legal_moves:
            return None
        
        if len(legal_moves) == 1:
            return legal_moves[0]
        
        best_move = legal_moves[0]
        best_score = float('-inf')
        
        # Iterative deepening
        for depth in range(1, self.max_depth + 1):
            if time.time() - self.start_time > self.time_limit * 0.8:
                break
            
            try:
                score, move = self.minimax_root(board, player, depth)
                if move is not None:
                    best_move = move
                    best_score = score
                
                # If we found a winning move, no need to search deeper
                if score >= 50000:
                    break
                    
            except TimeoutError:
                break
        
        return best_move
    
    def minimax_root(self, board, player, depth):
        """Root level minimax to get the best move"""
        legal_moves = self.get_legal_moves(board, player)
        
        # Order moves by heuristic value
        legal_moves = self.heuristic.order_moves(board, legal_moves, player)
        
        best_score = float('-inf')
        best_move = legal_moves[0]
        alpha = float('-inf')
        beta = float('+inf')
        
        for move in legal_moves:
            if self.is_timeout():
                raise TimeoutError("Time limit exceeded")
            
            row, col = move
            
            # Make the move
            new_board = board.copy()
            new_board.place_stone(row, col, player)
            
            # Apply captures
            captures = GameRules.find_captures(new_board, row, col, player)
            if captures:
                GameRules.apply_captures(new_board, captures, player)
            
            # Check for immediate win
            if self.is_terminal_state(new_board, player):
                return 100000, move
            
            # Recursive minimax
            opponent = Board.WHITE if player == Board.BLACK else Board.BLACK
            score = self.minimax(new_board, opponent, depth - 1, alpha, beta, False)
            
            if score > best_score:
                best_score = score
                best_move = move
            
            alpha = max(alpha, score)
            if beta <= alpha:
                break
        
        return best_score, best_move
    
    def minimax(self, board, player, depth, alpha, beta, is_maximizing):
        """Minimax algorithm with alpha-beta pruning"""
        if self.is_timeout():
            raise TimeoutError("Time limit exceeded")
        
        self.nodes_evaluated += 1
        
        # Check transposition table
        board_hash = self.hash_board(board)
        if board_hash in self.transposition_table:
            stored_depth, stored_score, stored_flag = self.transposition_table[board_hash]
            if stored_depth >= depth:
                if stored_flag == 'exact':
                    return stored_score
                elif stored_flag == 'lower' and stored_score >= beta:
                    return stored_score
                elif stored_flag == 'upper' and stored_score <= alpha:
                    return stored_score
        
        # Terminal state check
        if depth == 0 or self.is_terminal_state(board, player):
            score = self.heuristic.evaluate_position(board, player if is_maximizing else self.get_opponent(player))
            self.store_in_transposition_table(board_hash, depth, score, 'exact')
            return score
        
        legal_moves = self.get_legal_moves(board, player)
        if not legal_moves:
            score = self.heuristic.evaluate_position(board, player if is_maximizing else self.get_opponent(player))
            self.store_in_transposition_table(board_hash, depth, score, 'exact')
            return score
        
        # Order moves
        legal_moves = self.order_moves_with_killer(board, legal_moves, player, depth)
        
        if is_maximizing:
            max_score = float('-inf')
            for move in legal_moves:
                row, col = move
                
                # Make move
                new_board = board.copy()
                new_board.place_stone(row, col, player)
                captures = GameRules.find_captures(new_board, row, col, player)
                if captures:
                    GameRules.apply_captures(new_board, captures, player)
                
                opponent = self.get_opponent(player)
                score = self.minimax(new_board, opponent, depth - 1, alpha, beta, False)
                
                max_score = max(max_score, score)
                alpha = max(alpha, score)
                
                if beta <= alpha:
                    # Store killer move
                    self.add_killer_move(depth, move)
                    break
            
            # Store in transposition table
            flag = 'exact' if max_score > alpha else 'upper'
            self.store_in_transposition_table(board_hash, depth, max_score, flag)
            return max_score
        
        else:
            min_score = float('+inf')
            for move in legal_moves:
                row, col = move
                
                # Make move
                new_board = board.copy()
                new_board.place_stone(row, col, player)
                captures = GameRules.find_captures(new_board, row, col, player)
                if captures:
                    GameRules.apply_captures(new_board, captures, player)
                
                opponent = self.get_opponent(player)
                score = self.minimax(new_board, opponent, depth - 1, alpha, beta, True)
                
                min_score = min(min_score, score)
                beta = min(beta, score)
                
                if beta <= alpha:
                    # Store killer move
                    self.add_killer_move(depth, move)
                    break
            
            # Store in transposition table
            flag = 'exact' if min_score < beta else 'lower'
            self.store_in_transposition_table(board_hash, depth, min_score, flag)
            return min_score
    
    def get_legal_moves(self, board, player):
        """Get all legal moves, with smart pruning"""
        if self.is_early_game(board):
            return self.get_early_game_moves(board)
        
        moves = []
        
        # Get all empty positions near existing stones
        candidates = set()
        for row in range(Board.BOARD_SIZE):
            for col in range(Board.BOARD_SIZE):
                if board.get_stone(row, col) != Board.EMPTY:
                    # Add adjacent empty positions
                    for dr in range(-2, 3):
                        for dc in range(-2, 3):
                            nr, nc = row + dr, col + dc
                            if (board.is_valid_position(nr, nc) and 
                                board.is_empty(nr, nc) and
                                not GameRules.check_double_three_forbidden(board, nr, nc, player)):
                                candidates.add((nr, nc))
        
        return list(candidates) if candidates else [(9, 9)]  # Center if no moves
    
    def is_early_game(self, board):
        """Check if it's early in the game"""
        stone_count = 0
        for row in range(Board.BOARD_SIZE):
            for col in range(Board.BOARD_SIZE):
                if board.get_stone(row, col) != Board.EMPTY:
                    stone_count += 1
        return stone_count < 6
    
    def get_early_game_moves(self, board):
        """Get good moves for early game"""
        center = Board.BOARD_SIZE // 2
        moves = []
        
        # Prefer center and nearby positions
        for radius in range(4):
            for dr in range(-radius, radius + 1):
                for dc in range(-radius, radius + 1):
                    if abs(dr) == radius or abs(dc) == radius:  # Only boundary of current radius
                        row, col = center + dr, center + dc
                        if (board.is_valid_position(row, col) and 
                            board.is_empty(row, col)):
                            moves.append((row, col))
        
        return moves[:20]  # Limit early game moves
    
    def order_moves_with_killer(self, board, moves, player, depth):
        """Order moves with killer move heuristic"""
        ordered_moves = []
        killer_moves = self.killer_moves.get(depth, [])
        
        # Add killer moves first
        for move in killer_moves:
            if move in moves:
                ordered_moves.append(move)
                moves.remove(move)
        
        # Add remaining moves ordered by heuristic
        remaining_ordered = self.heuristic.order_moves(board, moves, player)
        ordered_moves.extend(remaining_ordered)
        
        return ordered_moves
    
    def add_killer_move(self, depth, move):
        """Add a killer move for a given depth"""
        if depth not in self.killer_moves:
            self.killer_moves[depth] = []
        
        if move not in self.killer_moves[depth]:
            self.killer_moves[depth].insert(0, move)  # Add at beginning
            if len(self.killer_moves[depth]) > 2:  # Keep only 2 killer moves per depth
                self.killer_moves[depth].pop()
    
    def hash_board(self, board):
        """Create a hash of the board state"""
        hash_val = 0
        for row in range(Board.BOARD_SIZE):
            for col in range(Board.BOARD_SIZE):
                stone = board.get_stone(row, col)
                hash_val = hash_val * 3 + stone
        return hash_val
    
    def store_in_transposition_table(self, board_hash, depth, score, flag):
        """Store position in transposition table"""
        if len(self.transposition_table) < 100000:  # Limit table size
            self.transposition_table[board_hash] = (depth, score, flag)
    
    def is_terminal_state(self, board, player):
        """Check if the current state is terminal"""
        # Check capture wins
        if board.captured_white >= 10 or board.captured_black >= 10:
            return True
        
        # Check for 5-in-a-row (simplified check)
        for row in range(Board.BOARD_SIZE):
            for col in range(Board.BOARD_SIZE):
                if board.get_stone(row, col) != Board.EMPTY:
                    if GameRules.check_alignment_win(board, row, col, board.get_stone(row, col)):
                        return True
        
        return False
    
    def get_opponent(self, player):
        """Get the opponent player"""
        return Board.WHITE if player == Board.BLACK else Board.BLACK
    
    def is_timeout(self):
        """Check if we've exceeded the time limit"""
        return time.time() - self.start_time > self.time_limit
    
    def get_search_stats(self):
        """Get search statistics"""
        return {
            'nodes_evaluated': self.nodes_evaluated,
            'time_used': time.time() - self.start_time,
            'transposition_entries': len(self.transposition_table)
        }