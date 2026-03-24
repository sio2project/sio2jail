#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../utils.sh"

build_pypy3_11() {
    local box_name="compiler-pypy.3.11"
    local archive_name="${box_name}.tar.gz"
    CHROOT_DIR="$WORK_DIR/$box_name"

    echo "=== Creating PyPy 3.11 sandbox ==="

    create_base_system "$CHROOT_DIR"

    # PyPy post-install requires a working locale
    install_packages "$CHROOT_DIR" locales
    sudo chroot "$CHROOT_DIR" bash -c "
        echo 'en_US.UTF-8 UTF-8' > /etc/locale.gen
        locale-gen
    "

    sudo LANG=en_US.UTF-8 chroot "$CHROOT_DIR" apt-get update
    sudo LANG=en_US.UTF-8 chroot "$CHROOT_DIR" apt-get install -y --no-install-recommends pypy3
    sudo chroot "$CHROOT_DIR" apt-get clean
    sudo chroot "$CHROOT_DIR" rm -rf /var/lib/apt/lists/*

    sudo chroot "$CHROOT_DIR" bash -c "
        if [ ! -e /usr/bin/python3 ]; then
            ln -s /usr/bin/pypy3 /usr/bin/python3
        fi
        if [ ! -e /usr/bin/python ]; then
            ln -s /usr/bin/pypy3 /usr/bin/python
        fi
    "

    minimize_system "$CHROOT_DIR"

    # Python requires /dev/urandom at runtime
    sudo mkdir -p "$CHROOT_DIR/dev"
    sudo touch "$CHROOT_DIR/dev/urandom"

    create_archive "$CHROOT_DIR" "$archive_name"

    echo "PyPy 3.11 box created: $WORK_DIR/$archive_name"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    check_dependencies
    init_work_dir "${1:-}"
    build_pypy3_11
fi
