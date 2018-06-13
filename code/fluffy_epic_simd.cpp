
internal void EpicSimdRenderOctreeNoSortNoClip(v3 Center, octree_id* Node, u32 Level, u8 Order[8])
{
#if GET_STATS
    GlobalRenderState->NumTraversals += 1;
#endif
    
    //GetNodeFMinMax4Pts(Center);
    node_minmax_result NodeData = GetNodeMinMaxAxis(GlobalEpicSimdState->RadiusTable,
                                                    GlobalEpicSimdState->DeltaX,
                                                    GlobalEpicSimdState->DeltaY,
                                                    GlobalEpicSimdState->DeltaZ,
                                                    GlobalEpicSimdState->RadiusX,
                                                    GlobalEpicSimdState->RadiusY,
                                                    GlobalEpicSimdState->RadiusFrontZ,
                                                    GlobalEpicSimdState->RadiusBackZ,
                                                    GlobalEpicSimdState->NearPlane, Center, Level);
    
    // NOTE: Default rounding mode is round to nearest
    __m256i MinX = _mm256_cvtps_epi32(NodeData.FMinX);
    __m256i MaxX = _mm256_cvtps_epi32(NodeData.FMaxX);
    __m256i MinY = _mm256_cvtps_epi32(NodeData.FMinY);
    __m256i MaxY = _mm256_cvtps_epi32(NodeData.FMaxY);
    
    // NOTE: Compute bool values for our rejection tests
    __m256 Rejection;
    {
        // NOTE: Node is of size 0 tests
        __m256i Size0Test = _mm256_or_si256(_mm256_cmpeq_epi32(MaxX, MinX),
                                            _mm256_cmpeq_epi32(MaxY, MinY));

        __m256 NoChildTest = _mm256_cmp_ps(_mm256_load_ps((f32*)Node->ChildrenId), _mm256_setzero_ps(),
                                           _CMP_EQ_OS);

        // TODO: I don't think this is the best way to tell the compiler that I want it to just use
        // the raw bits as they are stored in size0test
        Rejection = _mm256_or_ps(*((__m256*)&Size0Test), NoChildTest);
    }
            
    // NOTE: Compute bool values for our leaf tests
    __m256i LeafTest;
    {
        __m256i MaxLevelTest = _mm256_set1_epi32(Level == MAX_LEVEL ? 0xFFFFFFFF : 0);

        //LeafTest = _mm256_or_si256(LeafTest, _mm256_load_si256((__m256i*)Node->ChildrenId));

        __m256i TwoVec = _mm256_set1_epi32(2);
        __m256i PixelSizeTest = _mm256_and_si256(_mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxX, MinX)),
                                                 _mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxY, MinY)));
            
        LeafTest = _mm256_or_si256(MaxLevelTest, PixelSizeTest);
    }
    
    // TODO: Aligned somehow?
    
    i32 MinXStored[8];
    _mm256_storeu_si256((__m256i*)MinXStored, MinX);

    i32 MinYStored[8];
    _mm256_storeu_si256((__m256i*)MinYStored, MinY);

    i32 MaxXStored[8];
    _mm256_storeu_si256((__m256i*)MaxXStored, MaxX);

    i32 MaxYStored[8];
    _mm256_storeu_si256((__m256i*)MaxYStored, MaxY);
    
    f32 MinZStored[8];
    _mm256_storeu_ps(MinZStored, NodeData.FrontZ);

    f32 ChildCenterXStored[8];
    _mm256_storeu_ps(ChildCenterXStored, NodeData.ChildCenterX);

    f32 ChildCenterYStored[8];
    _mm256_storeu_ps(ChildCenterYStored, NodeData.ChildCenterY);

    f32 ChildCenterZStored[8];
    _mm256_storeu_ps(ChildCenterZStored, NodeData.ChildCenterZ);

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

        i32 CurrMinX = MinXStored[NodeId];
        i32 CurrMinY = MinYStored[NodeId];
        i32 CurrMaxX = MaxXStored[NodeId];
        i32 CurrMaxY = MaxYStored[NodeId];
        f32 CurrMinZ = MinZStored[NodeId];
        
        // NOTE: Leaf so render the point
        if (LeafTestMask & (1 << NodeId))
        {
#if GET_STATS
            GlobalRenderState->LowestLeafLevel = Max(Level, GlobalRenderState->LowestLeafLevel);
            GlobalRenderState->NumRendered += 1;

#if EPIC_SIMD_RENDER_DEPTH
            RenderPointDepthScalar(GlobalEpicSimdState->DepthMap,
                                   GlobalEpicSimdState->CoverageMap,
                                   Level, CurrMinX, CurrMinY, CurrMinZ);
#else
            RenderPointCoverageScalar(GlobalEpicSimdState->DepthMap,
                                      GlobalEpicSimdState->CoverageMap,
                                      GlobalEpicSimdState->LevelMap,
                                      GlobalEpicSimdState->OverdrawMap,
                                      Level, CurrMinX, CurrMinY, CurrMinZ);
#endif
            
#else
            
#if EPIC_SIMD_RENDER_DEPTH
            RenderPointDepthScalar(GlobalEpicSimdState->DepthMap,
                                   GlobalEpicSimdState->CoverageMap,
                                   Level, CurrMinX, CurrMinY, CurrMinZ);
#else
            RenderPointCoverageScalar(GlobalEpicSimdState->DepthMap,
                                      GlobalEpicSimdState->CoverageMap,
                                      Level, CurrMinX, CurrMinY, CurrMinZ);
#endif
            
#endif
            
            continue;
        }
        
        // NOTE: Occlusion test
        b32 Occluded;
