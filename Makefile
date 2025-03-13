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

# Test client source files
TEST_CLIENT_SRCS = $(TEST_DIR)/clients/test_client.c $(TEST_DIR)/clients/test_client_udp.c
TEST_CLIENT_BINS = $(TEST_DIR)/bin/test_client $(TEST_DIR)/bin/test_client_udp

# Main target name
TARGET = timed

# Header files
INCLUDES = -I$(INC_DIR) -I.

# Clang-tidy configuration
CLANG_TIDY = clang-tidy
CLANG_TIDY_CHECKS = -*,bugprone-*,cert-*,clang-analyzer-*,cppcoreguidelines-*,misc-*,performance-*,portability-*,readability-*,-readability-implicit-bool-conversion,-readability-magic-numbers
CLANG_TIDY_CONFIG = -config="{Checks: '$(CLANG_TIDY_CHECKS)', WarningsAsErrors: '', HeaderFilterRegex: '.*', FormatStyle: 'none'}"

# Valgrind configuration
VALGRIND = valgrind
VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose
VALGRIND_LOG = valgrind_report.txt

# Default target
.PHONY: all clean check valgrind dirs debug profile test_clients protocol_tests start_server start_server_valgrind stop_server tidy

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
	mkdir -p $(TEST_DIR)/bin
	mkdir -p $(TEST_DIR)/tcp
	mkdir -p $(TEST_DIR)/udp
	mkdir -p $(TEST_DIR)/udp_specific
	mkdir -p $(TEST_DIR)/clients
	mkdir -p $(TEST_DIR)/scripts

# Main program build
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Standard build rule for object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | dirs
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Test build rule
$(OBJ_DIR)/test_%.o: $(TEST_DIR)/%.c | dirs
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(INCLUDES) -c $< -o $@

# Test client build rules
$(TEST_DIR)/bin/test_client: $(TEST_DIR)/clients/test_client.c | dirs
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $< -o $@

$(TEST_DIR)/bin/test_client_udp: $(TEST_DIR)/clients/test_client_udp.c | dirs
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $< -o $@

# Build test clients
test_clients: $(TEST_CLIENT_BINS)

# Start server for testing
start_server: $(TARGET)
	@echo "Starting timed server in background..."
	@./$(TARGET) & echo $$! > .server.pid
	@sleep 1  # Give it time to start up properly

# Start server with Valgrind for testing
start_server_valgrind: $(TARGET)
	@echo "Starting timed server with Valgrind in background..."
	@$(VALGRIND) $(VALGRIND_FLAGS) --log-file=$(VALGRIND_LOG) ./$(TARGET) & echo $$! > .server.pid
	@sleep 2  # Give it more time to start up with Valgrind

# Stop server after testing
stop_server:
	@if [ -f .server.pid ]; then \
		echo "Stopping timed server..."; \
		kill -15 `cat .server.pid` || true; \
		rm .server.pid; \
	fi

# Run protocol tests
protocol_tests: test_clients
	@echo "Running protocol tests..."
	@./run_tests.sh

# Run all tests
check: $(TARGET) test_clients start_server protocol_tests stop_server
	@echo "All tests completed."

# Run all tests with Valgrind
valgrind: $(TARGET) test_clients start_server_valgrind protocol_tests stop_server
	@echo "All tests completed with Valgrind."
	@if [ -f $(VALGRIND_LOG) ]; then \
		echo "\nValgrind Report Summary:"; \
		grep -A 2 "LEAK SUMMARY" $(VALGRIND_LOG) || echo "No leak summary found"; \
		echo "\nCheck $(VALGRIND_LOG) for full details."; \
	else \
		echo "No Valgrind log found at $(VALGRIND_LOG)"; \
	fi

# Run clang-tidy on all source files
tidy:
	$(CLANG_TIDY) $(CLANG_TIDY_CONFIG) $(SRCS) -- $(CFLAGS) $(INCLUDES)

clean:
	rm -f $(TARGET) server.log .server.pid $(VALGRIND_LOG)
	rm -rf $(OBJ_DIR)
	rm -f $(TEST_CLIENT_BINS)
	rm -f test_summary_run_tcp_tests.sh