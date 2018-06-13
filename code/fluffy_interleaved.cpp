
#define RENDER_DEPTH 0
#define RENDER_COVERAGE 1
#define NODE_CHECK_COVERAGE_SCALAR_GROUP 1

#define INTERLEAVE_TEST_RESULTS 0

#define COVERAGE_FULL 0xFF

// TODO: 16 bit min/max types
// Maybe we should transpose the data into u8 (rejection), u8 (render), min/max xy + min z (occlusion),
// center x,y,z for traversal. It might even be better to keep centers seperate because of so many
// rejections, that it just fragments memory. Also the u8's can be in __m128's with 8 epi8 values

struct interleaved_minmax_result
{
    __m256 FMinXY1;
    __m256 FMinXY2;
    __m256 FMaxXY1;
    __m256 FMaxXY2;
    
    __m256 FrontZ;
    __m256 BackZ;
    __m256 NearPlane;

    __m256 BetweenXY1;
    __m256 BetweenXY2;
    
    __m256 ChildCenterXY1;
    __m256 ChildCenterXY2;
    __m256 ChildCenterZ;
};

// TODO: We can use not operations to reduce the amount of cmp instructions, which should
// reduce latency a lot, we would just have to figure out if the not will convert a >= to <= (equals!)

__forceinline interleaved_minmax_result GetNodeMinMaxAxisInterleaved(render_state* RenderState,
                                                                     v3 Center, u32 Level)
{
    interleaved_minmax_result Result;
    
    __m256 CenterRadius = _mm256_set1_ps(RadiusTable[Level]);
    __m256 CenterXY = _mm256_set_ps(Center.x, Center.y, Center.x, Center.y, Center.x, Center.y, Center.x, Center.y);
    Result.ChildCenterXY1 = _mm256_add_ps(CenterXY, _mm256_mul_ps(CenterRadius, RenderState->DeltaXY1));
    Result.ChildCenterXY2 = _mm256_add_ps(CenterXY, _mm256_mul_ps(CenterRadius, RenderState->DeltaXY2));
    Result.ChildCenterZ = _mm256_add_ps(_mm256_set1_ps(Center.z), _mm256_mul_ps(CenterRadius, RenderState->DeltaZ));

    __m256 ChildCenterZ1 = _mm256_unpacklo_ps(Result.ChildCenterZ, Result.ChildCenterZ);
    __m256 ChildCenterZ2 = _mm256_unpackhi_ps(Result.ChildCenterZ, Result.ChildCenterZ);
    
    __m256 CmpMaskXY1 = _mm256_cmp_ps(Result.ChildCenterXY1, _mm256_setzero_ps(), _CMP_LT_OS);
    __m256 CmpMaskXY2 = _mm256_cmp_ps(Result.ChildCenterXY2, _mm256_setzero_ps(), _CMP_LT_OS);
    __m256 P0XY1 = Result.ChildCenterXY1;
    __m256 P0XY2 = Result.ChildCenterXY2;
    __m256 P1XY1 = Result.ChildCenterXY1;
    __m256 P1XY2 = Result.ChildCenterXY2;

    __m256 RadiusXY = _mm256_set_ps(RenderState->RadiusX, RenderState->RadiusY, RenderState->RadiusX, RenderState->RadiusY, RenderState->RadiusX, RenderState->RadiusY, RenderState->RadiusX, RenderState->RadiusY);
    __m256 RadiusXY1 = _mm256_mul_ps(CenterRadius, RadiusXY);
    __m256 RadiusXY2 = _mm256_mul_ps(CenterRadius, RadiusXY);

    P0XY1 = _mm256_sub_ps(P0XY1, _mm256_and_ps(CmpMaskXY1, RadiusXY1));
    P0XY1 = _mm256_add_ps(P0XY1, _mm256_andnot_ps(CmpMaskXY1, RadiusXY1));
    P1XY1 = _mm256_add_ps(P1XY1, _mm256_and_ps(CmpMaskXY1, RadiusXY1));
    P1XY1 = _mm256_sub_ps(P1XY1, _mm256_andnot_ps(CmpMaskXY1, RadiusXY1));
    
    P0XY2 = _mm256_sub_ps(P0XY2, _mm256_and_ps(CmpMaskXY2, RadiusXY2));
    P0XY2 = _mm256_add_ps(P0XY2, _mm256_andnot_ps(CmpMaskXY2, RadiusXY2));
    P1XY2 = _mm256_add_ps(P1XY2, _mm256_and_ps(CmpMaskXY2, RadiusXY2));
    P1XY2 = _mm256_sub_ps(P1XY2, _mm256_andnot_ps(CmpMaskXY2, RadiusXY2));

    Result.NearPlane = _mm256_set1_ps(RenderState->NearPlane);
    Result.FrontZ = _mm256_max_ps(Result.NearPlane, _mm256_sub_ps(Result.ChildCenterZ, _mm256_mul_ps(CenterRadius, _mm256_set1_ps(RenderState->RadiusFrontZ))));
    __m256 FrontZ1 = _mm256_unpacklo_ps(Result.FrontZ, Result.FrontZ);
    __m256 FrontZ2 = _mm256_unpackhi_ps(Result.FrontZ, Result.FrontZ);

    Result.BackZ = _mm256_max_ps(Result.NearPlane, _mm256_add_ps(Result.ChildCenterZ, _mm256_mul_ps(CenterRadius, _mm256_set1_ps(RenderState->RadiusBackZ))));
    __m256 BackZ1 = _mm256_unpacklo_ps(Result.BackZ, Result.BackZ);
    __m256 BackZ2 = _mm256_unpackhi_ps(Result.BackZ, Result.BackZ);

    P0XY1 = _mm256_div_ps(P0XY1, FrontZ1);
    P0XY2 = _mm256_div_ps(P0XY2, FrontZ2);

    Result.BetweenXY1 = _mm256_and_ps(_mm256_cmp_ps(_mm256_setzero_ps(), P0XY1, _CMP_GE_OS),
                                      _mm256_cmp_ps(_mm256_setzero_ps(), P1XY1, _CMP_LE_OS));
    Result.BetweenXY1 = _mm256_or_ps(Result.BetweenXY1, _mm256_and_ps(_mm256_cmp_ps(_mm256_setzero_ps(), P1XY1, _CMP_GE_OS),
                                                                      _mm256_cmp_ps(_mm256_setzero_ps(), P0XY1, _CMP_LE_OS)));
    __m256 P1XY1Div = _mm256_or_ps(_mm256_and_ps(Result.BetweenXY1, FrontZ1),
                                   _mm256_andnot_ps(Result.BetweenXY1, BackZ1));
    P1XY1 = _mm256_div_ps(P1XY1, P1XY1Div);

    Result.BetweenXY2 = _mm256_and_ps(_mm256_cmp_ps(_mm256_setzero_ps(), P0XY2, _CMP_GE_OS),
                                      _mm256_cmp_ps(_mm256_setzero_ps(), P1XY2, _CMP_LE_OS));
    Result.BetweenXY2 = _mm256_or_ps(Result.BetweenXY2, _mm256_and_ps(_mm256_cmp_ps(_mm256_setzero_ps(), P1XY2, _CMP_GE_OS),
                                                                      _mm256_cmp_ps(_mm256_setzero_ps(), P0XY2, _CMP_LE_OS)));
    __m256 P1XY2Div = _mm256_or_ps(_mm256_and_ps(Result.BetweenXY2, FrontZ2),
                                   _mm256_andnot_ps(Result.BetweenXY2, BackZ2));
    P1XY2 = _mm256_div_ps(P1XY2, P1XY2Div);

    __m256 OneVec = _mm256_set1_ps(1.0f);
    __m256 HalfScreenXY = _mm256_set_ps(0.5f*ScreenX, 0.5f*ScreenY, 0.5f*ScreenX, 0.5f*ScreenY, 0.5f*ScreenX, 0.5f*ScreenY, 0.5f*ScreenX, 0.5f*ScreenY);

    Result.FMinXY1 = _mm256_mul_ps(HalfScreenXY, _mm256_add_ps(OneVec, _mm256_min_ps(P0XY1, P1XY1)));
    Result.FMinXY2 = _mm256_mul_ps(HalfScreenXY, _mm256_add_ps(OneVec, _mm256_min_ps(P0XY2, P1XY2)));

    Result.FMaxXY1 = _mm256_mul_ps(HalfScreenXY, _mm256_add_ps(OneVec, _mm256_max_ps(P0XY1, P1XY1)));
    Result.FMaxXY2 = _mm256_mul_ps(HalfScreenXY, _mm256_add_ps(OneVec, _mm256_max_ps(P0XY2, P1XY2)));

    return Result;
}

