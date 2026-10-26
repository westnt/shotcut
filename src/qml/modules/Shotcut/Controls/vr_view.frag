#version 440
// VR look-around preview: one eye of a packed VR frame, seen through a pinhole camera.
// Same math as the vr MLT module and port/xr/vr_view.frag: +x right, +y up, viewer looks along -z,
// lon = atan(x, -z), lat = asin(y), view R = Ry(-yaw) * Rx(pitch).
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec4 leftRect;     // eye rectangles inside the frame, normalized x, y, w, h
    vec4 rightRect;
    vec4 lonLatRange;  // lon0, lon1, lat0, lat1 (radians)
    vec2 tanFov;       // tan(hfov / 2), tan(vfov / 2)
    vec2 texelSize;    // 1 / source texture size
    float yawRad;
    float pitchRad;
    int eyeMode;       // 0 left, 1 right, 2 red/cyan anaglyph
    int full360;
};
layout(binding = 1) uniform sampler2D src;

const float TWO_PI = 6.283185307179586;

// Direction -> normalized position inside an eye, or (-1, -1) outside the picture.
vec2 eyeCoord(vec3 d)
{
    float lon = atan(d.x, -d.z);
    float lat = asin(clamp(d.y, -1.0, 1.0));
    float lo0 = lonLatRange.x, lo1 = lonLatRange.y, la0 = lonLatRange.z, la1 = lonLatRange.w;
    if (full360 == 1)
        lon = lo0 + mod(lon - lo0, TWO_PI);
    else if (lon < lo0 || lon > lo1)
        return vec2(-1.0);
    if (lat < la0 || lat > la1)
        return vec2(-1.0);
    return vec2((lon - lo0) / (lo1 - lo0), (la1 - lat) / (la1 - la0));
}

vec4 sampleEye(vec4 r, vec2 s)
{
    // Stay half a texel inside the eye so bilinear filtering never reads the other eye.
    vec2 texel = texelSize;
    vec2 uv = r.xy + s * r.zw;
    uv = clamp(uv, r.xy + 0.5 * texel, r.xy + r.zw - 0.5 * texel);
    return texture(src, uv);
}

void main()
{
    vec2 p = qt_TexCoord0 * 2.0 - 1.0; // -1..1, y down
    vec3 d = normalize(vec3(p.x * tanFov.x, -p.y * tanFov.y, -1.0));
    float cp = cos(pitchRad), sp = sin(pitchRad);
    d = vec3(d.x, cp * d.y - sp * d.z, sp * d.y + cp * d.z);   // Rx(pitch)
    float cy = cos(yawRad), sy = sin(yawRad);
    d = vec3(cy * d.x - sy * d.z, d.y, sy * d.x + cy * d.z);   // Ry(-yaw)

    vec2 s = eyeCoord(d);
    if (s.x < 0.0) {
        fragColor = vec4(0.0, 0.0, 0.0, qt_Opacity);
        return;
    }
    vec4 c;
    if (eyeMode == 2) {
        // Dubois red/cyan, ffmpeg's stereo3d coefficients / 65536 (same as vredit and vr_anaglyph).
        vec3 L = sampleEye(leftRect, s).rgb;
        vec3 R = sampleEye(rightRect, s).rgb;
        mat3 ML = mat3(29884.0, -2621.0, -983.0,
                       32768.0, -2490.0, -1376.0,
                       11534.0, -1049.0, -328.0) / 65536.0;
        mat3 MR = mat3(-2818.0, 24773.0, -4719.0,
                       -5767.0, 48103.0, -7406.0,
                       -131.0, -1180.0, 80347.0) / 65536.0;
        c = vec4(clamp(ML * L + MR * R, 0.0, 1.0), 1.0);
    } else {
        c = sampleEye(eyeMode == 1 ? rightRect : leftRect, s);
        c.a = 1.0;
    }
    fragColor = c * qt_Opacity;
}
