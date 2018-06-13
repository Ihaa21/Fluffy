
__forceinline b32 IsNodeOccludedCov1Bit(u8* CoverageMap1Bit, i32 MinX, i32 MaxX, i32 MinY, i32 MaxY)
{
    b32 Result = true;

    u32 MinLineX = MinX / 64;
    u32 MaxLineX = MaxX / 64;

    u32 InMinLineX = MinX % 64;
    u64 StartMask = ((u64)0xFFFFFFFFFFFFFFFF << (u64)InMinLineX);
    
    u32 InMaxLineX = MaxX % 64;
    u64 EndMask = ~((u64)0xFFFFFFFFFFFFFFFF << ((u64)InMaxLineX));
    
    u64* CoverageRow = (u64*)CoverageMap1Bit + MinY*(ScreenX/64) + MinLineX;
    for (i32 Y = MinY; Y < MaxY; ++Y)
    {
        u64* Coverage = CoverageRow;
        
        for (u32 ChunkX = MinLineX; ChunkX <= MaxLineX; ++ChunkX)
        {
#if GET_STATS
            GlobalRenderState->NumPixels += 1;
#endif
            
            u64 Mask = 0xFFFFFFFFFFFFFFFF;
            if (ChunkX == MinLineX)
            {
                Mask &= StartMask;
            }
            if (ChunkX == MaxLineX)
            {
                Mask &= EndMask;
            }

            // TODO: The last loop here can sometimes have a 0 mask, so guarenteed useless
            if ((*Coverage & Mask) != (0xFFFFFFFFFFFFFFFF & Mask))
            {
                Result = false;
                return Result;
            }

            ++Coverage;
        }

        CoverageRow += (ScreenX/64);
    }

    return Result;
}

__forceinline b32 IsNodeOccludedCov1Bit8x8(u8* CoverageMap1Bit, u64* MinMaskX8x8, i32 MinX, i32 MaxX,
                                           i32 MinY, i32 MaxY)
{
    b32 Result = true;

    //i32 MinBlockX = MinX / 8;
    //i32 MaxBlockX = MaxX / 8;
    //i32 MinBlockY = MinY / 8;
    //i32 MaxBlockY = MaxY / 8;

    // TODO: The same?
    i32 MinBlockX = MinX >> 3;
    i32 MaxBlockX = MaxX >> 3;
    i32 MinBlockY = MinY >> 3;
    i32 MaxBlockY = MaxY >> 3;

    u32 InMinBlockX = MinX & 7;
    u32 InMaxBlockX = MaxX & 7;
    u32 InMinBlockY = MinY & 7;
    u32 InMaxBlockY = MaxY & 7;

    // TODO: For performance, we probs only want precomputed masks for x values since those are the
    // most complicated to compute
#if 1
    u64 MinMaskX = MinMaskX8x8[InMinBlockX];
    u64 MaxMaskX = ~MinMaskX8x8[InMaxBlockX];
#else
    u64 MinMaskX = (u64)(u8)(0xFF << InMinBlockX) << (u64)0;
    MinMaskX |= (u64)(u8)(0xFF << InMinBlockX) << (u64)8;
    MinMaskX |= (u64)(u8)(0xFF << InMinBlockX) << (u64)16;
    MinMaskX |= (u64)(u8)(0xFF << InMinBlockX) << (u64)24;
    MinMaskX |= (u64)(u8)(0xFF << InMinBlockX) << (u64)32;
    MinMaskX |= (u64)(u8)(0xFF << InMinBlockX) << (u64)40;
    MinMaskX |= (u64)(u8)(0xFF << InMinBlockX) << (u64)48;
    MinMaskX |= (u64)(u8)(0xFF << InMinBlockX) << (u64)56;

    u64 MaxMaskX = (u64)(u8)(0xFF << InMaxBlockX) << (u64)0;
    MaxMaskX |= (u64)(u8)(0xFF << InMaxBlockX) << (u64)8;
    MaxMaskX |= (u64)(u8)(0xFF << InMaxBlockX) << (u64)16;
    MaxMaskX |= (u64)(u8)(0xFF << InMaxBlockX) << (u64)24;
    MaxMaskX |= (u64)(u8)(0xFF << InMaxBlockX) << (u64)32;
    MaxMaskX |= (u64)(u8)(0xFF << InMaxBlockX) << (u64)40;
    MaxMaskX |= (u64)(u8)(0xFF << InMaxBlockX) << (u64)48;
    MaxMaskX |= (u64)(u8)(0xFF << InMaxBlockX) << (u64)56;
    MaxMaskX = ~MaxMaskX;
#endif
    
    u64 MinMaskY = (u64)0xFFFFFFFFFFFFFFFF << (u64)(8*InMinBlockY);
    u64 MaxMaskY = ~((u64)0xFFFFFFFFFFFFFFFF << (u64)(8*InMaxBlockY));
    
    u64* CoverageRow = (u64*)CoverageMap1Bit + (MinBlockY*(ScreenX/8) + MinBlockX);
    for (i32 BlockY = MinBlockY; BlockY <= MaxBlockY; ++BlockY)
    {
        u64* Coverage = CoverageRow;
        
        for (i32 BlockX = MinBlockX; BlockX <= MaxBlockX; ++BlockX)
        {
#if GET_STATS
            GlobalRenderState->NumPixels += 1;
#endif
            
            u64 Mask = 0xFFFFFFFFFFFFFFFF;
            if (BlockX == MinBlockX)
            {
                Mask &= MinMaskX;
            }
            if (BlockX == MaxBlockX)
            {
                Mask &= MaxMaskX;
            }
            if (BlockY == MinBlockY)
            {
                Mask &= MinMaskY;
            }
            if (BlockY == MaxBlockY)
            {
                Mask &= MaxMaskY;
            }

            // TODO: The last loop here can sometimes have a 0 mask, so guarenteed useless
            if ((*Coverage & Mask) != (0xFFFFFFFFFFFFFFFF & Mask))
            {
                Result = false;
                return Result;
            }

            ++Coverage;
        }

        CoverageRow += (ScreenX/8);
    }

    return Result;
}

__forceinline b32 CheckNode8x8Row(u64* Coverage, u64 MinMaskX, u64 MaxMaskX, i16 MinBlockX,
                                  i16 MaxBlockX, u64 RowMask)
{
    b32 Result = true;
    
    if (MinBlockX == MaxBlockX)
    {
        u64 Mask = RowMask;
        Mask &= MinMaskX & MaxMaskX;

        // TODO: The last loop here can sometimes have a 0 mask, so guarenteed useless
        if ((*Coverage & Mask) != (0xFFFFFFFFFFFFFFFF & Mask))
        {
            Result = false;
            return Result;
        }
    }
    else
    {
        {
            u64 Mask = RowMask;
            Mask &= MinMaskX;
            if ((*Coverage & Mask) != (0xFFFFFFFFFFFFFFFF & Mask))
            {
                Result = false;
                return Result;
            }

            Coverage += 1;
        }
                
        for (i32 BlockX = MinBlockX + 1; BlockX < MaxBlockX; ++BlockX)
        {
#if GET_STATS
            GlobalRenderState->NumPixels += 1;
#endif
            
            u64 Mask = RowMask;

            // TODO: The last loop here can sometimes have a 0 mask, so guarenteed useless
            if ((*Coverage & Mask) != (0xFFFFFFFFFFFFFFFF & Mask))
            {
                Result = false;
                return Result;
            }

            ++Coverage;
        }

        {
            u64 Mask = RowMask;
            Mask &= MaxMaskX;
            if ((*Coverage & Mask) != (0xFFFFFFFFFFFFFFFF & Mask))
            {
                Result = false;
                return Result;
            }
        }
    }

    return Result;
}

