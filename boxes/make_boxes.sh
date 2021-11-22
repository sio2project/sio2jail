#!/usr/bin/env bash

set -e

source "`dirname "${BASH_SOURCE}"`/utils.sh"

MANIFEST="manifest.txt"

make_minimal() {
    export BOX="minimal"
    empty_box
    # nothing

    build_box
    clean_box
    manifest_box >> $MANIFEST
}

make_busybox() {
    export BOX="busybox"
    empty_box
    # busybox with dependencies
    extract_deb "busybox/busybox_1.22.0-19+b3_amd64.deb"
    extract_deb "glibc/libc6_2.24-11+deb9u4_amd64.deb"
    extract_deb "gcc-6/libgcc1_6.3.0-18+deb9u1_amd64.deb"
    extract_deb "gcc-6/gcc-6-base_6.3.0-18+deb9u1_amd64.deb"
    build_box
    clean_box
    manifest_box >> $MANIFEST
}

make_python2() {
    export BOX="python2"
    empty_box
    # python2.7 with dependencies
    extract_deb "python2.7/python2.7-minimal_2.7.13-2+deb9u3_amd64.deb"
    extract_deb "python2.7/libpython2.7-minimal_2.7.13-2+deb9u3_amd64.deb"
    extract_deb "zlib/zlib1g_1.2.8.dfsg-5_amd64.deb"
    extract_deb "glibc/libc6_2.24-11+deb9u4_amd64.deb"
    extract_deb "gcc-6/libgcc1_6.3.0-18+deb9u1_amd64.deb"
    extract_deb "gcc-6/gcc-6-base_6.3.0-18+deb9u1_amd64.deb"

    build_box
    clean_box
    manifest_box >> $MANIFEST
}

make_python3() {
    export BOX="python3"
    empty_box
    # python3.5 with dependencies
    extract_deb "python3.5/python3.5-minimal_3.5.3-1+deb9u1_amd64.deb"
    extract_deb "python3.5/libpython3.5-minimal_3.5.3-1+deb9u1_amd64.deb"
    extract_deb "expat/libexpat1_2.2.0-2+deb9u3_amd64.deb"
    extract_deb "zlib/zlib1g_1.2.8.dfsg-5_amd64.deb"
    extract_deb "openssl/libssl1.1_1.1.0l-1~deb9u1_amd64.deb"
    extract_deb "glibc/libc6_2.24-11+deb9u4_amd64.deb"
    extract_deb "gcc-6/libgcc1_6.3.0-18+deb9u1_amd64.deb"
    extract_deb "gcc-6/gcc-6-base_6.3.0-18+deb9u1_amd64.deb"

    create_file "/dev/urandom"

    build_box
    clean_box
    manifest_box >> $MANIFEST
}

make_python3_9() {
    short_box_name="python3_9"
    box_name="compiler-python3.9.2-numpy_amd64.tar.gz"
    box_csum=`cat $MANIFEST | grep $BOX | awk '{ print $1 }'`
    URL="https://downloads.sio2project.mimuw.edu.pl/sandboxes"

    if [ -e "$box_name" ]; then
        csum=`sha256sum "$box_name" | awk '{ print $1 }'`
        if [ "$csum" != "$box_csum" ]; then
            echo "** Box $box_name has changed, removing it"
            rm "$box_name"
            rm -r "$short_box_name"
            echo "** Downloading box $box_name"
            [ -e "$box_name" ] || wget "$URL/$box_name"
        else
            echo "** Box $box_name hasn't changed, using it"
        fi
    else
            echo "** Downloading box $box_name"
            [ -e "$box_name" ] || wget "$URL/$box_name"
    fi

}

echo -n > $MANIFEST
make_minimal
make_busybox
make_python2
make_python3
make_python3_9
