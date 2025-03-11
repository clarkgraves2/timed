# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -std=c99 -D_DEFAULT_SOURCE
DEBUG_FLAGS = -g -DDEBUG
RELEASE_FLAGS = -O2

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = test

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Executables
TARGET = 

# Header files
INCLUDES = -I$(INC_DIR)

.PHONY: all clean check dirs run_tests

all: dirs $(TARGET)

# Create necessary directories
dirs:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)
	mkdir -p $(TEST_DIR)

# Main program build
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Add directory creation dependency to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(INCLUDES) -c $< -o $@

# Order-only prerequisite for directory creation
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Run all tests
run_tests: $(TARGET)

# Test execution
check: dirs $(TARGET) run_tests

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(TARGET)