__forceinline b32 IsNodeOccludedCov1Bit8x8(u8* CoverageMap1Bit, u64* MinMaskX8x8, i16 MinBlockX,
                                           i16 MaxBlockX, i16 MinBlockY, i16 MaxBlockY, i16 InMinBlockX,
                                           i16 InMaxBlockX, i16 InMinBlockY, i16 InMaxBlockY)
{
    b32 Result = true;

    // TODO: For performance, we probs only want precomputed masks for x values since those are the
    // most complicated to compute
    u64 MinMaskX = MinMaskX8x8[InMinBlockX];
    u64 MaxMaskX = ~MinMaskX8x8[InMaxBlockX];
    
    u64 MinMaskY = (u64)0xFFFFFFFFFFFFFFFF << (u64)(8*InMinBlockY);
    u64 MaxMaskY = ~((u64)0xFFFFFFFFFFFFFFFF << (u64)(8*InMaxBlockY));
    
    u64* CoverageRow = (u64*)CoverageMap1Bit + (MinBlockY*(ScreenX/8) + MinBlockX);

    if (MinBlockY == MaxBlockY)
    {
        for (i32 BlockY = MinBlockY; BlockY <= MaxBlockY; ++BlockY)
        {
            u64* Coverage = CoverageRow;
        
            for (i32 BlockX = MinBlockX; BlockX <= MaxBlockX; ++BlockX)
            {
#if GET_STATS
                GlobalRenderState->NumPixels += 1;
#endif
            
                u64 Mask = 0xFFFFFFFFFFFFFFFF;
                if (BlockX == MinBlockX)
                {
                    Mask &= MinMaskX;
                }
                if (BlockX == MaxBlockX)
                {
                    Mask &= MaxMaskX;
                }
                if (BlockY == MinBlockY)
                {
                    Mask &= MinMaskY;
                }
                if (BlockY == MaxBlockY)
                {
                    Mask &= MaxMaskY;
                }

                // TODO: The last loop here can sometimes have a 0 mask, so guarenteed useless
                if ((*Coverage & Mask) != (0xFFFFFFFFFFFFFFFF & Mask))
                {
                    Result = false;
                    return Result;
                }

                ++Coverage;
            }

            CoverageRow += (ScreenX/8);
        }
    }
    else
    {
        {
            u64 Mask = 0xFFFFFFFFFFFFFFFF;
            Mask &= MinMaskY;
            
            Result = CheckNode8x8Row(CoverageRow, MinMaskX, MaxMaskX, MinBlockX, MaxBlockX,
                                     Mask);
            if (!Result)
            {
                return Result;
            }
            
            CoverageRow += (ScreenX/8);
        }
        
        for (i32 BlockY = MinBlockY + 1; BlockY < MaxBlockY; ++BlockY)
        {
            u64* Coverage = CoverageRow;

            Result = CheckNode8x8Row(Coverage, MinMaskX, MaxMaskX, MinBlockX, MaxBlockX,
                                     0xFFFFFFFFFFFFFFFF);
            if (!Result)
            {
                return Result;
            }
            
            CoverageRow += (ScreenX/8);
        }

        {
            u64 Mask = 0xFFFFFFFFFFFFFFFF;
            Mask &= MaxMaskY;
            
            Result = CheckNode8x8Row(CoverageRow, MinMaskX, MaxMaskX, MinBlockX, MaxBlockX,
                                     Mask);
            if (!Result)
            {
                return Result;
            }
            
            CoverageRow += (ScreenX/8);
        }
    }
    
    return Result;
}

__forceinline void RenderPointCov1Bit(u8* CoverageMap1Bit, f32* DepthMap, u32* LevelMap,
                                      f32* OverdrawMap, i32 MinX, i32 MinY, f32 MinZ, i32 Level)
{
    u32 PixelId = MinY*ScreenX + MinX;
    OverdrawMap[PixelId] += 1.0f;

    u32 LineX = MinX / 64;
    u32 InLineX = MinX & 63;
    u64 Mask = (u64)1 << (u64)InLineX;
    u32 CovPixelId = MinY*(ScreenX/64) + LineX;
    u64* Coverage = (u64*)CoverageMap1Bit + CovPixelId;
    if ((*Coverage & Mask) == 0)
    {
        *Coverage |= Mask;
        LevelMap[PixelId] = Level;
        DepthMap[PixelId] = MinZ;
    }
}

__forceinline void RenderPointCov1Bit(u8* CoverageMap1Bit, f32* DepthMap, i32 MinX, i32 MinY,
                                      f32 MinZ, i32 Level)
{
    u32 LineX = MinX / 64;
    u32 InLineX = MinX & 63;
    u64 Mask = (u64)1 << (u64)InLineX;
    u32 CovPixelId = MinY*(ScreenX/64) + LineX;
    u64* Coverage = (u64*)CoverageMap1Bit + CovPixelId;
    if ((*Coverage & Mask) == 0)
    {
        *Coverage |= Mask;
        u32 PixelId = MinY*ScreenX + MinX;
        DepthMap[PixelId] = MinZ;
    }
}

__forceinline void RenderPointCov1Bit8x8(u8* CoverageMap1Bit, f32* DepthMap, u32* LevelMap,
                                         f32* OverdrawMap, i16 MinBlockX, i16 MinBlockY,
                                         i16 InMinBlockX, i16 InMinBlockY, f32 MinZ, i32 Level)
{
    u32 PixelId = (MinBlockY*8 + InMinBlockY)*ScreenX + MinBlockX*8 + InMinBlockX;
    OverdrawMap[PixelId] += 1.0f;

    u64 Mask = (u64)1 << (u64)(InMinBlockY*8 + InMinBlockX);
    u64* Coverage = (u64*)CoverageMap1Bit + (MinBlockY*(ScreenX/8) + MinBlockX);
    if ((*Coverage & Mask) == 0)
    {
        *Coverage |= Mask;        
        DepthMap[PixelId] = MinZ;
        LevelMap[PixelId] = Level;
    }
}

__forceinline void RenderPointCov1Bit8x8(u8* CoverageMap1Bit, f32* DepthMap, i16 MinBlockX,
                                         i16 MinBlockY, i16 InMinBlockX, i16 InMinBlockY, f32 MinZ)
{
    u64 Mask = (u64)1 << (u64)(InMinBlockY*8 + InMinBlockX);
    u64* Coverage = (u64*)CoverageMap1Bit + (MinBlockY*(ScreenX/8) + MinBlockX);
    if ((*Coverage & Mask) == 0)
    {
        *Coverage |= Mask;        
        u32 PixelId = (MinBlockY*8 + InMinBlockY)*ScreenX + MinBlockX*8 + InMinBlockX;
        DepthMap[PixelId] = MinZ;
    }
}

