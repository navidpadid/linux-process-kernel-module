#!/bin/bash
# Quick reference for Linux Process Information Kernel Module testing

cat << 'EOF'
╔══════════════════════════════════════════════════════════════╗
║    Linux Process Info Kernel Module - Quick Reference       ║
╚══════════════════════════════════════════════════════════════╝

LOCAL TESTING (No Kernel Required)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  # Build and run unit tests
  make unit

  # Build everything
  make all

  # Clean artifacts
  make clean


QEMU SETUP (Run Once)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  ./scripts/qemu-setup.sh


START VM
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  ./scripts/qemu-run.sh

  Login: ubuntu / ubuntu
  Exit:  Ctrl+A then X


AUTO TEST (From Host, in Another Terminal)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  # Full automated test suite
  ./scripts/qemu-test.sh


CONNECT TO VM
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  # SSH (uses keys if configured, else password: ubuntu)
  ssh -p 2222 ubuntu@localhost

  # Copy files TO VM
  scp -P 2222 file.txt ubuntu@localhost:~/

  # Copy files FROM VM
  scp -P 2222 ubuntu@localhost:~/file.txt ./


MANUAL TESTING (Inside VM)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  # Build everything
  make clean && make all

  # Run unit tests (no kernel required)
  make unit

  # Install kernel module
  sudo make install
  # OR: sudo insmod build/elf_det.ko

  # Verify module loaded
  lsmod | grep elf_det

  # Test with user program
  ./build/proc_elf_ctrl

  # Or test manually with proc interface
  echo "1" | sudo tee /proc/elf_det/pid
  sudo cat /proc/elf_det/det

  # Check kernel logs
  sudo dmesg | tail -20

  # Unload module
  sudo make uninstall
  # OR: sudo rmmod elf_det


BUILD TARGETS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  make all       - Build module and user program
  make module    - Build kernel module only
  make user      - Build user program only
  make unit      - Build and run unit tests
  make install   - Install kernel module
  make uninstall - Remove kernel module
  make test      - Install module and run user program
  make clean     - Remove build artifacts
  make help      - Show all targets


CLEANUP
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  # Remove QEMU environment
  rm -rf scripts/qemu-env/

  # Start fresh
  ./scripts/qemu-setup.sh


PROJECT STRUCTURE
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  src/elf_det.c               - Kernel module source
  src/proc_elf_ctrl.c         - User program source
  src/elf_det_tests.c         - Unit tests for elf_det
  src/proc_elf_ctrl_tests.c   - Unit tests for proc_elf_ctrl
  src/elf_helpers.h           - Helper functions (CPU, BSS, heap)
  src/user_helpers.h          - Helper functions (path building)
  build/                      - Compiled artifacts


MEMORY INFORMATION EXTRACTED
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  PID          - Process ID
  NAME         - Process name
  CPU(%)       - CPU usage percentage
  START_CODE   - Start of executable code section
  END_CODE     - End of executable code section
  START_DATA   - Start of initialized data section
  END_DATA     - End of initialized data section
  BSS_START    - Start of uninitialized data section
  BSS_END      - End of uninitialized data section
  HEAP_START   - Start of heap (dynamic memory)
  HEAP_END     - Current end of heap (brk)
  ELF          - ELF header location


TIPS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  - Always run 'make unit' first (no kernel required)
  - Use QEMU for safe kernel module testing
  - First VM boot takes 1-2 minutes for cloud-init
  - VM is completely isolated - crashes won't affect host
  - KVM acceleration used automatically if available
  - Change default password after first login
  - Always unload module before rebuilding


TROUBLESHOOTING
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  # Module won't load
  dmesg | tail -20
  ls /lib/modules/$(uname -r)/build

  # Can't connect to VM
  ps aux | grep qemu
  ssh -p 2222 ubuntu@localhost

  # Reset QEMU environment
  rm -rf scripts/qemu-env/
  ./scripts/qemu-setup.sh


MORE INFO
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  README.md          - Complete project documentation
  scripts/README.md  - QEMU testing details
  LICENSE            - Dual BSD/GPL license

EOF
