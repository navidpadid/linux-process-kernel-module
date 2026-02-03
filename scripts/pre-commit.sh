#!/bin/bash
# Pre-commit hook to run code quality checks
# Automatically installed in dev container on startup

set -e

echo "Running pre-commit checks..."
echo ""

# Check if analysis tools are installed
check_tool() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Warning: $1 not found. Please use the dev container for development."
        return 1
    fi
    return 0
}

# Array to track failures
FAILED=0

# Run clang-format check
if check_tool clang-format; then
    echo "Checking code formatting..."
    if ! make format-check >/dev/null 2>&1; then
        echo "Code formatting check failed!"
        echo "   Run 'make format' to fix formatting issues"
        FAILED=1
    else
        echo "Code formatting check passed"
    fi
else
    echo "Skipping format check (clang-format not installed)"
fi

echo ""

# Run cppcheck
if check_tool cppcheck; then
    echo "Running cppcheck..."
    if make cppcheck 2>&1 | grep -q "error:"; then
        echo "Cppcheck found potential issues"
        FAILED=1
    else
        echo "Cppcheck passed"
    fi
else
    echo "Skipping cppcheck (not installed)"
fi

echo ""

# Run checkpatch if available
if [ -f /lib/modules/$(uname -r)/build/scripts/checkpatch.pl ]; then
    echo "Running checkpatch..."
    # Get list of staged .c and .h files
    STAGED_FILES=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(c|h)$' || true)
    
    if [ -n "$STAGED_FILES" ]; then
        for file in $STAGED_FILES; do
            if [ -f "$file" ]; then
                if /lib/modules/$(uname -r)/build/scripts/checkpatch.pl --no-tree --file "$file" 2>&1 | grep -q "ERROR:"; then
                    echo "Checkpatch found errors in $file"
                    FAILED=1
                fi
            fi
        done
        if [ $FAILED -eq 0 ]; then
            echo "Checkpatch passed"
        fi
    fi
else
    echo "Skipping checkpatch (kernel sources not found)"
fi

echo ""
echo "================================================"

if [ $FAILED -eq 1 ]; then
    echo "Pre-commit checks failed!"
    echo ""
    echo "Fix the issues or use 'git commit --no-verify' to skip checks"
    exit 1
else
    echo "All pre-commit checks passed!"
fi

exit 0
