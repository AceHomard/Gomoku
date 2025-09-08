"""
Heuristic evaluation function for Gomoku AI
"""

from game.board import Board
from game.rules import GameRules

class GomokuHeuristic:
    """Heuristic evaluation for Gomoku positions"""
    
    # Pattern scores
    SCORES = {
        # Alignment patterns
        'win': 100000,
        'open_four': 10000,
        'blocked_four': 1000,
        'open_three': 1000,
        'blocked_three': 100,
        'open_two': 100,
        'blocked_two': 10,
        'open_one': 10,
        
        # Capture patterns
        'capture_threat': 500,
        'capture_defense': 200,
        'capture_win': 50000,
        
        # Position values
        'center': 5,
        'edge': 1
    }
    
    DIRECTIONS = [
        (0, 1),   # Horizontal
        (1, 0),   # Vertical
        (1, 1),   # Diagonal \
        (1, -1)   # Diagonal /
    ]
    
    def __init__(self):
        self.pattern_cache = {}
    
    def evaluate_position(self, board, player):
        """Evaluate the board position for a given player"""
        opponent = Board.WHITE if player == Board.BLACK else Board.BLACK
        
        # Check terminal states first
        if board.captured_white >= 10 and player == Board.BLACK:
            return self.SCORES['capture_win']
        if board.captured_black >= 10 and player == Board.WHITE:
            return self.SCORES['capture_win']
        if board.captured_white >= 10 and player == Board.WHITE:
            return -self.SCORES['capture_win']
        if board.captured_black >= 10 and player == Board.BLACK:
            return -self.SCORES['capture_win']
        
        score = 0
        
        # Evaluate alignment patterns
        score += self.evaluate_alignments(board, player)
        score -= self.evaluate_alignments(board, opponent)
        
        # Evaluate capture opportunities
        score += self.evaluate_captures(board, player)
        score -= self.evaluate_captures(board, opponent)
        
        # Evaluate positional advantages
        score += self.evaluate_positions(board, player)
        score -= self.evaluate_positions(board, opponent)
        
        return score
    
    def evaluate_alignments(self, board, player):
        """Evaluate alignment patterns for a player"""
        score = 0
        
        for row in range(Board.BOARD_SIZE):
            for col in range(Board.BOARD_SIZE):
                if board.get_stone(row, col) == player:
                    # Check each direction from this stone
                    for dr, dc in self.DIRECTIONS:
                        pattern_score = self.evaluate_line_pattern(board, row, col, dr, dc, player)
                        score += pattern_score
        
        return score
    
    def evaluate_line_pattern(self, board, row, col, dr, dc, player):
        """Evaluate a line pattern in a specific direction"""
        # Count consecutive stones
        consecutive = 1
        open_ends = 0
        
        # Count forward
        r, c = row + dr, col + dc
        while board.is_valid_position(r, c) and board.get_stone(r, c) == player:
            consecutive += 1
            r, c = r + dr, c + dc
        
        # Check if forward end is open
        if board.is_valid_position(r, c) and board.get_stone(r, c) == Board.EMPTY:
            open_ends += 1
        
        # Count backward
        r, c = row - dr, col - dc
        while board.is_valid_position(r, c) and board.get_stone(r, c) == player:
            consecutive += 1
            r, c = r - dr, c - dc
        
        # Check if backward end is open
        if board.is_valid_position(r, c) and board.get_stone(r, c) == Board.EMPTY:
            open_ends += 1
        
        # Score based on pattern
        return self.score_pattern(consecutive, open_ends)
    
    def score_pattern(self, consecutive, open_ends):
        """Score a pattern based on consecutive stones and open ends"""
        if consecutive >= 5:
            return self.SCORES['win']
        elif consecutive == 4:
            if open_ends == 2:
                return self.SCORES['open_four']
            elif open_ends == 1:
                return self.SCORES['blocked_four']
        elif consecutive == 3:
            if open_ends == 2:
                return self.SCORES['open_three']
            elif open_ends == 1:
                return self.SCORES['blocked_three']
        elif consecutive == 2:
            if open_ends == 2:
                return self.SCORES['open_two']
            elif open_ends == 1:
                return self.SCORES['blocked_two']
        elif consecutive == 1:
            if open_ends > 0:
                return self.SCORES['open_one']
        
        return 0
    
    def evaluate_captures(self, board, player):
        """Evaluate capture opportunities"""
        score = 0
        opponent = Board.WHITE if player == Board.BLACK else Board.BLACK
        
        # Check capture opportunities for each empty position
        for row in range(Board.BOARD_SIZE):
            for col in range(Board.BOARD_SIZE):
                if board.get_stone(row, col) == Board.EMPTY:
                    captures = GameRules.find_captures(board, row, col, player)
                    if captures:
                        # Score based on number of captures
                        capture_count = len(captures) * 2  # Each capture is a pair
                        score += self.SCORES['capture_threat'] * capture_count
                        
                        # Bonus if this would win by capture
                        current_captured = board.captured_white if player == Board.BLACK else board.captured_black
                        if current_captured + capture_count >= 10:
                            score += self.SCORES['capture_win']
        
        return score
    
    def evaluate_positions(self, board, player):
        """Evaluate positional advantages"""
        score = 0
        center = Board.BOARD_SIZE // 2
        
        for row in range(Board.BOARD_SIZE):
            for col in range(Board.BOARD_SIZE):
                if board.get_stone(row, col) == player:
                    # Prefer center positions
                    distance_from_center = abs(row - center) + abs(col - center)
                    if distance_from_center <= 2:
                        score += self.SCORES['center']
                    else:
                        score += self.SCORES['edge']
        
        return score
    
    def get_move_urgency(self, board, row, col, player):
        """Get the urgency score for a specific move"""
        # Simulate the move
        temp_board = board.copy()
        temp_board.place_stone(row, col, player)
        
        urgency = 0
        opponent = Board.WHITE if player == Board.BLACK else Board.BLACK
        
        # Check if this move wins immediately
        if GameRules.check_alignment_win(temp_board, row, col, player):
            urgency += 10000
        
        # Check if this move prevents opponent from winning
        for opp_row in range(Board.BOARD_SIZE):
            for opp_col in range(Board.BOARD_SIZE):
                if board.is_empty(opp_row, opp_col):
                    temp_opp_board = board.copy()
                    temp_opp_board.place_stone(opp_row, opp_col, opponent)
                    if GameRules.check_alignment_win(temp_opp_board, opp_row, opp_col, opponent):
                        # Opponent can win, check if our move blocks it
                        if (opp_row, opp_col) == (row, col):
                            urgency += 5000
        
        # Check capture opportunities
        captures = GameRules.find_captures(board, row, col, player)
        if captures:
            urgency += len(captures) * 100
        
        return urgency
    
    def order_moves(self, board, moves, player):
        """Order moves by their potential value"""
        move_scores = []
        
        for row, col in moves:
            # Quick heuristic evaluation
            score = 0
            
            # Center preference
            center = Board.BOARD_SIZE // 2
            distance_from_center = abs(row - center) + abs(col - center)
            score -= distance_from_center
            
            # Adjacent to existing stones bonus
            for dr in [-1, 0, 1]:
                for dc in [-1, 0, 1]:
                    if dr == 0 and dc == 0:
                        continue
                    nr, nc = row + dr, col + dc
                    if board.is_valid_position(nr, nc) and board.get_stone(nr, nc) != Board.EMPTY:
                        score += 10
            
            # Urgency score
            score += self.get_move_urgency(board, row, col, player)
            
            move_scores.append((score, (row, col)))
        
        # Sort by score (descending)
        move_scores.sort(reverse=True)
        return [move for _, move in move_scores]