#if EPIC_SIMD_RENDER_DEPTH
        Occluded = ScalarIsNodeOccludedDepth(GlobalEpicSimdState->DepthMap, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY, CurrMinZ);
#elif NODE_CHECK_COVERAGE_SCALAR_GROUP
        Occluded = ScalarIsNodeOccludedMaskCoverage(GlobalEpicSimdState->CoverageMap, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#elif RENDER_COVERAGE
        Occluded = ScalarIsNodeOccludedCoverage(GlobalEpicSimdState->CoverageMap, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
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
        octree_id* ChildPtr = (octree_id*)(GlobalEpicSimdState->NodeBase + ChildNodeId);
        EpicSimdRenderOctreeNoSortNoClip(CurrCenter, ChildPtr, Level + 1, Order);
    }
}

internal void EpicSimdRenderOctreeNoClip(v3 Center, octree_id* Node,
                                             u32 Level)
{
#if GET_STATS
    GlobalRenderState->NumTraversals += 1;
#endif
    
    //GetNodeFMinMax4Pts(Center);
    node_minmax_result NodeData = GetNodeMinMaxAxis(GlobalEpicSimdState->RadiusTable,
                                                    GlobalEpicSimdState->DeltaX,
                                                    GlobalEpicSimdState->DeltaY,
                                                    GlobalEpicSimdState->DeltaZ,
                                                    GlobalEpicSimdState->RadiusX,
                                                    GlobalEpicSimdState->RadiusY,
                                                    GlobalEpicSimdState->RadiusFrontZ,
                                                    GlobalEpicSimdState->RadiusBackZ,
                                                    GlobalEpicSimdState->NearPlane, Center, Level);
    
    // NOTE: Check if we need to sort
    __m256 NeedSort = _mm256_or_ps(NodeData.BetweenX, NodeData.BetweenY);

    // NOTE: Default rounding mode is round to nearest
    __m256i MinX = _mm256_cvtps_epi32(NodeData.FMinX);
    __m256i MaxX = _mm256_cvtps_epi32(NodeData.FMaxX);
    __m256i MinY = _mm256_cvtps_epi32(NodeData.FMinY);
    __m256i MaxY = _mm256_cvtps_epi32(NodeData.FMaxY);

    // NOTE: Compute bool values for our rejection\leaf tests
    __m256 Rejection;
    {
        // NOTE: Node is of size 0 tests
        __m256i Size0Test = _mm256_or_si256(_mm256_cmpeq_epi32(MaxX, MinX),
                                            _mm256_cmpeq_epi32(MaxY, MinY));

        __m256 NoChildTest = _mm256_cmp_ps(_mm256_load_ps((f32*)Node->ChildrenId), _mm256_setzero_ps(),
                                           _CMP_EQ_OS);

        // TODO: I don't think this is the best way to tell the compiler that I want it to just use
        // the raw bits as they are stored in size0test
        Rejection = _mm256_or_ps(NoChildTest, *((__m256*)&Size0Test));
    }
    
    // NOTE: Compute bool values for our leaf tests
    __m256i LeafTest;
    {
        __m256i MaxLevelTest = _mm256_set1_epi32(Level == MAX_LEVEL ? 0xFFFFFFFF : 0);

        //LeafTest = _mm256_or_si256(LeafTest, _mm256_load_si256((__m256i*)Node->ChildrenId));

        __m256i TwoVec = _mm256_set1_epi32(2);
        __m256i PixelSizeTest = _mm256_and_si256(_mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxX, MinX)),
                                                 _mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxY, MinY)));

        LeafTest = _mm256_or_si256(MaxLevelTest, PixelSizeTest);
    }
    
    __m256i RejLeafTest = _mm256_or_si256(_mm256_castps_si256(Rejection), _mm256_slli_epi32(LeafTest, 16));

    // TODO: Aligned somehow?
    
    f32 MinZStored[8];
    _mm256_storeu_ps(MinZStored, NodeData.FrontZ);
    
    i32 MinXStored[8];
    _mm256_storeu_si256((__m256i*)MinXStored, MinX);

    i32 MaxXStored[8];
    _mm256_storeu_si256((__m256i*)MaxXStored, MaxX);

    i32 MinYStored[8];
    _mm256_storeu_si256((__m256i*)MinYStored, MinY);

    i32 MaxYStored[8];
    _mm256_storeu_si256((__m256i*)MaxYStored, MaxY);

    f32 ChildCenterXStored[8];
    _mm256_storeu_ps(ChildCenterXStored, NodeData.ChildCenterX);

    f32 ChildCenterYStored[8];
    _mm256_storeu_ps(ChildCenterYStored, NodeData.ChildCenterY);

    f32 ChildCenterZStored[8];
    _mm256_storeu_ps(ChildCenterZStored, NodeData.ChildCenterZ);

    u32 RejLeafTestMask = _mm256_movemask_epi8(RejLeafTest);

    // NOTE: Temp sort
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

        if (RejLeafTestMask & (1 << (4*NodeId)))
        {
#if GET_STATS
            GlobalRenderState->NumRejected += 1;
#endif
            continue;
        }            
        
        i32 CurrMinX = MinXStored[NodeId];
        i32 CurrMaxX = MaxXStored[NodeId];
        i32 CurrMinY = MinYStored[NodeId];
        i32 CurrMaxY = MaxYStored[NodeId];
        
        // NOTE: Leaf so render the point
        f32 CurrMinZ = MinZStored[NodeId];

        if (RejLeafTestMask & (1 << (4*NodeId + 2)))
        {
#if GET_STATS
            GlobalRenderState->LowestLeafLevel = Max(Level, GlobalRenderState->LowestLeafLevel);
            GlobalRenderState->NumRendered += 1;
                
#if EPIC_SIMD_RENDER_DEPTH
            RenderPointDepthScalar(GlobalEpicSimdState->DepthMap,
                                   GlobalEpicSimdState->CoverageMap,
                                   Level, CurrMinX, CurrMinY, CurrMinZ);
#else
            RenderPointCoverageScalar(GlobalEpicSimdState->DepthMap,
                                      GlobalEpicSimdState->CoverageMap,
                                      GlobalEpicSimdState->LevelMap,
                                      GlobalEpicSimdState->OverdrawMap,
                                      Level, CurrMinX, CurrMinY, CurrMinZ);
#endif
            
#else
            
#if EPIC_SIMD_RENDER_DEPTH
            RenderPointDepthScalar(GlobalEpicSimdState->DepthMap,
                                   GlobalEpicSimdState->CoverageMap,
                                   Level, CurrMinX, CurrMinY, CurrMinZ);
#else
            RenderPointCoverageScalar(GlobalEpicSimdState->DepthMap,
                                      GlobalEpicSimdState->CoverageMap,
                                      Level, CurrMinX, CurrMinY, CurrMinZ);
#endif
            
#endif

            continue;
        }

        // NOTE: Occlusion test
        b32 Occluded;
