#!/usr/bin/env bash
# Run the Shotcut built by vr/build.sh.
HERE=$(cd "$(dirname "$0")" && pwd)
P=${PREFIX:-$HERE/prefix}
. "$P/vr-env"
export LD_LIBRARY_PATH=$P/lib:$QT/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
export MLT_REPOSITORY=$P/lib/mlt-7 MLT_DATA=$P/share/mlt-7 MLT_PROFILES_PATH=$P/share/mlt-7/profiles
export MLT_PRESETS_PATH=$P/share/mlt-7/presets FREI0R_PATH=${FREI0R_PATH:-/usr/lib/frei0r-1}
export QT_PLUGIN_PATH=$QT/plugins QML2_IMPORT_PATH=$QT/qml
exec "$P/bin/shotcut" "$@"
