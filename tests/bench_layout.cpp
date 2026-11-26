#include "../src/bench_layout.hpp"
#include "../src/pico-game-engine/engine/collision_boxes.hpp"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main() {
    Map map; int total=0, parks=0, checks=0, maximum=0;
    FILE *file=fopen("src/assets/parkbench/parkbench.colliders","rb"); assert(file);
    uint8_t data[CollisionBoxes::MAX_BYTES]; const size_t n=fread(data,1,sizeof(data),file); fclose(file);
    CollisionBoxes boxes; assert(boxes.load(data,n));
    for(int col=-5;col<5;++col) for(int row=-5;row<5;++row) {
        BenchLayout::Placement b[2], again[2];
        const int count=BenchLayout::block(map,col,row,b); total+=count; parks+=count==2;
        assert(count==BenchLayout::block(map,col,row,again));
        assert(memcmp(b,again,count*sizeof(*b))==0);
        for(int i=0;i<count;++i) {
            if(count==1) {
                const float offsets[8][2]={{-30,-20},{-30,20},{30,-20},{30,20},{-20,-30},{20,-30},{-20,30},{20,30}};
                for(int slot=0;slot<8;++slot) {
                    const unsigned detail=map.cellHash(col,row,slot+2);
                    if(detail%5==0) continue;
                    const int type=(detail>>12)%3;
                    const float width=type==1 ? 7 : 9+(detail>>4)%5;
                    const float depth=type==1 ? 7 : 9+(detail>>7)%5;
                    const float bx=col*84+42+offsets[slot][0], bz=row*84+42+offsets[slot][1];
                    // 1.2 bounds the authored bench footprint for every orientation.
                    assert(fabsf(bx-b[i].x)>width/2+1.2f || fabsf(bz-b[i].z)>depth/2+1.2f);
                }
            }
            assert(fabsf(b[i].x)<420 && fabsf(b[i].z)<420);
            assert(boxes.blocksGround(b[i].x,b[i].z,0,b[i].x,b[i].z,b[i].yaw));
            // Bench footprint clears road and sidewalk walking centerlines.
            for(int road=-5;road<=5;++road) for(int sign=-1;sign<=1;++sign) {
                const float line=road*84.f+sign*3.35f;
                assert(!boxes.blocksGroundSegment(-420,line,420,line,.25f,b[i].x,b[i].z,b[i].yaw));
                assert(!boxes.blocksGroundSegment(line,-420,line,420,.25f,b[i].x,b[i].z,b[i].yaw));
                ++checks;
            }
        }
    }
    for(int x=-420;x<=420;x+=6) for(int z=-420;z<=420;z+=6) {
        BenchLayout::Placement nearby[BenchLayout::CAPACITY], repeated[BenchLayout::CAPACITY];
        const int count=BenchLayout::around(map,x,z,76,nearby);
        assert(count==BenchLayout::around(map,x,z,76,repeated));
        assert(memcmp(nearby,repeated,count*sizeof(*nearby))==0);
        int expected=0;
        for(int col=-5;col<5;++col) for(int row=-5;row<5;++row) {
            BenchLayout::Placement b[2]; const int n=BenchLayout::block(map,col,row,b);
            for(int i=0;i<n;++i) if((b[i].x-x)*(b[i].x-x)+(b[i].z-z)*(b[i].z-z)<=76*76) ++expected;
        }
        assert(count==expected); // Pool cannot silently drop nearby benches.
        if(count>maximum)maximum=count;
        ++checks;
    }
    printf("BENCH_LAYOUT_PASS total=%d parks=%d max_nearby=%d checks=%d\n",total,parks,maximum,checks);
}
