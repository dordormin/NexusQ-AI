#!/bin/bash

echo "╔═══════════════════════════════════╗"
echo "║     Building QVM Test Suite      ║"
echo "╚═══════════════════════════════════╝"
echo ""

echo "[1/1] Compiling QVM Unit Tests..."
gcc -o test_qvm \
    tests/test_qvm_unit.c \
    modules/quantum/qvm.c \
    -I modules/quantum/include \
    -lm

if [ $? -eq 0 ]; then
    echo "✓ Build successful!"
    echo ""
    echo "Run tests with: ./test_qvm"
    echo ""
else
    echo "✗ Build failed!"
    exit 1
fi
