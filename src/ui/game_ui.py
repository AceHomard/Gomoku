"""
Pygame-based Gomoku Game Interface
"""

import pygame
import sys
from game.board import Board
from game.game import GomokuGame, GameState
from game.player import HumanPlayer
from ai.ai_player import AIPlayer, SuggestiveAI
from .timer import TimerDisplay
from .debug_ui import DebugUI, GameModeSelector

class GameUI:
    """Pygame-based graphical interface for Gomoku"""
    
    # Colors
    BACKGROUND_COLOR = (222, 184, 135)  # Tan/beige
    LINE_COLOR = (0, 0, 0)  # Black
    BLACK_STONE_COLOR = (0, 0, 0)  # Black
    WHITE_STONE_COLOR = (255, 255, 255)  # White
    TEXT_COLOR = (0, 0, 0)  # Black
    HIGHLIGHT_COLOR = (255, 0, 0)  # Red for highlights
    
    def __init__(self, width=1000, height=700):
        """Initialize the game UI"""
        self.width = width
        self.height = height
        
        # Initialize Pygame
        pygame.init()
        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption("Gomoku")
        
        # Fonts
        self.font = pygame.font.Font(None, 24)
        self.large_font = pygame.font.Font(None, 36)
        
        # Game components
        self.game = GomokuGame()
        self.human_black = HumanPlayer(Board.BLACK, "Human (Black)")
        self.human_white = HumanPlayer(Board.WHITE, "Human (White)")
        self.game.players[Board.BLACK] = self.human_black
        self.game.players[Board.WHITE] = self.human_white
        
        # UI components
        self.timer_display = TimerDisplay(self.font)
        self.debug_ui = DebugUI(self.font, pygame.font.Font(None, 18))
        self.mode_selector = GameModeSelector(self.font)
        self.suggestion_ai = SuggestiveAI()
        
        # Board display settings
        self.board_size = 600
        self.board_margin = 50
        self.cell_size = self.board_size // (Board.BOARD_SIZE + 1)
        self.stone_radius = self.cell_size // 3
        
        # Board position
        self.board_x = self.board_margin
        self.board_y = self.board_margin
        
        # Game state
        self.selected_position = None
        self.show_coordinates = True
        self.game_mode = "human_vs_human"
        self.ai_thinking = False
        self.show_suggestions = False
        
        # Clock for FPS
        self.clock = pygame.time.Clock()
        
    def get_board_position(self, mouse_x, mouse_y):
        """Convert mouse coordinates to board position"""
        # Calculate which intersection is closest
        rel_x = mouse_x - (self.board_x + self.cell_size)
        rel_y = mouse_y - (self.board_y + self.cell_size)
        
        col = round(rel_x / self.cell_size)
        row = round(rel_y / self.cell_size)
        
        # Check if position is valid
        if 0 <= row < Board.BOARD_SIZE and 0 <= col < Board.BOARD_SIZE:
            return row, col
        return None
    
    def get_screen_position(self, row, col):
        """Convert board position to screen coordinates"""
        x = self.board_x + self.cell_size + col * self.cell_size
        y = self.board_y + self.cell_size + row * self.cell_size
        return x, y
    
    def draw_board(self):
        """Draw the Gomoku board"""
        # Fill background
        self.screen.fill(self.BACKGROUND_COLOR)
        
        # Draw grid lines
        for i in range(Board.BOARD_SIZE):
            # Vertical lines
            x = self.board_x + self.cell_size + i * self.cell_size
            pygame.draw.line(self.screen, self.LINE_COLOR, 
                           (x, self.board_y + self.cell_size), 
                           (x, self.board_y + self.cell_size + (Board.BOARD_SIZE - 1) * self.cell_size), 2)
            
            # Horizontal lines
            y = self.board_y + self.cell_size + i * self.cell_size
            pygame.draw.line(self.screen, self.LINE_COLOR,
                           (self.board_x + self.cell_size, y),
                           (self.board_x + self.cell_size + (Board.BOARD_SIZE - 1) * self.cell_size, y), 2)
        
        # Draw coordinate labels if enabled
        if self.show_coordinates:
            for i in range(Board.BOARD_SIZE):
                # Column numbers (top)
                text = self.font.render(str(i), True, self.TEXT_COLOR)
                x = self.board_x + self.cell_size + i * self.cell_size - text.get_width() // 2
                self.screen.blit(text, (x, self.board_y))
                
                # Row numbers (left)
                text = self.font.render(str(i), True, self.TEXT_COLOR)
                y = self.board_y + self.cell_size + i * self.cell_size - text.get_height() // 2
                self.screen.blit(text, (self.board_x, y))
        
        # Draw star points (traditional go board markers)
        star_points = [(3, 3), (3, 9), (3, 15), (9, 3), (9, 9), (9, 15), (15, 3), (15, 9), (15, 15)]
        for row, col in star_points:
            x, y = self.get_screen_position(row, col)
            pygame.draw.circle(self.screen, self.LINE_COLOR, (x, y), 4)
    
    def draw_stones(self):
        """Draw all stones on the board"""
        for row in range(Board.BOARD_SIZE):
            for col in range(Board.BOARD_SIZE):
                stone = self.game.board.get_stone(row, col)
                if stone != Board.EMPTY:
                    x, y = self.get_screen_position(row, col)
                    
                    if stone == Board.BLACK:
                        pygame.draw.circle(self.screen, self.BLACK_STONE_COLOR, (x, y), self.stone_radius)
                        pygame.draw.circle(self.screen, self.LINE_COLOR, (x, y), self.stone_radius, 2)
                    elif stone == Board.WHITE:
                        pygame.draw.circle(self.screen, self.WHITE_STONE_COLOR, (x, y), self.stone_radius)
                        pygame.draw.circle(self.screen, self.LINE_COLOR, (x, y), self.stone_radius, 2)
    
    def draw_ui_panels(self):
        """Draw game information panels"""
        panel_x = self.board_x + self.board_size + 20
        panel_y = self.board_y
        
        # Game mode selector
        self.mode_selector.draw(self.screen, panel_x, panel_y)
        panel_y += 50
        
        # Game info
        info = self.game.get_game_info()
        
        # Current player
        current_player_text = f"Current Player: {info['current_player']}"
        if self.ai_thinking:
            current_player_text += " (AI Thinking...)"
        text_surface = self.font.render(current_player_text, True, self.TEXT_COLOR)
        self.screen.blit(text_surface, (panel_x, panel_y))
        
        # Move count
        move_count_text = f"Move Count: {info['move_count']}"
        text_surface = self.font.render(move_count_text, True, self.TEXT_COLOR)
        self.screen.blit(text_surface, (panel_x, panel_y + 30))
        
        # Captures
        black_captured_text = f"Black Captured: {info['black_captured']} stones"
        white_captured_text = f"White Captured: {info['white_captured']} stones"
        
        text_surface = self.font.render(black_captured_text, True, self.TEXT_COLOR)
        self.screen.blit(text_surface, (panel_x, panel_y + 70))
        
        text_surface = self.font.render(white_captured_text, True, self.TEXT_COLOR)
        self.screen.blit(text_surface, (panel_x, panel_y + 100))
        
        # Game status
        if self.game.is_game_over():
            status_text = f"Game Over: {info['winner']} Wins!"
            if info['state'] in [GameState.BLACK_WIN_CAPTURE, GameState.WHITE_WIN_CAPTURE]:
                status_text += " (Capture)"
            elif info['state'] in [GameState.BLACK_WIN_ALIGNMENT, GameState.WHITE_WIN_ALIGNMENT]:
                status_text += " (Alignment)"
            
            text_surface = self.large_font.render(status_text, True, self.HIGHLIGHT_COLOR)
            self.screen.blit(text_surface, (panel_x, panel_y + 140))
        
        # Timer display
        self.timer_display.render(self.screen, panel_x, panel_y + 200)
        
        # Debug UI
        self.debug_ui.draw_debug_info(self.screen, panel_x, panel_y + 280)
        
        # Controls
        controls_y = panel_y + 450
        controls = [
            "Controls:",
            "Left Click: Place stone",
            "R: Reset game",
            "C: Toggle coordinates",
            "D: Toggle debug mode",
            "S: Toggle suggestions",
            "ESC: Quit"
        ]
        
        for i, control in enumerate(controls):
            text_surface = self.font.render(control, True, self.TEXT_COLOR)
            self.screen.blit(text_surface, (panel_x, controls_y + i * 25))
    
    def setup_game_mode(self):
        """Setup the game based on selected mode"""
        mode = self.mode_selector.get_current_mode()
        self.game_mode = mode
        
        if mode == "human_vs_human":
            self.game.players[Board.BLACK] = HumanPlayer(Board.BLACK, "Human (Black)")
            self.game.players[Board.WHITE] = HumanPlayer(Board.WHITE, "Human (White)")
        elif mode == "human_vs_ai_easy":
            self.game.players[Board.BLACK] = HumanPlayer(Board.BLACK, "Human")
            self.game.players[Board.WHITE] = AIPlayer(Board.WHITE, "AI", "easy")
        elif mode == "human_vs_ai_normal":
            self.game.players[Board.BLACK] = HumanPlayer(Board.BLACK, "Human")
            self.game.players[Board.WHITE] = AIPlayer(Board.WHITE, "AI", "normal")
        elif mode == "human_vs_ai_hard":
            self.game.players[Board.BLACK] = HumanPlayer(Board.BLACK, "Human")
            self.game.players[Board.WHITE] = AIPlayer(Board.WHITE, "AI", "hard")
    
    def handle_click(self, pos):
        """Handle mouse click on the board"""
        # Check mode selector click
        if self.mode_selector.handle_click(pos):
            self.setup_game_mode()
            return
        
        if self.game.is_game_over() or self.ai_thinking:
            return
        
        # Only allow human moves
        current_player = self.game.players[self.game.current_player]
        if isinstance(current_player, AIPlayer):
            return
        
        board_pos = self.get_board_position(pos[0], pos[1])
        if board_pos is not None:
            row, col = board_pos
            success, message = self.game.make_move(row, col)
            if success:
                print(f"Move successful: {message}")
            else:
                print(f"Invalid move: {message}")
    
    def update_ai_turn(self):
        """Handle AI turn"""
        if self.ai_thinking or self.game.is_game_over():
            return
        
        current_player = self.game.players[self.game.current_player]
        if isinstance(current_player, AIPlayer):
            self.ai_thinking = True
            self.timer_display.timer.start()
            
            # Get AI move
            move = current_player.get_move(self.game.board)
            
            self.timer_display.timer.stop()
            self.ai_thinking = False
            
            # Update debug info
            if hasattr(current_player, 'get_last_stats'):
                self.debug_ui.update_ai_stats(current_player.get_last_stats())
            
            if move:
                row, col = move
                success, message = self.game.make_move(row, col)
                if success:
                    print(f"AI move: ({row}, {col}) - {message}")
                else:
                    print(f"AI invalid move: {message}")
    
    def update_suggestions(self):
        """Update move suggestions for human players"""
        if self.show_suggestions and not self.ai_thinking:
            current_player = self.game.players[self.game.current_player]
            if isinstance(current_player, HumanPlayer):
                suggested_moves = self.suggestion_ai.get_top_moves(
                    self.game.board, self.game.current_player, 5
                )
                self.debug_ui.update_suggested_moves(suggested_moves)
    
    def handle_events(self):
        """Handle pygame events"""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False
            
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    return False
                elif event.key == pygame.K_r:
                    # Reset game
                    self.game.reset_game()
                    self.timer_display.timer.reset()
                    print("Game reset")
                elif event.key == pygame.K_c:
                    # Toggle coordinates
                    self.show_coordinates = not self.show_coordinates
                elif event.key == pygame.K_d:
                    # Toggle debug mode
                    self.debug_ui.toggle_debug()
                elif event.key == pygame.K_s:
                    # Toggle suggestions
                    self.show_suggestions = not self.show_suggestions
            
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:  # Left click
                    self.handle_click(event.pos)
        
        return True
    
    def run(self):
        """Main game loop"""
        running = True
        self.setup_game_mode()  # Initialize game mode
        
        while running:
            # Handle events
            running = self.handle_events()
            
            # Update AI turn if needed
            self.update_ai_turn()
            
            # Update suggestions
            self.update_suggestions()
            
            # Draw everything
            self.draw_board()
            self.draw_stones()
            
            # Draw debug overlays
            if self.debug_ui.enabled:
                self.debug_ui.draw_move_suggestions(self.screen, self)
            elif self.show_suggestions:
                self.debug_ui.draw_move_suggestions(self.screen, self)
            
            self.draw_ui_panels()
            
            # Update display
            pygame.display.flip()
            self.clock.tick(60)  # 60 FPS
        
        pygame.quit()