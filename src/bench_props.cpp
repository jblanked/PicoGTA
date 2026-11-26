#include "bench_props.hpp"
#include "general.hpp"
#include "pico-game-engine/engine/game.hpp"
#include "pico-game-engine/engine/level.hpp"
#ifdef ENGINE_STORAGE_INCLUDE
#include ENGINE_STORAGE_INCLUDE
#endif

namespace
{
class BenchEntity : public Entity
{
  public:
    explicit BenchEntity(Sprite3D *mesh);
    void render(Draw *draw, Game *game) override;
    void detach();

  private:
    Sprite3D *sharedMesh;
};

BenchEntity::BenchEntity(Sprite3D *mesh)
    : Entity("Park bench", ENTITY_3D_SPRITE, Vector(), Vector(2.0f, 1.14f), nullptr), sharedMesh(mesh)
{
    is_visible = false;
}

void BenchEntity::render(Draw *draw, Game *game)
{
    if (!sharedMesh || !draw || !game || !game->current_level)
        return;
    const Camera *camera = game->getCamera();
    if (!camera)
        return;
    sharedMesh->setPosition(position);
    sharedMesh->setRotation(sprite_rotation);
    game->current_level->render3DSprite(sharedMesh, draw, camera->position, camera->direction,
                                        camera->height);
}

void BenchEntity::detach()
{
    sharedMesh = nullptr;
    is_visible = false;
}
} // namespace

bool BenchProps::create(Level *level)
{
    if (!level || mesh)
        return false;
#if defined(ENGINE_STORAGE_READ) && defined(ENGINE_STORAGE_SIZE)
    uint8_t data[PropColliders::MAX_BYTES];
    const size_t size = ENGINE_STORAGE_SIZE(ASSETS_FOLDER "/parkbench/parkbench.colliders");
    if (size > sizeof(data) ||
        ENGINE_STORAGE_READ(ASSETS_FOLDER "/parkbench/parkbench.colliders", data, size) != size ||
        !boxes.load(data, size) || boxes.count() == 0)
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
    mesh->setDepthSortEnabled(true);
    // Missing texture sidecars leave the mesh's exported material colors intact.
    if (mesh->fromUVPath(ASSETS_FOLDER "/parkbench/parkbench.uv"))
        mesh->fromTexturePath(ASSETS_FOLDER "/parkbench/parkbench.rgb565", 64, 64);
    for (int i = 0; i < BenchLayout::CAPACITY; ++i)
    {
        BenchEntity *entity = ENGINE_MEM_NEW BenchEntity(mesh);
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
            entity->position_set(bench.x, bench.z);
            entity->sprite_rotation = bench.yaw;
        }
    }
}

bool BenchProps::blocks(float x0, float z0, float x1, float z1, float radius) const
{
    for (int i = 0; i < count; ++i)
    {
        const BenchLayout::Placement &bench = placements[i];
        if (boxes.blocksGroundSegment(x0, z0, x1, z1, radius, bench.x, bench.z, bench.yaw))
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
    ENGINE_MEM_DELETE mesh;
    mesh = nullptr;
}
