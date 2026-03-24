#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../utils.sh"

build_busybox_trixie() {
    export BOX="busybox"

    echo "=== Creating busybox sandbox ==="

    empty_box
    extract_deb "busybox/busybox_1.22.0-19+b3_amd64.deb"
    extract_deb "glibc/libc6_2.24-11+deb9u4_amd64.deb"
    extract_deb "gcc-6/libgcc1_6.3.0-18+deb9u1_amd64.deb"
    extract_deb "gcc-6/gcc-6-base_6.3.0-18+deb9u1_amd64.deb"

    build_legacy_box
    clean_legacy_box

    echo "Busybox box created: $WORK_DIR/$BOX.tar.gz"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    init_work_dir "${1:-}"
    build_busybox_trixie
fi
