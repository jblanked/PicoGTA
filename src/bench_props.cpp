#include "bench_props.hpp"
#include "general.hpp"
#include "pico-game-engine/engine/game.hpp"
#include "pico-game-engine/engine/level.hpp"
#include <stdint.h>
#include <string.h>
#ifdef ENGINE_STORAGE_INCLUDE
#include ENGINE_STORAGE_INCLUDE
#endif

namespace
{
class BenchEntity : public Entity
{
  public:
    explicit BenchEntity(Sprite3D *mesh, float visualRadius);
    void render(Draw *draw, Game *game) override;
    void detach();
    void place(const BenchLayout::Placement &bench, const Vector &center, const Vector &halfSize, float yaw);

  private:
    Sprite3D *sharedMesh;
    Vector meshPosition;
    float visualRadius;
};

BenchEntity::BenchEntity(Sprite3D *mesh, float visualRadius)
    : Entity("Park bench", ENTITY_3D_SPRITE, Vector(10000, 10000), Vector(), nullptr), sharedMesh(mesh),
      visualRadius(visualRadius)
{
    is_visible = false;
}

// A rotation-invariant sphere from the actual mesh, independent of collision boxes.
// Invalid bounds disable culling; they must never hide an otherwise drawable asset.
float meshRadius(const Sprite3D &mesh)
{
    float squared = 0;
    for (uint16_t i = 0; i < mesh.getTriangleCount(); ++i)
    {
        Triangle3D triangle;
        if (!mesh.getTriangle(i, triangle))
            return -1;
        const float xs[] = {triangle.x1, triangle.x2, triangle.x3};
        const float ys[] = {triangle.y1, triangle.y2, triangle.y3};
        const float zs[] = {triangle.z1, triangle.z2, triangle.z3};
        for (int j = 0; j < 3; ++j)
        {
            const float distance = xs[j] * xs[j] + ys[j] * ys[j] + zs[j] * zs[j];
            if (!isfinite(distance))
                return -1;
            squared = fmaxf(squared, distance);
        }
    }
    return sqrtf(squared) + .01f;
}

bool outsideCamera(const Vector &origin, float radius, const Camera &camera, const Vector &screen)
{
    if (!(radius > 0) || !isfinite(radius) || !(screen.x > 0) || !(screen.y > 0))
        return false;
    const float dx = origin.x - camera.position.x, dz = origin.y - camera.position.y;
    const float x = -camera.direction.y * dx + camera.direction.x * dz;
    const float z = camera.direction.x * dx + camera.direction.y * dz;
    const float directionSquared = camera.direction.x * camera.direction.x +
                                   camera.direction.y * camera.direction.y;
    if (!isfinite(x) || !isfinite(z) || !isfinite(directionSquared) || !isfinite(camera.height))
        return false;
    // Match the renderer's near plane and screen-height focal length. Plane support
    // radii also handle non-unit camera directions without normalizing the camera.
    if (z + radius * sqrtf(directionSquared) < .1f)
        return true;
    const float aspect = screen.x / (2 * screen.y);
    if (fabsf(x) > aspect * z + radius * sqrtf(directionSquared * (1 + aspect * aspect)))
        return true;
    return fabsf(camera.height) > .5f * z + radius * sqrtf(1 + .25f * directionSquared);
}

void BenchEntity::render(Draw *draw, Game *game)
{
    if (!sharedMesh || !draw || !game || !game->current_level)
        return;
    const Camera *camera = game->getCamera();
    if (!camera)
        return;
    // Keep shadow rendering conservative if a future scene enables it.
    if (!game->current_level->getShadowColor() &&
        outsideCamera(meshPosition, visualRadius, *camera, draw->getDisplaySize()))
        return;
    sharedMesh->setPosition(meshPosition);
    sharedMesh->setRotation(sprite_rotation);
    game->current_level->render3DSprite(sharedMesh, draw, camera->position, camera->direction,
                                        camera->height);
}

void BenchEntity::place(const BenchLayout::Placement &bench, const Vector &center, const Vector &halfSize,
                        float yaw)
{
    meshPosition = Vector(bench.x, bench.z);
    sprite_rotation = bench.yaw;
    const float c = cosf(bench.yaw), s = sinf(bench.yaw);
    const float bc = fabsf(cosf(bench.yaw + yaw)), bs = fabsf(sinf(bench.yaw + yaw));
    const float hx = bc * halfSize.x + bs * halfSize.y;
    const float hz = bs * halfSize.x + bc * halfSize.y;
    size = Vector(hx * 2, hz * 2);
    radius = hx;
    position_set(bench.x + c * center.x - s * center.y - hx, bench.z + s * center.x + c * center.y - hz);
}

// Decode one authored box from the Blender PCB1 sidecar.
// Its vertical extent is validated but ground movement uses its horizontal footprint.
bool loadEntityBounds(const uint8_t *data, size_t length, Vector &center, Vector &halfSize, float &yaw)
{
    static_assert(sizeof(float) == 4, "PCB1 requires 32-bit floats");
    if (length != 36 || memcmp(data, "PCB1\x01\x00\x00\x00", 8) != 0)
        return false;
    float values[7];
    for (unsigned i = 0; i < 7; ++i)
    {
        const uint8_t *p = data + 8 + i * 4;
        const uint32_t bits =
            uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
        memcpy(values + i, &bits, 4);
        if (!isfinite(values[i]) || fabsf(values[i]) > 1000000)
            return false;
    }
    if (values[3] <= 0 || values[4] <= 0 || values[5] <= 0)
        return false;
    center = Vector(values[0], values[2]);
    halfSize = Vector(values[3], values[5]);
    yaw = values[6];
    return true;
}

// Resolve movement before applying it, including steps which cross a thin entity.
// The engine supplies ordinary rectangle overlap; this sweep uses those same bounds.
bool crossesEntity(const Entity &entity, float x0, float z0, float x1, float z1, float radius)
{
    float enter = 0, leave = 1;
    const float start[2] = {x0, z0}, delta[2] = {x1 - x0, z1 - z0};
    const float low[2] = {entity.position.x - radius, entity.position.y - radius};
    const float high[2] = {entity.position.x + entity.size.x + radius,
                           entity.position.y + entity.size.y + radius};
    for (int axis = 0; axis < 2; ++axis)
    {
        if (delta[axis] == 0)
        {
            if (start[axis] <= low[axis] || start[axis] >= high[axis])
                return false;
            continue;
        }
        const float a = (low[axis] - start[axis]) / delta[axis];
        const float b = (high[axis] - start[axis]) / delta[axis];
        enter = fmaxf(enter, fminf(a, b));
        leave = fminf(leave, fmaxf(a, b));
        if (enter >= leave)
            return false;
    }
    return enter < 1 && leave > 0;
}

void BenchEntity::detach()
{
    sharedMesh = nullptr;
    is_visible = false;
    size = Vector();
    position_set(10000, 10000);
}
} // namespace

