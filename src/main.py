#!/usr/bin/env python3
"""
Gomoku Game - Main Entry Point
"""

import sys
import pygame
from ui.game_ui import GameUI

def main():
    """Main function to start the Gomoku game"""
    pygame.init()
    
    try:
        game_ui = GameUI()
        game_ui.run()
    except Exception as e:
        print(f"Error starting game: {e}")
        return 1
    finally:
        pygame.quit()
    
    return 0

if __name__ == "__main__":
    sys.exit(main())