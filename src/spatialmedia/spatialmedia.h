/*****************************************************************************
 *
 * Copyright 2016 Varol Okan. All rights reserved.
 * Copyright (c) 2020 Meltytech, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#ifndef SPATIALMEDIA_H
#define SPATIALMEDIA_H

#include <string>

class SpatialMedia
{
private:
    SpatialMedia() {};

public:
    static bool injectSpherical(const std::string& inFile, const std::string& outFile);

    enum StereoMode { Mono = 0, TopBottom = 1, LeftRight = 2 };
    // Spherical Video V2 (st3d + sv3d/proj/equi): what YouTube VR180, Quest and DeoVR read for stereo and 180.
    // bounds: fraction of the full 360x180 sphere cropped from top, bottom, left, right (0..1); VR180 = 0,0,.25,.25
    // pose: initial view yaw/pitch/roll in degrees (prhd, signed 16.16).
    static bool injectSphericalV2(const std::string& inFile, const std::string& outFile, StereoMode stereo,
                                  const double bounds[4], double yaw = 0, double pitch = 0, double roll = 0);
};

#endif // SPATIALMEDIA_H
