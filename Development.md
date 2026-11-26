## Development Guide
Set up the [configuration](Configuration.md), including the required input macros, then:
1. Clone the repository: `git clone https://github.com/jblanked/PicoGTA.git`
2. Create a `PicoGTA` instance (from `src/game.hpp`)
3. Call `PicoGTA::run()` once per application-loop iteration. It initializes drawing and starts the engine with the player and levels on its first call, then reads input, updates, and renders each frame.
4. Stop calling `run()` when `PicoGTA::isActive()` returns `false`, then free the instance.

No separate drawing initialization, input forwarding, or game update call is needed.

Check out [Picoware's version](https://github.com/jblanked/Picoware) for a complete example of how to set up the game on a specific platform. The code in this repository is designed to be portable and can be adapted to various platforms with minimal changes.

## Park benches

Copy `src/assets/parkbench` to `ASSETS_FOLDER/parkbench` on game storage (Picoware:
`picoware/apps/games/pico_gta/assets/parkbench`). `PICO_GTA_BENCH_ASSETS` can override
the complete directory prefix, including its trailing slash. Missing or invalid
assets disable both bench rendering and collisions without preventing startup.

The deterministic map has 114 benches: two facing the tree in each of 14 open
blocks, and one outside the sidewalk walking strip in every other block.
A pool of 12 entities shares eight 64x64 RGB332/mask images (36,864 pixel/mask
bytes before image metadata), plus one authored collider set. Nearby placements
refresh with city geometry; movement does not load assets or allocate entities.
Walking, driving, and traffic use swept-circle collision checks.

The vendored engine includes authored collision boxes and the opt-in shared
scene depth renderer from engine PR
https://github.com/pico-game-engine/pico-game-engine/pull/30. PicoGTA enables
shared depth during startup so buildings hide cars and billboard benches,
including partial occlusion. Failure to allocate the depth buffer fails startup.
At 320x320 it needs 204,800 bytes of PSRAM (heap on non-PSRAM platforms) plus
1,600 bytes of internal scanline scratch. The local city draw distance remains
within the depth buffer's tested 0.1 through 256-unit range.

Scenery callbacks retain a distant sort position along the player's heading to
keep the sky/background behind world rendering. City meshes keep their actual
fixed geometry anchor and participate in depth testing through the level.

Run the placement regression from the repository root:

```sh
g++ -std=c++11 -Wall -Wextra -Werror -pedantic tests/bench_layout.cpp src/map.cpp -o /tmp/bench-layout
/tmp/bench-layout
```

It checks every block, building clearance, road/sidewalk clearance, repeatable
placements, and pool coverage across 19,881 camera positions.
