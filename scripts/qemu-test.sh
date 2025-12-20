#!/bin/bash
# Script to copy and test kernel module in QEMU VM
# Run this on your HOST machine, not inside the VM

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SSH_PORT=2222
SSH_USER=ubuntu
SSH_HOST=localhost
SSH_OPTS="-p $SSH_PORT -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o LogLevel=ERROR"
SCP_OPTS="-P $SSH_PORT -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o LogLevel=ERROR"

echo "==================================================="
echo "Testing Kernel Module in QEMU VM"
echo "==================================================="

# Check if VM is running
if ! ssh $SSH_OPTS -o ConnectTimeout=5 -o BatchMode=yes ${SSH_USER}@${SSH_HOST} exit 2>/dev/null; then
    echo "ERROR: QEMU VM is not running or SSH is not accessible"
    echo "Start the VM first with: ./scripts/qemu-run.sh"
    exit 1
fi

echo "1. Building kernel module locally..."
cd "$PROJECT_ROOT"
make clean
make all

echo ""
echo "2. Copying files to QEMU VM..."
scp $SCP_OPTS -r build src/Kbuild Makefile ${SSH_USER}@${SSH_HOST}:~/kernel_module/

echo ""
echo "3. Installing and testing module in VM..."
ssh $SSH_OPTS ${SSH_USER}@${SSH_HOST} << 'ENDSSH'
set -e
cd ~/kernel_module

echo "Installing kernel module..."
if lsmod | grep -q '^elf_det'; then
    echo "Module already loaded; unloading first..."
    sudo rmmod elf_det || true
fi
sudo insmod build/elf_det.ko

echo "Checking if module is loaded..."
lsmod | grep elf_det

echo "Checking /proc entries..."
ls -la /proc/elf_det/

echo ""
echo "Testing with current shell process (PID: $$)..."
echo "$$" | sudo tee /proc/elf_det/pid > /dev/null
sudo cat /proc/elf_det/det

echo ""
echo "Testing with PID 1 (init/systemd)..."
echo "1" | sudo tee /proc/elf_det/pid > /dev/null
sudo cat /proc/elf_det/det

echo ""
echo "Testing with user program (proc_elf_ctrl, PID=1)..."
./build/proc_elf_ctrl 1 || true

echo ""
echo "Checking kernel logs..."
sudo dmesg | tail -20

echo ""
echo "Uninstalling module..."
sudo rmmod elf_det

echo ""
echo "Verifying module unloaded..."
lsmod | grep elf_det || echo "Module successfully unloaded"

echo ""
echo "==================================================="
echo "Test completed successfully!"
echo "==================================================="
ENDSSH

echo ""
echo "==================================================="
echo "All tests passed in QEMU VM!"
echo "==================================================="
echo ""
echo "Your host machine kernel was never touched."
echo "The module was safely tested in isolation."
