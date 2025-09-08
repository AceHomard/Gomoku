"""
Gomoku Game Rules Implementation
"""

from .board import Board

class GameRules:
    """Implements Gomoku game rules"""
    
    DIRECTIONS = [
        (-1, -1), (-1, 0), (-1, 1),  # Up-left, Up, Up-right
        (0, -1),           (0, 1),   # Left, Right
        (1, -1),  (1, 0),  (1, 1)    # Down-left, Down, Down-right
    ]
    
    @staticmethod
    def check_alignment_win(board, row, col, player, length=5):
        """Check if placing a stone creates an alignment of 5 or more"""
        for dr, dc in GameRules.DIRECTIONS:
            count = 1  # Count the placed stone
            
            # Count in positive direction
            r, c = row + dr, col + dc
            while board.is_valid_position(r, c) and board.get_stone(r, c) == player:
                count += 1
                r, c = r + dr, c + dc
            
            # Count in negative direction
            r, c = row - dr, col - dc
            while board.is_valid_position(r, c) and board.get_stone(r, c) == player:
                count += 1
                r, c = r - dr, c - dc
            
            if count >= length:
                return True
        
        return False
    
    @staticmethod
    def get_alignment_positions(board, row, col, player, length=5):
        """Get all positions that form alignments of given length or more"""
        alignments = []
        
        for dr, dc in GameRules.DIRECTIONS:
            positions = [(row, col)]  # Include the placed stone
            
            # Collect in positive direction
            r, c = row + dr, col + dc
            while board.is_valid_position(r, c) and board.get_stone(r, c) == player:
                positions.append((r, c))
                r, c = r + dr, c + dc
            
            # Collect in negative direction
            r, c = row - dr, col - dc
            while board.is_valid_position(r, c) and board.get_stone(r, c) == player:
                positions.insert(0, (r, c))  # Insert at beginning to maintain order
                r, c = r - dr, c - dc
            
            if len(positions) >= length:
                alignments.append(positions)
        
        return alignments
    
    @staticmethod
    def check_capture_win(board, player):
        """Check if a player has won by capturing 10 stones (5 pairs)"""
        if player == Board.BLACK:
            return board.captured_white >= 10
        else:
            return board.captured_black >= 10
    
    @staticmethod
    def find_captures(board, row, col, player):
        """Find all captures that would result from placing a stone"""
        captures = []
        opponent = Board.WHITE if player == Board.BLACK else Board.BLACK
        
        for dr, dc in GameRules.DIRECTIONS:
            # Check if there are exactly 2 opponent stones followed by our stone
            pos1 = (row + dr, col + dc)
            pos2 = (row + 2*dr, col + 2*dc)
            pos3 = (row + 3*dr, col + 3*dc)
            
            if (board.is_valid_position(*pos1) and board.is_valid_position(*pos2) and board.is_valid_position(*pos3) and
                board.get_stone(*pos1) == opponent and
                board.get_stone(*pos2) == opponent and
                board.get_stone(*pos3) == player):
                captures.append([pos1, pos2])
        
        return captures
    
    @staticmethod
    def apply_captures(board, captures, player):
        """Apply captures to the board and update capture count"""
        total_captured = 0
        
        for capture_pair in captures:
            for pos in capture_pair:
                removed = board.remove_stone(*pos)
                if removed != Board.EMPTY:
                    total_captured += 1
        
        # Update capture count
        if player == Board.BLACK:
            board.captured_white += total_captured
        else:
            board.captured_black += total_captured
        
        return total_captured
    
    @staticmethod
    def is_free_three(board, row, col, player, direction):
        """Check if a position creates a free three in a given direction"""
        dr, dc = direction
        
        # Count stones in both directions
        count_forward = 0
        count_backward = 0
        
        # Count forward
        r, c = row + dr, col + dc
        while board.is_valid_position(r, c) and board.get_stone(r, c) == player:
            count_forward += 1
            r, c = r + dr, c + dc
        
        # Count backward
        r, c = row - dr, col - dc
        while board.is_valid_position(r, c) and board.get_stone(r, c) == player:
            count_backward += 1
            r, c = r - dr, c - dc
        
        total_count = count_forward + count_backward + 1  # +1 for the placed stone
        
        # Check if it's exactly 3 and both ends are free
        if total_count == 3:
            # Check if both ends are free (empty and within board)
            end1 = (row + (count_forward + 1) * dr, col + (count_forward + 1) * dc)
            end2 = (row - (count_backward + 1) * dr, col - (count_backward + 1) * dc)
            
            if (board.is_valid_position(*end1) and board.is_empty(*end1) and
                board.is_valid_position(*end2) and board.is_empty(*end2)):
                return True
        
        return False
    
    @staticmethod
    def check_double_three_forbidden(board, row, col, player):
        """Check if placing a stone creates a forbidden double-three"""
        free_threes = 0
        
        for direction in GameRules.DIRECTIONS[:4]:  # Only check 4 directions to avoid duplicates
            if GameRules.is_free_three(board, row, col, player, direction):
                free_threes += 1
                if free_threes >= 2:
                    return True
        
        return False
    
    @staticmethod
    def can_break_alignment_by_capture(board, alignment_positions, opponent):
        """Check if an alignment can be broken by capturing a pair"""
        for i in range(len(alignment_positions) - 1):
            pos1, pos2 = alignment_positions[i], alignment_positions[i + 1]
            
            # Check all directions around these positions for potential captures
            for dr, dc in GameRules.DIRECTIONS:
                # Check if opponent can capture this pair
                capture_pos1 = (pos1[0] - dr, pos1[1] - dc)
                capture_pos2 = (pos2[0] + dr, pos2[1] + dc)
                
                if (board.is_valid_position(*capture_pos1) and board.is_empty(*capture_pos1) and
                    board.is_valid_position(*capture_pos2) and board.get_stone(*capture_pos2) == opponent):
                    return True
                    
                if (board.is_valid_position(*capture_pos2) and board.is_empty(*capture_pos2) and
                    board.is_valid_position(*capture_pos1) and board.get_stone(*capture_pos1) == opponent):
                    return True
        
        return False