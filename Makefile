# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -std=c99 -D_POSIX_C_SOURCE=200809L
DEBUG_FLAGS = -g -DDEBUG
PROFILE_FLAGS = -pg -DNDEBUG
RELEASE_FLAGS = -O2 -DNDEBUG

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
TEST_DIR = test
DOC_DIR = doc

# Source files
SRCS = $(SRC_DIR)/config.c $(SRC_DIR)/poll.c $(SRC_DIR)/signal_handler.c $(SRC_DIR)/socket.c $(SRC_DIR)/syslog.c $(SRC_DIR)/server.c $(SRC_DIR)/server_main.c
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS = $(TEST_SRCS:$(TEST_DIR)/%.c=$(OBJ_DIR)/test_%.o)

# Main target name
TARGET = timed

# Header files
INCLUDES = -I$(INC_DIR) -I.

# Default target
.PHONY: all clean check dirs debug profile

all: dirs $(TARGET)

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: dirs $(TARGET)

# Profile build
profile: CFLAGS += $(PROFILE_FLAGS)
profile: dirs $(TARGET)

# Create necessary directories
dirs:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(TEST_DIR)
	mkdir -p $(DOC_DIR)

# Main program build
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Standard build rule for object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | dirs
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Test build rule
$(OBJ_DIR)/test_%.o: $(TEST_DIR)/%.c | dirs
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(INCLUDES) -c $< -o $@

# Test program build
test_runner: $(TEST_OBJS) $(filter-out $(OBJ_DIR)/server_main.o, $(OBJS))
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $^ -lcheck -lm -lrt -lpthread -lsubunit -o $@

# Run all tests
check: test_runner
	./test_runner

clean:
	rm -f $(TARGET) test_runner server.log
	rm -rf $(OBJ_DIR)