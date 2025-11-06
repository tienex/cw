# MMIX Emulator Makefile
#
# Copyright (c) 2025, MMIX Emulator Project. All rights reserved.
# SPDX-License-Identifier: BSD-2-Clause-Patent

# Compiler and flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude -O2 -g
LDFLAGS = -lm

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Target executable
TARGET = $(BIN_DIR)/mmix-emulator

# Source files
SOURCES = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/Emulator.c \
	$(SRC_DIR)/core/Cpu.c \
	$(SRC_DIR)/core/Execute.c \
	$(SRC_DIR)/memory/Memory.c \
	$(SRC_DIR)/devices/Devices.c

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Default target
all: $(TARGET)

# Create directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/core
	mkdir -p $(BUILD_DIR)/memory
	mkdir -p $(BUILD_DIR)/hypervisor
	mkdir -p $(BUILD_DIR)/vector
	mkdir -p $(BUILD_DIR)/fpu
	mkdir -p $(BUILD_DIR)/ml
	mkdir -p $(BUILD_DIR)/devices
	mkdir -p $(BUILD_DIR)/pcie

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Link target
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Built: $(TARGET)"

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled: $<"

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Cleaned build artifacts"

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: clean all

# Release build
release: CFLAGS += -DNDEBUG -O3
release: clean all

# Run tests (placeholder)
test: all
	@echo "Running tests..."
	@echo "No tests implemented yet"

# Install (placeholder)
install: all
	@echo "Installing to /usr/local/bin..."
	@echo "Not implemented yet"

# Help target
help:
	@echo "MMIX Emulator Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build the emulator (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  debug    - Build with debug symbols"
	@echo "  release  - Build optimized release version"
	@echo "  test     - Run test suite"
	@echo "  install  - Install to system"
	@echo "  help     - Show this help message"

.PHONY: all clean debug release test install help
