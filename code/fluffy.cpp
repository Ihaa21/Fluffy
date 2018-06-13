#include "fluffy_platform.h"
#include "fluffy.h"

#include "fluffy_render.cpp"

// TODO: Remove this
#include <windows.h>

platform_api PlatformApi;

/*
    NOTE: The stuff thats making me not focus:
        - How do I choose which pixels are occluded. Do I round the floats to nearest? Do I try to
          do as many computations using floats? Because the level of detail results are very different
          depending on which I choose and its not obvious whats correct.
        - The setup code and which renderer is chosen isn't very clean.
        - The performance is really weird for these systems. Stuff like the orthogonal stuff seems to
          be performing very poorly.
        - Floats vs fixed point

        - We can raytrace nodes when they are a pixel size or 2x2, it might be faster than all the
          divisions we currently do. This might fall in line with bcmpinc's method

 */

/*
    TODO: Ideas:
            - Try to project the screen into the octree (do the math, see if its worth it)

            - Have a 1 level higher depth map (hierarchy) to more quickly test nodes that are large.
              We might want profiling data on how long the testing takes on nodes of a given size.
              
            - Morton order octree

            - Look at octree storage formats, maybe is leaf can be checked much faster

            - See if we have to cast our values to ints (min/max) or if we can keep them in float the
              whole way through like in the triangle rasterizer

            - Multi threading with binning
 */

inline octree* CreateOctreeNode(game_state* GameState)
{
    octree* Result = PushStruct(&GameState->NodeArena, octree);
    *Result = {};

    return Result;
}

inline octree_id* CreateOctreeIdNode(game_state* GameState, u32* NodeIdResult)
{
    octree_id* Result = PushStruct(&GameState->NodeArena, octree_id);
    *Result = {};
    *NodeIdResult = (u32)((u64)Result - (u64)GameState->NodeArena.Mem);

    return Result;
}

