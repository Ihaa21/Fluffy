#if !defined(FLUFFY_H)

#include "fluffy_render.h"

struct octree
{
    octree* Children[8];
};

struct octree_id
{
    u32 ChildrenId[8];
};

struct game_state
{    
    mem_arena Arena;
    mem_arena NodeArena;
    render_state RenderState;

    octree* Sponge;
    octree_id* SpongeNew;
    octree* Box;
    octree_id* BoxNew;
};

#define FLUFFY_H
#endif
