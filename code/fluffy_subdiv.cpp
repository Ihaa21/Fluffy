
struct box
{
    i32 TopRightX;
    i32 TopRightY;
    
    i32 TopLeftX;
    i32 TopLeftY;

    i32 BotLeftX;
    i32 BotLeftY;
    
    i32 BotRightX;
    i32 BotRightY;
};

internal void SubdivRenderOctreeScreen(octree* Node, i32 TopRightX, i32 TopRightY, i32 TopLeftX,
                                       i32 TopLeftY, i32 BotLeftX, i32 BotLeftY, i32 BotRightX,
                                       i32 BotRightY, f32 CenterZ, u32 Level)
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

    f32 Radius = GlobalSubdivState->RadiusTable[Level];
    if (CenterZ - Radius <= 0.0f)
    {
        // NOTE: Node is behind camera
#if GET_STATS
        GlobalRenderState->NumRejected += 1;
#endif
        return;
    }
    f32 NodeZ = Max(GlobalSubdivState->NearPlane, CenterZ + Radius);
    
    // NOTE: Find the min/max of this box
    i32 MinX = Min(Min(TopLeftX, TopRightX), Min(BotLeftX, BotRightX));
    i32 MaxX = Max(Max(TopLeftX, TopRightX), Max(BotLeftX, BotRightX));
    i32 MinY = Min(Min(TopLeftY, TopRightY), Min(BotLeftY, BotRightY));
    i32 MaxY = Max(Max(TopLeftY, TopRightY), Max(BotLeftY, BotRightY));
    
    if (MinX > ScreenX || MinY > ScreenY || MaxX < 0 || MaxY < 0)
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
    MaxY = Min(ScreenX, MaxY);

    // NOTE: Node is of minimal pixel size, just render it
    if (Abs(MinX - MaxX) <= 1 && Abs(MinY - MaxY) <= 1)
    {
        IsLeaf = true;
    }
        
    b32 IsOccluded = ScalarIsNodeOccluded(GlobalSubdivState->DepthMap, MinX, MaxX, MinY, MaxY, NodeZ);
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
        GlobalRenderState->LowestLeafLevel = Min(GlobalRenderState->LowestLeafLevel, Level);
        ScalarDrawNode(GlobalSubdivState->DepthMap, GlobalSubdivState->LevelMap, MinX, MaxX, MinY, MaxY, NodeZ, Level);
#else
        ScalarDrawNode(GlobalSubdivState->DepthMap, MinX, MaxX, MinY, MaxY, NodeZ, Level);
#endif
    }
    else
    {
        // NOTE: Sort in front to back order
        i32 TopMidX = RoundToU32(((f32)TopRightX - (f32)TopLeftX)*0.5f) + TopLeftX;
        i32 TopMidY = RoundToU32(((f32)TopRightY - (f32)TopLeftY)*0.5f) + TopLeftY;

        i32 BotMidX = RoundToU32(((f32)BotRightX - (f32)BotLeftX)*0.5f) + BotLeftX;
        i32 BotMidY = RoundToU32(((f32)BotRightY - (f32)BotLeftY)*0.5f) + BotLeftY;

        i32 LeftMidX = RoundToU32(((f32)TopLeftX - (f32)BotLeftX)*0.5f) + BotLeftX;
        i32 LeftMidY = RoundToU32(((f32)TopLeftY - (f32)BotLeftY)*0.5f) + BotLeftY;

        i32 RightMidX = RoundToU32(((f32)TopRightX - (f32)BotRightX)*0.5f) + BotRightX;
        i32 RightMidY = RoundToU32(((f32)TopRightY - (f32)BotRightY)*0.5f) + BotRightY;

        i32 MidX = RoundToU32(((f32)RightMidX - (f32)LeftMidX)*0.5f) + LeftMidX;
        i32 MidY = RoundToU32(((f32)RightMidY - (f32)LeftMidY)*0.5f) + LeftMidY;
                
        box Boxes[4] =
            {
                { TopRightX, TopRightY, TopMidX, TopMidY, MidX, MidY, RightMidX, RightMidY },
                { TopMidX, TopMidY, TopLeftX, TopLeftY, LeftMidX, LeftMidY, MidX, MidY },
                { MidX, MidY, LeftMidX, LeftMidY, BotLeftX, BotLeftY, BotMidX, BotMidY },
                { RightMidX, RightMidY, MidX, MidY, BotMidX, BotMidY, BotRightX, BotRightY },
            };

        f32 ZValues[8] =
            {
                CenterZ + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[0].z,
                CenterZ + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[1].z,
                CenterZ + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[2].z,
                CenterZ + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[3].z,
                CenterZ + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[4].z,
                CenterZ + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[5].z,
                CenterZ + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[6].z,
                CenterZ + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[7].z,
            };
        
        u8 Indicies[8] = {0, 1, 2, 3, 0, 1, 2, 3};

        BubbleSort(ZValues, Indicies, 8);
        for (u32 CurrNodeCount = 0; CurrNodeCount < 8; ++CurrNodeCount)
        {
            u32 NodeId = Indicies[CurrNodeCount];
            if (Node->Children[NodeId])
            {
                box CurrBox = Boxes[NodeId];
                SubdivRenderOctreeScreen(Node->Children[NodeId], CurrBox.TopRightX,
                                         CurrBox.TopRightY, CurrBox.TopLeftX, CurrBox.TopLeftY,
                                         CurrBox.BotLeftX, CurrBox.BotLeftY, CurrBox.BotRightX,
                                         CurrBox.BotRightY, ZValues[CurrNodeCount], Level + 1);
            }
        }
    }
}

