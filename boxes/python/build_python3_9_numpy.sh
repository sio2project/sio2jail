#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../utils.sh"

PYTHON3_9_URL="https://downloads.sio2project.mimuw.edu.pl/sandboxes"
PYTHON3_9_ARCHIVE="compiler-python3.9.2-numpy_amd64.tar.gz"

build_python3_9_numpy() {
    local box_name="python3_9"

    echo "=== Downloading Python 3.9 + numpy sandbox ==="

    cd "$WORK_DIR" || exit 1

    if [ ! -e "$PYTHON3_9_ARCHIVE" ]; then
        echo "Downloading $PYTHON3_9_ARCHIVE"
        wget "$PYTHON3_9_URL/$PYTHON3_9_ARCHIVE"
    fi

    # Extract and rename to python3_9
    tar -xf "$PYTHON3_9_ARCHIVE"
    if [ -d "compiler-python3.9.2-numpy_amd64" ] && [ ! -d "$box_name" ]; then
        mv "compiler-python3.9.2-numpy_amd64" "$box_name"
    fi

    # Re-archive as python3_9.tar.gz
    tar -czf "${box_name}.tar.gz" "$box_name"
    sha256sum "$WORK_DIR/${box_name}.tar.gz" >> "$MANIFEST"

    echo "Python 3.9 + numpy box created: $WORK_DIR/${box_name}.tar.gz"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    init_work_dir "${1:-}"
    build_python3_9_numpy
fi
