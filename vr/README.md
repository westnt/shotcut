# Shotcut VR

This branch adds VR180 and 360° editing to Shotcut, mono and stereo:

- **VR filters** ("VR:" in the filter list): Transform (drag on the player), Reframe to Flat, Mask,
  Eye Align, Color Match, Convergence, 3D Overlay, Anaglyph. They work on side-by-side and top / bottom
  stereo, so one setting drives both eyes. The filter engines are an MLT module, in the `vr` branch
  of [westnt/mlt](https://github.com/westnt/mlt/tree/vr) (`src/modules/vr`).
- **VR preview**: the **VR** button in the player toolbar. Drag to look, scroll to zoom, double-click
  to face front; left eye, right eye or red / cyan anaglyph. Preview only, it never changes the export.
- **VR export metadata**: on a finished export job, "Set VR180 Side-by-Side…" and
  "Set 360° Stereo Top/Bottom…" (Spherical Video V2), so players and YouTube see the file as VR.

## Build (Linux x86-64, tested on Ubuntu 24.04)

Packages:

```bash
sudo apt install --no-upgrade build-essential cmake git curl python3-venv pkg-config ffmpeg \
  libavformat-dev libavfilter-dev libavdevice-dev libswscale-dev libswresample-dev \
  libxml2-dev frei0r-plugins-dev libsamplerate0-dev librubberband-dev libsox-dev libfftw3-dev \
  libvidstab-dev libebur128-dev libpulse-dev libjack-jackd2-dev libasound2-dev libvulkan-dev \
  libxcb-cursor-dev liblilv-dev libvorbis-dev ladspa-sdk libgl1-mesa-dev libxkbcommon-x11-dev libexif-dev
```

Then:

```bash
git clone https://github.com/westnt/shotcut
cd shotcut
./vr/build.sh     # Qt 6.10 (aqtinstall), SDL2, libspatialaudio, MLT with the vr module, Shotcut -> vr/prefix
./vr/run.sh
```

Everything goes into `vr/deps`, `vr/build` and `vr/prefix` (about 3 GB). Nothing is installed
system-wide. Already have Qt 6.10? `QT_DIR=/path/to/6.10.3/gcc_64 ./vr/build.sh`.
A rerun skips the finished stages; `./vr/build.sh mlt shotcut` rebuilds those two.

Left out: the rnnoise noise-reduction filter (no Ubuntu package), Movit GPU effects, Glaxnimate, and
Shotcut's own 360° (bigsh0t) filters; the VR filters cover that ground.
