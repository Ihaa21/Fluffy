
inline void GetNodeSizeOriginal(f32 NearPlane, v3 Center, f32 Radius, i32* MinX, i32* MaxX, i32* MinY,
                                i32* MaxY, f32* MinZ, f32* MaxZ)
{
    v3 P0 = Center;
    v3 P1 = Center;

    if (Center.x < 0)
    {
        P0.x -= Radius;
        P1.x += Radius;
    }
    else
    {
        P0.x += Radius;
        P1.x -= Radius;
    }

    if (Center.y < 0)
    {
        P0.y -= Radius;
        P1.y += Radius;
    }
    else
    {
        P0.y += Radius;
        P1.y -= Radius;
    }

    P0.z -= Radius;
    P1.z += Radius;

    P0.z = Max(NearPlane, P0.z);
    P0.x /= P0.z;
    P0.y /= P0.z;
    
    P1.z = Max(NearPlane, P1.z);
    P1.x /= P1.z;
    P1.y /= P1.z;

    if (P0.x < P1.x)
    {
        *MinX = RoundToI32(0.5f*(P0.x + 1.0f)*ScreenX);
        *MaxX = RoundToI32(0.5f*(P1.x + 1.0f)*ScreenX);
    }
    else
    {
        *MinX = RoundToI32(0.5f*(P1.x + 1.0f)*ScreenX);
        *MaxX = RoundToI32(0.5f*(P0.x + 1.0f)*ScreenX);
    }

    if (P0.y < P1.y)
    {
        *MinY = RoundToI32(0.5f*(P0.y + 1.0f)*ScreenY);
        *MaxY = RoundToI32(0.5f*(P1.y + 1.0f)*ScreenY);
    }
    else
    {
        *MinY = RoundToI32(0.5f*(P1.y + 1.0f)*ScreenY);
        *MaxY = RoundToI32(0.5f*(P0.y + 1.0f)*ScreenY);
    }

    *MinZ = P0.z;
    *MaxZ = P1.z;
}

inline void GetNodeSize4Pts(f32 NearPlane, v3 Center, f32 Radius, i32* MinX, i32* MaxX, i32* MinY,
                            i32* MaxY, f32* MinZ, f32* MaxZ)
{
    v2 P0 = Center.xy;
    v2 P1 = Center.xy;

    if (Center.x < 0)
    {
        P0.x -= Radius;
        P1.x += Radius;
    }
    else
    {
        P0.x += Radius;
        P1.x -= Radius;
    }

    if (Center.y < 0)
    {
        P0.y -= Radius;
        P1.y += Radius;
    }
    else
    {
        P0.y += Radius;
        P1.y -= Radius;
    }

    f32 FrontZ = Max(NearPlane, Center.z - Radius);
    f32 BackZ = Max(NearPlane, Center.z + Radius);

    v2 P0Front = V2(P0.x / FrontZ, P0.y / FrontZ);
    v2 P1Front = V2(P1.x / FrontZ, P1.y / FrontZ);

    v2 P0Back = V2(P0.x / BackZ, P0.y / BackZ);
    v2 P1Back = V2(P1.x / BackZ, P1.y / BackZ);

    if (P0.x < P1.x)
    {
        *MinX = RoundToI32(0.5f*(Min(P0Front.x, P0Back.x) + 1.0f)*ScreenX);
        *MaxX = RoundToI32(0.5f*(Max(P1Front.x, P1Back.x) + 1.0f)*ScreenX);
    }
    else
    {
        *MinX = RoundToI32(0.5f*(Min(P1Front.x, P1Back.x) + 1.0f)*ScreenX);
        *MaxX = RoundToI32(0.5f*(Max(P0Front.x, P0Back.x) + 1.0f)*ScreenX);
    }

    if (P0.y < P1.y)
    {
        *MinY = RoundToI32(0.5f*(Min(P0Front.y, P0Back.y) + 1.0f)*ScreenY);
        *MaxY = RoundToI32(0.5f*(Max(P1Front.y, P1Back.y) + 1.0f)*ScreenY);
    }
    else
    {
        *MinY = RoundToI32(0.5f*(Min(P1Front.y, P1Back.y) + 1.0f)*ScreenY);
        *MaxY = RoundToI32(0.5f*(Max(P0Front.y, P0Back.y) + 1.0f)*ScreenY);
    }
    
    *MinZ = FrontZ;
    *MaxZ = BackZ;
}

