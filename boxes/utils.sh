# Various functions usefull during box creation

empty_box() {
    mkdir -pv "$BOX"/{,proc}
    touch "$BOX/exe"
    chmod +wx "$BOX/exe"
}

build_box() {
    tar -cvf "$BOX.tar.gz" "$BOX"
}

clean_box() {
    rm -rfv "$BOX"
}

extract_deb() {
    repo="http://archive.debian.org/debian/pool/main"
    path="$1"
    dpkg=`basename "$path"`

    [ -e "$dpkg" ] || wget -O "$dpkg" "$repo/${path:0:1}/$path"
    dpkg -X "$dpkg" "$BOX"
}

create_file() {
    mkdir -pv `dirname "$BOX/$1"`
    touch "$BOX/$1"
}

manifest_box() {
    box_file="$BOX.tar.gz"
    sha256sum "$box_file"
}