internal void SubdivRenderOctree(v3 Center, octree* Node, u32 Level)
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
    f32 FMinX = ScreenX;
    f32 FMaxX = 0;
    f32 FMinY = ScreenY;
    f32 FMaxY = 0;
    f32 MinZ = F32_MAX;
    f32 MaxZ = F32_MIN;

    // NOTE: Check for the delta
    f32 NodeRadius = GlobalSubdivState->RadiusTable[Level-1];
    v3 NodeCorners[8] =
        {
            Center + NodeRadius*GlobalSubdivState->Deltas[0],
            Center + NodeRadius*GlobalSubdivState->Deltas[1],
            Center + NodeRadius*GlobalSubdivState->Deltas[2],
            Center + NodeRadius*GlobalSubdivState->Deltas[3],
            Center + NodeRadius*GlobalSubdivState->Deltas[4],
            Center + NodeRadius*GlobalSubdivState->Deltas[5],
            Center + NodeRadius*GlobalSubdivState->Deltas[6],
            Center + NodeRadius*GlobalSubdivState->Deltas[7],
        };

    MinZ = Min(Min(Min(NodeCorners[0].z, NodeCorners[1].z), Min(NodeCorners[2].z, NodeCorners[3].z)),
               Min(Min(NodeCorners[4].z, NodeCorners[5].z), Min(NodeCorners[6].z, NodeCorners[7].z)));
    MaxZ = Max(Max(Max(NodeCorners[0].z, NodeCorners[1].z), Max(NodeCorners[2].z, NodeCorners[3].z)),
               Max(Max(NodeCorners[4].z, NodeCorners[5].z), Max(NodeCorners[6].z, NodeCorners[7].z)));

    NodeCorners[0].z = Max(GlobalSubdivState->NearPlane, NodeCorners[0].z);
    NodeCorners[1].z = Max(GlobalSubdivState->NearPlane, NodeCorners[1].z);
    NodeCorners[2].z = Max(GlobalSubdivState->NearPlane, NodeCorners[2].z);
    NodeCorners[3].z = Max(GlobalSubdivState->NearPlane, NodeCorners[3].z);
    NodeCorners[4].z = Max(GlobalSubdivState->NearPlane, NodeCorners[4].z);
    NodeCorners[5].z = Max(GlobalSubdivState->NearPlane, NodeCorners[5].z);
    NodeCorners[6].z = Max(GlobalSubdivState->NearPlane, NodeCorners[6].z);
    NodeCorners[7].z = Max(GlobalSubdivState->NearPlane, NodeCorners[7].z);
        
    v2 ProjectedCorners[8] =
        {
            ProjectPoint(NodeCorners[0]),
            ProjectPoint(NodeCorners[1]),
            ProjectPoint(NodeCorners[2]),
            ProjectPoint(NodeCorners[3]),
            ProjectPoint(NodeCorners[4]),
            ProjectPoint(NodeCorners[5]),
            ProjectPoint(NodeCorners[6]),
            ProjectPoint(NodeCorners[7]),
        };

    FMinX = Min(Min(Min(ProjectedCorners[0].x, ProjectedCorners[1].x), Min(ProjectedCorners[2].x, ProjectedCorners[3].x)),
                Min(Min(ProjectedCorners[4].x, ProjectedCorners[5].x), Min(ProjectedCorners[6].x, ProjectedCorners[7].x)));
    FMinX = Min(FMinX, ScreenX);
    FMaxX = Max(Max(Max(ProjectedCorners[0].x, ProjectedCorners[1].x), Max(ProjectedCorners[2].x, ProjectedCorners[3].x)),
                Max(Max(ProjectedCorners[4].x, ProjectedCorners[5].x), Max(ProjectedCorners[6].x, ProjectedCorners[7].x)));
    FMaxX = Max(FMaxX, 0);

    FMinY = Min(Min(Min(ProjectedCorners[0].y, ProjectedCorners[1].y), Min(ProjectedCorners[2].y, ProjectedCorners[3].y)),
                Min(Min(ProjectedCorners[4].y, ProjectedCorners[5].y), Min(ProjectedCorners[6].y, ProjectedCorners[7].y)));
    FMinY = Min(FMinY, ScreenY);
    FMaxY = Max(Max(Max(ProjectedCorners[0].y, ProjectedCorners[1].y), Max(ProjectedCorners[2].y, ProjectedCorners[3].y)),
                Max(Max(ProjectedCorners[4].y, ProjectedCorners[5].y), Max(ProjectedCorners[6].y, ProjectedCorners[7].y)));
    FMaxY = Max(FMaxY, 0);

    f32 MaxDelta = 0.0f;
    MaxDelta = Max(MaxDelta, Max(Abs(ProjectedCorners[4].x - ProjectedCorners[0].x), Abs(ProjectedCorners[4].y - ProjectedCorners[0].y)));
    MaxDelta = Max(MaxDelta, Max(Abs(ProjectedCorners[5].x - ProjectedCorners[1].x), Abs(ProjectedCorners[5].y - ProjectedCorners[1].y)));
    
    b32 ScreenSubdiv = MaxDelta < 0.5f;
    
    if (MaxZ <= 0.0f)
    {
        // NOTE: Node is behind camera, cull it
#if GET_STATS
        GlobalRenderState->NumRejected += 1;
#endif
        return;
    }

    if (FMinX > (ScreenX - 0.5f) || FMinY > (ScreenY - 0.5f) || FMaxX < 0.5f || FMaxY < 0.5f)
    {
        // NOTE: Node is outside of the screen, don't render it or its children
#if GET_STATS
        GlobalRenderState->NumRejected += 1;
#endif
        return;
    }

    FMinX = Max(0, FMinX);
    FMinY = Max(0, FMinY);
    FMaxX = Min(ScreenX, FMaxX);
    FMaxY = Min(ScreenX, FMaxY);
    
    i32 MinX, MaxX, MinY, MaxY;
    MinX = RoundToU32(FMinX);
    MaxX = RoundToU32(FMaxX);
    MinY = RoundToU32(FMinY);
    MaxY = RoundToU32(FMaxY);

    // NOTE: Node is of minimal pixel size, just render it
    if (Abs(MinX - MaxX) <= 1 && Abs(MinY - MaxY) <= 1)
    {
        IsLeaf = true;
    }
    
    b32 IsOccluded = ScalarIsNodeOccluded(GlobalSubdivState->DepthMap, MinX, MaxX, MinY, MaxY, MinZ);
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
        GlobalRenderState->LowestLeafLevel = Min(GlobalRenderState->LowestLeafLevel, Level);
        ScalarDrawNode(GlobalSubdivState->DepthMap, GlobalSubdivState->LevelMap, MinX, MaxX, MinY, MaxY, MinZ, Level);