bool BenchProps::create(Level *level)
{
    if (!level || mesh)
        return false;
#if defined(ENGINE_STORAGE_READ) && defined(ENGINE_STORAGE_SIZE)
    uint8_t data[36];
    const size_t size = ENGINE_STORAGE_SIZE(ASSETS_FOLDER "/parkbench/parkbench.colliders");
    if (size > sizeof(data) ||
        ENGINE_STORAGE_READ(ASSETS_FOLDER "/parkbench/parkbench.colliders", data, size) != size ||
        !loadEntityBounds(data, size, boxCenter, boxHalfSize, boxYaw))
        return true; // Optional assets never leave invisible colliders.
    mesh = ENGINE_MEM_NEW Sprite3D();
    if (!mesh)
        return false;
    if (!mesh->fromPath(ASSETS_FOLDER "/parkbench/parkbench.sprite3d", false))
    {
        ENGINE_MEM_DELETE mesh;
        mesh = nullptr;
        return true;
    }
    this->level = level;
    mesh->setDepthSortEnabled(true);
    // Missing texture sidecars leave the mesh's exported material colors intact.
    if (mesh->fromUVPath(ASSETS_FOLDER "/parkbench/parkbench.uv"))
        mesh->fromTexturePath(ASSETS_FOLDER "/parkbench/parkbench.rgb565", 64, 64);
    const float visualRadius = meshRadius(*mesh);
    for (int i = 0; i < BenchLayout::CAPACITY; ++i)
    {
        BenchEntity *entity = ENGINE_MEM_NEW BenchEntity(mesh, visualRadius);
        if (!entity)
        {
            release();
            return false;
        }
        const int before = level->getEntityCount();
        level->entity_add(entity);
        if (level->getEntityCount() != before + 1)
        {
            ENGINE_MEM_DELETE entity;
            release();
            return false;
        }
        entities[i] = entity;
    }
#endif
    return true;
}

void BenchProps::update(const Map &map, float x, float z, float reach)
{
    if (!mesh)
        return;
    count = BenchLayout::around(map, x, z, reach, placements);
    for (int i = 0; i < BenchLayout::CAPACITY; ++i)
    {
        Entity *entity = entities[i];
        entity->is_visible = i < count;
        if (i < count)
        {
            const BenchLayout::Placement &bench = placements[i];
            static_cast<BenchEntity *>(entity)->place(bench, boxCenter, boxHalfSize, boxYaw);
        }
        else
        {
            // Pooled entities remain active; park empty slots outside the bounded city.
            entity->size = Vector();
            entity->position_set(10000, 10000);
        }
    }
}

bool BenchProps::blocks(float x0, float z0, float x1, float z1, float radius) const
{
    if (!level || !count || radius < 0 || !isfinite(radius) || !isfinite(x0) || !isfinite(z0) ||
        !isfinite(x1) || !isfinite(z1))
        return false;
    Entity movement("Bench movement", ENTITY_3D_SPRITE,
                    Vector(fminf(x0, x1) - radius, fminf(z0, z1) - radius),
                    Vector(fabsf(x1 - x0) + radius * 2, fabsf(z1 - z0) + radius * 2), nullptr);
    for (int i = 0; i < count; ++i)
    {
        const Entity *entity = entities[i];
        if (entity->is_visible && level->is_collision(&movement, entity) &&
            crossesEntity(*entity, x0, z0, x1, z1, radius))
            return true;
    }
    return false;
}

void BenchProps::release()
{
    // Landscape releases assets before Level destroys its entities.
    for (int i = 0; i < BenchLayout::CAPACITY; ++i)
    {
        if (entities[i])
            static_cast<BenchEntity *>(entities[i])->detach();
        entities[i] = nullptr;
    }
    count = 0;
    level = nullptr;
    ENGINE_MEM_DELETE mesh;
    mesh = nullptr;
}
