NAME		= Gomoku

# Compiler and flags
CXX			= g++
CXXFLAGS	= -std=c++17 -Wall -Wextra -Werror -O3 -fPIC
INCLUDES	= -I./include -I./lib/SFML/include

# Directories
SRC_DIR		= src
INCLUDE_DIR	= include
OBJ_DIR		= obj
LIB_DIR		= lib
SFML_DIR	= $(LIB_DIR)/SFML
SFML_BUILD	= $(SFML_DIR)/build

# SFML configuration
SFML_VERSION = 3.0.1
SFML_URL	= https://github.com/SFML/SFML/archive/refs/tags/$(SFML_VERSION).tar.gz
SFML_LIBS	= -L$(SFML_BUILD)/lib -lsfml-graphics-s -lsfml-window-s -lsfml-system-s
SYSTEM_LIBS	= -lGL -lX11 -lXrandr -lXi -lXcursor -lpthread -ldl -ludev -lfreetype

# Source files
SOURCES		= $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/**/*.cpp)
OBJECTS		= $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))

# SFML library files to check
SFML_LIB_FILES = $(SFML_BUILD)/lib/libsfml-graphics.a \
				 $(SFML_BUILD)/lib/libsfml-window.a \
				 $(SFML_BUILD)/lib/libsfml-system.a

# Colors for output
RED		= \033[0;31m
GREEN	= \033[0;32m
YELLOW	= \033[0;33m
BLUE	= \033[0;34m
PURPLE	= \033[0;35m
CYAN	= \033[0;36m
WHITE	= \033[0;37m
RESET	= \033[0m

# Rules
all: $(NAME)

$(NAME): sfml $(OBJECTS)
	@echo "$(GREEN)Linking $(NAME)...$(RESET)"
	@$(CXX) $(OBJECTS) $(SFML_LIBS) $(SYSTEM_LIBS) -o $(NAME)
	@echo "$(GREEN)✓ $(NAME) built successfully!$(RESET)"

# Create obj directory structure and compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "$(BLUE)Compiling $<...$(RESET)"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# SFML build target
sfml: $(SFML_LIB_FILES)

$(SFML_LIB_FILES): $(SFML_DIR)/CMakeLists.txt
	@echo "$(YELLOW)Building SFML...$(RESET)"
	@mkdir -p $(SFML_BUILD)
	@cd $(SFML_BUILD) && cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DBUILD_SHARED_LIBS=FALSE \
		-DSFML_BUILD_EXAMPLES=FALSE \
		-DSFML_BUILD_DOC=FALSE \
		-DSFML_BUILD_NETWORK=FALSE \
		-DSFML_BUILD_AUDIO=FALSE \
		-DCMAKE_CXX_FLAGS="-fPIC" \
		-DCMAKE_C_FLAGS="-fPIC"
	@cd $(SFML_BUILD) && make -j$$(nproc)
	@echo "$(GREEN)✓ SFML built successfully!$(RESET)"

# Download and extract SFML
$(SFML_DIR)/CMakeLists.txt:
	@echo "$(CYAN)Downloading SFML $(SFML_VERSION)...$(RESET)"
	@mkdir -p $(LIB_DIR)
	@cd $(LIB_DIR) && wget -q $(SFML_URL) -O sfml.tar.gz
	@cd $(LIB_DIR) && tar -xzf sfml.tar.gz
	@cd $(LIB_DIR) && mv SFML-$(SFML_VERSION) SFML
	@cd $(LIB_DIR) && rm sfml.tar.gz
	@echo "$(GREEN)✓ SFML downloaded and extracted!$(RESET)"

# Clean object files
clean:
	@echo "$(RED)Cleaning object files...$(RESET)"
	@rm -rf $(OBJ_DIR)
	@echo "$(GREEN)✓ Object files cleaned!$(RESET)"

# Clean everything including SFML and executable
fclean: clean
	@echo "$(RED)Full clean...$(RESET)"
	@rm -f $(NAME)
	@rm -rf $(LIB_DIR)
	@echo "$(GREEN)✓ Full clean completed!$(RESET)"

# Rebuild everything
re: fclean all

# Debug target for development
debug: CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -g -DDEBUG
debug: $(NAME)

# AI Debug Visualizer target (separate window showing minimax tree)
debug_visu: clean
debug_visu: CXXFLAGS += -DDEBUG_VISU -g
debug_visu: $(NAME)
	@echo "$(PURPLE)✓ Built with AI Debug Visualizer enabled!$(RESET)"

# Install system dependencies (for Ubuntu/Debian)
deps:
	@echo "$(YELLOW)Installing system dependencies...$(RESET)"
	@sudo apt-get update
	@sudo apt-get install -y build-essential cmake wget \
		libgl1-mesa-dev libx11-dev libxrandr-dev libxi-dev \
		libudev-dev libfreetype6-dev libopenal-dev libflac-dev \
		libvorbis-dev
	@echo "$(GREEN)✓ Dependencies installed!$(RESET)"

# Print build information
info:
	@echo "$(CYAN)Build Information:$(RESET)"
	@echo "  Name:       $(NAME)"
	@echo "  Compiler:   $(CXX)"
	@echo "  Flags:      $(CXXFLAGS)"
	@echo "  SFML Ver:   $(SFML_VERSION)"
	@echo "  Sources:    $(words $(SOURCES)) files"

# Help target
help:
	@echo "$(CYAN)Available targets:$(RESET)"
	@echo "  $(GREEN)all$(RESET)     - Build the project (default)"
	@echo "  $(GREEN)clean$(RESET)   - Remove object files"
	@echo "  $(GREEN)fclean$(RESET)  - Remove all generated files including SFML"
	@echo "  $(GREEN)re$(RESET)      - Rebuild everything from scratch"
	@echo "  $(GREEN)debug$(RESET)      - Build with debug flags"
	@echo "  $(GREEN)debug_visu$(RESET) - Build with AI debug visualizer window"
	@echo "  $(GREEN)deps$(RESET)    - Install system dependencies"
	@echo "  $(GREEN)info$(RESET)    - Show build information"
	@echo "  $(GREEN)help$(RESET)    - Show this help message"

.PHONY: all clean fclean re sfml debug debug_visu deps info help