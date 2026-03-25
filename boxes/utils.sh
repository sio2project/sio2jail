#!/usr/bin/env bash

# Shared utilities for building sio2jail sandbox boxes using debootstrap

DEBIAN_SUITE="trixie"
DEBIAN_MIRROR="http://deb.debian.org/debian"

check_dependencies() {
    command -v debootstrap >/dev/null 2>&1 || { echo "debootstrap is required but not installed. Install with: sudo apt-get install debootstrap"; exit 1; }
    command -v tar >/dev/null 2>&1 || { echo "tar is required but not installed."; exit 1; }
    command -v chroot >/dev/null 2>&1 || { echo "chroot is required but not installed."; exit 1; }
}

cleanup() {
    if [ -n "$CHROOT_DIR" ] && [ -d "$CHROOT_DIR" ]; then
        echo "Cleaning up chroot directory: $CHROOT_DIR"
        if [ -d "$CHROOT_DIR/proc" ] && mountpoint -q "$CHROOT_DIR/proc"; then
            sudo umount "$CHROOT_DIR/proc" || true
        fi
        if [ -d "$CHROOT_DIR/sys" ] && mountpoint -q "$CHROOT_DIR/sys"; then
            sudo umount "$CHROOT_DIR/sys" || true
        fi
        if [ -d "$CHROOT_DIR/dev" ] && mountpoint -q "$CHROOT_DIR/dev"; then
            sudo umount "$CHROOT_DIR/dev" || true
        fi
        sudo rm -rf "$CHROOT_DIR"
    fi
}

# Create base minimal system
# Usage: create_base_system <target_dir> [suite]
create_base_system() {
    local target_dir="$1"
    local suite="${2:-$DEBIAN_SUITE}"
    echo "Creating base Debian $suite system in $target_dir"

    sudo debootstrap --variant=minbase --include=ca-certificates "$suite" "$target_dir" "$DEBIAN_MIRROR"

    sudo mount -t proc proc "$target_dir/proc"
    sudo mount -t sysfs sysfs "$target_dir/sys"
    sudo mount --bind /dev "$target_dir/dev"
}

install_packages() {
    local chroot_dir="$1"
    shift
    local packages="$@"

    echo "Installing packages: $packages"
    sudo chroot "$chroot_dir" apt-get update
    sudo chroot "$chroot_dir" apt-get install -y --no-install-recommends $packages
    sudo chroot "$chroot_dir" apt-get clean
    sudo chroot "$chroot_dir" rm -rf /var/lib/apt/lists/*
}

minimize_system() {
    local chroot_dir="$1"

    echo "Minimizing filesystem"
    sudo chroot "$chroot_dir" bash -c "
        rm -rf /usr/share/doc/* /usr/share/man/* /usr/share/locale/* /var/cache/apt/* /var/lib/apt/lists/*
        rm -rf /usr/share/info/* /usr/share/lintian/* /usr/share/common-licenses/*
        if [ -d /usr/share/i18n/locales/ ]; then
            find /usr/share/i18n/locales/ -mindepth 1 -maxdepth 1 ! -name 'en_US' -exec rm -r {} + 2>/dev/null || true
        fi
        find /var/log -type f -exec truncate -s 0 {} + 2>/dev/null || true
    "

    sudo umount "$chroot_dir/proc" || true
    sudo umount "$chroot_dir/sys" || true
    sudo umount "$chroot_dir/dev" || true

    # Remove mount points but keep /tmp and /dev (needed by sio2jail bind mounts)
    sudo rm -rf "$chroot_dir/proc"
    sudo rm -rf "$chroot_dir/sys"
    sudo mkdir -p "$chroot_dir/proc"
    sudo mkdir -p "$chroot_dir/tmp"
    sudo mkdir -p "$chroot_dir/dev"
}

create_archive() {
    local source_dir="$1"
    local archive_name="$2"

    echo "Creating archive: $archive_name"
    cd "$WORK_DIR" || exit 1
    sudo tar -czf "$archive_name" "$(basename "$source_dir")"

    sha256sum "$WORK_DIR/$archive_name" >> "$MANIFEST"
}

# Initialize work directory
# Usage: init_work_dir [output_dir]
init_work_dir() {
    WORK_DIR="${1:-$(pwd)/build}"
    MANIFEST="$WORK_DIR/manifest.txt"
    mkdir -p "$WORK_DIR"
}

# --- Legacy box functions (lightweight boxes from individual .deb extractions) ---

DEB_REPO="http://archive.debian.org/debian/pool/main"

empty_box() {
    mkdir -pv "$WORK_DIR/$BOX"/{,proc}
    touch "$WORK_DIR/$BOX/exe"
    chmod +wx "$WORK_DIR/$BOX/exe"
}

extract_deb() {
    local path="$1"
    local dpkg_file
    dpkg_file="$(basename "$path")"

    [ -e "$WORK_DIR/$dpkg_file" ] || wget -O "$WORK_DIR/$dpkg_file" "$DEB_REPO/${path:0:1}/$path"
    dpkg -X "$WORK_DIR/$dpkg_file" "$WORK_DIR/$BOX"
}

create_file() {
    mkdir -pv "$(dirname "$WORK_DIR/$BOX$1")"
    touch "$WORK_DIR/$BOX$1"
}

build_legacy_box() {
    echo "Creating archive: $BOX.tar.gz"
    cd "$WORK_DIR" || exit 1
    tar -cvf "$BOX.tar.gz" "$BOX"
    sha256sum "$WORK_DIR/$BOX.tar.gz" >> "$MANIFEST"
}

clean_legacy_box() {
    rm -rfv "$WORK_DIR/$BOX"
}
