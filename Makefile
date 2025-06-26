# ========== Config ==========
CXX := g++
CXXFLAGS := -Wall -Wextra -O2 -std=c++17 -Iinclude -fPIC

SRC_DIR := src
OBJ_DIR := obj
LIB_DIR := lib
BUILD_DIR := build
DOC_DIR := doc

ARCH := amd64
PACKAGE_NAME := curling
VERSION := 1.0
MAINTAINER := Paul Caron <you@example.com>
PKG_BUILD_DIR := $(BUILD_DIR)/$(PACKAGE_NAME)-$(VERSION)

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
TARGET_LIB_SHARED := $(LIB_DIR)/lib$(PACKAGE_NAME).so

LDLIBS := -lcurl

.PHONY: all clean doc deb doc-clean install

# ========== Build ==========

all: $(TARGET_LIB_SHARED)

$(TARGET_LIB_SHARED): $(OBJS)
	@mkdir -p $(LIB_DIR)
	$(CXX) -shared -o $@ $^ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ========== Install Stage (for deb) ==========

install: $(TARGET_LIB_SHARED)
	@echo "Installing to staging dir..."
	@mkdir -p $(PKG_BUILD_DIR)/usr/local/lib
	@mkdir -p $(PKG_BUILD_DIR)/usr/local/include
	cp $(TARGET_LIB_SHARED) $(PKG_BUILD_DIR)/usr/local/lib/
	cp include/*.hpp $(PKG_BUILD_DIR)/usr/local/include/

# ========== Debian Package ==========

deb: install
	@mkdir -p $(PKG_BUILD_DIR)/DEBIAN
	chmod 0755 $(PKG_BUILD_DIR)/DEBIAN
	echo "Package: $(PACKAGE_NAME)" > $(PKG_BUILD_DIR)/DEBIAN/control
	echo "Version: $(VERSION)" >> $(PKG_BUILD_DIR)/DEBIAN/control
	echo "Architecture: $(ARCH)" >> $(PKG_BUILD_DIR)/DEBIAN/control
	echo "Maintainer: $(MAINTAINER)" >> $(PKG_BUILD_DIR)/DEBIAN/control
	echo "Description: Shared C++ wrapper for libcurl" >> $(PKG_BUILD_DIR)/DEBIAN/control
	echo "Section: libs" >> $(PKG_BUILD_DIR)/DEBIAN/control
	echo "Priority: optional" >> $(PKG_BUILD_DIR)/DEBIAN/control
	echo "Depends: libcurl4, libcurl4-openssl-dev" >> $(PKG_BUILD_DIR)/DEBIAN/control
	dpkg-deb --build $(PKG_BUILD_DIR)
	@mv $(PKG_BUILD_DIR).deb $(PACKAGE_NAME)_$(VERSION)_$(ARCH).deb
	@echo "Package created: $(PACKAGE_NAME)_$(VERSION)_$(ARCH).deb"

# ========== Documentation ==========

doc:
	doxygen Doxyfile

doc-clean:
	rm -rf $(DOC_DIR)

# ========== Cleanup ==========

clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(BUILD_DIR)
