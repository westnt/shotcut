/* Spherical Video V2 boxes (st3d, sv3d > svhd + proj > prhd + equi) as prebuilt raw boxes.
 * Layout follows Google's spatial-media (Apache-2.0) mpeg/sv3d.py byte for byte, checked by
 * port/tests/test_port_spatialmedia.py. Part of the vredit Shotcut port; GPL-3.0-or-later like Shotcut.
 */
#ifndef SV3D_H
#define SV3D_H

#include <cstdint>
#include "box.h"

Box *st3d_create(uint8_t stereoMode);
Box *sv3d_create(const uint32_t bounds_tblr[4], int32_t yaw_16_16, int32_t pitch_16_16, int32_t roll_16_16,
                 const char *metadataSource = "Spherical Metadata Tool");

#endif
