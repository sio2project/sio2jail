#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../utils.sh"

build_gcc14() {
    local box_name="compiler-gcc.14_2_0"
    local archive_name="${box_name}.tar.gz"
    CHROOT_DIR="$WORK_DIR/$box_name"

    echo "=== Creating G++ 14 sandbox ==="

    create_base_system "$CHROOT_DIR"

    install_packages "$CHROOT_DIR" \
        g++-14 \
        libc6-dev \
        libstdc++-14-dev

    sudo chroot "$CHROOT_DIR" update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 100

    minimize_system "$CHROOT_DIR"
    create_archive "$CHROOT_DIR" "$archive_name"

    echo "G++ 14 box created: $WORK_DIR/$archive_name"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    check_dependencies
    init_work_dir "${1:-}"
    build_gcc14
fi
