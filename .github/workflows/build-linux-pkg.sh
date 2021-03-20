#!/usr/bin/env bash

set -euo pipefail

readonly buildDirPath=/home/runner/local

mkdir $buildDirPath

qmake PREFIX=$buildDirPath Seamly2D.pro -r CONFIG+=no_ccache CONFIG+=noDebugSymbols
make
make install

# The AppImage bundler relies on fuse during packaging
sudo apt update
sudo apt install fuse
# The libdrm pkg not found on base ubuntu-latest build
sudo apt-get install libdrm-dev
sudo apt-get install -f

mkdir -p $buildDirPath/{share/applications,share/icons/hicolor/256x256,share/translations}
cp dist/seamly2d.desktop $buildDirPath/share/applications
cp share/img/Seamly2D_logo_256x256.png $buildDirPath/share/icons/hicolor/256x256/seamly2d.png
cp share/translations/*.qm $buildDirPath/share/translations
cp src/app/seamlyme/bin/diagrams.rcc $buildDirPath/share

docker run --cap-add SYS_ADMIN --device /dev/fuse \
    --security-opt apparmor:unconfined --security-opt seccomp=unconfined \
    -v $buildDirPath:/app/usr \
    -e EXTRA_BINARIES="seamlyme" \
    --rm mhitza/linuxdeployqt:"$QT_VERSION" \
    gcc:9.3 gcc

mv $buildDirPath/Seamly2D*.AppImage Seamly2D.AppImage
