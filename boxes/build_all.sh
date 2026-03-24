#!/usr/bin/env bash
set -e

# Build all sio2jail sandbox boxes (new debootstrap-based boxes only)
#
# Usage:
#   sudo ./build_all.sh                          # build all boxes to ./build
#   sudo ./build_all.sh /tmp/boxes               # build all boxes to /tmp/boxes
#   sudo ./build_all.sh ./build gcc14 openjdk21  # build only selected boxes

BOXES_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$BOXES_ROOT/utils.sh"
trap cleanup EXIT

source "$BOXES_ROOT/cpp/build_gcc14.sh"
source "$BOXES_ROOT/python/build_cpython3.13_numpy.sh"
source "$BOXES_ROOT/python/build_pypy3.11.sh"
source "$BOXES_ROOT/java/build_openjdk21.sh"
source "$BOXES_ROOT/kotlin/build_kotlin1.9.24.sh"

ALL_BOXES="gcc14 cpython3_13_numpy pypy3_11 openjdk21 kotlin1_9_24"

build_box() {
    local name="$1"
    case "$name" in
        gcc14)              build_gcc14 ;;
        cpython3_13_numpy)  build_cpython3_13_numpy ;;
        pypy3_11)           build_pypy3_11 ;;
        openjdk21)          build_openjdk21 ;;
        kotlin1_9_24)       build_kotlin1_9_24 ;;
        *)
            echo "Unknown box: $name"
            echo "Available boxes: $ALL_BOXES"
            exit 1
            ;;
    esac
    cleanup
    CHROOT_DIR=""
}

main() {
    check_dependencies

    # First argument is output dir (or default)
    local output_dir="${1:-$(pwd)/build}"
    shift 2>/dev/null || true

    init_work_dir "$output_dir"

    # Clear manifest
    echo -n > "$MANIFEST"

    # Remaining arguments are box names, or build all
    local boxes_to_build="${@:-$ALL_BOXES}"

    echo "Starting box builds in: $WORK_DIR"
    echo "Boxes: $boxes_to_build"
    echo

    for box in $boxes_to_build; do
        build_box "$box"
    done

    echo
    echo "=== Build complete ==="
    echo "Archives created in: $WORK_DIR/"
    echo "Manifest: $MANIFEST"
    cat "$MANIFEST"
}

main "$@"
