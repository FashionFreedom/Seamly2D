#!/bin/sh
# Package the app with a prepared Finder layout and Applications shortcut.
set -eu

app_path=$1
volume_name=$2
output_dmg=$3
assets_dir="$(dirname "$0")/../dist/macx/dmg"
staging_dir=$(mktemp -d)
trap 'rm -rf "$staging_dir"' EXIT

mkdir "$staging_dir/.background"
ditto "$app_path" "$staging_dir/$(basename "$app_path")"
ln -s /Applications "$staging_dir/Applications"
cp "$assets_dir/background.tiff" "$staging_dir/.background/"
cp "$assets_dir/$volume_name.DS_Store" "$staging_dir/.DS_Store"
hdiutil create -fs HFS+ -format UDZO -srcfolder "$staging_dir" -volname "$volume_name" "$output_dmg"
