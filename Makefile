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

.PHONY: all clean module user install uninstall test help unit check format checkpatch sparse cppcheck

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

# Function-level unit tests (user-space)
unit:
	@echo "Building function-level unit tests..."
	@mkdir -p $(BUILD_DIR)
	gcc -Wall -I$(SRC_DIR) -o $(BUILD_DIR)/elf_det_tests $(SRC_DIR)/elf_det_tests.c
	gcc -Wall -I$(SRC_DIR) -o $(BUILD_DIR)/proc_elf_ctrl_tests $(SRC_DIR)/proc_elf_ctrl_tests.c
	@echo "Running unit tests..."
	@$(BUILD_DIR)/elf_det_tests
	@$(BUILD_DIR)/proc_elf_ctrl_tests
	@echo "All function-level unit tests passed!"

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

# Static Analysis and Code Quality Checks

# Run all code quality checks
check: checkpatch sparse cppcheck
	@echo ""
	@echo "================================================"
	@echo "All static analysis checks completed!"
	@echo "================================================"

# Check kernel coding style with checkpatch.pl
checkpatch:
	@echo "Running checkpatch.pl (kernel coding style)..."
	@if [ -f /lib/modules/$(shell uname -r)/build/scripts/checkpatch.pl ]; then \
		for file in $(SRC_DIR)/*.c $(SRC_DIR)/*.h; do \
			case "$$file" in \
				*.mod.c) ;; \
				*) if [ -f "$$file" ]; then \
					echo "Checking $$file..."; \
					/lib/modules/$(shell uname -r)/build/scripts/checkpatch.pl --no-tree --strict --file $$file || true; \
				fi ;; \
			esac; \
		done; \
	else \
		echo "checkpatch.pl not found. Install kernel sources."; \
	fi

# Run sparse static analyzer
sparse:
	@echo "Running sparse static analyzer..."
	@if command -v sparse >/dev/null 2>&1; then \
		$(MAKE) -C $(KDIR) M=$(PWD)/$(SRC_DIR) C=2 CF="-D__CHECK_ENDIAN__" modules || true; \
	else \
		echo "sparse not found. Install with: sudo apt-get install sparse"; \
	fi

# Run cppcheck static analyzer
cppcheck:
	@echo "Running cppcheck static analyzer..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --inconclusive --force \
			--suppressions-list=.cppcheck-suppressions \
			--inline-suppr \
			-I$(SRC_DIR) \
			-I/lib/modules/$(shell uname -r)/build/include \
			--error-exitcode=0 \
			$(SRC_DIR)/*.c 2>&1 | grep -v "Cppcheck cannot find" || true; \
	else \
		echo "cppcheck not found. Install with: sudo apt-get install cppcheck"; \
	fi

# Format code with clang-format
format:
	@echo "Formatting code with clang-format..."
	@if command -v clang-format >/dev/null 2>&1; then \
		for file in $(SRC_DIR)/*.c $(SRC_DIR)/*.h; do \
			if [ -f "$$file" ]; then \
				echo "Formatting $$file..."; \
				clang-format -i $$file; \
			fi \
		done; \
		echo "Code formatting complete!"; \
	else \
		echo "clang-format not found. Install with: sudo apt-get install clang-format"; \
	fi

# Check if code is properly formatted (CI-friendly)
format-check:
	@echo "Checking code formatting..."
	@if command -v clang-format >/dev/null 2>&1; then \
		UNFORMATTED=$$(for file in $(SRC_DIR)/*.c $(SRC_DIR)/*.h; do \
			if [ -f "$$file" ]; then \
				clang-format -output-replacements-xml $$file | grep -q "<replacement " && echo "$$file"; \
			fi \
		done); \
		if [ -n "$$UNFORMATTED" ]; then \
			echo "The following files need formatting:"; \
			echo "$$UNFORMATTED"; \
			exit 1; \
		else \
			echo "All files are properly formatted!"; \
		fi; \
	else \
		echo "clang-format not found. Install with: sudo apt-get install clang-format"; \
		exit 1; \
	fi

# Help target
help:
	@echo "Linux Process Information Kernel Module - Build Targets:"
	@echo ""
	@echo "Build Targets:"
	@echo "  make all        - Build both kernel module and user program (default)"
	@echo "  make module     - Build kernel module only"
	@echo "  make user       - Build user program only"
	@echo "  make unit       - Build and run function-level unit tests"
	@echo "  make install    - Install kernel module (requires root)"
	@echo "  make uninstall  - Remove kernel module (requires root)"
	@echo "  make test       - Install module and run user program"
	@echo "  make clean      - Remove all build artifacts"
	@echo ""
	@echo "Code Quality Targets:"
	@echo "  make check      - Run all static analysis checks"
	@echo "  make checkpatch - Check kernel coding style with checkpatch.pl"
	@echo "  make sparse     - Run sparse static analyzer"
	@echo "  make cppcheck   - Run cppcheck static analyzer"
	@echo "  make format     - Format code with clang-format"
	@echo "  make format-check - Check if code is properly formatted (CI)"
	@echo ""
	@echo "  make help       - Show this help message"
	@echo ""
	@echo "Note: Building the kernel module requires kernel headers to be installed."
