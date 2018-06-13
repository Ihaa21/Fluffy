# Fluffy

This repository contains multiple octree renderers, implemented in C++. This program is built for visual studio with the .sln files stored in build_win32.

Build: To build the program, open a command promt and navigate to the build directory. Run build.bat and the project should be built.

Program Entry Point: The main entry point for the program is found at win32_fluffy.cpp.

Renderers: The program contains multiple renderers. To switch which renderer is being used to display the octree on screen, go to fluffy_render.h, and set the OCTREE_RENDER_... pragma to 1 for whichever renderer you want to use (NOTE, SIMD renderers require fluffy.cpp to call PushOctreeId instead of PushOctree to render the octree). To get info about the renderer being used, set GET_STATS in fluffy_render.h to 1. To change which image is being rendered to the screen, set the DRAW_... pragma to 1 for whichever image you want to display (NOTE, some rendered images such as overdraw map, level map, etc require GET_STATS to be set to 1). 
