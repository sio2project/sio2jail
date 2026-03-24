#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../utils.sh"

build_python2_7() {
    export BOX="python2"

    echo "=== Creating Python 2.7 sandbox ==="

    empty_box
    extract_deb "python2.7/python2.7-minimal_2.7.13-2+deb9u3_amd64.deb"
    extract_deb "python2.7/libpython2.7-minimal_2.7.13-2+deb9u3_amd64.deb"
    extract_deb "zlib/zlib1g_1.2.8.dfsg-5_amd64.deb"
    extract_deb "glibc/libc6_2.24-11+deb9u4_amd64.deb"
    extract_deb "gcc-6/libgcc1_6.3.0-18+deb9u1_amd64.deb"
    extract_deb "gcc-6/gcc-6-base_6.3.0-18+deb9u1_amd64.deb"

    build_legacy_box
    clean_legacy_box

    echo "Python 2.7 box created: $WORK_DIR/$BOX.tar.gz"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    init_work_dir "${1:-}"
    build_python2_7
fi
