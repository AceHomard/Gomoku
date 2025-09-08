"""
Timer for measuring AI thinking time
"""

import time
import pygame

class AITimer:
    """Timer to measure AI thinking time"""
    
    def __init__(self):
        self.start_time = None
        self.end_time = None
        self.last_move_time = 0.0
        self.total_time = 0.0
        self.move_count = 0
        self.is_running = False
    
    def start(self):
        """Start the timer"""
        self.start_time = time.time()
        self.is_running = True
    
    def stop(self):
        """Stop the timer and record the time"""
        if self.is_running and self.start_time is not None:
            self.end_time = time.time()
            self.last_move_time = self.end_time - self.start_time
            self.total_time += self.last_move_time
            self.move_count += 1
            self.is_running = False
            return self.last_move_time
        return 0.0
    
    def get_current_time(self):
        """Get current elapsed time (for running timer)"""
        if self.is_running and self.start_time is not None:
            return time.time() - self.start_time
        return self.last_move_time
    
    def get_last_move_time(self):
        """Get the time taken for the last move"""
        return self.last_move_time
    
    def get_average_time(self):
        """Get the average time per move"""
        if self.move_count == 0:
            return 0.0
        return self.total_time / self.move_count
    
    def reset(self):
        """Reset the timer"""
        self.start_time = None
        self.end_time = None
        self.last_move_time = 0.0
        self.total_time = 0.0
        self.move_count = 0
        self.is_running = False

class TimerDisplay:
    """Visual display for the timer"""
    
    def __init__(self, font, color=(0, 0, 0)):
        self.font = font
        self.color = color
        self.timer = AITimer()
    
    def render(self, surface, x, y):
        """Render the timer display"""
        current_time = self.timer.get_current_time()
        last_time = self.timer.get_last_move_time()
        avg_time = self.timer.get_average_time()
        
        # Current/Last move time
        if self.timer.is_running:
            time_text = f"AI Thinking: {current_time:.2f}s"
        else:
            time_text = f"Last Move: {last_time:.2f}s"
        
        # Average time
        avg_text = f"Average: {avg_time:.2f}s"
        
        # Render texts
        time_surface = self.font.render(time_text, True, self.color)
        avg_surface = self.font.render(avg_text, True, self.color)
        
        # Display
        surface.blit(time_surface, (x, y))
        surface.blit(avg_surface, (x, y + 25))
        
        # Warning if last move was too slow
        if last_time > 0.5:
            warning_text = "⚠ AI too slow!"
            warning_color = (255, 0, 0)  # Red
            warning_surface = self.font.render(warning_text, True, warning_color)
            surface.blit(warning_surface, (x, y + 50))