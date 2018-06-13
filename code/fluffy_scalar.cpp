
internal void ScalarRenderOctree(v3 Center, octree* Node, u32 Level)
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
    f32 FMinX, FMaxX, FMinY, FMaxY;
    f32 MinZ, MaxZ;
    ApproximateNodeSize(GlobalScalarState->RadiusTable, Center, GlobalScalarState->Deltas,
                        GlobalScalarState->NearPlane, Level, &FMinX, &FMaxX, &FMinY, &FMaxY,
                        &MinZ, &MaxZ);
    
    if (MaxZ <= 0.0f)
    {
        // NOTE: Node is behind camera, cull it
#if GET_STATS
        GlobalRenderState->NumRejected += 1;
#endif
        return;
    }
    
    MinX = RoundToI32(FMinX);
    MaxX = RoundToI32(FMaxX);
    MinY = RoundToI32(FMinY);
    MaxY = RoundToI32(FMaxY);

    if (MaxX == MinX || MaxY == MinY)
    {
#if GET_STATS
        GlobalRenderState->NumRejected += 1;
#endif
        return;
    }

    if ((MaxX - MinX) <= 1 && (MaxY - MinY) <= 1)
    {
#if GET_STATS
        GlobalRenderState->NumRendered += 1;
#endif
        
        IsLeaf = true;

        if (MinX < 0 || MinX >= ScreenX ||
            MinY < 0 || MinY >= ScreenY)
        {
#if GET_STATS
            GlobalRenderState->NumRejected += 1;
#endif
            return;
        }

        u32 PixelId = MinY*ScreenX + MinX;
#if GET_STATS
        GlobalScalarState->OverdrawMap[PixelId] += 1.0f;
#endif
        f32 Depth = GlobalScalarState->DepthMap[PixelId];
        if (Depth > MinZ)
        {
            GlobalScalarState->DepthMap[PixelId] = MinZ;
#if GET_STATS
            GlobalScalarState->LevelMap[PixelId] = Level;
#endif
        }
        
        return;
    }
    
    if (MinX >= ScreenX || MinY >= ScreenX || MaxX < 0 || MaxY < 0)
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
    
    b32 IsOccluded = ScalarIsNodeOccluded(GlobalScalarState->DepthMap, MinX, MaxX, MinY, MaxY, MinZ);
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
        ScalarDrawNode(GlobalScalarState->DepthMap, GlobalScalarState->LevelMap, MinX, MaxX, MinY, MaxY, MinZ, Level);
#else
        ScalarDrawNode(GlobalScalarState->DepthMap, MinX, MaxX, MinY, MaxY, MinZ, Level);
#endif
    }
    else
    {
        // NOTE: Sort in front to back order
        v3 Centers[8] =
        {
            Center + GlobalScalarState->RadiusTable[Level]*GlobalScalarState->Deltas[0],
            Center + GlobalScalarState->RadiusTable[Level]*GlobalScalarState->Deltas[1],
            Center + GlobalScalarState->RadiusTable[Level]*GlobalScalarState->Deltas[2],
            Center + GlobalScalarState->RadiusTable[Level]*GlobalScalarState->Deltas[3],
            Center + GlobalScalarState->RadiusTable[Level]*GlobalScalarState->Deltas[4],
            Center + GlobalScalarState->RadiusTable[Level]*GlobalScalarState->Deltas[5],
            Center + GlobalScalarState->RadiusTable[Level]*GlobalScalarState->Deltas[6],
            Center + GlobalScalarState->RadiusTable[Level]*GlobalScalarState->Deltas[7],
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
                ScalarRenderOctree(Centers[NodeId], Node->Children[NodeId], Level + 1);
            }
        }
    }
}