#if RENDER_DEPTH
        Occluded = ScalarIsNodeOccludedDepth(GlobalEpicSimdState->DepthMap, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY, CurrMinZ);
#elif NODE_CHECK_COVERAGE_SCALAR_GROUP
        Occluded = ScalarIsNodeOccludedMaskCoverage(GlobalEpicSimdState->CoverageMap, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#else
        Occluded = ScalarIsNodeOccludedCoverage(GlobalEpicSimdState->CoverageMap, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
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
        octree_id* ChildPtr = (octree_id*)(GlobalEpicSimdState->NodeBase + ChildNodeId);
        if (NeedSortStored[NodeId] == 0xFFFFFFFF)
        {
            EpicSimdRenderOctreeNoClip(CurrCenter, ChildPtr, Level + 1);
        }
        else
        {
            EpicSimdRenderOctreeNoSortNoClip(CurrCenter, ChildPtr, Level + 1, Order);
        }
    }                
}

internal void EpicSimdRenderOctreeNoSort(v3 Center, octree_id* Node,
                                             u32 Level, u8 Order[8])
{
#if GET_STATS
    GlobalRenderState->NumTraversals += 1;
#endif
    
    //GetNodeFMinMax4Pts(Center);
    node_minmax_result NodeData = GetNodeMinMaxAxis(GlobalEpicSimdState->RadiusTable,
                                                    GlobalEpicSimdState->DeltaX,
                                                    GlobalEpicSimdState->DeltaY,
                                                    GlobalEpicSimdState->DeltaZ,
                                                    GlobalEpicSimdState->RadiusX,
                                                    GlobalEpicSimdState->RadiusY,
                                                    GlobalEpicSimdState->RadiusFrontZ,
                                                    GlobalEpicSimdState->RadiusBackZ,
                                                    GlobalEpicSimdState->NearPlane, Center, Level);

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
        NoClip = _mm256_or_si256(NoClip, _mm256_castps_si256(BehindCamTest));

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
    
    // NOTE: Compute bool values for our leaf tests
    __m256i LeafTest;
    {
        __m256i MaxLevelTest = _mm256_set1_epi32(Level == MAX_LEVEL ? 0xFFFFFFFF : 0);

        //LeafTest = _mm256_or_si256(LeafTest, _mm256_load_si256((__m256i*)Node->ChildrenId));

        __m256i TwoVec = _mm256_set1_epi32(2);
        __m256i PixelSizeTest = _mm256_and_si256(_mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxX, MinX)),
                                                 _mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxY, MinY)));

        LeafTest = _mm256_or_si256(MaxLevelTest, PixelSizeTest);
    }
    
    __m256i RejLeafTest = _mm256_or_si256(_mm256_castps_si256(Rejection), _mm256_slli_epi32(LeafTest, 16));

    // TODO: Transposing most of these elements would be super helpful maybe
    // TODO: Aligned somehow?
    f32 MinZStored[8];
    _mm256_storeu_ps(MinZStored, NodeData.FrontZ);

    i32 MinXStored[8];
    _mm256_storeu_si256((__m256i*)MinXStored, MinX);

    i32 MaxXStored[8];
    _mm256_storeu_si256((__m256i*)MaxXStored, MaxX);

    i32 MinYStored[8];
    _mm256_storeu_si256((__m256i*)MinYStored, MinY);

    i32 MaxYStored[8];
    _mm256_storeu_si256((__m256i*)MaxYStored, MaxY);

    f32 ChildCenterXStored[8];
    _mm256_storeu_ps(ChildCenterXStored, NodeData.ChildCenterX);

    f32 ChildCenterYStored[8];
    _mm256_storeu_ps(ChildCenterYStored, NodeData.ChildCenterY);

    f32 ChildCenterZStored[8];
    _mm256_storeu_ps(ChildCenterZStored, NodeData.ChildCenterZ);

    i32 NoClipStored[8];
    _mm256_storeu_si256((__m256i*)NoClipStored, NoClip);

    u32 RejLeafTestMask = _mm256_movemask_epi8(RejLeafTest);
    
    // NOTE: For each child, behind camera test, clip test, occlusion test, render/traverse
    for (u32 CurrNodeCount = 0; CurrNodeCount < 8; ++CurrNodeCount)
    {
        u8 NodeId = Order[CurrNodeCount];
        
        // NOTE: Rejection test
        if (RejLeafTestMask & (1 << (4*NodeId)))
        {
#if GET_STATS
            GlobalRenderState->NumRejected += 1;
#endif
            continue;
        }            
        
        i32 CurrMinX = MinXStored[NodeId];
        i32 CurrMaxX = MaxXStored[NodeId];
        i32 CurrMinY = MinYStored[NodeId];
        i32 CurrMaxY = MaxYStored[NodeId];
        f32 CurrMinZ = MinZStored[NodeId];
        
        // NOTE: Leaf so render the point
        if (RejLeafTestMask & (1 << (4*NodeId + 2)))
        {
#if GET_STATS
            GlobalRenderState->LowestLeafLevel = Max(Level, GlobalRenderState->LowestLeafLevel);
            GlobalRenderState->NumRendered += 1;
                
#if EPIC_SIMD_RENDER_DEPTH
            RenderPointDepthScalar(GlobalEpicSimdState->DepthMap,
                                   GlobalEpicSimdState->CoverageMap,
                                   Level, CurrMinX, CurrMinY, CurrMinZ);
#else
            RenderPointCoverageScalar(GlobalEpicSimdState->DepthMap,
                                      GlobalEpicSimdState->CoverageMap,
                                      GlobalEpicSimdState->LevelMap,
                                      GlobalEpicSimdState->OverdrawMap,
                                      Level, CurrMinX, CurrMinY, CurrMinZ);
#endif
            
#else
            
#if EPIC_SIMD_RENDER_DEPTH
            RenderPointDepthScalar(GlobalEpicSimdState->DepthMap,
                                   GlobalEpicSimdState->CoverageMap,
                                   Level, CurrMinX, CurrMinY, CurrMinZ);
#else
            RenderPointCoverageScalar(GlobalEpicSimdState->DepthMap,
                                      GlobalEpicSimdState->CoverageMap,
                                      Level, CurrMinX, CurrMinY, CurrMinZ);
#endif
            
#endif

            continue;
        }
            
        // NOTE: Occlusion test
        b32 Occluded;
#if RENDER_DEPTH
        Occluded = ScalarIsNodeOccludedDepth(GlobalEpicSimdState->DepthMap, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY, CurrMinZ);
#elif NODE_CHECK_COVERAGE_SCALAR_GROUP
        Occluded = ScalarIsNodeOccludedMaskCoverage(GlobalEpicSimdState->CoverageMap, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#else
        Occluded = ScalarIsNodeOccludedCoverage(GlobalEpicSimdState->CoverageMap, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
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
        octree_id* ChildPtr = (octree_id*)(GlobalEpicSimdState->NodeBase + ChildNodeId);
        
        if (NoClipStored[NodeId] == 0xFFFFFFFF)
        {
            EpicSimdRenderOctreeNoSort(CurrCenter, ChildPtr, Level + 1, Order);
        }
        else
        {
            EpicSimdRenderOctreeNoSortNoClip(CurrCenter, ChildPtr, Level + 1, Order);
        }
    }
}

internal void EpicSimdRenderOctree(v3 Center, octree_id* Node, u32 Level)
{
#if GET_STATS
    GlobalRenderState->NumTraversals += 1;
#endif
    
    //GetNodeFMinMax4Pts(Center);
    node_minmax_result NodeData = GetNodeMinMaxAxis(GlobalEpicSimdState->RadiusTable,
                                                    GlobalEpicSimdState->DeltaX,
                                                    GlobalEpicSimdState->DeltaY,
                                                    GlobalEpicSimdState->DeltaZ,
                                                    GlobalEpicSimdState->RadiusX,
                                                    GlobalEpicSimdState->RadiusY,
                                                    GlobalEpicSimdState->RadiusFrontZ,
                                                    GlobalEpicSimdState->RadiusBackZ,
                                                    GlobalEpicSimdState->NearPlane, Center, Level);

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
        NoClip = _mm256_or_si256(NoClip, _mm256_castps_si256(BehindCamTest));

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
    
    // NOTE: Compute bool values for our leaf tests
    __m256i LeafTest;
    {
        __m256i MaxLevelTest = _mm256_set1_epi32(Level == MAX_LEVEL ? 0xFFFFFFFF : 0);

        //LeafTest = _mm256_or_si256(LeafTest, _mm256_load_si256((__m256i*)Node->ChildrenId));

        __m256i TwoVec = _mm256_set1_epi32(2);
        __m256i PixelSizeTest = _mm256_and_si256(_mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxX, MinX)),
                                                 _mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxY, MinY)));

        LeafTest = _mm256_or_si256(MaxLevelTest, PixelSizeTest);
    }
    
    __m256i RejLeafTest = _mm256_or_si256(_mm256_castps_si256(Rejection), _mm256_slli_epi32(LeafTest, 16));

    // TODO: Aligned somehow?
    f32 MinZStored[8];
    _mm256_storeu_ps(MinZStored, NodeData.FrontZ);

    i32 MinXStored[8];
    _mm256_storeu_si256((__m256i*)MinXStored, MinX);

    i32 MaxXStored[8];
    _mm256_storeu_si256((__m256i*)MaxXStored, MaxX);

    i32 MinYStored[8];
    _mm256_storeu_si256((__m256i*)MinYStored, MinY);

    i32 MaxYStored[8];
    _mm256_storeu_si256((__m256i*)MaxYStored, MaxY);

    f32 ChildCenterXStored[8];
    _mm256_storeu_ps(ChildCenterXStored, NodeData.ChildCenterX);

    f32 ChildCenterYStored[8];
    _mm256_storeu_ps(ChildCenterYStored, NodeData.ChildCenterY);

    f32 ChildCenterZStored[8];
    _mm256_storeu_ps(ChildCenterZStored, NodeData.ChildCenterZ);

    i32 NoClipStored[8];
    _mm256_storeu_si256((__m256i*)NoClipStored, NoClip);

    u32 RejLeafTestMask = _mm256_movemask_epi8(RejLeafTest);

    // NOTE: Temp sort
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
        if (RejLeafTestMask & (1 << (4*NodeId)))
        {
#if GET_STATS
            GlobalRenderState->NumRejected += 1;
#endif
            continue;
        }            
        
        i32 CurrMinX = MinXStored[NodeId];
        i32 CurrMaxX = MaxXStored[NodeId];
        i32 CurrMinY = MinYStored[NodeId];
        i32 CurrMaxY = MaxYStored[NodeId];
        f32 CurrMinZ = MinZStored[NodeId];
        
        // NOTE: Leaf so render the point
        if (RejLeafTestMask & (1 << (4*NodeId + 2)))
        {
#if GET_STATS
            GlobalRenderState->LowestLeafLevel = Max(Level, GlobalRenderState->LowestLeafLevel);
            GlobalRenderState->NumRendered += 1;
                
#if EPIC_SIMD_RENDER_DEPTH
            RenderPointDepthScalar(GlobalEpicSimdState->DepthMap,
                                   GlobalEpicSimdState->CoverageMap,
                                   Level, CurrMinX, CurrMinY, CurrMinZ);
#else
            RenderPointCoverageScalar(GlobalEpicSimdState->DepthMap,
                                      GlobalEpicSimdState->CoverageMap,
                                      GlobalEpicSimdState->LevelMap,
                                      GlobalEpicSimdState->OverdrawMap,
                                      Level, CurrMinX, CurrMinY, CurrMinZ);
#endif
            
#else
            
#if EPIC_SIMD_RENDER_DEPTH
            RenderPointDepthScalar(GlobalEpicSimdState->DepthMap,
                                   GlobalEpicSimdState->CoverageMap,
                                   Level, CurrMinX, CurrMinY, CurrMinZ);
#else
            RenderPointCoverageScalar(GlobalEpicSimdState->DepthMap,
                                      GlobalEpicSimdState->CoverageMap,
                                      Level, CurrMinX, CurrMinY, CurrMinZ);
#endif
            
#endif

            continue;
        }
            
        // NOTE: Occlusion test
        b32 Occluded;
