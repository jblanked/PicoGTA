# Park bench

Original parkbench model authored in Blender for this integration. These files
are distributed under this repository's GPL-3.0 license.

Install this directory at `ASSETS_FOLDER/parkbench/` on the game storage.
Each `64-N.bin` contains 4,096 RGB332 pixels followed by a 512-byte alpha mask.
Eight views share a 2.031746-unit billboard height. Direction zero faces Blender
negative Y; the engine instance uses direction `(sin(yaw), -cos(yaw))`.
`parkbench.colliders` is the Blender-exported PCB1 sidecar (one upright box).
All instances share the same eight images and collider data.
