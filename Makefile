# Kernel Module and User Program Makefile

# Kernel module name
obj-m := elf_det.o

# Kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build

# Current directory
PWD := $(shell pwd)

# User program
USER_PROG := proc_elf_ctrl

# Source directory
SRC_DIR := src

# Build directory for user program
BUILD_DIR := build

.PHONY: all clean module user install uninstall test help

# Default target
all: module user

# Build kernel module
module:
	@echo "Building kernel module..."
	$(MAKE) -C $(KDIR) M=$(PWD)/$(SRC_DIR) modules
	@mkdir -p $(BUILD_DIR)
	@cp $(SRC_DIR)/*.ko $(BUILD_DIR)/ 2>/dev/null || true
	@echo "Kernel module built successfully!"

# Build user program
user:
	@echo "Building user program..."
	@mkdir -p $(BUILD_DIR)
	gcc -Wall -o $(BUILD_DIR)/$(USER_PROG) $(SRC_DIR)/$(USER_PROG).c
	@echo "User program built successfully!"

# Install kernel module (requires root)
install: module
	@echo "Installing kernel module..."
	sudo insmod $(BUILD_DIR)/elf_det.ko
	@echo "Module installed. Check with: lsmod | grep elf_det"

# Uninstall kernel module (requires root)
uninstall:
	@echo "Uninstalling kernel module..."
	sudo rmmod elf_det 2>/dev/null || true
	@echo "Module uninstalled."

# Test: install module and run user program
test: install
	@echo "Running user program..."
	@echo "Enter PID when prompted, or press Ctrl+C to exit"
	$(BUILD_DIR)/$(USER_PROG)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	$(MAKE) -C $(KDIR) M=$(PWD)/$(SRC_DIR) clean
	rm -rf $(BUILD_DIR)
	rm -f $(SRC_DIR)/*.o $(SRC_DIR)/*.ko $(SRC_DIR)/*.mod.c $(SRC_DIR)/*.mod $(SRC_DIR)/.*.cmd
	rm -f $(SRC_DIR)/Module.symvers $(SRC_DIR)/modules.order
	rm -rf $(SRC_DIR)/.tmp_versions
	@echo "Clean complete!"

# Help target
help:
	@echo "Linux Process Information Kernel Module - Build Targets:"
	@echo ""
	@echo "  make all        - Build both kernel module and user program (default)"
	@echo "  make module     - Build kernel module only"
	@echo "  make user       - Build user program only"
	@echo "  make install    - Install kernel module (requires root)"
	@echo "  make uninstall  - Remove kernel module (requires root)"
	@echo "  make test       - Install module and run user program"
	@echo "  make clean      - Remove all build artifacts"
	@echo "  make help       - Show this help message"
	@echo ""
	@echo "Note: Building the kernel module requires kernel headers to be installed."
