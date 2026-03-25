#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../utils.sh"

build_kotlin1_9_24() {
    local kotlin_version="1.9.24"
    local box_name="compiler-kotlin.${kotlin_version}"
    local archive_name="${box_name}.tar.gz"
    CHROOT_DIR="$WORK_DIR/$box_name"

    echo "=== Creating Kotlin ${kotlin_version} sandbox ==="

    create_base_system "$CHROOT_DIR"

    install_packages "$CHROOT_DIR" \
        openjdk-21-jdk-headless \
        curl \
        unzip

    echo "/usr/lib/jvm/java-21-openjdk-amd64/lib" | sudo tee "$CHROOT_DIR/etc/ld.so.conf.d/java.conf"
    sudo chroot "$CHROOT_DIR" /sbin/ldconfig

    local kotlin_url="https://github.com/JetBrains/kotlin/releases/download/v${kotlin_version}/kotlin-compiler-${kotlin_version}.zip"
    sudo chroot "$CHROOT_DIR" bash -c "
        cd /tmp
        curl -L -o kotlin-compiler.zip '${kotlin_url}'
        unzip -q kotlin-compiler.zip -d /opt
        rm kotlin-compiler.zip

        ln -sf /opt/kotlinc/bin/kotlin /usr/local/bin/kotlin
        ln -sf /opt/kotlinc/bin/kotlinc /usr/local/bin/kotlinc
    "

    sudo chroot "$CHROOT_DIR" apt-get remove -y --purge curl unzip
    sudo chroot "$CHROOT_DIR" apt-get autoremove -y
    sudo chroot "$CHROOT_DIR" apt-get clean

    sudo chroot "$CHROOT_DIR" java -version
    sudo chroot "$CHROOT_DIR" kotlinc -version

    minimize_system "$CHROOT_DIR"
    create_archive "$CHROOT_DIR" "$archive_name"

    echo "Kotlin ${kotlin_version} box created: $WORK_DIR/$archive_name"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    check_dependencies
    init_work_dir "${1:-}"
    build_kotlin1_9_24
fi
