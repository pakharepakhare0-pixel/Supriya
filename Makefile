# ------------------------------------------------------------------ #
#  Hybrid Inventory Manager — Makefile                                 #
# ------------------------------------------------------------------ #

CXX      := g++
CC       := gcc
CFLAGS   := -Wall -Wextra -std=c11   -Iinclude
CXXFLAGS := -Wall -Wextra -std=c++17 -Iinclude
LDFLAGS  :=

TARGET   := inventory_manager
BUILD    := build

# Source files
C_SRCS   := src/inventory.c
CXX_SRCS := src/InventoryManager.cpp src/main.cpp

# Object files (placed in build/)
C_OBJS   := $(patsubst src/%.c,   $(BUILD)/%.o, $(C_SRCS))
CXX_OBJS := $(patsubst src/%.cpp, $(BUILD)/%.o, $(CXX_SRCS))
ALL_OBJS := $(C_OBJS) $(CXX_OBJS)

# ------------------------------------------------------------------ #
.PHONY: all clean run

all: $(BUILD) $(TARGET)

$(BUILD):
	mkdir -p $(BUILD)

# Link with g++ so the C++ runtime is included
$(TARGET): $(ALL_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo ">>> Built: $(TARGET)"

# Compile C sources
$(BUILD)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile C++ sources
$(BUILD)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD) $(TARGET) inventory.dat
	@echo ">>> Cleaned."

run: all
	./$(TARGET)
