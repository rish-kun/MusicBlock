#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
app="$project_dir/build/MusicBlock.app"
contents="$app/Contents"

mkdir -p "$contents/MacOS"
cp "$project_dir/Info.plist" "$contents/Info.plist"
clang -arch arm64 -Os -Wl,-dead_strip \
    "$project_dir/main.m" -framework AppKit \
    -o "$contents/MacOS/MusicBlock"
codesign --force --sign - "$app"

printf 'Built %s\n' "$app"
