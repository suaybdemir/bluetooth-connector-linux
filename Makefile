# ToothDroid - Modern Linux Bluetooth Manager
# Version: 2.0.0

# Compiler settings
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic
INCLUDES := -I.

# Source files - CLI
CLI_SRCS := main.cpp
CLI_TARGET := toothdroid

# Source files - GUI
# Source files - GUI
GUI_SRCS := qt-gui/main.cpp qt-gui/MainWindow.cpp qt-gui/DeviceItemWidget.cpp
GUI_TARGET := toothdroid-gui
GUI_MOC_HEADERS := qt-gui/MainWindow.h qt-gui/DeviceItemWidget.h

# Qt6 Configuration
QT_CFLAGS := $(shell pkg-config --cflags Qt6Widgets Qt6Core Qt6Concurrent) -fPIC
QT_LIBS := $(shell pkg-config --libs Qt6Widgets Qt6Core Qt6Concurrent)

# MOC Rules
MOC := /usr/lib/qt6/moc
qt-gui/moc_%.cpp: qt-gui/%.h
	$(MOC) $< -o $@

GUI_MOC_SRCS := $(GUI_MOC_HEADERS:qt-gui/%.h=qt-gui/moc_%.cpp)

# Headers
HEADERS := $(wildcard include/*.h) $(wildcard qt-gui/*.h)

# Legacy target
LEGACY_TARGET := output

# Build modes
DEBUG_FLAGS := -g -O0 -DDEBUG
RELEASE_FLAGS := -O2 -DNDEBUG

# Default to release build
BUILD_TYPE ?= release

ifeq ($(BUILD_TYPE),debug)
    CXXFLAGS += $(DEBUG_FLAGS)
else
    CXXFLAGS += $(RELEASE_FLAGS)
endif

# Colors for output
GREEN := \033[0;32m
CYAN := \033[0;36m
YELLOW := \033[1;33m
MAGENTA := \033[0;35m
NC := \033[0m

.PHONY: all cli gui build run run-gui clean install uninstall debug release help legacy

# Default target - build both
all: cli gui

# CLI build
cli: $(CLI_TARGET)
	@echo "$(GREEN)✓ CLI build complete: $(CLI_TARGET)$(NC)"

$(CLI_TARGET): $(CLI_SRCS) $(HEADERS)
	@echo "$(CYAN)Building ToothDroid CLI...$(NC)"
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(CLI_SRCS) -o $(CLI_TARGET)

# GUI build
gui: check-qt $(GUI_TARGET)
	@echo "$(GREEN)✓ GUI build complete: $(GUI_TARGET)$(NC)"

$(GUI_TARGET): $(GUI_SRCS) $(GUI_MOC_SRCS) $(HEADERS)
	@echo "$(MAGENTA)Building ToothDroid GUI (Qt6)...$(NC)"
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(QT_CFLAGS) $(GUI_SRCS) $(GUI_MOC_SRCS) -o $(GUI_TARGET) $(QT_LIBS)

# Clean moc files
clean-moc:
	@rm -f qt-gui/moc_*.cpp

# Update clean target
clean: clean-moc

# Check for GTK
# Check for Qt6
check-qt:
	@pkg-config --exists Qt6Widgets || (echo "$(YELLOW)⚠ Qt6Widgets not found. Install qt6-base/qt6-qtbase-devel$(NC)" && exit 1)

# Debug builds
debug:
	@$(MAKE) BUILD_TYPE=debug all

# Release builds
release:
	@$(MAKE) BUILD_TYPE=release all

# Run CLI
run: cli
	@echo "$(CYAN)Starting ToothDroid CLI...$(NC)"
	@./$(CLI_TARGET)

# Run GUI
run-gui: gui
	@echo "$(MAGENTA)Starting ToothDroid GUI...$(NC)"
	@./$(GUI_TARGET)

# Legacy support
legacy: $(CLI_SRCS) $(HEADERS)
	@echo "$(YELLOW)Building legacy output binary...$(NC)"
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(CLI_SRCS) -o $(LEGACY_TARGET)

# Clean build artifacts
clean:
	@echo "$(CYAN)Cleaning...$(NC)"
	@rm -f $(CLI_TARGET) $(GUI_TARGET) $(LEGACY_TARGET) *.o qt-gui/*.o qt-gui/moc_*.cpp
	@rm -f *.gch include/*.gch qt-gui/*.gch
	@echo "$(GREEN)✓ Clean complete$(NC)"

# Install both binaries
install: release
	@echo "$(CYAN)Installing ToothDroid to /usr/local/bin...$(NC)"
	@sudo cp $(CLI_TARGET) /usr/local/bin/$(CLI_TARGET)
	@sudo chmod +x /usr/local/bin/$(CLI_TARGET)
	@if [ -f $(GUI_TARGET) ]; then \
		sudo cp $(GUI_TARGET) /usr/local/bin/$(GUI_TARGET); \
		sudo chmod +x /usr/local/bin/$(GUI_TARGET); \
	fi
	@echo "$(GREEN)✓ Installed!$(NC)"
	@echo "  Run 'toothdroid' for CLI"
	@echo "  Run 'toothdroid-gui' for GUI"

# Create desktop file for GUI
install-desktop: install
	@echo "$(CYAN)Creating desktop entry...$(NC)"
	@echo "[Desktop Entry]" | sudo tee /usr/share/applications/toothdroid.desktop > /dev/null
	@echo "Name=ToothDroid" | sudo tee -a /usr/share/applications/toothdroid.desktop > /dev/null
	@echo "Comment=Bluetooth Device Manager" | sudo tee -a /usr/share/applications/toothdroid.desktop > /dev/null
	@echo "Exec=/usr/local/bin/toothdroid-gui" | sudo tee -a /usr/share/applications/toothdroid.desktop > /dev/null
	@echo "Icon=bluetooth" | sudo tee -a /usr/share/applications/toothdroid.desktop > /dev/null
	@echo "Terminal=false" | sudo tee -a /usr/share/applications/toothdroid.desktop > /dev/null
	@echo "Type=Application" | sudo tee -a /usr/share/applications/toothdroid.desktop > /dev/null
	@echo "Categories=Settings;HardwareSettings;" | sudo tee -a /usr/share/applications/toothdroid.desktop > /dev/null
	@echo "Keywords=bluetooth;wireless;audio;" | sudo tee -a /usr/share/applications/toothdroid.desktop > /dev/null
	@echo "$(GREEN)✓ Desktop entry created$(NC)"

# Uninstall
uninstall:
	@echo "$(CYAN)Uninstalling ToothDroid...$(NC)"
	@sudo rm -f /usr/local/bin/$(CLI_TARGET)
	@sudo rm -f /usr/local/bin/$(GUI_TARGET)
	@sudo rm -f /usr/share/applications/toothdroid.desktop
	@echo "$(GREEN)✓ Uninstalled$(NC)"

# Check dependencies
check-deps:
	@echo "$(CYAN)Checking dependencies...$(NC)"
	@which bluetoothctl > /dev/null || (echo "$(YELLOW)⚠ bluetoothctl not found. Install bluez.$(NC)" && exit 1)
	@which pactl > /dev/null || echo "$(YELLOW)⚠ pactl not found. Audio features limited.$(NC)"
	@which rfkill > /dev/null || echo "$(YELLOW)⚠ rfkill not found. Adapter unblock may fail.$(NC)"
	@pkg-config --exists gtkmm-4.0 && echo "$(GREEN)✓ gtkmm-4.0 found$(NC)" || echo "$(YELLOW)⚠ gtkmm-4.0 not found (GUI disabled)$(NC)"
	@echo "$(GREEN)✓ Core dependencies OK$(NC)"

# Lint check
lint:
	@echo "$(CYAN)Running static analysis...$(NC)"
	@cppcheck --enable=all --std=c++17 --suppress=missingIncludeSystem \
		--suppress=unusedFunction $(CLI_SRCS) $(GUI_SRCS) include/*.h gui/*.h 2>&1 | head -50

# Format code
format:
	@echo "$(CYAN)Formatting code...$(NC)"
	@clang-format -i $(CLI_SRCS) $(GUI_SRCS) include/*.h gui/*.h 2>/dev/null || echo "$(YELLOW)⚠ clang-format not found$(NC)"

# Help
help:
	@echo ""
	@echo "$(CYAN)ToothDroid Makefile$(NC)"
	@echo "===================="
	@echo ""
	@echo "Targets:"
	@echo "  $(GREEN)make$(NC)             - Build both CLI and GUI"
	@echo "  $(GREEN)make cli$(NC)         - Build CLI only"
	@echo "  $(GREEN)make gui$(NC)         - Build GUI only"
	@echo "  $(GREEN)make run$(NC)         - Build and run CLI"
	@echo "  $(GREEN)make run-gui$(NC)     - Build and run GUI"
	@echo "  $(GREEN)make debug$(NC)       - Build with debug symbols"
	@echo "  $(GREEN)make release$(NC)     - Build optimized release"
	@echo "  $(GREEN)make clean$(NC)       - Remove build artifacts"
	@echo "  $(GREEN)make install$(NC)     - Install to /usr/local/bin"
	@echo "  $(GREEN)make install-desktop$(NC) - Create .desktop file"
	@echo "  $(GREEN)make uninstall$(NC)   - Remove installation"
	@echo "  $(GREEN)make check-deps$(NC)  - Check system dependencies"
	@echo "  $(GREEN)make lint$(NC)        - Run cppcheck"
	@echo ""