inline void GetNodeSizeAxis(f32 NearPlane, v3 Center, f32 Radius, i32* MinX, i32* MaxX, i32* MinY,
                            i32* MaxY, f32* MinZ, f32* MaxZ)
{
    v3 P0 = Center;
    v3 P1 = Center;

    if (Center.x < 0)
    {
        P0.x -= Radius;
        P1.x += Radius;
    }
    else
    {
        P0.x += Radius;
        P1.x -= Radius;
    }

    if (Center.y < 0)
    {
        P0.y -= Radius;
        P1.y += Radius;
    }
    else
    {
        P0.y += Radius;
        P1.y -= Radius;
    }

    P0.z -= Radius;
    P1.z += Radius;

    P0.z = Max(NearPlane, P0.z);
    P1.z = Max(NearPlane, P1.z);

    if (((0 >= P0.y && 0 <= P1.y) || (0 >= P1.y && 0 <= P0.y)) &&
        ((0 >= P0.x && 0 <= P1.x) || (0 >= P1.x && 0 <= P0.x)))
    {
        P0.x /= P0.z;
        P0.y /= P0.z;
        P1.x /= P0.z;
        P1.y /= P0.z;
    }
    else if ((0 >= P0.y && 0 <= P1.y) || (0 >= P1.y && 0 <= P0.y))
    {
        // NOTE: Intersect y axis
        P0.x /= P0.z;
        P0.y /= P0.z;
        P1.x /= P1.z;
        P1.y /= P0.z;
    }
    else if ((0 >= P0.x && 0 <= P1.x) || (0 >= P1.x && 0 <= P0.x))
    {
        // NOTE: Intersect y axis
        P0.x /= P0.z;
        P0.y /= P0.z;
        P1.x /= P0.z;
        P1.y /= P1.z;
    }
    else
    {
        // NOTE: Doesn't intersect x/y axis
        P0.x /= P0.z;
        P0.y /= P0.z;
        P1.x /= P1.z;
        P1.y /= P1.z;
    }

    if (P0.x < P1.x)
    {
        *MinX = RoundToI32(0.5f*(P0.x + 1.0f)*ScreenX);
        *MaxX = RoundToI32(0.5f*(P1.x + 1.0f)*ScreenX);
    }
    else
    {
        *MinX = RoundToI32(0.5f*(P1.x + 1.0f)*ScreenX);
        *MaxX = RoundToI32(0.5f*(P0.x + 1.0f)*ScreenX);
    }

    if (P0.y < P1.y)
    {
        *MinY = RoundToI32(0.5f*(P0.y + 1.0f)*ScreenY);
        *MaxY = RoundToI32(0.5f*(P1.y + 1.0f)*ScreenY);
    }
    else
    {
        *MinY = RoundToI32(0.5f*(P1.y + 1.0f)*ScreenY);
        *MaxY = RoundToI32(0.5f*(P0.y + 1.0f)*ScreenY);
    }

    *MinZ = P0.z;
    *MaxZ = P1.z;
}

inline void GetNodeSizeAxis(f32 NearPlane, v3 Center, f32 RadiusX, f32 RadiusY, f32 RadiusZ, i32* MinX,
                            i32* MaxX, i32* MinY, i32* MaxY, f32* MinZ, f32* MaxZ)
{
    v3 P0 = Center;
    v3 P1 = Center;

    if (Center.x < 0)
    {
        P0.x -= RadiusX;
        P1.x += RadiusX;
    }
    else
    {
        P0.x += RadiusX;
        P1.x -= RadiusX;
    }

    if (Center.y < 0)
    {
        P0.y -= RadiusY;
        P1.y += RadiusY;
    }
    else
    {
        P0.y += RadiusY;
        P1.y -= RadiusY;
    }

    P0.z -= RadiusZ;
    P1.z += RadiusZ;

    P0.z = Max(NearPlane, P0.z);
    P1.z = Max(NearPlane, P1.z);

    if (((0 >= P0.y && 0 <= P1.y) || (0 >= P1.y && 0 <= P0.y)) &&
        ((0 >= P0.x && 0 <= P1.x) || (0 >= P1.x && 0 <= P0.x)))
    {
        P0.x /= P0.z;
        P0.y /= P0.z;
        P1.x /= P0.z;
        P1.y /= P0.z;
    }
    else if ((0 >= P0.y && 0 <= P1.y) || (0 >= P1.y && 0 <= P0.y))
    {
        // NOTE: Intersect y axis
        P0.x /= P0.z;
        P0.y /= P0.z;
        P1.x /= P1.z;
        P1.y /= P0.z;
    }
    else if ((0 >= P0.x && 0 <= P1.x) || (0 >= P1.x && 0 <= P0.x))
    {
        // NOTE: Intersect y axis
        P0.x /= P0.z;
        P0.y /= P0.z;
        P1.x /= P0.z;
        P1.y /= P1.z;
    }
    else
    {
        // NOTE: Doesn't intersect x/y axis
        P0.x /= P0.z;
        P0.y /= P0.z;
        P1.x /= P1.z;
        P1.y /= P1.z;
    }

    if (P0.x < P1.x)
    {
        *MinX = RoundToI32(0.5f*(P0.x + 1.0f)*ScreenX);
        *MaxX = RoundToI32(0.5f*(P1.x + 1.0f)*ScreenX);
    }
    else
    {
        *MinX = RoundToI32(0.5f*(P1.x + 1.0f)*ScreenX);
        *MaxX = RoundToI32(0.5f*(P0.x + 1.0f)*ScreenX);
    }

    if (P0.y < P1.y)
    {
        *MinY = RoundToI32(0.5f*(P0.y + 1.0f)*ScreenY);
        *MaxY = RoundToI32(0.5f*(P1.y + 1.0f)*ScreenY);
    }
    else
    {
        *MinY = RoundToI32(0.5f*(P1.y + 1.0f)*ScreenY);
        *MaxY = RoundToI32(0.5f*(P0.y + 1.0f)*ScreenY);
    }

    *MinZ = P0.z;
    *MaxZ = P1.z;
}

