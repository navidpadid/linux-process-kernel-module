#!/bin/bash
# Post-create script for dev container setup
# Runs after the container is created

set -e

echo "Setting up development environment..."
echo ""

# Display tool versions
echo "Build tools:"
make --version | head -1
gcc --version | head -1
clang-format --version

echo ""
echo "Installing kernel headers for running kernel..."
sudo apt-get update -qq
if sudo apt-get install -y linux-headers-$(uname -r); then
    echo "✓ Installed linux-headers-$(uname -r)"
else
    echo "⚠ Warning: Could not install headers for $(uname -r), using linux-headers-generic"
fi

echo ""
echo "Installing Git hooks..."
mkdir -p .git/hooks

if [ -f .github/pre-commit.sh ]; then
    cp .github/pre-commit.sh .git/hooks/pre-commit
    chmod +x .git/hooks/pre-commit
    echo "✓ Git pre-commit hook installed"
else
    echo "⚠ Warning: .github/pre-commit.sh not found"
fi

echo ""
echo "================================================"
echo "✓ Dev environment ready!"
echo "================================================"
echo ""
echo "Available kernel headers:"
ls -d /lib/modules/*/build 2>/dev/null | sed 's|/lib/modules/||' | sed 's|/build||' || echo "None found"
echo ""
