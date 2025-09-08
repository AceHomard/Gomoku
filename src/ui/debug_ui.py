"""
Debug UI for AI analysis and visualization
"""

import pygame
import math

class DebugUI:
    """Debug interface to visualize AI thinking"""
    
    def __init__(self, font, small_font):
        self.font = font
        self.small_font = small_font
        self.enabled = False
        self.ai_stats = {}
        self.suggested_moves = []
        self.evaluation_grid = {}
        
    def toggle_debug(self):
        """Toggle debug mode on/off"""
        self.enabled = not self.enabled
        
    def update_ai_stats(self, stats):
        """Update AI statistics"""
        self.ai_stats = stats
        
    def update_suggested_moves(self, moves):
        """Update suggested moves list"""
        self.suggested_moves = moves
        
    def update_evaluation_grid(self, evaluations):
        """Update position evaluations for visualization"""
        self.evaluation_grid = evaluations
        
    def draw_debug_info(self, surface, x, y):
        """Draw debug information panel"""
        if not self.enabled:
            return
            
        debug_y = y
        
        # Title
        title = self.font.render("Debug Info", True, (0, 0, 0))
        surface.blit(title, (x, debug_y))
        debug_y += 30
        
        # AI Statistics
        if self.ai_stats:
            stats_text = [
                f"Nodes: {self.ai_stats.get('nodes_evaluated', 0)}",
                f"Time: {self.ai_stats.get('time_used', 0):.3f}s",
                f"TT Entries: {self.ai_stats.get('transposition_entries', 0)}"
            ]
            
            for stat in stats_text:
                text_surface = self.small_font.render(stat, True, (0, 0, 0))
                surface.blit(text_surface, (x, debug_y))
                debug_y += 20
        
        debug_y += 10
        
        # Suggested moves
        if self.suggested_moves:
            suggestion_title = self.font.render("Top Moves:", True, (0, 0, 0))
            surface.blit(suggestion_title, (x, debug_y))
            debug_y += 25
            
            for i, (row, col) in enumerate(self.suggested_moves[:5]):
                move_text = f"{i+1}. ({row},{col})"
                text_surface = self.small_font.render(move_text, True, (0, 0, 0))
                surface.blit(text_surface, (x, debug_y))
                debug_y += 18
    
    def draw_move_suggestions(self, surface, board_ui):
        """Draw move suggestions on the board"""
        if not self.enabled or not self.suggested_moves:
            return
            
        colors = [
            (255, 0, 0),    # Red for best move
            (255, 165, 0),  # Orange for 2nd best
            (255, 255, 0),  # Yellow for 3rd best
            (0, 255, 0),    # Green for 4th best
            (0, 0, 255)     # Blue for 5th best
        ]
        
        for i, (row, col) in enumerate(self.suggested_moves[:5]):
            x, y = board_ui.get_screen_position(row, col)
            color = colors[i % len(colors)]
            
            # Draw suggestion circle
            pygame.draw.circle(surface, color, (x, y), board_ui.stone_radius // 2, 3)
            
            # Draw rank number
            rank_text = board_ui.font.render(str(i + 1), True, color)
            text_rect = rank_text.get_rect(center=(x, y))
            surface.blit(rank_text, text_rect)
    
    def draw_evaluation_heatmap(self, surface, board_ui, board):
        """Draw heatmap of position evaluations"""
        if not self.enabled or not self.evaluation_grid:
            return
            
        # Find min/max values for normalization
        values = list(self.evaluation_grid.values())
        if not values:
            return
            
        min_val = min(values)
        max_val = max(values)
        
        if max_val == min_val:
            return
        
        # Draw heatmap
        for (row, col), value in self.evaluation_grid.items():
            if board.is_empty(row, col):
                x, y = board_ui.get_screen_position(row, col)
                
                # Normalize value to 0-1
                normalized = (value - min_val) / (max_val - min_val)
                
                # Create color based on value (red for negative, green for positive)
                if value >= 0:
                    color = (0, int(255 * normalized), 0, 100)  # Green with alpha
                else:
                    color = (int(255 * (1 - normalized)), 0, 0, 100)  # Red with alpha
                
                # Draw semi-transparent rectangle
                rect_surface = pygame.Surface((board_ui.cell_size, board_ui.cell_size))
                rect_surface.set_alpha(100)
                rect_surface.fill(color[:3])
                
                rect_x = x - board_ui.cell_size // 2
                rect_y = y - board_ui.cell_size // 2
                surface.blit(rect_surface, (rect_x, rect_y))

class GameModeSelector:
    """UI component for selecting game modes"""
    
    def __init__(self, font):
        self.font = font
        self.modes = [
            ("human_vs_human", "Human vs Human"),
            ("human_vs_ai_easy", "Human vs AI (Easy)"),
            ("human_vs_ai_normal", "Human vs AI (Normal)"),
            ("human_vs_ai_hard", "Human vs AI (Hard)")
        ]
        self.current_mode = 0
        self.rect = pygame.Rect(0, 0, 200, 30)
        
    def draw(self, surface, x, y):
        """Draw the mode selector"""
        self.rect.x = x
        self.rect.y = y
        
        # Background
        pygame.draw.rect(surface, (200, 200, 200), self.rect)
        pygame.draw.rect(surface, (0, 0, 0), self.rect, 2)
        
        # Current mode text
        mode_text = self.modes[self.current_mode][1]
        text_surface = self.font.render(mode_text, True, (0, 0, 0))
        text_rect = text_surface.get_rect(center=self.rect.center)
        surface.blit(text_surface, text_rect)
        
        # Navigation arrows
        left_arrow = "<"
        right_arrow = ">"
        
        left_surface = self.font.render(left_arrow, True, (0, 0, 0))
        right_surface = self.font.render(right_arrow, True, (0, 0, 0))
        
        surface.blit(left_surface, (x - 20, y + 5))
        surface.blit(right_surface, (x + self.rect.width + 5, y + 5))
    
    def handle_click(self, pos):
        """Handle mouse click on mode selector"""
        x, y = pos
        
        # Check left arrow
        if x < self.rect.x - 20 and x > self.rect.x - 35:
            self.current_mode = (self.current_mode - 1) % len(self.modes)
            return True
        
        # Check right arrow
        elif x > self.rect.x + self.rect.width + 5 and x < self.rect.x + self.rect.width + 25:
            self.current_mode = (self.current_mode + 1) % len(self.modes)
            return True
            
        return False
    
    def get_current_mode(self):
        """Get the currently selected mode"""
        return self.modes[self.current_mode][0]