internal void Cov1BitRenderOctreeNoSortNoClip(v3 Center, octree_id* Node, u32 Level, u8 Order[8])
{
#if GET_STATS
    GlobalRenderState->NumTraversals += 1;
#endif
    
    //GetNodeFMinMax4Pts(Center);
    node_minmax_result NodeData = GetNodeMinMaxAxis(GlobalCov1BitState->RadiusTable,
                                                    GlobalCov1BitState->DeltaX,
                                                    GlobalCov1BitState->DeltaY,
                                                    GlobalCov1BitState->DeltaZ,
                                                    GlobalCov1BitState->RadiusX,
                                                    GlobalCov1BitState->RadiusY,
                                                    GlobalCov1BitState->RadiusFrontZ,
                                                    GlobalCov1BitState->RadiusBackZ,
                                                    GlobalCov1BitState->NearPlane, Center, Level);
    
    // NOTE: Default rounding mode is round to nearest
    __m256i MinXY = _mm256_or_si256(_mm256_cvtps_epi32(NodeData.FMinX), _mm256_slli_epi32(_mm256_cvtps_epi32(NodeData.FMinY), 16));
    __m256i MaxXY = _mm256_or_si256(_mm256_cvtps_epi32(NodeData.FMaxX), _mm256_slli_epi32(_mm256_cvtps_epi32(NodeData.FMaxY), 16));

    // NOTE: Compute bool values for our rejection\leaf tests
    __m256 Rejection;
    {
        // NOTE: Node behind camera rejection test
        __m256 BehindCamTest = _mm256_cmp_ps(NodeData.BackZ, NodeData.NearPlane, _CMP_LE_OS);

        // NOTE: Node is of size 0 tests
        __m256i Size0Test = _mm256_cmpeq_epi16(MaxXY, MinXY);
        Size0Test = _mm256_or_si256(Size0Test, _mm256_slli_epi32(Size0Test, 16));

        __m256 NoChildTest = _mm256_cmp_ps(_mm256_load_ps((f32*)Node->ChildrenId), _mm256_setzero_ps(),
                                           _CMP_EQ_OS);

        // TODO: I don't think this is the best way to tell the compiler that I want it to just use
        // the raw bits as they are stored in size0test
        Rejection = _mm256_or_ps(_mm256_or_ps(BehindCamTest, *((__m256*)&Size0Test)),
                                 NoChildTest);
    }
    
    // NOTE: Compute bool values for our leaf tests
    __m256i LeafTest;
    {
        __m256i MaxLevelTest = _mm256_set1_epi32(Level == MAX_LEVEL ? 0xFFFFFFFF : 0);

        //LeafTest = _mm256_or_si256(LeafTest, _mm256_load_si256((__m256i*)Node->ChildrenId));

        __m256i PixelSizeTest = _mm256_cmpgt_epi16(_mm256_set1_epi16(2), _mm256_sub_epi16(MaxXY, MinXY));
        PixelSizeTest = _mm256_and_si256(PixelSizeTest, _mm256_slli_epi32(PixelSizeTest, 16));

        LeafTest = _mm256_or_si256(MaxLevelTest, PixelSizeTest);
    }

    // NOTE: Calc vals needed for blocked pixel traversals (assumes 8x8)
    __m256i MinBlockXY = _mm256_srai_epi16(MinXY, 3);
    __m256i MaxBlockXY = _mm256_srai_epi16(MaxXY, 3);
    __m256i Vec7 = _mm256_set1_epi16(7);
    __m256i InMinBlockXY = _mm256_and_si256(MinXY, Vec7);
    __m256i InMaxBlockXY = _mm256_and_si256(MaxXY, Vec7);
    
    // TODO: Aligned somehow?
    
    i16 MinBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)MinBlockXYStored, MinBlockXY);

    i16 MaxBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)MaxBlockXYStored, MaxBlockXY);

    i16 InMinBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)InMinBlockXYStored, InMinBlockXY);

    i16 InMaxBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)InMaxBlockXYStored, InMaxBlockXY);
    
    f32 MinZStored[8];
    _mm256_storeu_ps(MinZStored, NodeData.FrontZ);

    f32 ChildCenterXStored[8];
    _mm256_storeu_ps(ChildCenterXStored, NodeData.ChildCenterX);

    f32 ChildCenterYStored[8];
    _mm256_storeu_ps(ChildCenterYStored, NodeData.ChildCenterY);

    f32 ChildCenterZStored[8];
    _mm256_storeu_ps(ChildCenterZStored, NodeData.ChildCenterZ);

    i32 LeafTestStored[8];
    _mm256_storeu_si256((__m256i*)LeafTestStored, LeafTest);

    u32 RejTestMask = _mm256_movemask_ps(Rejection);
    u32 LeafTestMask = _mm256_movemask_ps(_mm256_castsi256_ps(LeafTest));

    // TODO: Nice way to force unroll? Or partially unrolled
    // NOTE: For each child, behind camera test, clip test, occlusion test, render/traverse
    for (u32 CurrNodeCount = 0; CurrNodeCount < 8; ++CurrNodeCount)
    {
        u8 NodeId = Order[CurrNodeCount];
    
        if (RejTestMask & (1 << NodeId))
        {
#if GET_STATS
            GlobalRenderState->NumRejected += 1;
#endif
            continue;
        }            

        i16 CurrMinBlockX = MinBlockXYStored[2*NodeId];
        i16 CurrMinBlockY = MinBlockXYStored[2*NodeId + 1];
        i16 CurrMaxBlockX = MaxBlockXYStored[2*NodeId];
        i16 CurrMaxBlockY = MaxBlockXYStored[2*NodeId + 1];

        i16 CurrInMinBlockX = InMinBlockXYStored[2*NodeId];
        i16 CurrInMinBlockY = InMinBlockXYStored[2*NodeId + 1];
        i16 CurrInMaxBlockX = InMaxBlockXYStored[2*NodeId];
        i16 CurrInMaxBlockY = InMaxBlockXYStored[2*NodeId + 1];

        f32 CurrMinZ = MinZStored[NodeId];
            
        // NOTE: Leaf so render the point
        if (LeafTestMask & (1 << NodeId))
        {
#if GET_STATS
            GlobalRenderState->LowestLeafLevel = Max(Level, GlobalRenderState->LowestLeafLevel);
            GlobalRenderState->NumRendered += 1;
            
#if COVERAGE_1BIT_ENABLE_8X8
            RenderPointCov1Bit8x8(GlobalCov1BitState->CoverageMap1Bit,
                                  GlobalCov1BitState->DepthMap,
                                  GlobalCov1BitState->LevelMap,
                                  GlobalCov1BitState->OverdrawMap,
                                  CurrMinBlockX, CurrMinBlockY, CurrInMinBlockX, CurrInMinBlockY,
                                  CurrMinZ, Level);
#else
            RenderPointCov1Bit(GlobalCov1BitState->CoverageMap1Bit,
                               GlobalCov1BitState->DepthMap,
                               GlobalCov1BitState->LevelMap,
                               GlobalCov1BitState->OverdrawMap,
                               CurrMinX, CurrMinY, CurrMinZ, Level);
#endif
            
#endif
            
#if COVERAGE_1BIT_ENABLE_8X8
            RenderPointCov1Bit8x8(GlobalCov1BitState->CoverageMap1Bit,
                                  GlobalCov1BitState->DepthMap,
                                  CurrMinBlockX, CurrMinBlockY, CurrInMinBlockX, CurrInMinBlockY,
                                  CurrMinZ);
#else
            RenderPointCov1Bit(GlobalCov1BitState->CoverageMap1Bit,
                               GlobalCov1BitState->DepthMap,
                               CurrMinX, CurrMinY, CurrMinZ, Level);
#endif

            continue;
        }

        // NOTE: Occlusion test
        b32 Occluded = true;
        
#if COVERAGE_1BIT_ENABLE_8X8
        Occluded = IsNodeOccludedCov1Bit8x8(GlobalCov1BitState->CoverageMap1Bit, GlobalCov1BitState->MinMaskX8x8,
                                            CurrMinBlockX, CurrMaxBlockX, CurrMinBlockY, CurrMaxBlockY,
                                            CurrInMinBlockX, CurrInMaxBlockX, CurrInMinBlockY, CurrInMaxBlockY);
#else
        Occluded = IsNodeOccludedCov1Bit(GlobalCov1BitState->CoverageMap1Bit, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#endif        
        if (Occluded)
        {
#if GET_STATS
            GlobalRenderState->NumRejected += 1;
#endif

            continue;
        }
        
        v3 CurrCenter = V3(ChildCenterXStored[NodeId], ChildCenterYStored[NodeId], ChildCenterZStored[NodeId]);
        u32 ChildNodeId = Node->ChildrenId[NodeId];
        octree_id* ChildPtr = (octree_id*)(GlobalCov1BitState->NodeBase + ChildNodeId);
        Cov1BitRenderOctreeNoSortNoClip(CurrCenter, ChildPtr, Level + 1, Order);
    }                
}

