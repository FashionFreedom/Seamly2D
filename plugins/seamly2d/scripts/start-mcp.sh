#!/bin/sh
set -eu
if [ "$(uname -s)" = Darwin ]; then
    app_path=${SEAMLY2D_APP:-/Applications/Seamly2D.app}
    executable="$app_path/Contents/MacOS/seamly2d-mcp"
else
    executable=$(command -v seamly2d-mcp) || {
        echo 'Install seamly2d-mcp before starting the Seamly2D plugin.' >&2
        exit 1
    }
fi
exec "$executable" "$@"