extern "C" GAME_INIT(GameInit)
{
    game_state* GameState = (game_state*)GameMem->PermanentMem;
    PlatformApi = GameMem->PlatformApi;

    GameState->Arena = InitArena((u8*)GameState + sizeof(game_state),
                                 GameMem->PermanentMemSize - sizeof(game_state));
    GameState->NodeArena = SubArena(&GameState->Arena, KiloBytes(500));
    InitRenderState(&GameState->RenderState, &GameState->Arena, 0.0001f, 25.0f, (u8*)GameState->NodeArena.Mem);
    GlobalRenderState = &GameState->RenderState;

    // NOTE: Build octree
    {
        // NOTE: We make a null node
        u32 NullNodeId;
        CreateOctreeIdNode(GameState, &NullNodeId);
    }
    
    GameState->Sponge = CreateOctreeNode(GameState);
    {
        octree* BackTopRight = CreateOctreeNode(GameState);
        BackTopRight->Children[0] = GameState->Sponge;
        BackTopRight->Children[3] = GameState->Sponge;
        BackTopRight->Children[1] = GameState->Sponge;
        BackTopRight->Children[4] = GameState->Sponge;

        octree* BackTopLeft = CreateOctreeNode(GameState);
        BackTopLeft->Children[0] = GameState->Sponge;
        BackTopLeft->Children[1] = GameState->Sponge;
        BackTopLeft->Children[2] = GameState->Sponge;
        BackTopLeft->Children[5] = GameState->Sponge;

        octree* BackBotLeft = CreateOctreeNode(GameState);
        BackBotLeft->Children[1] = GameState->Sponge;
        BackBotLeft->Children[2] = GameState->Sponge;
        BackBotLeft->Children[3] = GameState->Sponge;
        BackBotLeft->Children[6] = GameState->Sponge;

        octree* BackBotRight = CreateOctreeNode(GameState);
        BackBotRight->Children[0] = GameState->Sponge;
        BackBotRight->Children[2] = GameState->Sponge;
        BackBotRight->Children[3] = GameState->Sponge;
        BackBotRight->Children[7] = GameState->Sponge;

        octree* FrontTopRight = CreateOctreeNode(GameState);
        FrontTopRight->Children[4] = GameState->Sponge;
        FrontTopRight->Children[5] = GameState->Sponge;
        FrontTopRight->Children[7] = GameState->Sponge;
        FrontTopRight->Children[0] = GameState->Sponge;

        octree* FrontTopLeft = CreateOctreeNode(GameState);
        FrontTopLeft->Children[4] = GameState->Sponge;
        FrontTopLeft->Children[5] = GameState->Sponge;
        FrontTopLeft->Children[6] = GameState->Sponge;
        FrontTopLeft->Children[1] = GameState->Sponge;

        octree* FrontBotLeft = CreateOctreeNode(GameState);
        FrontBotLeft->Children[5] = GameState->Sponge;
        FrontBotLeft->Children[6] = GameState->Sponge;
        FrontBotLeft->Children[7] = GameState->Sponge;
        FrontBotLeft->Children[2] = GameState->Sponge;

        octree* FrontBotRight = CreateOctreeNode(GameState);
        FrontBotRight->Children[4] = GameState->Sponge;
        FrontBotRight->Children[6] = GameState->Sponge;
        FrontBotRight->Children[7] = GameState->Sponge;
        FrontBotRight->Children[3] = GameState->Sponge;

        GameState->Sponge->Children[0] = BackTopRight;
        GameState->Sponge->Children[1] = BackTopLeft;
        GameState->Sponge->Children[2] = BackBotLeft;
        GameState->Sponge->Children[3] = BackBotRight;
        GameState->Sponge->Children[4] = FrontTopRight;
        GameState->Sponge->Children[5] = FrontTopLeft;
        GameState->Sponge->Children[6] = FrontBotLeft;
        GameState->Sponge->Children[7] = FrontBotRight;
    }
    
    u32 SpongeNewId;
    GameState->SpongeNew = CreateOctreeIdNode(GameState, &SpongeNewId);
    {
        // NOTE: Menger sponge
        u32 BackTopRightId;
        octree_id* BackTopRight = CreateOctreeIdNode(GameState, &BackTopRightId);
        BackTopRight->ChildrenId[0] = SpongeNewId;
        BackTopRight->ChildrenId[3] = SpongeNewId;
        BackTopRight->ChildrenId[1] = SpongeNewId;
        BackTopRight->ChildrenId[4] = SpongeNewId;

        u32 BackTopLeftId;
        octree_id* BackTopLeft = CreateOctreeIdNode(GameState, &BackTopLeftId);
        BackTopLeft->ChildrenId[0] = SpongeNewId;
        BackTopLeft->ChildrenId[1] = SpongeNewId;
        BackTopLeft->ChildrenId[2] = SpongeNewId;
        BackTopLeft->ChildrenId[5] = SpongeNewId;

        u32 BackBotLeftId;
        octree_id* BackBotLeft = CreateOctreeIdNode(GameState, &BackBotLeftId);
        BackBotLeft->ChildrenId[1] = SpongeNewId;
        BackBotLeft->ChildrenId[2] = SpongeNewId;
        BackBotLeft->ChildrenId[3] = SpongeNewId;
        BackBotLeft->ChildrenId[6] = SpongeNewId;

        u32 BackBotRightId;
        octree_id* BackBotRight = CreateOctreeIdNode(GameState, &BackBotRightId);
        BackBotRight->ChildrenId[0] = SpongeNewId;
        BackBotRight->ChildrenId[2] = SpongeNewId;
        BackBotRight->ChildrenId[3] = SpongeNewId;
        BackBotRight->ChildrenId[7] = SpongeNewId;

        u32 FrontTopRightId;
        octree_id* FrontTopRight = CreateOctreeIdNode(GameState, &FrontTopRightId);
        FrontTopRight->ChildrenId[4] = SpongeNewId;
        FrontTopRight->ChildrenId[5] = SpongeNewId;
        FrontTopRight->ChildrenId[7] = SpongeNewId;
        FrontTopRight->ChildrenId[0] = SpongeNewId;

        u32 FrontTopLeftId;
        octree_id* FrontTopLeft = CreateOctreeIdNode(GameState, &FrontTopLeftId);
        FrontTopLeft->ChildrenId[4] = SpongeNewId;
        FrontTopLeft->ChildrenId[5] = SpongeNewId;
        FrontTopLeft->ChildrenId[6] = SpongeNewId;
        FrontTopLeft->ChildrenId[1] = SpongeNewId;

        u32 FrontBotLeftId;
        octree_id* FrontBotLeft = CreateOctreeIdNode(GameState, &FrontBotLeftId);
        FrontBotLeft->ChildrenId[5] = SpongeNewId;
        FrontBotLeft->ChildrenId[6] = SpongeNewId;
        FrontBotLeft->ChildrenId[7] = SpongeNewId;
        FrontBotLeft->ChildrenId[2] = SpongeNewId;

        u32 FrontBotRightId;
        octree_id* FrontBotRight = CreateOctreeIdNode(GameState, &FrontBotRightId);
        FrontBotRight->ChildrenId[4] = SpongeNewId;
        FrontBotRight->ChildrenId[6] = SpongeNewId;
        FrontBotRight->ChildrenId[7] = SpongeNewId;
        FrontBotRight->ChildrenId[3] = SpongeNewId;

        GameState->SpongeNew->ChildrenId[0] = BackTopRightId;
        GameState->SpongeNew->ChildrenId[1] = BackTopLeftId;
        GameState->SpongeNew->ChildrenId[2] = BackBotLeftId;
        GameState->SpongeNew->ChildrenId[3] = BackBotRightId;
        GameState->SpongeNew->ChildrenId[4] = FrontTopRightId;
        GameState->SpongeNew->ChildrenId[5] = FrontTopLeftId;
        GameState->SpongeNew->ChildrenId[6] = FrontBotLeftId;
        GameState->SpongeNew->ChildrenId[7] = FrontBotRightId;        
    }

    // NOTE: New Box
    u32 BoxNewId;
    GameState->BoxNew = CreateOctreeIdNode(GameState, &BoxNewId);
    {
        GameState->BoxNew->ChildrenId[0] = BoxNewId;
        GameState->BoxNew->ChildrenId[1] = BoxNewId;
        GameState->BoxNew->ChildrenId[2] = BoxNewId;
        GameState->BoxNew->ChildrenId[3] = BoxNewId;
        GameState->BoxNew->ChildrenId[4] = BoxNewId;
        GameState->BoxNew->ChildrenId[5] = BoxNewId;
        GameState->BoxNew->ChildrenId[6] = BoxNewId;
        GameState->BoxNew->ChildrenId[7] = BoxNewId;        
    }
    
    // NOTE: Box octree
#if 0
    {
        octree* Octree = &GameState->Octree;
        *Octree = {};
        Octree->Children[0] = Octree;
        Octree->Children[1] = Octree;
        Octree->Children[2] = Octree;
        Octree->Children[3] = Octree;
        Octree->Children[4] = Octree;
        Octree->Children[5] = Octree;
        Octree->Children[6] = Octree;
        Octree->Children[7] = Octree;
    }
#endif
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    game_state* GameState = (game_state*)GameMem->PermanentMem;
    PlatformApi = GameMem->PlatformApi;
    render_state* RenderState = &GameState->RenderState;
    GlobalRenderState = &GameState->RenderState;

    // NOTE: Project the octree
    local_global f32 T = 0.0f;
    T += 0.01f;

    v3 Pos = V3(0, 0, 0.5f + 0.4f*Sin(T));
    m4 ModelMat = PosMat(Pos);//*RotMat(2.0f*Sin(T), Sin(T), 2.0f*Sin(T))*ScaleMat(1, 1, 1);

    // IMPORTANT: Use Octree for non simd versions, octree id for simd versions
    //PushOctree(RenderState, GameState->Sponge, ModelMat);
    PushOctreeId(RenderState, GameState->SpongeNew, ModelMat);

#if 0
    for (f32 Z = -2; Z <= 20; Z += 1.25f)
    {
        for (f32 Y = -2; Y <= 2; Y += 1.25f)
        {
            for (f32 X = -2; X <= 2; X += 1.25f)
            {
                v3 Pos = V3(X, Y, Z);
                m4 ModelMat = PosMat(Pos)*RotMat(2.0f*Sin(T), Sin(T), 2.0f*Sin(T))*ScaleMat(0.7, 0.7, 0.7);
                PushOctreeId(RenderState, GameState->SpongeNew, ModelMat);
                //PushOctreeId(RenderState, GameState->BoxNew, ModelMat);
            }
        }
    }
#endif
    
    RenderAndDisplay(RenderState, IdentityM4());

    // NOTE: Print algorithm stats info
#if GET_STATS
    {
        char Buffer[256];
        snprintf(Buffer, sizeof(Buffer), "Num Traversals: %d NumRejected: %d LowestLeafLevel: %d NumRendered: %d \n",
                 RenderState->NumTraversals, RenderState->NumRejected, RenderState->LowestLeafLevel, RenderState->NumRendered);
        OutputDebugString(Buffer);
    }
#endif
}
