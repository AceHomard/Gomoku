"""
Gomoku Board Implementation
"""

class Board:
    """Represents a 19x19 Gomoku board"""
    
    EMPTY = 0
    BLACK = 1
    WHITE = 2
    BOARD_SIZE = 19
    
    def __init__(self):
        """Initialize an empty 19x19 board"""
        self.grid = [[self.EMPTY for _ in range(self.BOARD_SIZE)] for _ in range(self.BOARD_SIZE)]
        self.captured_black = 0
        self.captured_white = 0
        self.move_history = []
    
    def is_valid_position(self, row, col):
        """Check if position is within board boundaries"""
        return 0 <= row < self.BOARD_SIZE and 0 <= col < self.BOARD_SIZE
    
    def is_empty(self, row, col):
        """Check if a position is empty"""
        if not self.is_valid_position(row, col):
            return False
        return self.grid[row][col] == self.EMPTY
    
    def place_stone(self, row, col, player):
        """Place a stone on the board"""
        if not self.is_empty(row, col):
            return False
        
        self.grid[row][col] = player
        self.move_history.append((row, col, player))
        return True
    
    def remove_stone(self, row, col):
        """Remove a stone from the board"""
        if not self.is_valid_position(row, col):
            return False
        
        removed_player = self.grid[row][col]
        self.grid[row][col] = self.EMPTY
        return removed_player
    
    def get_stone(self, row, col):
        """Get the stone at a position"""
        if not self.is_valid_position(row, col):
            return None
        return self.grid[row][col]
    
    def get_empty_positions(self):
        """Get all empty positions on the board"""
        empty_positions = []
        for row in range(self.BOARD_SIZE):
            for col in range(self.BOARD_SIZE):
                if self.grid[row][col] == self.EMPTY:
                    empty_positions.append((row, col))
        return empty_positions
    
    def copy(self):
        """Create a deep copy of the board"""
        new_board = Board()
        new_board.grid = [row[:] for row in self.grid]
        new_board.captured_black = self.captured_black
        new_board.captured_white = self.captured_white
        new_board.move_history = self.move_history[:]
        return new_board
    
    def __str__(self):
        """String representation of the board"""
        result = "   " + " ".join(f"{i:2d}" for i in range(self.BOARD_SIZE)) + "\n"
        for i, row in enumerate(self.grid):
            row_str = f"{i:2d} "
            for cell in row:
                if cell == self.BLACK:
                    row_str += " ●"
                elif cell == self.WHITE:
                    row_str += " ○"
                else:
                    row_str += " ·"
            result += row_str + "\n"
        return result