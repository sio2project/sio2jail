#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../utils.sh"

build_openjdk21() {
    local box_name="compiler-java.21"
    local archive_name="${box_name}.tar.gz"
    CHROOT_DIR="$WORK_DIR/$box_name"

    echo "=== Creating OpenJDK 21 sandbox ==="

    create_base_system "$CHROOT_DIR"

    install_packages "$CHROOT_DIR" openjdk-21-jdk-headless

    echo "/usr/lib/jvm/java-21-openjdk-amd64/lib" | sudo tee "$CHROOT_DIR/etc/ld.so.conf.d/java.conf"
    sudo chroot "$CHROOT_DIR" /sbin/ldconfig

    sudo chroot "$CHROOT_DIR" java -version
    sudo chroot "$CHROOT_DIR" javac -version

    minimize_system "$CHROOT_DIR"
    create_archive "$CHROOT_DIR" "$archive_name"

    echo "OpenJDK 21 box created: $WORK_DIR/$archive_name"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    check_dependencies
    init_work_dir "${1:-}"
    build_openjdk21
fi
