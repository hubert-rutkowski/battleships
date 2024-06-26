# Makefile for a C project
# ---------------------------------

# Compiler and compiler flags
CC = gcc
CFLAGS = -std=c11 -Iinclude

# Set the build mode (debug or release)
MODE ?= debug

# Check if MODE is set to either debug or release
check_mode:
ifeq ($(filter $(MODE),debug release),)
	$(error MODE must be set to either debug or release)
else
# Null operation prevents message "Nothing to be done" when MODE is set properly
	@: 
endif

# Adjust compiler flags based on the build mode
ifeq ($(MODE),release)
	CFLAGS += -O2  # Optimization for release mode
else
	CFLAGS += -g -Wall  # Debug symbols and warnings for debug mode
endif

# Directories and paths
BUILD_DIR = target
SRC_DIR = src
TARGET_DIR = $(BUILD_DIR)/$(MODE)
OBJ_DIR = $(TARGET_DIR)/obj
TARGET = $(TARGET_DIR)/hello_world

# List all source files in the src directory
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Generate a list of object files in the obj directory
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Default target builds the executable
build: check_mode $(TARGET)

# Rule to compile object files from source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link object files into the final executable
$(TARGET): $(OBJ_FILES)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@ -lSDL2 -lSDL2_ttf

# Clean target removes all generated files
clean:
	@rm -rf $(OBJ_DIR) $(BUILD_DIR)

# Compile and Run
run: $(TARGET)
	./$(TARGET)

# Remove generated files and rebuild the project
rebuild: clean build