internal void Cov1BitRenderOctreeNoClip(v3 Center, octree_id* Node, u32 Level, b32 Sort = true)
{
#if GET_STATS
    GlobalRenderState->NumTraversals += 1;
#endif
    
    node_minmax_result NodeData = GetNodeMinMaxAxis(GlobalCov1BitState->RadiusTable,
                                                    GlobalCov1BitState->DeltaX,
                                                    GlobalCov1BitState->DeltaY,
                                                    GlobalCov1BitState->DeltaZ,
                                                    GlobalCov1BitState->RadiusX,
                                                    GlobalCov1BitState->RadiusY,
                                                    GlobalCov1BitState->RadiusFrontZ,
                                                    GlobalCov1BitState->RadiusBackZ,
                                                    GlobalCov1BitState->NearPlane, Center, Level);
    
    // NOTE: Check if we need to sort
    __m256 NeedSort = _mm256_or_ps(NodeData.BetweenX, NodeData.BetweenY);

    // NOTE: Default rounding mode is round to nearest
    __m256i MinXY = _mm256_or_si256(_mm256_cvtps_epi32(NodeData.FMinX), _mm256_slli_epi32(_mm256_cvtps_epi32(NodeData.FMinY), 16));
    __m256i MaxXY = _mm256_or_si256(_mm256_cvtps_epi32(NodeData.FMaxX), _mm256_slli_epi32(_mm256_cvtps_epi32(NodeData.FMaxY), 16));

    // NOTE: Compute bool values for our rejection\leaf tests
    __m256 Rejection;
    {
        // NOTE: Node behind camera rejection test
        __m256 BehindCamTest = _mm256_cmp_ps(NodeData.BackZ, NodeData.NearPlane, _CMP_LE_OS);

        // NOTE: Node is of size 0 tests
        __m256i Size0Test = _mm256_cmpeq_epi16(MaxXY, MinXY);
        Size0Test = _mm256_or_si256(Size0Test, _mm256_slli_epi32(Size0Test, 16));

        __m256 NoChildTest = _mm256_cmp_ps(_mm256_load_ps((f32*)Node->ChildrenId), _mm256_setzero_ps(),
                                           _CMP_EQ_OS);

        // TODO: I don't think this is the best way to tell the compiler that I want it to just use
        // the raw bits as they are stored in size0test
        Rejection = _mm256_or_ps(_mm256_or_ps(BehindCamTest, *((__m256*)&Size0Test)),
                                 NoChildTest);
    }
    
    // NOTE: Compute bool values for our leaf tests
    __m256i LeafTest;
    {
        __m256i MaxLevelTest = _mm256_set1_epi32(Level == MAX_LEVEL ? 0xFFFFFFFF : 0);

        //LeafTest = _mm256_or_si256(LeafTest, _mm256_load_si256((__m256i*)Node->ChildrenId));

        __m256i PixelSizeTest = _mm256_cmpgt_epi16(_mm256_set1_epi16(2), _mm256_sub_epi16(MaxXY, MinXY));
        PixelSizeTest = _mm256_and_si256(PixelSizeTest, _mm256_slli_epi32(PixelSizeTest, 16));

        LeafTest = _mm256_or_si256(MaxLevelTest, PixelSizeTest);
    }

    // NOTE: Calc vals needed for blocked pixel traversals (assumes 8x8)
    __m256i MinBlockXY = _mm256_srai_epi16(MinXY, 3);
    __m256i MaxBlockXY = _mm256_srai_epi16(MaxXY, 3);
    __m256i Vec7 = _mm256_set1_epi16(7);
    __m256i InMinBlockXY = _mm256_and_si256(MinXY, Vec7);
    __m256i InMaxBlockXY = _mm256_and_si256(MaxXY, Vec7);
    
    // TODO: Aligned somehow?
    
    i16 MinBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)MinBlockXYStored, MinBlockXY);

    i16 MaxBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)MaxBlockXYStored, MaxBlockXY);

    i16 InMinBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)InMinBlockXYStored, InMinBlockXY);

    i16 InMaxBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)InMaxBlockXYStored, InMaxBlockXY);
    
    f32 MinZStored[8];
    _mm256_storeu_ps(MinZStored, NodeData.FrontZ);

    f32 ChildCenterXStored[8];
    _mm256_storeu_ps(ChildCenterXStored, NodeData.ChildCenterX);

    f32 ChildCenterYStored[8];
    _mm256_storeu_ps(ChildCenterYStored, NodeData.ChildCenterY);

    f32 ChildCenterZStored[8];
    _mm256_storeu_ps(ChildCenterZStored, NodeData.ChildCenterZ);

    u32 RejectionMask = _mm256_movemask_ps(Rejection);
    u32 LeafTestMask = _mm256_movemask_ps(_mm256_castsi256_ps(LeafTest));

    // NOTE: Sort front to back
    f32 NeedSortStored[8];
    _mm256_storeu_ps(NeedSortStored, NeedSort);
    
    __m256 Distances = _mm256_mul_ps(NodeData.ChildCenterX, NodeData.ChildCenterX);
    Distances = _mm256_add_ps(Distances, _mm256_mul_ps(NodeData.ChildCenterY, NodeData.ChildCenterY));
    Distances = _mm256_add_ps(Distances, _mm256_mul_ps(NodeData.ChildCenterZ, NodeData.ChildCenterZ));

    f32 Keys[8];
    _mm256_storeu_ps(Keys, Distances);
    u8 Order[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    BubbleSort(Keys, Order, 8);

    // TODO: Nice way to force unroll? Or partially unrolled
    // NOTE: For each child, behind camera test, clip test, occlusion test, render/traverse
    for (u32 CurrNodeCount = 0; CurrNodeCount < 8; ++CurrNodeCount)
    {
        u8 NodeId = Order[CurrNodeCount];
    
        if (RejectionMask & (1 << NodeId))
        {
#if GET_STATS
            GlobalRenderState->NumRejected += 1;
#endif
            continue;
        }
    
        i16 CurrMinBlockX = MinBlockXYStored[2*NodeId];
        i16 CurrMinBlockY = MinBlockXYStored[2*NodeId + 1];
        i16 CurrMaxBlockX = MaxBlockXYStored[2*NodeId];
        i16 CurrMaxBlockY = MaxBlockXYStored[2*NodeId + 1];

        i16 CurrInMinBlockX = InMinBlockXYStored[2*NodeId];
        i16 CurrInMinBlockY = InMinBlockXYStored[2*NodeId + 1];
        i16 CurrInMaxBlockX = InMaxBlockXYStored[2*NodeId];
        i16 CurrInMaxBlockY = InMaxBlockXYStored[2*NodeId + 1];

        f32 CurrMinZ = MinZStored[NodeId];
  
        // NOTE: Leaf so render the point
        if (LeafTestMask & (1 << NodeId))
        {
#if GET_STATS
            GlobalRenderState->LowestLeafLevel = Max(Level, GlobalRenderState->LowestLeafLevel);
            GlobalRenderState->NumRendered += 1;

#if COVERAGE_1BIT_ENABLE_8X8
            RenderPointCov1Bit8x8(GlobalCov1BitState->CoverageMap1Bit,
                                  GlobalCov1BitState->DepthMap,
                                  GlobalCov1BitState->LevelMap,
                                  GlobalCov1BitState->OverdrawMap,
                                  CurrMinBlockX, CurrMinBlockY, CurrInMinBlockX, CurrInMinBlockY,
                                  CurrMinZ, Level);
#else
            RenderPointCov1Bit(GlobalCov1BitState->CoverageMap1Bit,
                               GlobalCov1BitState->DepthMap,
                               GlobalCov1BitState->LevelMap,
                               GlobalCov1BitState->OverdrawMap,
                               CurrMinX, CurrMinY, CurrMinZ, Level);
#endif
            
#endif
            
#if COVERAGE_1BIT_ENABLE_8X8
            RenderPointCov1Bit8x8(GlobalCov1BitState->CoverageMap1Bit,
                                  GlobalCov1BitState->DepthMap,
                                  CurrMinBlockX, CurrMinBlockY, CurrInMinBlockX, CurrInMinBlockY,
                                  CurrMinZ);
#else
            RenderPointCov1Bit(GlobalCov1BitState->CoverageMap1Bit,
                               GlobalCov1BitState->DepthMap,
                               CurrMinX, CurrMinY, CurrMinZ, Level);
#endif

            continue;
        }

        // NOTE: Occlusion test
        b32 Occluded = true;
#if COVERAGE_1BIT_ENABLE_8X8
        Occluded = IsNodeOccludedCov1Bit8x8(GlobalCov1BitState->CoverageMap1Bit, GlobalCov1BitState->MinMaskX8x8,
                                            CurrMinBlockX, CurrMaxBlockX, CurrMinBlockY, CurrMaxBlockY,
                                            CurrInMinBlockX, CurrInMaxBlockX, CurrInMinBlockY, CurrInMaxBlockY);
#else
        Occluded = IsNodeOccludedCov1Bit(GlobalCov1BitState->CoverageMap1Bit, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#endif
        if (Occluded)
        {
#if GET_STATS
            GlobalRenderState->NumRejected += 1;
#endif

            continue;
        }
        
        v3 CurrCenter = V3(ChildCenterXStored[NodeId], ChildCenterYStored[NodeId], ChildCenterZStored[NodeId]);
        u32 ChildNodeId = Node->ChildrenId[NodeId];
        octree_id* ChildPtr = (octree_id*)(GlobalCov1BitState->NodeBase + ChildNodeId);
        if (NeedSortStored[NodeId] == 0xFFFFFFFF)
        {
            Cov1BitRenderOctreeNoClip(CurrCenter, ChildPtr, Level + 1);
        }
        else
        {
            Cov1BitRenderOctreeNoSortNoClip(CurrCenter, ChildPtr, Level + 1, Order);
        }
    }                
}

internal void Cov1BitRenderOctreeNoSort(v3 Center, octree_id* Node, u32 Level, u8 Order[8])
{
#if GET_STATS
    GlobalRenderState->NumTraversals += 1;
#endif
    
    node_minmax_result NodeData = GetNodeMinMaxAxis(GlobalCov1BitState->RadiusTable,
                                                    GlobalCov1BitState->DeltaX,
                                                    GlobalCov1BitState->DeltaY,
                                                    GlobalCov1BitState->DeltaZ,
                                                    GlobalCov1BitState->RadiusX,
                                                    GlobalCov1BitState->RadiusY,
                                                    GlobalCov1BitState->RadiusFrontZ,
                                                    GlobalCov1BitState->RadiusBackZ,
                                                    GlobalCov1BitState->NearPlane, Center, Level);

    __m256i NoClip = _mm256_setzero_si256();
    
    // NOTE: Default rounding mode is round to nearest
    __m256i MinX = _mm256_cvtps_epi32(NodeData.FMinX);
    NoClip = _mm256_cmpgt_epi32(MinX, _mm256_set1_epi32(ScreenX-1));
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(_mm256_set1_epi32(1), MinX));
    MinX = _mm256_max_epi32(_mm256_setzero_si256(), _mm256_min_epi32(_mm256_set1_epi32(ScreenX), MinX));

    __m256i MaxX = _mm256_cvtps_epi32(NodeData.FMaxX);
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(MaxX, _mm256_set1_epi32(ScreenX-1)));
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(_mm256_set1_epi32(1), MaxX));
    MaxX = _mm256_max_epi32(_mm256_setzero_si256(), _mm256_min_epi32(_mm256_set1_epi32(ScreenX), MaxX));

    __m256i MinY = _mm256_cvtps_epi32(NodeData.FMinY);
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(MinY, _mm256_set1_epi32(ScreenY-1)));
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(_mm256_set1_epi32(1), MinY));
    MinY = _mm256_max_epi32(_mm256_setzero_si256(), _mm256_min_epi32(_mm256_set1_epi32(ScreenY), MinY));

    __m256i MaxY = _mm256_cvtps_epi32(NodeData.FMaxY);
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(MaxY, _mm256_set1_epi32(ScreenY-1)));
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(_mm256_set1_epi32(1), MaxY));
    MaxY = _mm256_max_epi32(_mm256_setzero_si256(), _mm256_min_epi32(_mm256_set1_epi32(ScreenY), MaxY));

    // NOTE: Compute bool values for our rejection\leaf tests
    __m256 Rejection;
    {
        // NOTE: Node behind camera rejection test
        __m256 BehindCamTest = _mm256_cmp_ps(NodeData.BackZ, NodeData.NearPlane, _CMP_LE_OS);

        // NOTE: Check if node is outside the screen
        __m256 OutsideScreenTest = _mm256_cvtepi32_ps(_mm256_cmpgt_epi32(MinX, _mm256_set1_epi32(ScreenX-1)));
        OutsideScreenTest = _mm256_or_ps(OutsideScreenTest, _mm256_cvtepi32_ps(_mm256_cmpgt_epi32(MinY, _mm256_set1_epi32(ScreenY-1))));
        OutsideScreenTest = _mm256_or_ps(OutsideScreenTest, _mm256_cvtepi32_ps(_mm256_cmpgt_epi32(_mm256_set1_epi32(1), MaxX)));
        OutsideScreenTest = _mm256_or_ps(OutsideScreenTest, _mm256_cvtepi32_ps(_mm256_cmpgt_epi32(_mm256_set1_epi32(1), MaxY)));
        
        // NOTE: Node is of size 0 tests
        __m256i Size0Test = _mm256_or_si256(_mm256_cmpeq_epi32(MaxX, MinX),
                                            _mm256_cmpeq_epi32(MaxY, MinY));
        __m256 NoChildTest = _mm256_cmp_ps(_mm256_load_ps((f32*)Node->ChildrenId), _mm256_setzero_ps(),
                                           _CMP_EQ_OS);

        Rejection = _mm256_or_ps(_mm256_or_ps(BehindCamTest, *((__m256*)&Size0Test)),
                                 _mm256_or_ps(OutsideScreenTest, NoChildTest));
    }    

    // NOTE: Compute bool values for our rejection tests
    // NOTE: Here we do it in int space
    __m256i TwoVec = _mm256_set1_epi32(2);
    __m256i LeafTest = _mm256_set1_epi32(Level == MAX_LEVEL ? 0xFFFFFFFF : 0);
    LeafTest = _mm256_or_si256(LeafTest, _mm256_and_si256(_mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxX, MinX)),
                                                          _mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxY, MinY))));

    __m256i MinXY = _mm256_or_si256(MinX, _mm256_slli_epi32(MinY, 16));
    __m256i MaxXY = _mm256_or_si256(MaxX, _mm256_slli_epi32(MaxY, 16));

    // NOTE: Calc vals needed for blocked pixel traversals (assumes 8x8)
    __m256i MinBlockXY = _mm256_srai_epi16(MinXY, 3);
    __m256i MaxBlockXY = _mm256_srai_epi16(MaxXY, 3);
    __m256i Vec7 = _mm256_set1_epi16(7);
    __m256i InMinBlockXY = _mm256_and_si256(MinXY, Vec7);
    __m256i InMaxBlockXY = _mm256_and_si256(MaxXY, Vec7);

    // TODO: Transposing most of these elements would be super helpful maybe
    // TODO: Aligned somehow?
    
    i16 MinBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)MinBlockXYStored, MinBlockXY);

    i16 MaxBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)MaxBlockXYStored, MaxBlockXY);

    i16 InMinBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)InMinBlockXYStored, InMinBlockXY);

    i16 InMaxBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)InMaxBlockXYStored, InMaxBlockXY);
    
    f32 MinZStored[8];
    _mm256_storeu_ps(MinZStored, NodeData.FrontZ);

    f32 ChildCenterXStored[8];
    _mm256_storeu_ps(ChildCenterXStored, NodeData.ChildCenterX);

    f32 ChildCenterYStored[8];
    _mm256_storeu_ps(ChildCenterYStored, NodeData.ChildCenterY);

    f32 ChildCenterZStored[8];
    _mm256_storeu_ps(ChildCenterZStored, NodeData.ChildCenterZ);

    u32 NotClippedStored[8];
    _mm256_storeu_si256((__m256i*)NotClippedStored, NoClip);

    u32 RejectionMask = _mm256_movemask_ps(Rejection);
    u32 LeafTestMask = _mm256_movemask_ps(_mm256_castsi256_ps(LeafTest));
    
    // NOTE: For each child, behind camera test, clip test, occlusion test, render/traverse
    for (u32 CurrNodeCount = 0; CurrNodeCount < 8; ++CurrNodeCount)
    {
        u8 NodeId = Order[CurrNodeCount];
            
        // NOTE: Rejection test
        if (RejectionMask & (1 << NodeId))
        {
#if GET_STATS
            GlobalRenderState->NumRejected += 1;
#endif
            continue;
        }

        i16 CurrMinBlockX = MinBlockXYStored[2*NodeId];
        i16 CurrMinBlockY = MinBlockXYStored[2*NodeId + 1];
        i16 CurrMaxBlockX = MaxBlockXYStored[2*NodeId];
        i16 CurrMaxBlockY = MaxBlockXYStored[2*NodeId + 1];

        i16 CurrInMinBlockX = InMinBlockXYStored[2*NodeId];
        i16 CurrInMinBlockY = InMinBlockXYStored[2*NodeId + 1];
        i16 CurrInMaxBlockX = InMaxBlockXYStored[2*NodeId];
        i16 CurrInMaxBlockY = InMaxBlockXYStored[2*NodeId + 1];

        f32 CurrMinZ = MinZStored[NodeId];
                            
        // NOTE: Leaf so render the point
        if (LeafTestMask & (1 << NodeId))
        {
#if GET_STATS
            GlobalRenderState->LowestLeafLevel = Max(Level, GlobalRenderState->LowestLeafLevel);
            GlobalRenderState->NumRendered += 1;

#if COVERAGE_1BIT_ENABLE_8X8
            RenderPointCov1Bit8x8(GlobalCov1BitState->CoverageMap1Bit,
                                  GlobalCov1BitState->DepthMap,
                                  GlobalCov1BitState->LevelMap,
                                  GlobalCov1BitState->OverdrawMap,
                                  CurrMinBlockX, CurrMinBlockY, CurrInMinBlockX, CurrInMinBlockY,
                                  CurrMinZ, Level);
#else
            RenderPointCov1Bit(GlobalCov1BitState->CoverageMap1Bit,
                               GlobalCov1BitState->DepthMap,
                               GlobalCov1BitState->LevelMap,
                               GlobalCov1BitState->OverdrawMap,
                               CurrMinX, CurrMinY, CurrMinZ, Level);
#endif
            
#endif
            
#if COVERAGE_1BIT_ENABLE_8X8
            RenderPointCov1Bit8x8(GlobalCov1BitState->CoverageMap1Bit,
                                  GlobalCov1BitState->DepthMap,
                                  CurrMinBlockX, CurrMinBlockY, CurrInMinBlockX, CurrInMinBlockY,
                                  CurrMinZ);
#else
            RenderPointCov1Bit(GlobalCov1BitState->CoverageMap1Bit,
                               GlobalCov1BitState->DepthMap,
                               CurrMinX, CurrMinY, CurrMinZ, Level);
#endif

            continue;
        }
            
        // NOTE: Occlusion test
        b32 Occluded = true;
#if COVERAGE_1BIT_ENABLE_8X8
        Occluded = IsNodeOccludedCov1Bit8x8(GlobalCov1BitState->CoverageMap1Bit, GlobalCov1BitState->MinMaskX8x8,
                                            CurrMinBlockX, CurrMaxBlockX, CurrMinBlockY, CurrMaxBlockY,
                                            CurrInMinBlockX, CurrInMaxBlockX, CurrInMinBlockY, CurrInMaxBlockY);
#else
        Occluded = IsNodeOccludedCov1Bit(GlobalCov1BitState->CoverageMap1Bit, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#endif
        if (Occluded)
        {
#if GET_STATS
            GlobalRenderState->NumRejected += 1;
#endif
            continue;
        }

        
        v3 CurrCenter = V3(ChildCenterXStored[NodeId], ChildCenterYStored[NodeId], ChildCenterZStored[NodeId]);
        u32 ChildNodeId = Node->ChildrenId[NodeId];
        octree_id* ChildPtr = (octree_id*)(GlobalCov1BitState->NodeBase + ChildNodeId);
        
        if (NotClippedStored[NodeId])
        {
            Cov1BitRenderOctreeNoSort(CurrCenter, ChildPtr, Level + 1, Order);
        }
        else
        {
            Cov1BitRenderOctreeNoSortNoClip(CurrCenter, ChildPtr, Level + 1, Order);
        }
    }
}

