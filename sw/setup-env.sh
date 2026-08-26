#!/usr/bin/env bash
# MiniHIL Yocto Environment Initialization Wrapper Script
# Must be sourced from a bash-compatible shell.

if [ "$0" = "${BASH_SOURCE[0]}" ]; then
    echo "Error: This script must be sourced, not executed directly."
    echo "Usage: source sw/setup-env.sh [build-dir]"
    exit 1 2>/dev/null || return 1
fi

# Detect SCRIPT_PATH dynamically
if [ -n "$BASH_SOURCE" ]; then
    SCRIPT_PATH="${BASH_SOURCE[0]}"
elif [ -n "$ZSH_NAME" ]; then
    SCRIPT_PATH="$0"
else
    # Fallback to local directory
    SCRIPT_PATH="./sw/setup-env.sh"
fi

# Resolve absolute paths
SCRIPT_DIR=$(dirname "$(readlink -f "$SCRIPT_PATH" 2>/dev/null || perl -MCwd -e 'print Cwd::abs_path shift' "$SCRIPT_PATH")")
PROJECT_ROOT=$(dirname "$SCRIPT_DIR")

SUBMODULE_DIR="$SCRIPT_DIR/rpi-base-platform"

# 1. Verify Git submodules are initialized
if [ ! -f "$SUBMODULE_DIR/layers/poky/oe-init-build-env" ]; then
    echo "[*] Yocto layers not found. Attempting to initialize Git submodules..."
    (cd "$PROJECT_ROOT" && git submodule update --init --recursive)
    
    if [ ! -f "$SUBMODULE_DIR/layers/poky/oe-init-build-env" ]; then
        echo "[!] Error: Failed to initialize Git submodules. Please check your internet connection and run:"
        echo "    git submodule update --init --recursive"
        return 1
    fi
fi

# 2. Export local config directory as template for Yocto
export TEMPLATECONF="$SCRIPT_DIR/meta-minihil/conf/templates/default"

# 3. Setup build directory path (relative to PROJECT_ROOT if not absolute)
BUILD_DIR="${1:-build-minihil}"
if [[ "$BUILD_DIR" != /* ]]; then
    BUILD_DIR="$SCRIPT_DIR/$BUILD_DIR"
fi

echo "[-] Sourcing Yocto build environment for MiniHIL..."
echo "    Build Directory: $BUILD_DIR"
echo "    Template Conf  : $TEMPLATECONF"

source "$SUBMODULE_DIR/layers/poky/oe-init-build-env" "$BUILD_DIR"
