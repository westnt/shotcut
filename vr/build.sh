#!/usr/bin/env bash
# Build Shotcut (this checkout, branch vr) with the VR filters, into vr/prefix. Linux x86-64.
# Stages: qt, sdl2, spatialaudio, mlt, shotcut. Finished stages are skipped on a rerun;
# force some again with: ./vr/build.sh mlt shotcut
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/.." && pwd)
DEPS=${DEPS:-$HERE/deps}          # downloaded sources and Qt
BUILD=${BUILD:-$HERE/build}
PREFIX=${PREFIX:-$HERE/prefix}
STATE=$BUILD/state
QT_VERSION=6.10.3
QT=${QT_DIR:-$DEPS/qt/$QT_VERSION/gcc_64}
MLT_REPO=${MLT_REPO:-https://github.com/westnt/mlt.git}
MLT_BRANCH=${MLT_BRANCH:-vr}
SDL_VERSION=2.32.10
SPATIALAUDIO_TAG=0.4.1
JOBS=${JOBS:-$(nproc)}

# System compilers and pkg-config only (conda / brew toolchains break the Qt and FFmpeg link).
export PATH=/usr/local/bin:/usr/bin:/bin
export CC=${CC:-/usr/bin/gcc} CXX=${CXX:-/usr/bin/g++}
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig
mkdir -p "$DEPS" "$BUILD" "$PREFIX" "$STATE"
for s in "$@"; do rm -f "$STATE/$s.done"; done

stage() {
  local name=$1; shift
  if [ -f "$STATE/$name.done" ]; then echo "== $name: done already"; return; fi
  echo "== $name"
  "$@"
  touch "$STATE/$name.done"
}

cmake_build() {  # src build-dir [cmake args...]
  local src=$1 bdir=$2; shift 2
  cmake -S "$src" -B "$bdir" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_INSTALL_LIBDIR=lib "$@"
  cmake --build "$bdir" -j"$JOBS"
  cmake --install "$bdir"
}

qt() {
  if [ -n "${QT_DIR:-}" ]; then echo "using QT_DIR=$QT_DIR"; return; fi
  python3 -m venv "$DEPS/aqt-venv"
  "$DEPS/aqt-venv/bin/pip" -q install aqtinstall
  "$DEPS/aqt-venv/bin/aqt" install-qt linux desktop "$QT_VERSION" linux_gcc_64 -O "$DEPS/qt" \
    -m qtcharts qtmultimedia qtwebsockets qt5compat qtimageformats qtshadertools
}

sdl2() {
  # Built here: Ubuntu's libsdl2-dev can pull in a Mesa driver upgrade.
  [ -d "$DEPS/sdl2" ] || { mkdir -p "$DEPS/sdl2"; curl -fsSL \
    "https://github.com/libsdl-org/SDL/releases/download/release-$SDL_VERSION/SDL2-$SDL_VERSION.tar.gz" \
    | tar xz -C "$DEPS/sdl2" --strip-components=1; }
  cmake_build "$DEPS/sdl2" "$BUILD/sdl2" -DSDL_STATIC=OFF -DSDL_TEST=OFF
}

spatialaudio() {
  [ -d "$DEPS/libspatialaudio" ] || git clone -q --depth 1 -b "$SPATIALAUDIO_TAG" \
    https://github.com/videolabs/libspatialaudio.git "$DEPS/libspatialaudio"
  cmake_build "$DEPS/libspatialaudio" "$BUILD/spatialaudio" -DBUILD_SHARED_LIBS=ON
}

mlt() {
  [ -d "$DEPS/mlt" ] || git clone -q --depth 1 -b "$MLT_BRANCH" "$MLT_REPO" "$DEPS/mlt"
  # Qt only through Qt6_DIR: with Qt in CMAKE_PREFIX_PATH, MLT's FindFFmpeg picks Qt's private FFmpeg
  # libraries (no headers) and links them against the system headers -> crash on the first video.
  cmake_build "$DEPS/mlt" "$BUILD/mlt" -DCMAKE_PREFIX_PATH="$PREFIX" -DQt6_DIR="$QT/lib/cmake/Qt6" \
    -DMOD_QT=OFF -DMOD_QT6=ON -DMOD_GLAXNIMATE=OFF -DMOD_GLAXNIMATE_QT6=OFF \
    -DMOD_MOVIT=OFF -DMOD_OPENCV=OFF -DMOD_GDK=OFF -DMOD_PLACEBO=OFF -DMOD_XINE=OFF \
    -DMOD_SDL1=OFF -DMOD_RNNOISE=OFF -DMOD_SDL2=ON -DMOD_RTAUDIO=ON -DMOD_SPATIALAUDIO=ON -DMOD_VR=ON \
    -DSWIG_PYTHON=OFF -DBUILD_TESTING=OFF
  if readelf -d "$PREFIX/lib/mlt-7/libmltavformat.so" | grep -q "$QT"; then
    echo "MLT linked Qt's private FFmpeg; refusing (see the comment above)" >&2; exit 1
  fi
}

shotcut() {
  cmake_build "$ROOT" "$BUILD/shotcut" -DCMAKE_PREFIX_PATH="$QT;$PREFIX" -DSHOTCUT_VERSION="${SHOTCUT_VERSION:-$(date +%y.%m.%d)}"
  # Shotcut runs ffmpeg / ffprobe from its own bin folder (proxies, Convert, Export helpers).
  for t in ffmpeg ffprobe; do ln -sf "$(command -v $t)" "$PREFIX/bin/$t"; done
  # run.sh needs to know where Qt is
  echo "QT=$QT" > "$PREFIX/vr-env"
}

stage qt qt
stage sdl2 sdl2
stage spatialaudio spatialaudio
stage mlt mlt
stage shotcut shotcut
echo "== done. Run: $HERE/run.sh"
