# Compiler settings
CXX = g++
CXXFLAGS = -Iinclude -Wall -std=c++17

# Linker flags
LDFLAGS = -lcurl -static-libgcc

# Directories
SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib
DOC_DIR = doc

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Target library
TARGET_LIB = $(LIB_DIR)/libcurling.a

# Phony targets for make
.PHONY: all clean doc

# Default target to build the library
all: $(TARGET_LIB)

# Rule to create the static library from object files
$(TARGET_LIB): $(OBJS)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $^
	ranlib $@

# Pattern rule for compiling source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Target to clean the build artifacts
clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR)

# New target for generating documentation with Doxygen
doc:
	doxygen Doxyfile

# Optional: Clean documentation
.PHONY: doc-clean
doc-clean:
	rm -rf $(DOC_DIR)
