#pragma once
#include "general.hpp"
#include "bench_layout.hpp"
#include "pico-game-engine/engine/entity.hpp"
#include "pico-game-engine/engine/level.hpp"
#include "pico-game-engine/engine/collision_boxes.hpp"
#ifdef ENGINE_STORAGE_INCLUDE
#include ENGINE_STORAGE_INCLUDE
#endif
#include <stdio.h>
#ifndef PICO_GTA_BENCH_ASSETS
#define PICO_GTA_BENCH_ASSETS ASSETS_FOLDER "parkbench/"
#endif

// One shared eight-view asset and collider set; Level owns the pooled entities.
class BenchProps {
    Billboard views;
    CollisionBoxes boxes;
    Entity *entities[BenchLayout::CAPACITY] = {};
    BenchLayout::Placement placements[BenchLayout::CAPACITY];
    int count = 0;
    bool ready = false;
    void clearViews() {
        for (int i=0;i<Billboard::MAX_DIRS;++i)
            for (int j=0;j<Billboard::MAX_POSES;++j) {
                if (views.dirs[i][j]) ENGINE_MEM_DELETE views.dirs[i][j];
                views.dirs[i][j]=nullptr;
            }
    }
    bool load() {
#if defined(ENGINE_STORAGE_READ) && defined(ENGINE_STORAGE_SIZE)
        const size_t pixels=64*64, mask=pixels/8, stride=pixels+mask;
        uint8_t collider[CollisionBoxes::MAX_BYTES];
        const size_t n=ENGINE_STORAGE_SIZE(PICO_GTA_BENCH_ASSETS "parkbench.colliders");
        if (n>sizeof(collider) || ENGINE_STORAGE_READ(PICO_GTA_BENCH_ASSETS "parkbench.colliders",collider,n)!=n ||
            !boxes.load(collider,n) || boxes.count()==0) return false;
        uint8_t *buffer=static_cast<uint8_t *>(ENGINE_MEM_MALLOC(stride));
        if (!buffer) return false;
        bool ok=true;
        views.setCounts(8,1);
        for (int i=0;ok && i<8;++i) {
            char path[256];
            const int written=snprintf(path,sizeof(path),"%s64-%d.bin",PICO_GTA_BENCH_ASSETS,i);
            ok=written>0 && size_t(written)<sizeof(path) && ENGINE_STORAGE_SIZE(path)==stride &&
               ENGINE_STORAGE_READ(path,buffer,stride)==stride &&
               views.setDirectionPoseData(i,0,64,64,buffer,pixels,buffer+pixels,mask);
        }
        ENGINE_MEM_FREE(buffer);
        if (!ok) clearViews();
        return ok;
#else
        return false;
#endif
    }
public:
    bool create(Level *level) {
        ready=load();
        if (!ready) {
            ENGINE_LOG_INFO("[BenchProps] Bench assets unavailable; scenery disabled\n");
            return true; // Optional assets never leave invisible colliders behind.
        }
        for (int i=0;i<BenchLayout::CAPACITY;++i) {
            Entity *e=ENGINE_MEM_NEW Entity("Park bench",ENTITY_3D_SPRITE,Vector(),Vector(2,1.14f),nullptr);
            if (!e) return false;
            e->setBillboard(&views,2.031746f);
            e->is_visible=false;

            const int before=level->getEntityCount();
            level->entity_add(e);
            if (level->getEntityCount()!=before+1) { ENGINE_MEM_DELETE e; return false; }
            entities[i]=e;
        }
        return true;
    }
    void update(const Map &map,float x,float z,float reach) {
        if (!ready) return;
        count=BenchLayout::around(map,x,z,reach,placements);
        for (int i=0;i<BenchLayout::CAPACITY;++i) {
            Entity *e=entities[i];
            e->is_visible=i<count;
            if (i>=count) continue;
            const BenchLayout::Placement &b=placements[i];
            e->position=Vector(b.x,b.z);
            e->direction=Vector(sinf(b.yaw),-cosf(b.yaw));
        }
    }
    bool blocks(float x0,float z0,float x1,float z1,float radius) const {
        for (int i=0;i<count;++i) {
            const BenchLayout::Placement &b=placements[i];
            if (boxes.blocksGroundSegment(x0,z0,x1,z1,radius,b.x,b.z,b.yaw)) return true;
        }
        return false;
    }
    void release() {
        // Called before Level deletes its entities. No entity owns the shared views.
        for (int i=0;i<BenchLayout::CAPACITY;++i) {
            if (entities[i]) { entities[i]->billboard=nullptr; entities[i]->is_visible=false; }
            entities[i]=nullptr;
        }
        count=0; ready=false; clearViews();
    }
};
