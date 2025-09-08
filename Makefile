#!/usr/bin/make -f

NAME = Gomoku
PYTHON = python3
PIP = pip3
SRC_DIR = src
MAIN_FILE = $(SRC_DIR)/main.py
REQUIREMENTS = requirements.txt

# Installation check
PYGAME_CHECK = $(shell $(PYTHON) -c "import pygame" 2>/dev/null && echo "OK" || echo "MISSING")

.PHONY: all clean fclean re install check_deps run test

all: $(NAME)

# Create the executable script
$(NAME): check_deps
	@echo "Creating executable $(NAME)..."
	@echo '#!/bin/bash' > $(NAME)
	@echo 'cd "$$(dirname "$$0")"' >> $(NAME)
	@echo '$(PYTHON) $(MAIN_FILE) "$$@"' >> $(NAME)
	@chmod +x $(NAME)
	@echo "$(NAME) executable created successfully!"

# Check and install dependencies
check_deps:
	@echo "Checking dependencies..."
	@$(PYTHON) --version
	@if [ "$(PYGAME_CHECK)" = "MISSING" ]; then \
		echo "Installing pygame..."; \
		$(PIP) install -r $(REQUIREMENTS); \
	else \
		echo "All dependencies are satisfied."; \
	fi

# Install dependencies manually
install:
	@echo "Installing dependencies..."
	@$(PIP) install -r $(REQUIREMENTS)

# Run the game directly
run: check_deps
	@cd $(SRC_DIR) && $(PYTHON) main.py

# Test the game (basic import test)
test: check_deps
	@echo "Running basic import tests..."
	@cd $(SRC_DIR) && $(PYTHON) -c "import game.board; import game.game; import ai.minimax; import ui.game_ui; print('All modules import successfully!')"

# Clean compiled Python files
clean:
	@echo "Cleaning compiled files..."
	@find . -name "*.pyc" -delete
	@find . -name "__pycache__" -type d -exec rm -rf {} + 2>/dev/null || true
	@echo "Clean completed."

# Remove executable and clean
fclean: clean
	@echo "Removing executable..."
	@rm -f $(NAME)
	@echo "Full clean completed."

# Rebuild everything
re: fclean all

# Help message
help:
	@echo "Gomoku Makefile"
	@echo "Usage:"
	@echo "  make [all]    - Build the executable"
	@echo "  make install  - Install dependencies"
	@echo "  make run      - Run the game directly"
	@echo "  make test     - Run basic tests"
	@echo "  make clean    - Clean compiled files"
	@echo "  make fclean   - Remove executable and clean"
	@echo "  make re       - Rebuild everything"
	@echo "  make help     - Show this help message"