internal void InterleavedRenderOctreeNoSortNoClip(render_state* RenderState, v3 Center,
                                                   octree_id* Node, u32 Level, u8 Order[8])
{
#if GET_STATS
    RenderState->NumTraversals += 1;
#endif
    
    interleaved_minmax_result NodeData = GetNodeMinMaxAxisInterleaved(RenderState, Center, Level);
    
    // NOTE: Default rounding mode is round to nearest
    __m256i MinXY1 = _mm256_cvtps_epi32(NodeData.FMinXY1);
    __m256i MinXY2 = _mm256_cvtps_epi32(NodeData.FMinXY2);
    __m256i MaxXY1 = _mm256_cvtps_epi32(NodeData.FMaxXY1);
    __m256i MaxXY2 = _mm256_cvtps_epi32(NodeData.FMaxXY2);

    // NOTE: Compute bool values for our rejection tests
    __m256 Rejection;
    {
        // NOTE: Node behind camera rejection test
        __m256 BehindCamTest = _mm256_cmp_ps(NodeData.BackZ, NodeData.NearPlane, _CMP_LE_OS);

        // NOTE: Node is of size 0 tests
        // TODO: This test is hard to do with interleaved results
        __m256i Size0Test = _mm256_or_si256(_mm256_cmpeq_epi32(MaxX, MinX),
                                            _mm256_cmpeq_epi32(MaxY, MinY));

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

        __m256i TwoVec = _mm256_set1_epi32(2);
        __m256i PixelSizeTest = _mm256_and_si256(_mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxX, MinX)),
                                                 _mm256_cmpgt_epi32(TwoVec, _mm256_sub_epi32(MaxY, MinY)));

        LeafTest = _mm256_or_si256(MaxLevelTest, PixelSizeTest);
    }
    
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

    u32 RejTestMask = _mm256_movemask_ps(Rejection);
    u32 LeafTestMask = _mm256_movemask_ps(_mm256_castsi256_ps(LeafTest));

    // NOTE: For each child, behind camera test, clip test, occlusion test, render/traverse
    #pragma unroll
    for (u32 CurrNodeCount = 0; CurrNodeCount < 8; ++CurrNodeCount)
    {
        u8 NodeId = Order[CurrNodeCount];

        if (RejTestMask & (1 << NodeId))
        {
#if GET_STATS
            RenderState->NumRejected += 1;
#endif
            continue;
        }            
        
        i32 CurrMinX = MinXStored[NodeId];
        i32 CurrMaxX = MaxXStored[NodeId];
        i32 CurrMinY = MinYStored[NodeId];
        i32 CurrMaxY = MaxYStored[NodeId];
        f32 CurrMinZ = MinZStored[NodeId];
        
        // NOTE: Leaf so render the point
        if (LeafTestMask & (1 << NodeId))
        {
#if GET_STATS
            RenderState->LowestLeafLevel = Max(Level, RenderState->LowestLeafLevel);
            RenderState->NumRendered += 1;
#endif
            
#if RENDER_DEPTH
            RenderPointDepthScalar(RenderState, Level, CurrMinX, CurrMinY, CurrMinZ);
#elif RENDER_COVERAGE
            RenderPointCoverageScalar(RenderState, Level, CurrMinX, CurrMinY, CurrMinZ);
#endif

            continue;
        }
        
        // NOTE: Occlusion test
        b32 Occluded;
#if RENDER_DEPTH
        Occluded = IsNodeOccludedDepthScalar(RenderState, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY, CurrMinZ);
#elif NODE_CHECK_COVERAGE_SCALAR_GROUP
        Occluded = IsNodeOccludedMaskScalarCoverage(RenderState, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#elif RENDER_COVERAGE
        Occluded = IsNodeOccludedCoverageScalar(RenderState, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#endif
        if (Occluded)
        {
#if GET_STATS
            RenderState->NumRejected += 1;
#endif

            continue;
        }
        
        v3 CurrCenter = V3(ChildCenterXStored[NodeId], ChildCenterYStored[NodeId], ChildCenterZStored[NodeId]);
        u32 ChildNodeId = Node->ChildrenId[NodeId];
        octree_id* ChildPtr = (octree_id*)(RenderState->NodeBase + ChildNodeId);
        InterleavedRenderOctreeNoSortNoClip(RenderState, CurrCenter, ChildPtr, Level + 1, Order);
    }                
}

internal void InterleavedRenderOctreeNoClip(render_state* RenderState, v3 Center, octree_id* Node,
                                             u32 Level)
{
#if GET_STATS
    RenderState->NumTraversals += 1;
#endif
    
    //GetNodeFMinMax4Pts(Center);
    node_minmax_result NodeData = GetNodeMinMaxAxis(RenderState, Center, Level);
    
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
        // NOTE: Node behind camera rejection test
        __m256 BehindCamTest = _mm256_cmp_ps(NodeData.BackZ, NodeData.NearPlane, _CMP_LE_OS);

        // NOTE: Node is of size 0 tests
        __m256i Size0Test = _mm256_or_si256(_mm256_cmpeq_epi32(MaxX, MinX),
                                            _mm256_cmpeq_epi32(MaxY, MinY));

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

    BubbleSort(Keys, Order);

    // TODO: Nice way to force unroll? Or partially unrolled
    // NOTE: For each child, behind camera test, clip test, occlusion test, render/traverse
    for (u32 CurrNodeCount = 0; CurrNodeCount < 8; ++CurrNodeCount)
    {
        u8 NodeId = Order[CurrNodeCount];

        if (RejLeafTestMask & (1 << (4*NodeId)))
        {
#if GET_STATS
            RenderState->NumRejected += 1;
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
            RenderState->LowestLeafLevel = Max(Level, RenderState->LowestLeafLevel);
            RenderState->NumRendered += 1;
#endif
            
#if RENDER_DEPTH
            RenderPointDepthScalar(RenderState, Level, CurrMinX, CurrMinY, CurrMinZ);
#elif RENDER_COVERAGE
            RenderPointCoverageScalar(RenderState, Level, CurrMinX, CurrMinY, CurrMinZ);
#endif

            continue;
        }

        // NOTE: Occlusion test
        b32 Occluded;
#if RENDER_DEPTH
        Occluded = IsNodeOccludedDepthScalar(RenderState, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY, CurrMinZ);
#elif NODE_CHECK_COVERAGE_SCALAR_GROUP
        Occluded = IsNodeOccludedMaskScalarCoverage(RenderState, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#elif RENDER_COVERAGE
        Occluded = IsNodeOccludedCoverageScalar(RenderState, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#endif
        if (Occluded)
        {
#if GET_STATS
            RenderState->NumRejected += 1;
#endif

            continue;
        }
        
        v3 CurrCenter = V3(ChildCenterXStored[NodeId], ChildCenterYStored[NodeId], ChildCenterZStored[NodeId]);
        u32 ChildNodeId = Node->ChildrenId[NodeId];
        octree_id* ChildPtr = (octree_id*)(RenderState->NodeBase + ChildNodeId);
        if (NeedSortStored[NodeId] == 0xFFFFFFFF)
        {
            InterleavedRenderOctreeNoClip(RenderState, CurrCenter, ChildPtr, Level + 1);
        }
        else
        {
            InterleavedRenderOctreeNoSortNoClip(RenderState, CurrCenter, ChildPtr, Level + 1, Order);
        }
    }                
}

internal void InterleavedRenderOctreeNoSort(render_state* RenderState, v3 Center, octree_id* Node,
                                             u32 Level, u8 Order[8])
{
#if GET_STATS
    RenderState->NumTraversals += 1;
#endif
    
    //GetNodeFMinMax4Pts(Center);
    node_minmax_result NodeData = GetNodeMinMaxAxis(RenderState, Center, Level);

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
            RenderState->NumRejected += 1;
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
            RenderState->LowestLeafLevel = Max(Level, RenderState->LowestLeafLevel);
            RenderState->NumRendered += 1;
#endif
            
#if RENDER_DEPTH
            RenderPointDepthScalar(RenderState, Level, CurrMinX, CurrMinY, CurrMinZ);
#elif RENDER_COVERAGE
            RenderPointCoverageScalar(RenderState, Level, CurrMinX, CurrMinY, CurrMinZ);
#endif

            continue;
        }
            
        // NOTE: Occlusion test
        b32 Occluded;
#if RENDER_DEPTH
        Occluded = IsNodeOccludedDepthScalar(RenderState, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY, CurrMinZ);
#elif NODE_CHECK_COVERAGE_SCALAR_GROUP
        Occluded = IsNodeOccludedMaskScalarCoverage(RenderState, CurrMinX, CurrMaxX, CurrMinY,
                                                    CurrMaxY);
#elif RENDER_COVERAGE
        Occluded = IsNodeOccludedCoverageScalar(RenderState, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#endif
        if (Occluded)
        {
#if GET_STATS
            RenderState->NumRejected += 1;
#endif
            continue;
        }

        
        v3 CurrCenter = V3(ChildCenterXStored[NodeId], ChildCenterYStored[NodeId], ChildCenterZStored[NodeId]);
        u32 ChildNodeId = Node->ChildrenId[NodeId];
        octree_id* ChildPtr = (octree_id*)(RenderState->NodeBase + ChildNodeId);
        
        if (NoClipStored[NodeId] == 0xFFFFFFFF)
        {
            InterleavedRenderOctreeNoSort(RenderState, CurrCenter, ChildPtr, Level + 1, Order);
        }
        else
        {
            InterleavedRenderOctreeNoSortNoClip(RenderState, CurrCenter, ChildPtr, Level + 1, Order);
        }
    }
}

internal void InterleavedRenderOctree(render_state* RenderState, v3 Center, octree_id* Node, u32 Level)
{
#if GET_STATS
    RenderState->NumTraversals += 1;
#endif
    
    //GetNodeFMinMax4Pts(Center);
    node_minmax_result NodeData = GetNodeMinMaxAxis(RenderState, Center, Level);

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

    BubbleSort(Keys, Order);
    
    // NOTE: For each child, behind camera test, clip test, occlusion test, render/traverse
    for (u32 CurrNodeCount = 0; CurrNodeCount < 8; ++CurrNodeCount)
    {
        u8 NodeId = Order[CurrNodeCount];
            
        // NOTE: Rejection test
        if (RejLeafTestMask & (1 << (4*NodeId)))
        {
#if GET_STATS
            RenderState->NumRejected += 1;
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
            RenderState->LowestLeafLevel = Max(Level, RenderState->LowestLeafLevel);
            RenderState->NumRendered += 1;
#endif
            
#if RENDER_DEPTH
            RenderPointDepthScalar(RenderState, Level, CurrMinX, CurrMinY, CurrMinZ);
#elif RENDER_COVERAGE
            RenderPointCoverageScalar(RenderState, Level, CurrMinX, CurrMinY, CurrMinZ);
#endif

            continue;
        }
            
        // NOTE: Occlusion test
        b32 Occluded;
#if RENDER_DEPTH
        Occluded = IsNodeOccludedDepthScalar(RenderState, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY, CurrMinZ);
#elif NODE_CHECK_COVERAGE_SCALAR_GROUP
        Occluded = IsNodeOccludedMaskScalarCoverage(RenderState, CurrMinX, CurrMaxX, CurrMinY,
                                                    CurrMaxY);
#elif RENDER_COVERAGE
        Occluded = IsNodeOccludedCoverageScalar(RenderState, CurrMinX, CurrMaxX, CurrMinY, CurrMaxY);
#endif
        if (Occluded)
        {
#if GET_STATS
            RenderState->NumRejected += 1;
#endif
            continue;
        }

        
        v3 CurrCenter = V3(ChildCenterXStored[NodeId], ChildCenterYStored[NodeId], ChildCenterZStored[NodeId]);
        u32 ChildNodeId = Node->ChildrenId[NodeId];
        octree_id* ChildPtr = (octree_id*)(RenderState->NodeBase + ChildNodeId);
        if (NoClipStored[NodeId] == 0xFFFFFFFF)
        {
            if (NeedSortStored[NodeId] == 0xFFFFFFFF)
            {
                InterleavedRenderOctree(RenderState, CurrCenter, ChildPtr, Level + 1);
            }
            else
            {
                InterleavedRenderOctreeNoSort(RenderState, CurrCenter, ChildPtr, Level + 1, Order);
            }
        }
        else
        {
            if (NeedSortStored[NodeId] == 0xFFFFFFFF)
            {
                InterleavedRenderOctreeNoClip(RenderState, CurrCenter, ChildPtr, Level + 1);
            }
            else
            {
                InterleavedRenderOctreeNoSortNoClip(RenderState, CurrCenter, ChildPtr, Level + 1,
                                                     Order);
            }
        }
    }
}
