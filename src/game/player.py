"""
Gomoku Player Classes
"""

class Player:
    """Base player class"""
    
    def __init__(self, color, name="Player"):
        self.color = color  # Board.BLACK or Board.WHITE
        self.name = name
        self.captured_pairs = 0
    
    def get_move(self, board):
        """Get the next move - to be implemented by subclasses"""
        raise NotImplementedError("Subclasses must implement get_move")
    
    def __str__(self):
        return f"{self.name} ({'Black' if self.color == 1 else 'White'})"

class HumanPlayer(Player):
    """Human player - moves come from UI input"""
    
    def __init__(self, color, name="Human"):
        super().__init__(color, name)
        self.pending_move = None
    
    def set_move(self, row, col):
        """Set the move from UI input"""
        self.pending_move = (row, col)
    
    def get_move(self, board):
        """Get the pending move"""
        if self.pending_move is not None:
            move = self.pending_move
            self.pending_move = None
            return move
        return None