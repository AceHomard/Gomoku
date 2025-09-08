"""
Main Gomoku Game Logic
"""

from .board import Board
from .rules import GameRules
from .player import Player

class GameState:
    """Enum for game states"""
    PLAYING = "playing"
    BLACK_WIN_ALIGNMENT = "black_win_alignment"
    WHITE_WIN_ALIGNMENT = "white_win_alignment"
    BLACK_WIN_CAPTURE = "black_win_capture"
    WHITE_WIN_CAPTURE = "white_win_capture"
    DRAW = "draw"

class GomokuGame:
    """Main Gomoku game class"""
    
    def __init__(self, player_black=None, player_white=None):
        self.board = Board()
        self.current_player = Board.BLACK
        self.state = GameState.PLAYING
        self.winner = None
        self.move_count = 0
        
        # Players
        self.players = {
            Board.BLACK: player_black,
            Board.WHITE: player_white
        }
        
        # Game history for debugging
        self.game_log = []
    
    def switch_player(self):
        """Switch to the other player"""
        self.current_player = Board.WHITE if self.current_player == Board.BLACK else Board.BLACK
    
    def is_valid_move(self, row, col):
        """Check if a move is valid"""
        if not self.board.is_empty(row, col):
            return False, "Position is not empty"
        
        # Check double-three rule (only applies to current player)
        if GameRules.check_double_three_forbidden(self.board, row, col, self.current_player):
            return False, "Move creates forbidden double-three"
        
        return True, "Valid move"
    
    def make_move(self, row, col):
        """Make a move and update game state"""
        if self.state != GameState.PLAYING:
            return False, "Game is over"
        
        valid, message = self.is_valid_move(row, col)
        if not valid:
            return False, message
        
        # Place the stone
        if not self.board.place_stone(row, col, self.current_player):
            return False, "Failed to place stone"
        
        self.move_count += 1
        
        # Log the move
        player_name = "Black" if self.current_player == Board.BLACK else "White"
        self.game_log.append(f"Move {self.move_count}: {player_name} plays ({row}, {col})")
        
        # Check for captures
        captures = GameRules.find_captures(self.board, row, col, self.current_player)
        if captures:
            captured_count = GameRules.apply_captures(self.board, captures, self.current_player)
            self.game_log.append(f"  Captured {captured_count} stones")
            
            # Check capture win condition
            if GameRules.check_capture_win(self.board, self.current_player):
                self.state = GameState.BLACK_WIN_CAPTURE if self.current_player == Board.BLACK else GameState.WHITE_WIN_CAPTURE
                self.winner = self.current_player
                return True, f"Game over: {player_name} wins by capture!"
        
        # Check alignment win condition
        if GameRules.check_alignment_win(self.board, row, col, self.current_player):
            # Check if alignment can be broken by capture (endgame capture rule)
            alignment_positions = GameRules.get_alignment_positions(self.board, row, col, self.current_player, 5)
            opponent = Board.WHITE if self.current_player == Board.BLACK else Board.BLACK
            
            can_break = False
            for alignment in alignment_positions:
                if GameRules.can_break_alignment_by_capture(self.board, alignment, opponent):
                    can_break = True
                    break
            
            # Check if current player would lose by capture if opponent captures
            opponent_captured = self.board.captured_black if self.current_player == Board.BLACK else self.board.captured_white
            if can_break and opponent_captured >= 8:  # If opponent already captured 4 pairs (8 stones)
                # The alignment doesn't win because opponent can capture and win
                pass
            else:
                self.state = GameState.BLACK_WIN_ALIGNMENT if self.current_player == Board.BLACK else GameState.WHITE_WIN_ALIGNMENT
                self.winner = self.current_player
                return True, f"Game over: {player_name} wins by alignment!"
        
        # Switch players
        self.switch_player()
        return True, "Move made successfully"
    
    def get_current_player_name(self):
        """Get the name of the current player"""
        return "Black" if self.current_player == Board.BLACK else "White"
    
    def is_game_over(self):
        """Check if the game is over"""
        return self.state != GameState.PLAYING
    
    def get_winner_name(self):
        """Get the name of the winner"""
        if self.winner == Board.BLACK:
            return "Black"
        elif self.winner == Board.WHITE:
            return "White"
        return None
    
    def get_game_info(self):
        """Get current game information"""
        info = {
            'current_player': self.get_current_player_name(),
            'move_count': self.move_count,
            'black_captured': self.board.captured_white,  # Black's captured white stones
            'white_captured': self.board.captured_black,  # White's captured black stones
            'state': self.state,
            'winner': self.get_winner_name()
        }
        return info
    
    def reset_game(self):
        """Reset the game to initial state"""
        self.board = Board()
        self.current_player = Board.BLACK
        self.state = GameState.PLAYING
        self.winner = None
        self.move_count = 0
        self.game_log = []
    
    def get_legal_moves(self):
        """Get all legal moves for the current player"""
        legal_moves = []
        for row in range(Board.BOARD_SIZE):
            for col in range(Board.BOARD_SIZE):
                if self.is_valid_move(row, col)[0]:
                    legal_moves.append((row, col))
        return legal_moves