#if RENDER_DEPTH
        Occluded = ScalarIsNodeOccludedDepth(GlobalEpicSimdState->DepthMap, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY, CurrMinZ);
#elif NODE_CHECK_COVERAGE_SCALAR_GROUP
        Occluded = ScalarIsNodeOccludedMaskCoverage(GlobalEpicSimdState->CoverageMap, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#else
        Occluded = ScalarIsNodeOccludedCoverage(GlobalEpicSimdState->CoverageMap, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
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
        octree_id* ChildPtr = (octree_id*)(GlobalEpicSimdState->NodeBase + ChildNodeId);
        if (NoClipStored[NodeId] == 0xFFFFFFFF)
        {
            if (NeedSortStored[NodeId] == 0xFFFFFFFF)
            {
                EpicSimdRenderOctree(CurrCenter, ChildPtr, Level + 1);
            }
            else
            {
                EpicSimdRenderOctreeNoSort(CurrCenter, ChildPtr, Level + 1, Order);
            }
        }
        else
        {
            if (NeedSortStored[NodeId] == 0xFFFFFFFF)
            {
                EpicSimdRenderOctreeNoClip(CurrCenter, ChildPtr, Level + 1);
            }
            else
            {
                EpicSimdRenderOctreeNoSortNoClip(CurrCenter, ChildPtr, Level + 1, Order);
            }
        }
    }
}
