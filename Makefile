# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -std=c99 -D_POSIX_C_SOURCE=200809L
DEBUG_FLAGS = -g
PROFILE_FLAGS = -pg
RELEASE_FLAGS = -O2

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
TEST_DIR = test

# Target executable
TARGET = timed

# Source files - adjust these patterns to match your directory structure
SRCS = $(wildcard *.c) $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SRCS)))

# Test related
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS = $(TEST_SRCS:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.o)
TEST_TARGET = $(TEST_DIR)/test_runner

# Header files
INCLUDES = -I$(INC_DIR) -I.

# Libraries
LIBS = -pthread

# Vpath directive to find source files
VPATH = $(SRC_DIR):$(TEST_DIR):.

# Phony targets
.PHONY: all debug profile check clean dirs

# Default target
all: dirs $(TARGET)

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: dirs $(TARGET)

# Profile build
profile: CFLAGS += $(PROFILE_FLAGS)
profile: dirs $(TARGET)

# Create necessary directories
dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(TEST_DIR)

# Main program build
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# Pattern rule for object files
$(OBJ_DIR)/%.o: %.c | dirs
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Testing targets
$(TEST_TARGET): $(filter-out $(OBJ_DIR)/server_main.o, $(OBJS)) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $^ -o $@ $(LIBS)

# Run tests
check: dirs $(TEST_TARGET)
	./$(TEST_TARGET)

# Clean everything
clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TEST_TARGET)
	@find . -name "*.o" -type f -delete
	@find . -name "*.gcda" -type f -delete
	@find . -name "*.gcno" -type f -delete
	@find . -name "*.out" -type f -delete