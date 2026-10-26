/* See sv3d.h. */
#include "sv3d.h"
#include "constants.h"

#include <cstring>
#include <string>
#include <vector>

static void put32(std::vector<uint8_t> &b, uint32_t v)
{
    b.push_back(v >> 24); b.push_back(v >> 16); b.push_back(v >> 8); b.push_back(v);
}
static void putBox(std::vector<uint8_t> &b, const char *tag, const std::vector<uint8_t> &content)
{
    put32(b, (uint32_t) (8 + content.size()));
    b.insert(b.end(), tag, tag + 4);
    b.insert(b.end(), content.begin(), content.end());
}
static Box *rawBox(const char *tag, const std::vector<uint8_t> &content)
{
    Box *box = new Box();
    memcpy(box->m_name, tag, 4);
    box->m_iHeaderSize = 8;
    uint8_t *data = new uint8_t[content.size()];
    memcpy(data, content.data(), content.size());
    box->set(data, (uint32_t) content.size());
    return box;
}

Box *st3d_create(uint8_t stereoMode)
{
    std::vector<uint8_t> c;
    put32(c, 0);  // version + flags
    c.push_back(stereoMode);
    return rawBox(constants::TAG_ST3D, c);
}

Box *sv3d_create(const uint32_t b[4], int32_t yaw, int32_t pitch, int32_t roll, const char *source)
{
    std::vector<uint8_t> svhd, prhd, equi, proj, sv3d;
    put32(svhd, 0);
    svhd.insert(svhd.end(), source, source + strlen(source) + 1);  // NUL-terminated
    put32(prhd, 0);
    put32(prhd, (uint32_t) yaw);
    put32(prhd, (uint32_t) pitch);
    put32(prhd, (uint32_t) roll);
    put32(equi, 0);
    for (int i = 0; i < 4; ++i) put32(equi, b[i]);
    putBox(proj, constants::TAG_PRHD, prhd);
    putBox(proj, constants::TAG_EQUI, equi);
    putBox(sv3d, constants::TAG_SVHD, svhd);
    putBox(sv3d, constants::TAG_PROJ, proj);
    return rawBox(constants::TAG_SV3D, sv3d);
}
