#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../utils.sh"

build_cpython3_13_numpy() {
    local box_name="compiler-python.3.13.5-numpy"
    local archive_name="${box_name}.tar.gz"
    CHROOT_DIR="$WORK_DIR/$box_name"

    echo "=== Creating Python 3.13 + numpy sandbox ==="

    create_base_system "$CHROOT_DIR"

    install_packages "$CHROOT_DIR" \
        python3.13 \
        python3-numpy

    sudo chroot "$CHROOT_DIR" bash -c "
        if [ ! -e /usr/bin/python3 ]; then
            ln -s /usr/bin/python3.13 /usr/bin/python3
        fi
        if [ ! -e /usr/bin/python ]; then
            ln -s /usr/bin/python3.13 /usr/bin/python
        fi
    "

    minimize_system "$CHROOT_DIR"

    # Python requires /dev/urandom at runtime
    sudo mkdir -p "$CHROOT_DIR/dev"
    sudo touch "$CHROOT_DIR/dev/urandom"

    create_archive "$CHROOT_DIR" "$archive_name"

    echo "Python 3.13 + numpy box created: $WORK_DIR/$archive_name"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    check_dependencies
    init_work_dir "${1:-}"
    build_cpython3_13_numpy
fi
