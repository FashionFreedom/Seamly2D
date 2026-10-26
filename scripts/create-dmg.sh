#!/bin/sh
# Stage the app with an Applications shortcut for drag-to-install.
set -eu

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 app-path volume-name output-dmg" >&2
    exit 1
fi

app_path=$1
volume_name=$2
output_dmg=$3

staging_dir=$(mktemp -d "${TMPDIR:-/tmp}/seamly-dmg.XXXXXX")
trap 'rm -rf "$staging_dir"' EXIT
trap 'exit 1' HUP INT TERM

ditto "$app_path" "$staging_dir/$(basename "$app_path")"
ln -s /Applications "$staging_dir/Applications"
hdiutil create -fs HFS+ -srcfolder "$staging_dir" -volname "$volume_name" "$output_dmg"
