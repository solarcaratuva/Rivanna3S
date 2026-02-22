#!/bin/bash
# Build and run the VN200 IMU test

set -e

DIR="$(cd "$(dirname "$0")" && pwd)"

mkdir -p "$DIR/build"
cd "$DIR/build"
cmake ..
make -j$(nproc)

echo ""
echo "=== Running vn_test ==="
echo ""
./vn_test
