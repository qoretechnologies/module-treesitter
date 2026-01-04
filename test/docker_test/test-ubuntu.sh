#!/bin/bash
set -e
set -x

# Install dependencies
apt-get update
apt-get install -y build-essential cmake flex bison libpcre2-dev libssl-dev \
    libbz2-dev zlib-dev libmpfr-dev libgmp-dev pkg-config git

# Build Qore if not installed
if ! command -v qore &> /dev/null; then
    echo "Qore not found in PATH, building from source..."
    cd /tmp
    git clone --depth 1 https://github.com/qorelanguage/qore.git
    cd qore
    mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    make install
    ldconfig
fi

# Build the treesitter module
cd /src
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run tests
export QORE_MODULE_DIR=/src/build
qore /src/test/treesitter.qtest -v

echo "All tests passed!"
