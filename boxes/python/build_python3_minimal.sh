#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../utils.sh"

build_python3_minimal() {
    export BOX="python3"

    echo "=== Creating Python 3.5 minimal sandbox ==="

    empty_box
    extract_deb "python3.5/python3.5-minimal_3.5.3-1+deb9u1_amd64.deb"
    extract_deb "python3.5/libpython3.5-minimal_3.5.3-1+deb9u1_amd64.deb"
    extract_deb "expat/libexpat1_2.2.0-2+deb9u3_amd64.deb"
    extract_deb "zlib/zlib1g_1.2.8.dfsg-5_amd64.deb"
    extract_deb "openssl/libssl1.1_1.1.0l-1~deb9u1_amd64.deb"
    extract_deb "glibc/libc6_2.24-11+deb9u4_amd64.deb"
    extract_deb "gcc-6/libgcc1_6.3.0-18+deb9u1_amd64.deb"
    extract_deb "gcc-6/gcc-6-base_6.3.0-18+deb9u1_amd64.deb"

    create_file "/dev/urandom"

    build_legacy_box
    clean_legacy_box

    echo "Python 3.5 box created: $WORK_DIR/$BOX.tar.gz"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    init_work_dir "${1:-}"
    build_python3_minimal
fi