internal void EpicRenderOctree(v3 Center, octree* Node, u32 Level)
{
#if GET_STATS
    GlobalRenderState->NumTraversals += 1;
#endif
    
    b32 IsLeaf = true;
    for (u32 ChildId = 0; ChildId < 8; ++ChildId)
    {
        if (Node->Children[ChildId])
        {
            IsLeaf = false;
            break;
        }
    }
    
    // NOTE: Check if we are occluded
    i32 MinX, MaxX, MinY, MaxY;
    f32 MinZ, MaxZ;
    //GetNodeSizeOriginal(GlobalEpicState->NearPlane, Center, GlobalEpicState->RadiusTable[Level], &MinX, &MaxX, &MinY, &MaxY, &MinZ, &MaxZ);
    //GetNodeSize4Pts(GlobalEpicState->NearPlane, Center, GlobalEpicState->RadiusTable[Level], &MinX, &MaxX, &MinY, &MaxY, &MinZ, &MaxZ);
    GetNodeSizeAxis(GlobalEpicState->NearPlane, Center, GlobalEpicState->RadiusTable[Level], &MinX, &MaxX, &MinY, &MaxY, &MinZ, &MaxZ);
    
    if (MaxZ <= GlobalEpicState->NearPlane)
    {
        // NOTE: Node is behind camera, cull it
#if GET_STATS
        GlobalRenderState->NumRejected += 1;
#endif
        return;
    }

    if (MinX >= ScreenX || MinY >= ScreenY || MaxX <= 0 || MaxY <= 0)
    {
        // NOTE: Node is outside of the screen, don't render it or its children
#if GET_STATS
        GlobalRenderState->NumRejected += 1;
#endif
        return;
    }
    
    MinX = Max(0, MinX);
    MinY = Max(0, MinY);
    MaxX = Min(ScreenX, MaxX);
    MaxY = Min(ScreenY, MaxY);
        
    // NOTE: Node is of minimal pixel size, just render it
    if ((MaxX - MinX) <= 1 && (MaxY - MinY) <= 1)
    {
        IsLeaf = true;
    }

    b32 IsOccluded = ScalarIsNodeOccluded(GlobalEpicState->DepthMap, MinX, MaxX, MinY, MaxY, MinZ);
    if (IsOccluded)
    {
#if GET_STATS
        GlobalRenderState->NumRejected += 1;
#endif
        return;
    }

    if (Level == MAX_LEVEL || IsLeaf)
    {
#if GET_STATS
        GlobalRenderState->LowestLeafLevel = Max(GlobalRenderState->LowestLeafLevel, Level);
        ScalarDrawNode(GlobalEpicState->DepthMap, GlobalEpicState->LevelMap, MinX, MaxX, MinY, MaxY, MinZ, Level);
#else
        ScalarDrawNode(GlobalEpicState->DepthMap, MinX, MaxX, MinY, MaxY, MinZ, Level);
#endif
    }
    else
    {
        // NOTE: Sort in front to back order
        v3 Centers[8] =
        {
            Center + GlobalEpicState->RadiusTable[Level]*GlobalEpicState->Deltas[0],
            Center + GlobalEpicState->RadiusTable[Level]*GlobalEpicState->Deltas[1],
            Center + GlobalEpicState->RadiusTable[Level]*GlobalEpicState->Deltas[2],
            Center + GlobalEpicState->RadiusTable[Level]*GlobalEpicState->Deltas[3],
            Center + GlobalEpicState->RadiusTable[Level]*GlobalEpicState->Deltas[4],
            Center + GlobalEpicState->RadiusTable[Level]*GlobalEpicState->Deltas[5],
            Center + GlobalEpicState->RadiusTable[Level]*GlobalEpicState->Deltas[6],
            Center + GlobalEpicState->RadiusTable[Level]*GlobalEpicState->Deltas[7],
        };

        f32 Keys[8] =
        {
            Length(Centers[0]),
            Length(Centers[1]),
            Length(Centers[2]),
            Length(Centers[3]),
            Length(Centers[4]),
            Length(Centers[5]),
            Length(Centers[6]),
            Length(Centers[7]),
        };
        
        u8 Indicies[8] = {0, 1, 2, 3, 4, 5, 6, 7};

        BubbleSort(Keys, Indicies, 8);
        for (u32 CurrNodeCount = 0; CurrNodeCount < 8; ++CurrNodeCount)
        {
            u32 NodeId = Indicies[CurrNodeCount];
            if (Node->Children[NodeId])
            {
                EpicRenderOctree(Centers[NodeId], Node->Children[NodeId], Level + 1);
            }
        }
    }
}
