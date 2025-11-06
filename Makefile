# MMIX Emulator Makefile
#
# Copyright (c) 2025, MMIX Emulator Project. All rights reserved.
# SPDX-License-Identifier: BSD-2-Clause-Patent

# Compiler and flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude -O2 -g -D_DEFAULT_SOURCE
LDFLAGS = -lm

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Target executables
EMULATOR = $(BIN_DIR)/mmix-emulator
ASSEMBLER = $(BIN_DIR)/mmix-as
LINKER = $(BIN_DIR)/mmix-ld
OBJDUMP = $(BIN_DIR)/mmix-objdump
LIBRARIAN = $(BIN_DIR)/mmix-ar
COMPILER = $(BIN_DIR)/mmix-cc

ALL_TARGETS = $(EMULATOR) $(ASSEMBLER) $(LINKER) $(OBJDUMP) $(LIBRARIAN) $(COMPILER)

# Common/shared object files
COMMON_OBJS = \
	$(BUILD_DIR)/loader/Loader.o \
	$(BUILD_DIR)/loader/ElfLoader.o \
	$(BUILD_DIR)/loader/MachoLoader.o \
	$(BUILD_DIR)/loader/PeLoader.o \
	$(BUILD_DIR)/loader/MmoLoader.o

DISASM_OBJS = \
	$(BUILD_DIR)/tools/Disassembler.o

# Emulator source files and objects
EMULATOR_SOURCES = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/Emulator.c \
	$(SRC_DIR)/core/Cpu.c \
	$(SRC_DIR)/core/Execute.c \
	$(SRC_DIR)/core/Compressed.c \
	$(SRC_DIR)/memory/Memory.c \
	$(SRC_DIR)/memory/PageTable.c \
	$(SRC_DIR)/fpu/Fpu.c \
	$(SRC_DIR)/devices/Devices.c

EMULATOR_OBJS = $(EMULATOR_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Assembler objects
ASSEMBLER_LIB_OBJS = \
	$(BUILD_DIR)/tools/Assembler.o

ASSEMBLER_OBJS = \
	$(BUILD_DIR)/tools/AssemblerMain.o \
	$(ASSEMBLER_LIB_OBJS)

# Linker objects
LINKER_OBJS = \
	$(BUILD_DIR)/tools/Linker.o

# Objdump objects
OBJDUMP_OBJS = \
	$(BUILD_DIR)/tools/Objdump.o

# Librarian objects
LIBRARIAN_OBJS = \
	$(BUILD_DIR)/tools/Librarian.o

# Compiler objects
COMPILER_OBJS = \
	$(BUILD_DIR)/compiler/Token.o \
	$(BUILD_DIR)/compiler/Lexer.o \
	$(BUILD_DIR)/compiler/Ast.o \
	$(BUILD_DIR)/compiler/Parser.o \
	$(BUILD_DIR)/compiler/Sema.o \
	$(BUILD_DIR)/compiler/IrGen.o \
	$(BUILD_DIR)/compiler/CodeGen.o

# Default target
all: $(ALL_TARGETS)
	@echo ""
	@echo "=== MMIX Toolchain Build Complete ==="
	@echo "Built executables:"
	@echo "  $(EMULATOR)"
	@echo "  $(ASSEMBLER)"
	@echo "  $(LINKER)"
	@echo "  $(OBJDUMP)"
	@echo "  $(LIBRARIAN)"
	@echo "  $(COMPILER)"

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
	mkdir -p $(BUILD_DIR)/loader
	mkdir -p $(BUILD_DIR)/tools
	mkdir -p $(BUILD_DIR)/debug
	mkdir -p $(BUILD_DIR)/compiler

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Build emulator
$(EMULATOR): $(EMULATOR_OBJS) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) $(EMULATOR_OBJS) $(COMMON_OBJS) -o $(EMULATOR) $(LDFLAGS)
	@echo "Built: $(EMULATOR)"

# Build assembler
$(ASSEMBLER): $(ASSEMBLER_OBJS) | $(BIN_DIR)
	$(CC) $(ASSEMBLER_OBJS) -o $(ASSEMBLER) $(LDFLAGS)
	@echo "Built: $(ASSEMBLER)"

# Build linker (needs assembler library but not main)
$(LINKER): $(LINKER_OBJS) $(ASSEMBLER_LIB_OBJS) | $(BIN_DIR)
	$(CC) $(LINKER_OBJS) $(ASSEMBLER_LIB_OBJS) -o $(LINKER) $(LDFLAGS)
	@echo "Built: $(LINKER)"

# Build objdump (needs only disassembler, no loader)
$(OBJDUMP): $(OBJDUMP_OBJS) $(DISASM_OBJS) | $(BIN_DIR)
	$(CC) $(OBJDUMP_OBJS) $(DISASM_OBJS) -o $(OBJDUMP) $(LDFLAGS)
	@echo "Built: $(OBJDUMP)"

# Build librarian
$(LIBRARIAN): $(LIBRARIAN_OBJS) | $(BIN_DIR)
	$(CC) $(LIBRARIAN_OBJS) -o $(LIBRARIAN) $(LDFLAGS)
	@echo "Built: $(LIBRARIAN)"

# Build compiler
$(COMPILER): $(COMPILER_OBJS) $(BUILD_DIR)/compiler/CompilerMain.o | $(BIN_DIR)
	$(CC) $(COMPILER_OBJS) $(BUILD_DIR)/compiler/CompilerMain.o -o $(COMPILER) $(LDFLAGS)
	@echo "Built: $(COMPILER)"

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

# Build just the emulator
emulator: $(EMULATOR)

# Build just the toolchain (no emulator)
toolchain: $(ASSEMBLER) $(LINKER) $(OBJDUMP) $(LIBRARIAN)
	@echo ""
	@echo "=== MMIX Toolchain Build Complete ==="

# Run tests
test: all
	@echo "Running tests..."
	@echo "Generating test binaries..."
	python3 tools/mktest.py
	@echo ""
	@echo "Testing arithmetic operations..."
	./$(EMULATOR) -dump arithmetic_test.bin || true
	@echo ""
	@echo "Testing objdump..."
	./$(OBJDUMP) -d arithmetic_test.bin | head -20 || true
	@echo ""
	@echo "Test suite completed"

# Install
install: all
	@echo "Installing MMIX toolchain to /usr/local/bin..."
	install -m 755 $(EMULATOR) /usr/local/bin/
	install -m 755 $(ASSEMBLER) /usr/local/bin/
	install -m 755 $(LINKER) /usr/local/bin/
	install -m 755 $(OBJDUMP) /usr/local/bin/
	install -m 755 $(LIBRARIAN) /usr/local/bin/
	@echo "Installation complete"

# Help target
help:
	@echo "MMIX Emulator and Toolchain Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build everything (default)"
	@echo "  emulator  - Build just the emulator"
	@echo "  toolchain - Build just the toolchain utilities"
	@echo "  clean     - Remove build artifacts"
	@echo "  debug     - Build with debug symbols"
	@echo "  release   - Build optimized release version"
	@echo "  test      - Run test suite"
	@echo "  install   - Install to /usr/local/bin"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Toolchain components:"
	@echo "  mmix-emulator - MMIX emulator"
	@echo "  mmix-as       - MMIX assembler"
	@echo "  mmix-ld       - MMIX linker"
	@echo "  mmix-objdump  - Object file inspector"
	@echo "  mmix-ar       - Archive/library manager"

.PHONY: all clean debug release test install help emulator toolchain
