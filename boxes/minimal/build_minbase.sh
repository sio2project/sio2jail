#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../utils.sh"

build_minbase() {
    export BOX="minimal"

    echo "=== Creating minimal sandbox ==="

    empty_box
    build_legacy_box
    clean_legacy_box

    echo "Minimal box created: $WORK_DIR/$BOX.tar.gz"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    init_work_dir "${1:-}"
    build_minbase
fi
