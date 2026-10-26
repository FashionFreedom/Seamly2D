#!/bin/sh
# Build a drag-to-install image with a saved Finder layout.
set -eu

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 app-path volume-name output-dmg" >&2
    exit 1
fi

app_path=$1
volume_name=$2
output_dmg=$3

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
assets_dir="$script_dir/../dist/macx/dmg"

staging_dir=$(mktemp -d "${TMPDIR:-/tmp}/seamly-dmg.XXXXXX")
mounted=false
cleanup() {
    if [ "$mounted" = true ]; then
        hdiutil detach "$staging_dir/mount" || hdiutil detach -force "$staging_dir/mount" || return 1
    fi
    rm -rf "$staging_dir"
}
trap cleanup EXIT
trap 'exit 1' HUP INT TERM

mkdir "$staging_dir/contents" "$staging_dir/mount"
ditto "$app_path" "$staging_dir/contents/$(basename "$app_path")"
ln -s /Applications "$staging_dir/contents/Applications"
mkdir "$staging_dir/contents/.background"
cp "$assets_dir/background.tiff" "$staging_dir/contents/.background/background.tiff"

# Use a unique temporary volume name so Finder cannot confuse this image
# with an already mounted release of the same app.
staging_volume="${volume_name}-$(basename "$staging_dir")"
hdiutil create -fs HFS+ -format UDRW -srcfolder "$staging_dir/contents" -volname "$staging_volume" "$staging_dir/writable.dmg"
hdiutil attach -readwrite -noautoopen -nobrowse -mountpoint "$staging_dir/mount" "$staging_dir/writable.dmg"
mounted=true
osascript "$assets_dir/layout.applescript" "$staging_dir/mount" "$(basename "$app_path")"
# Wait until Finder has written its window settings before detaching.
attempt=0
while [ ! -f "$staging_dir/mount/.DS_Store" ]; do
    attempt=$((attempt + 1))
    if [ "$attempt" -ge 10 ]; then
        echo "Finder did not save the DMG layout" >&2
        exit 1
    fi
    sleep 1
done
sync
diskutil renameVolume "$staging_dir/mount" "$volume_name"
hdiutil detach "$staging_dir/mount"
mounted=false
hdiutil convert "$staging_dir/writable.dmg" -format UDZO -o "$output_dmg"