#else
        ScalarDrawNode(GlobalSubdivState->DepthMap, MinX, MaxX, MinY, MaxY, MinZ, Level);
#endif
    }
    else
    {
        // NOTE: Sort in front to back order
        if (ScreenSubdiv)
        {
            // NOTE: Setup projected corners of the cube
            i32 TopMidX = RoundToI32((ProjectedCorners[0].x - ProjectedCorners[1].x)/2 + ProjectedCorners[1].x);
            i32 TopMidY = RoundToI32((ProjectedCorners[0].y - ProjectedCorners[1].y)/2 + ProjectedCorners[1].y);
            
            i32 BotMidX = RoundToI32((ProjectedCorners[3].x - ProjectedCorners[2].x)/2 + ProjectedCorners[2].x);
            i32 BotMidY = RoundToI32((ProjectedCorners[3].y - ProjectedCorners[2].y)/2 + ProjectedCorners[2].y);

            i32 LeftMidX = RoundToI32((ProjectedCorners[1].x - ProjectedCorners[2].x)/2 + ProjectedCorners[2].x);
            i32 LeftMidY = RoundToI32((ProjectedCorners[1].y - ProjectedCorners[2].y)/2 + ProjectedCorners[2].y);

            i32 RightMidX = RoundToI32((ProjectedCorners[0].x - ProjectedCorners[3].x)/2 + ProjectedCorners[3].x);
            i32 RightMidY = RoundToI32((ProjectedCorners[0].y - ProjectedCorners[3].y)/2 + ProjectedCorners[3].y);

            i32 MidX = (RightMidX - LeftMidX)/2 + LeftMidX;
            i32 MidY = (RightMidY - LeftMidY)/2 + LeftMidY;
            
            box Boxes[4] =
                {
                    { RoundToI32(ProjectedCorners[0].x), RoundToI32(ProjectedCorners[0].y), TopMidX, TopMidY, MidX, MidY, RightMidX, RightMidY },
                    { TopMidX, TopMidY, RoundToI32(ProjectedCorners[1].x), RoundToI32(ProjectedCorners[1].y), LeftMidX, LeftMidY, MidX, MidY },
                    { MidX, MidY, LeftMidX, LeftMidY, RoundToI32(ProjectedCorners[2].x), RoundToI32(ProjectedCorners[2].y), BotMidX, BotMidY },
                    { RightMidX, RightMidY, MidX, MidY, BotMidX, BotMidY, RoundToI32(ProjectedCorners[3].x), RoundToI32(ProjectedCorners[3].y) },
                };

            f32 ZValues[8] =
                {
                    Center.z + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[0].z,
                    Center.z + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[1].z,
                    Center.z + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[2].z,
                    Center.z + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[3].z,
                    Center.z + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[4].z,
                    Center.z + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[5].z,
                    Center.z + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[6].z,
                    Center.z + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[7].z,
                };
        
            u8 Indicies[8] = {0, 1, 2, 3, 0, 1, 2, 3};

            BubbleSort(ZValues, Indicies, 8);
            for (u32 CurrNodeCount = 0; CurrNodeCount < 8; ++CurrNodeCount)
            {
                u32 NodeId = Indicies[CurrNodeCount];
                if (Node->Children[NodeId])
                {
                    box CurrBox = Boxes[NodeId];
                    SubdivRenderOctreeScreen(Node->Children[NodeId], CurrBox.TopRightX,
                                             CurrBox.TopRightY, CurrBox.TopLeftX, CurrBox.TopLeftY,
                                             CurrBox.BotLeftX, CurrBox.BotLeftY, CurrBox.BotRightX,
                                             CurrBox.BotRightY, ZValues[CurrNodeCount], Level + 1);
                }
            }
        }
        else
        {
            v3 Centers[8] =
            {
                Center + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[0],
                Center + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[1],
                Center + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[2],
                Center + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[3],
                Center + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[4],
                Center + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[5],
                Center + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[6],
                Center + GlobalSubdivState->RadiusTable[Level]*GlobalSubdivState->Deltas[7],
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
                    SubdivRenderOctree(Centers[NodeId], Node->Children[NodeId], Level + 1);
                }
            }
        }
    }
}