internal void Cov1BitRenderOctree(v3 Center, octree_id* Node, u32 Level)
{
#if GET_STATS
    GlobalRenderState->NumTraversals += 1;
#endif
    
    node_minmax_result NodeData = GetNodeMinMaxAxis(GlobalCov1BitState->RadiusTable,
                                                    GlobalCov1BitState->DeltaX,
                                                    GlobalCov1BitState->DeltaY,
                                                    GlobalCov1BitState->DeltaZ,
                                                    GlobalCov1BitState->RadiusX,
                                                    GlobalCov1BitState->RadiusY,
                                                    GlobalCov1BitState->RadiusFrontZ,
                                                    GlobalCov1BitState->RadiusBackZ,
                                                    GlobalCov1BitState->NearPlane, Center, Level);

    // NOTE: Check if we need to sort
    __m256 NeedSort = _mm256_or_ps(NodeData.BetweenX, NodeData.BetweenY);

    __m256i NoClip = _mm256_setzero_si256();
    // NOTE: Default rounding mode is round to nearest
    __m256i MinX = _mm256_cvtps_epi32(NodeData.FMinX);
    NoClip = _mm256_cmpgt_epi32(MinX, _mm256_set1_epi32(ScreenX-1));
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(_mm256_set1_epi32(1), MinX));
    MinX = _mm256_max_epi32(_mm256_setzero_si256(), _mm256_min_epi32(_mm256_set1_epi32(ScreenX), MinX));

    __m256i MaxX = _mm256_cvtps_epi32(NodeData.FMaxX);
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(MaxX, _mm256_set1_epi32(ScreenX-1)));
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(_mm256_set1_epi32(1), MaxX));
    MaxX = _mm256_max_epi32(_mm256_setzero_si256(), _mm256_min_epi32(_mm256_set1_epi32(ScreenX), MaxX));

    __m256i MinY = _mm256_cvtps_epi32(NodeData.FMinY);
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(MinY, _mm256_set1_epi32(ScreenY-1)));
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(_mm256_set1_epi32(1), MinY));
    MinY = _mm256_max_epi32(_mm256_setzero_si256(), _mm256_min_epi32(_mm256_set1_epi32(ScreenY), MinY));

    __m256i MaxY = _mm256_cvtps_epi32(NodeData.FMaxY);
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(MaxY, _mm256_set1_epi32(ScreenY-1)));
    NoClip = _mm256_or_si256(NoClip, _mm256_cmpgt_epi32(_mm256_set1_epi32(1), MaxY));
    MaxY = _mm256_max_epi32(_mm256_setzero_si256(), _mm256_min_epi32(_mm256_set1_epi32(ScreenY), MaxY));

    // NOTE: Compute bool values for our rejection\leaf tests
    __m256 Rejection;
    {
        // NOTE: Node behind camera rejection test
        __m256 BehindCamTest = _mm256_cmp_ps(NodeData.BackZ, NodeData.NearPlane, _CMP_LE_OS);

        // NOTE: Check if node is outside the screen
        __m256 OutsideScreenTest = _mm256_cvtepi32_ps(_mm256_cmpgt_epi32(MinX, _mm256_set1_epi32(ScreenX-1)));
        OutsideScreenTest = _mm256_or_ps(OutsideScreenTest, _mm256_cvtepi32_ps(_mm256_cmpgt_epi32(MinY, _mm256_set1_epi32(ScreenY-1))));
        OutsideScreenTest = _mm256_or_ps(OutsideScreenTest, _mm256_cvtepi32_ps(_mm256_cmpgt_epi32(_mm256_set1_epi32(1), MaxX)));
        OutsideScreenTest = _mm256_or_ps(OutsideScreenTest, _mm256_cvtepi32_ps(_mm256_cmpgt_epi32(_mm256_set1_epi32(1), MaxY)));
        
        // NOTE: Node is of size 0 tests
        __m256i Size0Test = _mm256_or_si256(_mm256_cmpeq_epi32(MaxX, MinX),
                                            _mm256_cmpeq_epi32(MaxY, MinY));
        __m256 NoChildTest = _mm256_cmp_ps(_mm256_load_ps((f32*)Node->ChildrenId), _mm256_setzero_ps(),
                                           _CMP_EQ_OS);

        Rejection = _mm256_or_ps(_mm256_or_ps(BehindCamTest, *((__m256*)&Size0Test)),
                                 _mm256_or_ps(OutsideScreenTest, NoChildTest));
    }    
    
    // NOTE: Compute bool values for our rejection tests
    // NOTE: Here we do it in int space
    __m256i TwoVec = _mm256_set1_epi32(2);
    __m256i LeafTest = _mm256_set1_epi32(Level == MAX_LEVEL ? 0xFFFFFFFF : 0);
    LeafTest = _mm256_or_si256(LeafTest, _mm256_and_si256(_mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxX, MinX)),
                                                          _mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxY, MinY))));

    __m256i MinXY = _mm256_or_si256(MinX, _mm256_slli_epi32(MinY, 16));
    __m256i MaxXY = _mm256_or_si256(MaxX, _mm256_slli_epi32(MaxY, 16));

    // NOTE: Calc vals needed for blocked pixel traversals (assumes 8x8)
    __m256i MinBlockXY = _mm256_srai_epi16(MinXY, 3);
    __m256i MaxBlockXY = _mm256_srai_epi16(MaxXY, 3);
    __m256i Vec7 = _mm256_set1_epi16(7);
    __m256i InMinBlockXY = _mm256_and_si256(MinXY, Vec7);
    __m256i InMaxBlockXY = _mm256_and_si256(MaxXY, Vec7);

    i16 MinBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)MinBlockXYStored, MinBlockXY);

    i16 MaxBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)MaxBlockXYStored, MaxBlockXY);

    i16 InMinBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)InMinBlockXYStored, InMinBlockXY);

    i16 InMaxBlockXYStored[16];
    _mm256_storeu_si256((__m256i*)InMaxBlockXYStored, InMaxBlockXY);
    
    f32 MinZStored[8];
    _mm256_storeu_ps(MinZStored, NodeData.FrontZ);

    f32 ChildCenterXStored[8];
    _mm256_storeu_ps(ChildCenterXStored, NodeData.ChildCenterX);

    f32 ChildCenterYStored[8];
    _mm256_storeu_ps(ChildCenterYStored, NodeData.ChildCenterY);

    f32 ChildCenterZStored[8];
    _mm256_storeu_ps(ChildCenterZStored, NodeData.ChildCenterZ);

    u32 NotClippedStored[8];
    _mm256_storeu_si256((__m256i*)NotClippedStored, NoClip);

    u32 RejectionMask = _mm256_movemask_ps(Rejection);
    u32 LeafTestMask = _mm256_movemask_ps(_mm256_castsi256_ps(LeafTest));

    // NOTE: Sort front to back
    f32 NeedSortStored[8];
    _mm256_storeu_ps(NeedSortStored, NeedSort);

    __m256 Distances = _mm256_mul_ps(NodeData.ChildCenterX, NodeData.ChildCenterX);
    Distances = _mm256_add_ps(Distances, _mm256_mul_ps(NodeData.ChildCenterY, NodeData.ChildCenterY));
    Distances = _mm256_add_ps(Distances, _mm256_mul_ps(NodeData.ChildCenterZ, NodeData.ChildCenterZ));

    f32 Keys[8];
    _mm256_storeu_ps(Keys, Distances);
    u8 Order[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    BubbleSort(Keys, Order, 8);
    
    // NOTE: For each child, behind camera test, clip test, occlusion test, render/traverse
    for (u32 CurrNodeCount = 0; CurrNodeCount < 8; ++CurrNodeCount)
    {
        u8 NodeId = Order[CurrNodeCount];
            
        // NOTE: Rejection test
        if (RejectionMask & (1 << NodeId))
        {
#if GET_STATS
            GlobalRenderState->NumRejected += 1;
#endif
            continue;
        }

        i16 CurrMinBlockX = MinBlockXYStored[2*NodeId];
        i16 CurrMinBlockY = MinBlockXYStored[2*NodeId + 1];
        i16 CurrMaxBlockX = MaxBlockXYStored[2*NodeId];
        i16 CurrMaxBlockY = MaxBlockXYStored[2*NodeId + 1];

        i16 CurrInMinBlockX = InMinBlockXYStored[2*NodeId];
        i16 CurrInMinBlockY = InMinBlockXYStored[2*NodeId + 1];
        i16 CurrInMaxBlockX = InMaxBlockXYStored[2*NodeId];
        i16 CurrInMaxBlockY = InMaxBlockXYStored[2*NodeId + 1];
             
        f32 CurrMinZ = MinZStored[NodeId];
                       
        // NOTE: Leaf so render the point
        if (LeafTestMask & (1 << NodeId))
        {
#if GET_STATS
            GlobalRenderState->LowestLeafLevel = Max(Level, GlobalRenderState->LowestLeafLevel);
            GlobalRenderState->NumRendered += 1;

#if COVERAGE_1BIT_ENABLE_8X8
            RenderPointCov1Bit8x8(GlobalCov1BitState->CoverageMap1Bit,
                                  GlobalCov1BitState->DepthMap,
                                  GlobalCov1BitState->LevelMap,
                                  GlobalCov1BitState->OverdrawMap,
                                  CurrMinBlockX, CurrMinBlockY, CurrInMinBlockX, CurrInMinBlockY,
                                  CurrMinZ, Level);
#else
            RenderPointCov1Bit(GlobalCov1BitState->CoverageMap1Bit,
                               GlobalCov1BitState->DepthMap,
                               GlobalCov1BitState->LevelMap,
                               GlobalCov1BitState->OverdrawMap,
                               CurrMinX, CurrMinY, CurrMinZ, Level);
#endif
            
#endif
            
#if COVERAGE_1BIT_ENABLE_8X8
            RenderPointCov1Bit8x8(GlobalCov1BitState->CoverageMap1Bit,
                                  GlobalCov1BitState->DepthMap,
                                  CurrMinBlockX, CurrMinBlockY, CurrInMinBlockX, CurrInMinBlockY,
                                  CurrMinZ);
#else
            RenderPointCov1Bit(GlobalCov1BitState->CoverageMap1Bit,
                               GlobalCov1BitState->DepthMap,
                               CurrMinX, CurrMinY, CurrMinZ, Level);
#endif

            continue;
        }
        
        // NOTE: Occlusion test
        b32 Occluded = true;
#if COVERAGE_1BIT_ENABLE_8X8
        Occluded = IsNodeOccludedCov1Bit8x8(GlobalCov1BitState->CoverageMap1Bit, GlobalCov1BitState->MinMaskX8x8,
                                            CurrMinBlockX, CurrMaxBlockX, CurrMinBlockY, CurrMaxBlockY,
                                            CurrInMinBlockX, CurrInMaxBlockX, CurrInMinBlockY, CurrInMaxBlockY);
#else
        Occluded = IsNodeOccludedCov1Bit(GlobalCov1BitState->CoverageMap1Bit, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#endif
        if (Occluded)
        {
#if GET_STATS
            GlobalRenderState->NumRejected += 1;
#endif
            continue;
        }
        
        v3 CurrCenter = V3(ChildCenterXStored[NodeId], ChildCenterYStored[NodeId], ChildCenterZStored[NodeId]);
        u32 ChildNodeId = Node->ChildrenId[NodeId];
        octree_id* ChildPtr = (octree_id*)(GlobalCov1BitState->NodeBase + ChildNodeId);
        if (NotClippedStored[NodeId])
        {
            if (NeedSortStored[NodeId] == 0xFFFFFFFF)
            {
                Cov1BitRenderOctree(CurrCenter, ChildPtr, Level + 1);
            }
            else
            {
                Cov1BitRenderOctreeNoSort(CurrCenter, ChildPtr, Level + 1, Order);
            }
        }
        else
        {
            if (NeedSortStored[NodeId] == 0xFFFFFFFF)
            {
                Cov1BitRenderOctreeNoClip(CurrCenter, ChildPtr, Level + 1);
            }
            else
            {
                Cov1BitRenderOctreeNoSortNoClip(CurrCenter, ChildPtr, Level + 1, Order);
            }
        }
    }
}
