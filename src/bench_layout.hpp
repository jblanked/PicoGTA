#pragma once
#include "map.hpp"
#include <math.h>

// Stable world-space placements; no random state or per-refresh allocation.
namespace BenchLayout
{
struct Placement
{
    float x, z, yaw;
};
static const int CAPACITY = 12;
static const float PI = 3.14159265359f;
int block(const Map &map, int col, int row, Placement out[2]);
int around(const Map &map, float x, float z, float reach, Placement *out);
} // namespace BenchLayout
