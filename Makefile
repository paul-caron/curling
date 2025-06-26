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










# Packaging variables
PACKAGE_NAME = curling
VERSION = 1.0
ARCH = amd64
PKG_BUILD_DIR = build/$(PACKAGE_NAME)-$(VERSION)
DESTDIR = $(PKG_BUILD_DIR)/usr/local

.PHONY: install deb

# Install to a fake root (for packaging)
install: all
	@mkdir -p $(DESTDIR)/lib
	@mkdir -p $(DESTDIR)/include
	cp $(TARGET_LIB) $(DESTDIR)/lib/
	cp include/curling.hpp $(DESTDIR)/include/

# Build .deb package
deb: install
	@mkdir -p $(PKG_BUILD_DIR)/DEBIAN
	echo "Package: $(PACKAGE_NAME)" > $(PKG_BUILD_DIR)/DEBIAN/control
	echo "Version: $(VERSION)" >> $(PKG_BUILD_DIR)/DEBIAN/control
	echo "Section: libs" >> $(PKG_BUILD_DIR)/DEBIAN/control
	echo "Priority: optional" >> $(PKG_BUILD_DIR)/DEBIAN/control
	echo "Architecture: $(ARCH)" >> $(PKG_BUILD_DIR)/DEBIAN/control
	echo "Maintainer: Your Name <you@example.com>" >> $(PKG_BUILD_DIR)/DEBIAN/control
	echo "Description: Static C++ wrapper for libcurl" >> $(PKG_BUILD_DIR)/DEBIAN/control

	dpkg-deb --build $(PKG_BUILD_DIR)
	mv $(PKG_BUILD_DIR).deb $(PACKAGE_NAME)_$(VERSION)_$(ARCH).deb
