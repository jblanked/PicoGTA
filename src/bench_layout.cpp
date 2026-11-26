#include "bench_layout.hpp"

namespace BenchLayout
{
int block(const Map &map, int col, int row, Placement out[2])
{
    if (col < -5 || col >= 5 || row < -5 || row >= 5)
        return 0;
    const float x = col * 84.0f + 42.0f, z = row * 84.0f + 42.0f;
    if (map.cellHash(col, row, 1) % 7 == 0)
    {
        // Face the central tree from opposite sides of the open block.
        out[0] = {x, z - 5.0f, PI};
        out[1] = {x, z + 5.0f, 0};
        return 2;
    }
    const unsigned side = map.cellHash(col, row, 47) % 4;
    // Outside the walking strip, midway between building slots/intersections.
    const Placement sides[4] = {{x, z - 37, 0}, {x + 37, z, PI / 2}, {x, z + 37, PI}, {x - 37, z, -PI / 2}};
    out[0] = sides[side];
    return 1;
}
int around(const Map &map, float x, float z, float reach, Placement *out)
{
    int count = 0;
    const int col0 = (int)floorf((x - reach) / 84), col1 = (int)floorf((x + reach) / 84);
    const int row0 = (int)floorf((z - reach) / 84), row1 = (int)floorf((z + reach) / 84);
    for (int col = col0; col <= col1; ++col)
        for (int row = row0; row <= row1; ++row)
        {
            Placement candidates[2];
            const int n = block(map, col, row, candidates);
            for (int i = 0; i < n; ++i)
            {
                const Placement b = candidates[i];
                const float d = (b.x - x) * (b.x - x) + (b.z - z) * (b.z - z);
                if (d > reach * reach)
                    continue;
                // Nearest first, with coordinate iteration resolving ties consistently.
                int at = count;
                while (at > 0)
                {
                    const Placement prev = out[at - 1];
                    if ((prev.x - x) * (prev.x - x) + (prev.z - z) * (prev.z - z) <= d)
                        break;
                    if (at < CAPACITY)
                        out[at] = prev;
                    --at;
                }
                if (at < CAPACITY)
                    out[at] = b;
                if (count < CAPACITY)
                    ++count;
            }
        }
    return count;
}
} // namespace BenchLayout
