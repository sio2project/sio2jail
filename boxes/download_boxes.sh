#!/usr/bin/env bash

DEFAULT_URL="https://hitagi.dasie.mimuw.edu.pl/files/boxes/"

if [ -z "$URL" ]; then
    echo "** URL not set, using default $DEFAULT_URL"
    URL="$DEFAULT_URL"
fi

if [ -z "$1" ]; then
    TARGET="."
else
    TARGET="$1"
fi

mkdir -pv "$TARGET"

curl -L -s $URL/manifest.txt | while read line; do
    box_name=`echo $line | awk '{ print $2 }'`
    box_csum=`echo $line | awk '{ print $1 }'`

    if [ -e "$TARGET/$box_name" ]; then
        csum=`sha256sum "$TARGET/$box_name" | awk '{ print $1 }'`
        if [ "$csum" != "$box_csum" ]; then
            echo "** Box $box_name has changed, removing it"
            rm "$TARGET/$box_name"
        else
            echo "** Box $box_name hasn't changed, using it"
        fi
    fi

    echo "** Downloading box $box_name"
    [ -e "$TARGET/$box_name" ] || wget -P "$TARGET" "$URL/$box_name"

    csum=`sha256sum "$TARGET/$box_name" | awk '{ print $1 }'`
    if [ "$csum" != "$box_csum" ]; then
        echo "** Verification of box $box_name failed"
    else
        echo "** Extracting box $box_name"
        tar -C "$TARGET" -xvf "$TARGET/$box_name"
    fi
done
