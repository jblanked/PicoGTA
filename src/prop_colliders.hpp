#pragma once
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Optional asset-owned collider data, never allocated for entities without it.
// PCB1: magic + LE uint32 count + count * 7 LE float32 values:
// center x/y/z, half extents x/y/z, yaw radians. Y is vertical.
class PropColliders
{
  public:
    static constexpr unsigned MAX_BOXES = 4;
    static constexpr size_t MAX_BYTES = 8 + MAX_BOXES * 28;
    struct Box
    {
        float x, y, z, hx, hy, hz, yaw;
    };

  private:
    Box boxes_[MAX_BOXES] = {};
    unsigned count_ = 0;
    static uint32_t word(const uint8_t *p);
    static bool segmentRect(float x0, float z0, float x1, float z1, float hx, float hz);
    static bool segmentCircle(float x0, float z0, float x1, float z1, float cx, float cz, float radius);

  public:
    unsigned count() const;
    // Invalid input leaves the previous valid collider set unchanged.
    bool load(const uint8_t *data, size_t size);
    // Circle against oriented ground footprints. Asset translation, yaw and
    // positive uniform scale are independent of its mesh/billboard rendering.
    // Height intentionally ignored: games choose whether a prop blocks ground movement.
    bool blocksGround(float x, float z, float radius, float assetX = 0, float assetZ = 0, float assetYaw = 0,
                      float scale = 1) const;
    // Swept circle versus the full ground footprint: prevents tunnelling when
    // one frame's movement is wider than a thin collider. Corners remain rounded.
    bool blocksGroundSegment(float x0, float z0, float x1, float z1, float radius, float assetX = 0,
                             float assetZ = 0, float assetYaw = 0, float scale = 1) const;
};
