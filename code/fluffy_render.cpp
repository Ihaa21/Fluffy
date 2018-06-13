
internal void InitRenderState(render_state* RenderState, mem_arena* Arena, f32 NearPlane, f32 FarPlane,
                              u8* NodeBase)
{
    RenderState->Arena = SubArena(Arena, MegaBytes(5));

    RenderState->NearPlane = NearPlane;
    RenderState->FarPlane = FarPlane;
    RenderState->NodeBase = NodeBase;

    // NOTE: Setup radius table
    RenderState->RadiusTable[0] = 1.0f;
    for (u32 LevelId = 1; LevelId < ArrayCount(RenderState->RadiusTable); ++LevelId)
    {
        RenderState->RadiusTable[LevelId] = RenderState->RadiusTable[LevelId - 1]*0.5f;
    }
    
#if GET_STATS
    // NOTE: Setup color values for drawing the level map
    RenderState->LevelColorTable[0] = 0x00;
    RenderState->LevelColorTable[1] = 0x00FF;
    RenderState->LevelColorTable[2] = 0xFF00FF;
    RenderState->LevelColorTable[3] = 0x00FFAA;
    RenderState->LevelColorTable[4] = 0xAA00FF;
    RenderState->LevelColorTable[5] = 0xFFAA00;
    RenderState->LevelColorTable[6] = 0xF0F0F0;
    RenderState->LevelColorTable[7] = 0x0F0F0F;
    RenderState->LevelColorTable[8] = 0xAF0AF0;
    RenderState->LevelColorTable[9] = 0xFF5733;
    RenderState->LevelColorTable[10] = 0x1712A0;
    RenderState->LevelColorTable[11] = 0xC38826;
    RenderState->LevelColorTable[12] = 0x45C326;
    RenderState->LevelColorTable[13] = 0xC32696;
    RenderState->LevelColorTable[14] = 0xFF;
    RenderState->LevelColorTable[15] = 0x00FF;
    RenderState->LevelColorTable[16] = 0xFF00FF;
    RenderState->LevelColorTable[17] = 0x00FFAA;
    RenderState->LevelColorTable[18] = 0xAA00FF;
    RenderState->LevelColorTable[19] = 0xFFAA00;
    RenderState->LevelColorTable[20] = 0xF0F0F0;
    RenderState->LevelColorTable[21] = 0x0F0F0F;
    RenderState->LevelColorTable[22] = 0xAF0AF0;
    RenderState->LevelColorTable[23] = 0xFF5733;
    RenderState->LevelColorTable[24] = 0x1712A0;
#endif

    // NOTE: Init render specific data
#if OCTREE_RENDER_SCALAR
    
    render_scalar_state* ScalarState = &RenderState->ScalarState;
    ScalarState->DepthMap = PushArrayAligned(&RenderState->Arena, f32, ScreenX*ScreenY);
    RenderState->DepthMap = ScalarState->DepthMap;
    
#if GET_STATS
    
    ScalarState->OverdrawMap = PushArrayAligned(&RenderState->Arena, f32, ScreenX*ScreenY);
    RenderState->OverdrawMap = ScalarState->OverdrawMap;
    
    ScalarState->LevelMap = PushArrayAligned(&RenderState->Arena, u32, ScreenX*ScreenY);
    RenderState->LevelMap = ScalarState->LevelMap;

#endif

#elif OCTREE_RENDER_EPIC
    
    render_epic_state* EpicState = &RenderState->EpicState;
    EpicState->DepthMap = PushArrayAligned(&RenderState->Arena, f32, ScreenX*ScreenY);
    RenderState->DepthMap = EpicState->DepthMap;
    
#if GET_STATS
    
    EpicState->OverdrawMap = PushArrayAligned(&RenderState->Arena, f32, ScreenX*ScreenY);
    RenderState->OverdrawMap = EpicState->OverdrawMap;
    
    EpicState->LevelMap = PushArrayAligned(&RenderState->Arena, u32, ScreenX*ScreenY);
    RenderState->LevelMap = EpicState->LevelMap;

#endif

#elif OCTREE_RENDER_SUBDIV

    render_subdiv_state* SubdivState = &RenderState->SubdivState;
    SubdivState->DepthMap = PushArrayAligned(&RenderState->Arena, f32, ScreenX*ScreenY);
    RenderState->DepthMap = SubdivState->DepthMap;
    
#if GET_STATS
    
    SubdivState->OverdrawMap = PushArrayAligned(&RenderState->Arena, f32, ScreenX*ScreenY);
    RenderState->OverdrawMap = SubdivState->OverdrawMap;
    
    SubdivState->LevelMap = PushArrayAligned(&RenderState->Arena, u32, ScreenX*ScreenY);
    RenderState->LevelMap = SubdivState->LevelMap;

#endif

#elif OCTREE_RENDER_EPIC_SIMD

    render_epic_simd_state* EpicSimdState = &RenderState->EpicSimdState;
    EpicSimdState->NodeBase = RenderState->NodeBase;
    
    EpicSimdState->DepthMap = PushArrayAligned(&RenderState->Arena, f32, ScreenX*ScreenY);
    RenderState->DepthMap = EpicSimdState->DepthMap;

    EpicSimdState->CoverageMap = PushArrayAligned(&RenderState->Arena, u8, ScreenX*ScreenY);
    RenderState->CoverageMap = EpicSimdState->CoverageMap;
    
#if GET_STATS
    
    EpicSimdState->OverdrawMap = PushArrayAligned(&RenderState->Arena, f32, ScreenX*ScreenY);
    RenderState->OverdrawMap = EpicSimdState->OverdrawMap;
    
    EpicSimdState->LevelMap = PushArrayAligned(&RenderState->Arena, u32, ScreenX*ScreenY);
    RenderState->LevelMap = EpicSimdState->LevelMap;

#endif

#elif OCTREE_RENDER_1BIT

    render_cov_1bit_state* Cov1BitState = &RenderState->Cov1BitState;
    Cov1BitState->NodeBase = RenderState->NodeBase;

    Cov1BitState->DepthMap = PushArrayAligned(&RenderState->Arena, f32, ScreenX*ScreenY);
    RenderState->DepthMap = Cov1BitState->DepthMap;

    // TODO: Aligned
    Cov1BitState->CoverageMap1Bit = (u8*)PushSize(&RenderState->Arena, ScreenX*ScreenY/8);
    RenderState->CoverageMap1Bit = Cov1BitState->CoverageMap1Bit;

#if COVERAGE_1BIT_ENABLE_8X8

    Cov1BitState->MinMaskX8x8[0] = ((u64)(u8)(0xFF << 0) << (u64)0 | (u64)(u8)(0xFF << 0) << (u64)8 |
                                    (u64)(u8)(0xFF << 0) << (u64)16 | (u64)(u8)(0xFF << 0) << (u64)24 |
                                    (u64)(u8)(0xFF << 0) << (u64)32 | (u64)(u8)(0xFF << 0) << (u64)40 |
                                    (u64)(u8)(0xFF << 0) << (u64)48 | (u64)(u8)(0xFF << 0) << (u64)56);
    
    Cov1BitState->MinMaskX8x8[1] = ((u64)(u8)(0xFF << 1) << (u64)0 | (u64)(u8)(0xFF << 1) << (u64)8 |
                                    (u64)(u8)(0xFF << 1) << (u64)16 | (u64)(u8)(0xFF << 1) << (u64)24 |
                                    (u64)(u8)(0xFF << 1) << (u64)32 | (u64)(u8)(0xFF << 1) << (u64)40 |
                                    (u64)(u8)(0xFF << 1) << (u64)48 | (u64)(u8)(0xFF << 1) << (u64)56);
    
    Cov1BitState->MinMaskX8x8[2] = ((u64)(u8)(0xFF << 2) << (u64)0 | (u64)(u8)(0xFF << 2) << (u64)8 |
                                    (u64)(u8)(0xFF << 2) << (u64)16 | (u64)(u8)(0xFF << 2) << (u64)24 |
                                    (u64)(u8)(0xFF << 2) << (u64)32 | (u64)(u8)(0xFF << 2) << (u64)40 |
                                    (u64)(u8)(0xFF << 2) << (u64)48 | (u64)(u8)(0xFF << 2) << (u64)56);
    
    Cov1BitState->MinMaskX8x8[3] = ((u64)(u8)(0xFF << 3) << (u64)0 | (u64)(u8)(0xFF << 3) << (u64)8 |
                                    (u64)(u8)(0xFF << 3) << (u64)16 | (u64)(u8)(0xFF << 3) << (u64)24 |
                                    (u64)(u8)(0xFF << 3) << (u64)32 | (u64)(u8)(0xFF << 3) << (u64)40 |
                                    (u64)(u8)(0xFF << 3) << (u64)48 | (u64)(u8)(0xFF << 3) << (u64)56);
    
    Cov1BitState->MinMaskX8x8[4] = ((u64)(u8)(0xFF << 4) << (u64)0 | (u64)(u8)(0xFF << 4) << (u64)8 |
                                    (u64)(u8)(0xFF << 4) << (u64)16 | (u64)(u8)(0xFF << 4) << (u64)24 |
                                    (u64)(u8)(0xFF << 4) << (u64)32 | (u64)(u8)(0xFF << 4) << (u64)40 |
                                    (u64)(u8)(0xFF << 4) << (u64)48 | (u64)(u8)(0xFF << 4) << (u64)56);
    
    Cov1BitState->MinMaskX8x8[5] = ((u64)(u8)(0xFF << 5) << (u64)0 | (u64)(u8)(0xFF << 5) << (u64)8 |
                                    (u64)(u8)(0xFF << 5) << (u64)16 | (u64)(u8)(0xFF << 5) << (u64)24 |
                                    (u64)(u8)(0xFF << 5) << (u64)32 | (u64)(u8)(0xFF << 5) << (u64)40 |
                                    (u64)(u8)(0xFF << 5) << (u64)48 | (u64)(u8)(0xFF << 5) << (u64)56);
    
    Cov1BitState->MinMaskX8x8[6] = ((u64)(u8)(0xFF << 6) << (u64)0 | (u64)(u8)(0xFF << 6) << (u64)8 |
                                    (u64)(u8)(0xFF << 6) << (u64)16 | (u64)(u8)(0xFF << 6) << (u64)24 |
                                    (u64)(u8)(0xFF << 6) << (u64)32 | (u64)(u8)(0xFF << 6) << (u64)40 |
                                    (u64)(u8)(0xFF << 6) << (u64)48 | (u64)(u8)(0xFF << 6) << (u64)56);
    
    Cov1BitState->MinMaskX8x8[7] = ((u64)(u8)(0xFF << 7) << (u64)0 | (u64)(u8)(0xFF << 7) << (u64)8 |
                                    (u64)(u8)(0xFF << 7) << (u64)16 | (u64)(u8)(0xFF << 7) << (u64)24 |
                                    (u64)(u8)(0xFF << 7) << (u64)32 | (u64)(u8)(0xFF << 7) << (u64)40 |
                                    (u64)(u8)(0xFF << 7) << (u64)48 | (u64)(u8)(0xFF << 7) << (u64)56);
    
#endif
    
#if GET_STATS
    
    Cov1BitState->OverdrawMap = PushArrayAligned(&RenderState->Arena, f32, ScreenX*ScreenY);
    RenderState->OverdrawMap = Cov1BitState->OverdrawMap;
    
    Cov1BitState->LevelMap = PushArrayAligned(&RenderState->Arena, u32, ScreenX*ScreenY);
    RenderState->LevelMap = Cov1BitState->LevelMap;

#endif

#else

    RenderState->LevelMap = PushArrayAligned(&RenderState->Arena, u32, ScreenX*ScreenY);

#endif

    RenderState->CommandByte = (u8*)RenderState->Arena.Mem + RenderState->Arena.Used;
    RenderState->TempMem = BeginTempMem(&RenderState->Arena);
}

inline void PushOctree(render_state* RenderState, octree* Octree, m4 ModelMat)
{
    render_cmd* Command = PushStruct(&RenderState->Arena, render_cmd);
    Command->Type = CommandType_Octree;
    Command->ModelMat = ModelMat;
    Command->Octree = Octree;
}

inline void PushOctreeId(render_state* RenderState, octree_id* OctreeId, m4 ModelMat)
{
    render_cmd* Command = PushStruct(&RenderState->Arena, render_cmd);
    Command->Type = CommandType_OctreeId;
    Command->ModelMat = ModelMat;
    Command->OctreeId = OctreeId;
}

#define Control(a, b, c, d) (i32)((a << 0) | (b  << 2) | (c << 4) | (d << 6))

__forceinline v2 ProjectPoint(v3 Pos)
{
    v2 Result = {};
    Result = Hadamard(V2(ScreenX, ScreenY), 0.5f*((Pos.xy / Pos.z) + V2(1, 1)));
    
    return Result;
}

__forceinline void BubbleSort(f32* SortKeys, u8* SortVals, u32 NumElements)
{
    while (true)
    {
        b32 Swapped = false;
        for (u32 Index = 1; Index < NumElements; ++Index)
        {
            if (SortKeys[Index-1] > SortKeys[Index])
            {
                {
                    f32 Temp = SortKeys[Index-1];
                    SortKeys[Index-1] = SortKeys[Index];
                    SortKeys[Index] = Temp;
                }

                {
                    u8 Temp = SortVals[Index-1];
                    SortVals[Index-1] = SortVals[Index];
                    SortVals[Index] = Temp;
                }
                
                Swapped = true;
            }
        }

        if (!Swapped)
        {
            break;
        }
    }
}

__forceinline void BubbleSort(f32* SortKeys, u32* SortVals, u32 NumElements)
{
    while (true)
    {
        b32 Swapped = false;
        for (u32 Index = 1; Index < NumElements; ++Index)
        {
            if (SortKeys[Index-1] > SortKeys[Index])
            {
                {
                    f32 Temp = SortKeys[Index-1];
                    SortKeys[Index-1] = SortKeys[Index];
                    SortKeys[Index] = Temp;
                }

                {
                    u32 Temp = SortVals[Index-1];
                    SortVals[Index-1] = SortVals[Index];
                    SortVals[Index] = Temp;
                }
                
                Swapped = true;
            }
        }

        if (!Swapped)
        {
            break;
        }
    }
}

__forceinline void ApproximateNodeSize(f32* RadiusTable, v3 Center, v3* Deltas, f32 NearPlane,
                                       u32 Level, f32* OutMinX, f32* OutMaxX, f32* OutMinY,
                                       f32* OutMaxY, f32* OutMinZ, f32* OutMaxZ)
{
    f32 MinX = ScreenX;
    f32 MaxX = 0;
    f32 MinY = ScreenY;
    f32 MaxY = 0;
    f32 MinZ = F32_MAX;
    f32 MaxZ = F32_MIN;
    for (u32 CornerId = 0; CornerId < 8; ++CornerId)
    {
        v3 NodeCorner = Center + RadiusTable[Level-1]*Deltas[CornerId];
        
        MinZ = Min(MinZ, NodeCorner.z);
        MaxZ = Max(MaxZ, NodeCorner.z);

        // NOTE: Clip the corner to the near plane
        NodeCorner.z = Max(NearPlane, NodeCorner.z);
        
        v2 ProjectedCorner = ProjectPoint(NodeCorner);

        MinX = Min(MinX, ProjectedCorner.x);
        MaxX = Max(MaxX, ProjectedCorner.x);
        
        MinY = Min(MinY, ProjectedCorner.y);
        MaxY = Max(MaxY, ProjectedCorner.y);
    }

    *OutMinX = MinX;
    *OutMaxX = MaxX;
    *OutMinY = MinY;
    *OutMaxY = MaxY;
    *OutMinZ = MinZ;
    *OutMaxZ = MaxZ;
}

struct node_minmax_result
{
    __m256 FMinX;
    __m256 FMaxX;
    __m256 FMinY;
    __m256 FMaxY;
    
    __m256 FrontZ;
    __m256 BackZ;
    __m256 NearPlane;

    __m256 BetweenX;
    __m256 BetweenY;
    
    __m256 ChildCenterX;
    __m256 ChildCenterY;
    __m256 ChildCenterZ;
};

// NOTE: Gets the 8 childrens node sizes (min/max) using 4 points
__forceinline node_minmax_result GetNodeMinMax4Pts(f32* RadiusTable, __m256 DeltaX, __m256 DeltaY,
                                                   __m256 DeltaZ, f32 NearPlane, v3 Center, u32 Level)
{
    node_minmax_result Result;
    
    __m256 CenterRadius = _mm256_set1_ps(RadiusTable[Level-1]);
    Result.ChildCenterX = _mm256_add_ps(_mm256_set1_ps(Center.x), _mm256_mul_ps(CenterRadius, DeltaX));
    Result.ChildCenterY = _mm256_add_ps(_mm256_set1_ps(Center.y), _mm256_mul_ps(CenterRadius, DeltaY));
    Result.ChildCenterZ = _mm256_add_ps(_mm256_set1_ps(Center.z), _mm256_mul_ps(CenterRadius, DeltaZ));
    __m256 P0X = Result.ChildCenterX;
    __m256 P0Y = Result.ChildCenterY;
    __m256 P1X = Result.ChildCenterX;
    __m256 P1Y = Result.ChildCenterY;

    __m256 RadiusVec = _mm256_set1_ps(RadiusTable[Level]);

    {
        __m256 CmpXMask = _mm256_cmp_ps(Result.ChildCenterX, _mm256_setzero_ps(), _CMP_LT_OS);
        P0X = _mm256_sub_ps(P0X, _mm256_and_ps(CmpXMask, RadiusVec));
        P1X = _mm256_add_ps(P1X, _mm256_and_ps(CmpXMask, RadiusVec));
        P0X = _mm256_add_ps(P0X, _mm256_andnot_ps(CmpXMask, RadiusVec));
        P1X = _mm256_sub_ps(P1X, _mm256_andnot_ps(CmpXMask, RadiusVec));
    }
    
    {
        __m256 CmpYMask = _mm256_cmp_ps(Result.ChildCenterY, _mm256_setzero_ps(), _CMP_LT_OS);
        P0Y = _mm256_sub_ps(P0Y, _mm256_and_ps(CmpYMask, RadiusVec));
        P1Y = _mm256_add_ps(P1Y, _mm256_and_ps(CmpYMask, RadiusVec));
        P0Y = _mm256_add_ps(P0Y, _mm256_andnot_ps(CmpYMask, RadiusVec));
        P1Y = _mm256_sub_ps(P1Y, _mm256_andnot_ps(CmpYMask, RadiusVec));
    }
    
    Result.NearPlane = _mm256_set1_ps(NearPlane);
    Result.FrontZ = _mm256_max_ps(Result.NearPlane, _mm256_sub_ps(Result.ChildCenterZ, RadiusVec));
    Result.BackZ = _mm256_max_ps(Result.NearPlane, _mm256_add_ps(Result.ChildCenterZ, RadiusVec));

    __m256 P0FrontX = _mm256_div_ps(P0X, Result.FrontZ);
    __m256 P0FrontY = _mm256_div_ps(P0Y, Result.FrontZ);

    __m256 P1FrontX = _mm256_div_ps(P1X, Result.FrontZ);
    __m256 P1FrontY = _mm256_div_ps(P1Y, Result.FrontZ);

    __m256 P0BackX = _mm256_div_ps(P0X, Result.BackZ);
    __m256 P0BackY = _mm256_div_ps(P0Y, Result.BackZ);

    __m256 P1BackX = _mm256_div_ps(P1X, Result.BackZ);
    __m256 P1BackY = _mm256_div_ps(P1Y, Result.BackZ);

    __m256 OneVec = _mm256_set1_ps(1.0f);
    __m256 HalfScreenX = _mm256_set1_ps(0.5f*ScreenX);
    __m256 HalfScreenY = _mm256_set1_ps(0.5f*ScreenY);

    __m256 CmpXMask = _mm256_cmp_ps(P0X, P1X, _CMP_LT_OS);
    Result.FMinX = _mm256_and_ps(CmpXMask, _mm256_mul_ps(HalfScreenX, _mm256_add_ps(OneVec, _mm256_min_ps(P0FrontX, P0BackX))));
    Result.FMaxX = _mm256_and_ps(CmpXMask, _mm256_mul_ps(HalfScreenX, _mm256_add_ps(OneVec, _mm256_max_ps(P1FrontX, P1BackX))));

    Result.FMinX = _mm256_or_ps(Result.FMinX, _mm256_andnot_ps(CmpXMask, _mm256_mul_ps(HalfScreenX, _mm256_add_ps(OneVec, _mm256_min_ps(P1FrontX, P1BackX)))));
    Result.FMaxX = _mm256_or_ps(Result.FMaxX, _mm256_andnot_ps(CmpXMask, _mm256_mul_ps(HalfScreenX, _mm256_add_ps(OneVec, _mm256_max_ps(P0FrontX, P0BackX)))));

    __m256 CmpYMask = _mm256_cmp_ps(P0Y, P1Y, _CMP_LT_OS);
    Result.FMinY = _mm256_and_ps(CmpYMask, _mm256_mul_ps(HalfScreenY, _mm256_add_ps(OneVec, _mm256_min_ps(P0FrontY, P0BackY))));
    Result.FMaxY = _mm256_and_ps(CmpYMask, _mm256_mul_ps(HalfScreenY, _mm256_add_ps(OneVec, _mm256_max_ps(P1FrontY, P1BackY))));

    Result.FMinY = _mm256_or_ps(Result.FMinY, _mm256_andnot_ps(CmpYMask, _mm256_mul_ps(HalfScreenY, _mm256_add_ps(OneVec, _mm256_min_ps(P1FrontY, P1BackY)))));
    Result.FMaxY = _mm256_or_ps(Result.FMaxY, _mm256_andnot_ps(CmpYMask, _mm256_mul_ps(HalfScreenY, _mm256_add_ps(OneVec, _mm256_max_ps(P0FrontY, P0BackY)))));

    return Result;
}

// TODO: We can use not operations to reduce the amount of cmp instructions, which should
// reduce latency a lot, we would just have to figure out if the not will convert a >= to <= (equals!)

__forceinline node_minmax_result GetNodeMinMaxAxis(f32* RadiusTable, __m256 DeltaX, __m256 DeltaY,
                                                   __m256 DeltaZ, f32 RadiusX, f32 RadiusY, f32 RadiusFrontZ,
                                                   f32 RadiusBackZ, f32 NearPlane, v3 Center, u32 Level)
{
    node_minmax_result Result;
    
    __m256 CenterRadius = _mm256_set1_ps(RadiusTable[Level]);
    Result.ChildCenterX = _mm256_add_ps(_mm256_set1_ps(Center.x), _mm256_mul_ps(CenterRadius, DeltaX));
    Result.ChildCenterY = _mm256_add_ps(_mm256_set1_ps(Center.y), _mm256_mul_ps(CenterRadius, DeltaY));
    Result.ChildCenterZ = _mm256_add_ps(_mm256_set1_ps(Center.z), _mm256_mul_ps(CenterRadius, DeltaZ));

    __m256 CmpXMask = _mm256_cmp_ps(Result.ChildCenterX, _mm256_setzero_ps(), _CMP_LT_OS);
    __m256 CmpYMask = _mm256_cmp_ps(Result.ChildCenterY, _mm256_setzero_ps(), _CMP_LT_OS);
    __m256 P0X = Result.ChildCenterX;
    __m256 P0Y = Result.ChildCenterY;
    __m256 P1X = Result.ChildCenterX;
    __m256 P1Y = Result.ChildCenterY;
    __m256 RadiusXVec = _mm256_mul_ps(CenterRadius, _mm256_set1_ps(RadiusX));
    __m256 RadiusYVec = _mm256_mul_ps(CenterRadius, _mm256_set1_ps(RadiusY));

    P0X = _mm256_sub_ps(P0X, _mm256_and_ps(CmpXMask, RadiusXVec));
    P0X = _mm256_add_ps(P0X, _mm256_andnot_ps(CmpXMask, RadiusXVec));
    P1X = _mm256_add_ps(P1X, _mm256_and_ps(CmpXMask, RadiusXVec));
    P1X = _mm256_sub_ps(P1X, _mm256_andnot_ps(CmpXMask, RadiusXVec));
    
    P0Y = _mm256_sub_ps(P0Y, _mm256_and_ps(CmpYMask, RadiusYVec));
    P0Y = _mm256_add_ps(P0Y, _mm256_andnot_ps(CmpYMask, RadiusYVec));
    P1Y = _mm256_add_ps(P1Y, _mm256_and_ps(CmpYMask, RadiusYVec));
    P1Y = _mm256_sub_ps(P1Y, _mm256_andnot_ps(CmpYMask, RadiusYVec));

    Result.NearPlane = _mm256_set1_ps(NearPlane);
    Result.FrontZ = _mm256_max_ps(Result.NearPlane, _mm256_sub_ps(Result.ChildCenterZ, _mm256_mul_ps(CenterRadius, _mm256_set1_ps(RadiusFrontZ))));
    Result.BackZ = _mm256_max_ps(Result.NearPlane, _mm256_add_ps(Result.ChildCenterZ, _mm256_mul_ps(CenterRadius, _mm256_set1_ps(RadiusBackZ))));

    P0X = _mm256_div_ps(P0X, Result.FrontZ);
    P0Y = _mm256_div_ps(P0Y, Result.FrontZ);

    __m256 P1XDiv;
    Result.BetweenX = _mm256_and_ps(_mm256_cmp_ps(_mm256_setzero_ps(), P0X, _CMP_GE_OS),
                                    _mm256_cmp_ps(_mm256_setzero_ps(), P1X, _CMP_LE_OS));
    Result.BetweenX = _mm256_or_ps(Result.BetweenX, _mm256_and_ps(_mm256_cmp_ps(_mm256_setzero_ps(), P1X, _CMP_GE_OS),
                                                    _mm256_cmp_ps(_mm256_setzero_ps(), P0X, _CMP_LE_OS)));
    P1XDiv = _mm256_or_ps(_mm256_and_ps(Result.BetweenX, Result.FrontZ),
                          _mm256_andnot_ps(Result.BetweenX, Result.BackZ));
    P1X = _mm256_div_ps(P1X, P1XDiv);

    __m256 P1YDiv;
    Result.BetweenY = _mm256_and_ps(_mm256_cmp_ps(_mm256_setzero_ps(), P0Y, _CMP_GE_OS),
                                    _mm256_cmp_ps(_mm256_setzero_ps(), P1Y, _CMP_LE_OS));
    Result.BetweenY = _mm256_or_ps(Result.BetweenY, _mm256_and_ps(_mm256_cmp_ps(_mm256_setzero_ps(), P1Y, _CMP_GE_OS),
                                                    _mm256_cmp_ps(_mm256_setzero_ps(), P0Y, _CMP_LE_OS)));
    P1YDiv = _mm256_or_ps(_mm256_and_ps(Result.BetweenY, Result.FrontZ),
                          _mm256_andnot_ps(Result.BetweenY, Result.BackZ));
    P1Y = _mm256_div_ps(P1Y, P1YDiv);

    __m256 OneVec = _mm256_set1_ps(1.0f);
    __m256 HalfScreenX = _mm256_set1_ps(0.5f*ScreenX);
    __m256 HalfScreenY = _mm256_set1_ps(0.5f*ScreenY);

    Result.FMinX = _mm256_mul_ps(HalfScreenX, _mm256_add_ps(OneVec, _mm256_min_ps(P0X, P1X)));
    Result.FMaxX = _mm256_mul_ps(HalfScreenX, _mm256_add_ps(OneVec, _mm256_max_ps(P0X, P1X)));

    Result.FMinY = _mm256_mul_ps(HalfScreenY, _mm256_add_ps(OneVec, _mm256_min_ps(P0Y, P1Y)));
    Result.FMaxY = _mm256_mul_ps(HalfScreenY, _mm256_add_ps(OneVec, _mm256_max_ps(P0Y, P1Y)));

    return Result;
}

//
// NOTE: Occlusion/render checks with depth
//

__forceinline b32 ScalarIsNodeOccluded(f32* DepthMap, i32 MinX, i32 MaxX, i32 MinY, i32 MaxY, f32 NodeZ)
{
    b32 Result = true;
    f32* DepthRow = DepthMap + MinY*ScreenX + MinX;
    for (i32 Y = MinY; Y < MaxY; ++Y)
    {
        f32* Depth = DepthRow;
        for (i32 X = MinX; X < MaxX; ++X)
        {
            if (*Depth > NodeZ)
            {
                // NOTE: We aren't occluded
                Result = false;
                return Result;
            }

            ++Depth;
        }

        DepthRow += ScreenX;
    }

    return Result;
}

__forceinline void ScalarDrawNode(f32* DepthMap, i32 MinX, i32 MaxX, i32 MinY, i32 MaxY, f32 MinZ,
                                  u32 Level)
{
    // NOTE: Render splat
    f32* DepthRow = DepthMap + MinY*ScreenX + MinX;
    for (i32 Y = MinY; Y < MaxY; ++Y)
    {
        f32* Depth = DepthRow;
        for (i32 X = MinX; X < MaxX; ++X)
        {
            if (*Depth > MinZ)
            {
                *Depth = MinZ;
            }

            ++Depth;
        }

        DepthRow += ScreenX;
    }
}

__forceinline void ScalarDrawNode(f32* DepthMap, u32* LevelMap, i32 MinX, i32 MaxX, i32 MinY, i32 MaxY,
                                  f32 MinZ, u32 Level)
{
    // NOTE: Render splat
    f32* DepthRow = DepthMap + MinY*ScreenX + MinX;
    u32* LevelRow = LevelMap + MinY*ScreenX + MinX;
    for (i32 Y = MinY; Y < MaxY; ++Y)
    {
        f32* Depth = DepthRow;
        u32* LevelVal = LevelRow;
        for (i32 X = MinX; X < MaxX; ++X)
        {
            if (*Depth > MinZ)
            {
                *Depth = MinZ;
                *LevelVal = Level;
            }

            ++Depth;
            ++LevelVal;
        }

        DepthRow += ScreenX;
        LevelRow += ScreenX; 
    }
}

__forceinline void ScalarRenderPointDepth(f32* DepthMap, u32 Level, i32 MinX, i32 MinY, f32 MinZ)
{
    u32 PixelId = MinY*ScreenX + MinX;
    f32* DepthVal = DepthMap + PixelId;

    if (*DepthVal > MinZ)
    {
        *DepthVal = MinZ;
    }
}

__forceinline void ScalarRenderPointDepth(f32* DepthMap, u32* LevelMap, f32* OverdrawMap, u32 Level,
                                          i32 MinX, i32 MinY, f32 MinZ)
{
    u32 PixelId = MinY*ScreenX + MinX;
    f32* DepthVal = DepthMap + PixelId;
    OverdrawMap[PixelId] += 1;

    if (*DepthVal > MinZ)
    {
        *DepthVal = MinZ;
        LevelMap[PixelId] = Level;
    }
}

//
// NOTE: Occlusion/render checks with coverage
//

#define COVERAGE_FULL 0xFF

__forceinline b32 ScalarIsNodeOccludedCoverage(u8* CoverageMap, i32 MinX, i32 MaxX, i32 MinY, i32 MaxY)
{
    b32 Result = true;
    u8* CoverageRow = CoverageMap + MinY*ScreenX + MinX;
    for (i32 Y = MinY; Y < MaxY; ++Y)
    {
        u8* Coverage = CoverageRow;
        for (i32 X = MinX; X < MaxX; ++X)
        {
            if (*Coverage == 0)
            {
                Result = false;
                return Result;
            }
            
            ++Coverage;
        }
        
        CoverageRow += ScreenX;
    }

    return Result;
}

__forceinline b32 ScalarIsNodeOccludedMaskCoverage(u8* CoverageMap, i32 MinX, i32 MaxX, i32 MinY,
                                                   i32 MaxY)
{
    b32 Result = true;
    
    u8* CoverageRow = CoverageMap + MinY*ScreenX + MinX;
    u32 NumFull = (MaxX - MinX) / 8;
    u32 InTile = (MaxX - MinX) % 8;
    u64 EndMask = ~(0xFFFFFFFFFFFFFFFF << (8*(u64)InTile));
    
    for (i32 Y = MinY; Y < MaxY; ++Y)
    {
        u64* Coverage = (u64*)CoverageRow;
        for (u32 ChunkX = 0; ChunkX < NumFull; ++ChunkX)
        {
#if GET_STATS
            GlobalRenderState->NumPixels += 1;
#endif
            if (*Coverage != 0xFFFFFFFFFFFFFFFF)
            {
                Result = false;
                return Result;
            }

            ++Coverage;
        }

#if GET_STATS
        GlobalRenderState->NumPixels += 1;
#endif
        if ((*Coverage & EndMask) != (0xFFFFFFFFFFFFFFFF & EndMask))
        {
            Result = false;
            return Result;
        }

        CoverageRow += ScreenX;
    }

    return Result;
}

__forceinline void RenderPointCoverageScalar(f32* DepthMap, u8* CoverageMap, u32 Level, i32 MinX,
                                             i32 MinY, f32 MinZ)
{
    u32 PixelId = MinY*ScreenX + MinX;
    u8* Coverage = CoverageMap+ PixelId;

    if (*Coverage == 0)
    {
        *Coverage = COVERAGE_FULL;
        DepthMap[PixelId] = MinZ;
    }
}

__forceinline void RenderPointCoverageScalar(f32* DepthMap, u8* CoverageMap, u32* LevelMap,
                                             f32* OverdrawMap, u32 Level, i32 MinX, i32 MinY, f32 MinZ)
{
    u32 PixelId = MinY*ScreenX + MinX;
    u8* Coverage = CoverageMap + PixelId;
    OverdrawMap[PixelId] += 1;

    if (*Coverage == 0)
    {
        *Coverage = COVERAGE_FULL;
        LevelMap[PixelId] = Level;
        DepthMap[PixelId] = MinZ;
    }
}

#include "fluffy_scalar.cpp"
#include "fluffy_epic.cpp"
#include "fluffy_subdiv.cpp"
#include "fluffy_epic_simd.cpp"
#include "fluffy_cov_1bit.cpp"
#include <memory>

inline void RenderAndDisplay(render_state* RenderState, m4 CameraMat)
{
#if GET_STATS
    RenderState->NumRejected = 0;
    RenderState->NumTraversals = 0;
    RenderState->LowestLeafLevel = 0;
    RenderState->NumRendered = 0;
    RenderState->NumPixels = 0;
#endif

    // NOTE: Clear render buffers
    {
        
        for (u32 PixelId = 0; PixelId < ScreenX*ScreenY; ++PixelId)
        {
            if (RenderState->DepthMap)
            {
                RenderState->DepthMap[PixelId] = RenderState->FarPlane;
            }

            if (RenderState->CoverageMap)
            {
                RenderState->CoverageMap[PixelId] = 0;
            }

            if (RenderState->OverdrawMap)
            {
                RenderState->OverdrawMap[PixelId] = 0;
            }

            if (RenderState->LevelMap)
            {
                RenderState->LevelMap[PixelId] = 0;
            }
        }

        if (RenderState->CoverageMap1Bit)
        {
            for (u32 PixelId = 0; PixelId < (ScreenX*ScreenY/8); ++PixelId)
            {
                RenderState->CoverageMap1Bit[PixelId] = 0;
            }
        }

#if OCTREE_RENDER_SCALAR
    
        GlobalScalarState = &RenderState->ScalarState;
        GlobalScalarState->NearPlane = RenderState->NearPlane;
    
        // NOTE: Copy the radius table
        memcpy(GlobalScalarState->RadiusTable, RenderState->RadiusTable, sizeof(f32)*NUM_RADIUS_ENTRIES);
    
#elif OCTREE_RENDER_EPIC

        GlobalEpicState = &RenderState->EpicState;
        GlobalEpicState->NearPlane = RenderState->NearPlane;
    
        // NOTE: Copy the radius table
        memcpy(GlobalEpicState->RadiusTable, RenderState->RadiusTable, sizeof(f32)*NUM_RADIUS_ENTRIES);
        
#elif OCTREE_RENDER_SUBDIV

        GlobalSubdivState = &RenderState->SubdivState;
        GlobalSubdivState->NearPlane = RenderState->NearPlane;
    
        // NOTE: Copy the radius table
        memcpy(GlobalSubdivState->RadiusTable, RenderState->RadiusTable, sizeof(f32)*NUM_RADIUS_ENTRIES);

#elif OCTREE_RENDER_EPIC_SIMD

        GlobalEpicSimdState = &RenderState->EpicSimdState;
        GlobalEpicSimdState->NearPlane = RenderState->NearPlane;
    
        // NOTE: Copy the radius table
        memcpy(GlobalEpicSimdState->RadiusTable, RenderState->RadiusTable, sizeof(f32)*NUM_RADIUS_ENTRIES);

#elif OCTREE_RENDER_1BIT

        GlobalCov1BitState = &RenderState->Cov1BitState;
        GlobalCov1BitState->NearPlane = RenderState->NearPlane;
    
        // NOTE: Copy the radius table
        memcpy(GlobalCov1BitState->RadiusTable, RenderState->RadiusTable, sizeof(f32)*NUM_RADIUS_ENTRIES);

#endif
    } 
    
    u64 NumEntities = ((u64)((u8*)RenderState->Arena.Mem + RenderState->Arena.Used) -
                       (u64)(RenderState->CommandByte)) / sizeof(render_cmd);
    render_cmd* Command = (render_cmd*)RenderState->CommandByte;

    // NOTE: Sort entities by distance to camera
    u32* EntityOrder = PushArray(&RenderState->Arena, u32, NumEntities);
    f32* Keys = PushArray(&RenderState->Arena, f32, NumEntities);
    for (u32 EntityId = 0; EntityId < NumEntities; ++EntityId)
    {
        EntityOrder[EntityId] = EntityId;
        Keys[EntityId] = Length(Command[EntityId].ModelMat.v[3].xyz);
    }
    BubbleSort(Keys, EntityOrder, (u32)NumEntities);

    // NOTE: Render entities front to back
    for (u32 EntityId = 0; EntityId < NumEntities; ++EntityId)
    {
        render_cmd* CurrCmd = Command + EntityOrder[EntityId];
        
#if OCTREE_RENDER_SCALAR

        Assert(CurrCmd->Type == CommandType_Octree);
        
        // NOTE: Setup model deltas
        m4 ModelMat = CameraMat*CurrCmd->ModelMat;
        GlobalScalarState->Deltas[0] = (V4(0.5, 0.5, 0.5, 0)*ModelMat).xyz;
        GlobalScalarState->Deltas[1] = (V4(-0.5, 0.5, 0.5, 0)*ModelMat).xyz;
        GlobalScalarState->Deltas[2] = (V4(-0.5, -0.5, 0.5, 0)*ModelMat).xyz;
        GlobalScalarState->Deltas[3] = (V4(0.5, -0.5, 0.5, 0)*ModelMat).xyz;
        GlobalScalarState->Deltas[4] = (V4(0.5, 0.5, -0.5, 0)*ModelMat).xyz;
        GlobalScalarState->Deltas[5] = (V4(-0.5, 0.5, -0.5, 0)*ModelMat).xyz;
        GlobalScalarState->Deltas[6] = (V4(-0.5, -0.5, -0.5, 0)*ModelMat).xyz;
        GlobalScalarState->Deltas[7] = (V4(0.5, -0.5, -0.5, 0)*ModelMat).xyz;
        
        ScalarRenderOctree(CurrCmd->ModelMat.v[3].xyz, CurrCmd->Octree, 1);

#elif OCTREE_RENDER_EPIC

        Assert(CurrCmd->Type == CommandType_Octree);
        
        // NOTE: Setup model deltas
        m4 ModelMat = CameraMat*CurrCmd->ModelMat;
        GlobalEpicState->Deltas[0] = (V4(0.5, 0.5, 0.5, 0)*ModelMat).xyz;
        GlobalEpicState->Deltas[1] = (V4(-0.5, 0.5, 0.5, 0)*ModelMat).xyz;
        GlobalEpicState->Deltas[2] = (V4(-0.5, -0.5, 0.5, 0)*ModelMat).xyz;
        GlobalEpicState->Deltas[3] = (V4(0.5, -0.5, 0.5, 0)*ModelMat).xyz;
        GlobalEpicState->Deltas[4] = (V4(0.5, 0.5, -0.5, 0)*ModelMat).xyz;
        GlobalEpicState->Deltas[5] = (V4(-0.5, 0.5, -0.5, 0)*ModelMat).xyz;
        GlobalEpicState->Deltas[6] = (V4(-0.5, -0.5, -0.5, 0)*ModelMat).xyz;
        GlobalEpicState->Deltas[7] = (V4(0.5, -0.5, -0.5, 0)*ModelMat).xyz;
        
        EpicRenderOctree(CurrCmd->ModelMat.v[3].xyz, CurrCmd->Octree, 1);
        
#elif OCTREE_RENDER_SUBDIV

        Assert(CurrCmd->Type == CommandType_Octree);

        // NOTE: Setup model deltas
        m4 ModelMat = CameraMat*CurrCmd->ModelMat;
        GlobalSubdivState->Deltas[0] = (V4(0.5, 0.5, 0.5, 0)*ModelMat).xyz;
        GlobalSubdivState->Deltas[1] = (V4(-0.5, 0.5, 0.5, 0)*ModelMat).xyz;
        GlobalSubdivState->Deltas[2] = (V4(-0.5, -0.5, 0.5, 0)*ModelMat).xyz;
        GlobalSubdivState->Deltas[3] = (V4(0.5, -0.5, 0.5, 0)*ModelMat).xyz;
        GlobalSubdivState->Deltas[4] = (V4(0.5, 0.5, -0.5, 0)*ModelMat).xyz;
        GlobalSubdivState->Deltas[5] = (V4(-0.5, 0.5, -0.5, 0)*ModelMat).xyz;
        GlobalSubdivState->Deltas[6] = (V4(-0.5, -0.5, -0.5, 0)*ModelMat).xyz;
        GlobalSubdivState->Deltas[7] = (V4(0.5, -0.5, -0.5, 0)*ModelMat).xyz;
        
        SubdivRenderOctree(CurrCmd->ModelMat.v[3].xyz, CurrCmd->Octree, 1);

#elif OCTREE_RENDER_EPIC_SIMD

        Assert(CurrCmd->Type == CommandType_OctreeId);

        // NOTE: Setup model deltas
        m4 ModelMat = CameraMat*CurrCmd->ModelMat;
        
        v3 Deltas[8] =
        {
            (V4(0.5, 0.5, 0.5, 0)*ModelMat).xyz,
            (V4(-0.5, 0.5, 0.5, 0)*ModelMat).xyz,
            (V4(-0.5, -0.5, 0.5, 0)*ModelMat).xyz,
            (V4(0.5, -0.5, 0.5, 0)*ModelMat).xyz,
            (V4(0.5, 0.5, -0.5, 0)*ModelMat).xyz,
            (V4(-0.5, 0.5, -0.5, 0)*ModelMat).xyz,
            (V4(-0.5, -0.5, -0.5, 0)*ModelMat).xyz,
            (V4(0.5, -0.5, -0.5, 0)*ModelMat).xyz,
        };

        GlobalEpicSimdState->RadiusX = Max(Max(Max(Abs(Deltas[0].x), Abs(Deltas[1].x)),
                                               Max(Abs(Deltas[2].x), Abs(Deltas[3].x))),
                                           Max(Max(Abs(Deltas[4].x), Abs(Deltas[5].x)),
                                               Max(Abs(Deltas[6].x), Abs(Deltas[7].x))));

        GlobalEpicSimdState->RadiusY = Max(Max(Max(Abs(Deltas[0].y), Abs(Deltas[1].y)),
                                               Max(Abs(Deltas[2].y), Abs(Deltas[3].y))),
                                           Max(Max(Abs(Deltas[4].y), Abs(Deltas[5].y)),
                                               Max(Abs(Deltas[6].y), Abs(Deltas[7].y))));

        GlobalEpicSimdState->RadiusFrontZ = Abs(Min(Min(Min(Deltas[0].z, Deltas[1].z),
                                                        Min(Deltas[2].z, Deltas[3].z)),
                                                    Min(Min(Deltas[4].z, Deltas[5].z),
                                                        Min(Deltas[6].z, Deltas[7].z))));

        GlobalEpicSimdState->RadiusBackZ = Abs(Max(Max(Max(Deltas[0].z, Deltas[1].z),
                                                       Max(Deltas[2].z, Deltas[3].z)),
                                                   Max(Max(Deltas[4].z, Deltas[5].z),
                                                       Max(Deltas[6].z, Deltas[7].z))));
                
        GlobalEpicSimdState->DeltaX = _mm256_setr_ps(Deltas[0].x, Deltas[1].x, Deltas[2].x, Deltas[3].x, Deltas[4].x, Deltas[5].x, Deltas[6].x, Deltas[7].x);
        GlobalEpicSimdState->DeltaY = _mm256_setr_ps(Deltas[0].y, Deltas[1].y, Deltas[2].y, Deltas[3].y, Deltas[4].y, Deltas[5].y, Deltas[6].y, Deltas[7].y);
        GlobalEpicSimdState->DeltaZ = _mm256_setr_ps(Deltas[0].z, Deltas[1].z, Deltas[2].z, Deltas[3].z, Deltas[4].z, Deltas[5].z, Deltas[6].z, Deltas[7].z);
        
        EpicSimdRenderOctree(CurrCmd->ModelMat.v[3].xyz, CurrCmd->OctreeId, 1);

#elif OCTREE_RENDER_1BIT

        Assert(CurrCmd->Type == CommandType_OctreeId);

        // NOTE: Setup model deltas
        m4 ModelMat = CameraMat*CurrCmd->ModelMat;
        
        v3 Deltas[8] =
        {
            (V4(0.5, 0.5, 0.5, 0)*ModelMat).xyz,
            (V4(-0.5, 0.5, 0.5, 0)*ModelMat).xyz,
            (V4(-0.5, -0.5, 0.5, 0)*ModelMat).xyz,
            (V4(0.5, -0.5, 0.5, 0)*ModelMat).xyz,
            (V4(0.5, 0.5, -0.5, 0)*ModelMat).xyz,
            (V4(-0.5, 0.5, -0.5, 0)*ModelMat).xyz,
            (V4(-0.5, -0.5, -0.5, 0)*ModelMat).xyz,
            (V4(0.5, -0.5, -0.5, 0)*ModelMat).xyz,
        };

        GlobalCov1BitState->RadiusX = Max(Max(Max(Abs(Deltas[0].x), Abs(Deltas[1].x)),
                                              Max(Abs(Deltas[2].x), Abs(Deltas[3].x))),
                                          Max(Max(Abs(Deltas[4].x), Abs(Deltas[5].x)),
                                              Max(Abs(Deltas[6].x), Abs(Deltas[7].x))));

        GlobalCov1BitState->RadiusY = Max(Max(Max(Abs(Deltas[0].y), Abs(Deltas[1].y)),
                                              Max(Abs(Deltas[2].y), Abs(Deltas[3].y))),
                                          Max(Max(Abs(Deltas[4].y), Abs(Deltas[5].y)),
                                              Max(Abs(Deltas[6].y), Abs(Deltas[7].y))));

        GlobalCov1BitState->RadiusFrontZ = Abs(Min(Min(Min(Deltas[0].z, Deltas[1].z),
                                                       Min(Deltas[2].z, Deltas[3].z)),
                                                   Min(Min(Deltas[4].z, Deltas[5].z),
                                                       Min(Deltas[6].z, Deltas[7].z))));

        GlobalCov1BitState->RadiusBackZ = Abs(Max(Max(Max(Deltas[0].z, Deltas[1].z),
                                                      Max(Deltas[2].z, Deltas[3].z)),
                                                  Max(Max(Deltas[4].z, Deltas[5].z),
                                                      Max(Deltas[6].z, Deltas[7].z))));
                
        GlobalCov1BitState->DeltaX = _mm256_setr_ps(Deltas[0].x, Deltas[1].x, Deltas[2].x, Deltas[3].x, Deltas[4].x, Deltas[5].x, Deltas[6].x, Deltas[7].x);
        GlobalCov1BitState->DeltaY = _mm256_setr_ps(Deltas[0].y, Deltas[1].y, Deltas[2].y, Deltas[3].y, Deltas[4].y, Deltas[5].y, Deltas[6].y, Deltas[7].y);
        GlobalCov1BitState->DeltaZ = _mm256_setr_ps(Deltas[0].z, Deltas[1].z, Deltas[2].z, Deltas[3].z, Deltas[4].z, Deltas[5].z, Deltas[6].z, Deltas[7].z);
        
        Cov1BitRenderOctree(CurrCmd->ModelMat.v[3].xyz, CurrCmd->OctreeId, 1);

#endif
    }
    
    //
    // NOTE: Render to the screen
    //
    
#if DRAW_DEPTH_MAP
    
    f32 DepthRange = RenderState->FarPlane;
    for (u32 PixelId = 0; PixelId < ScreenX*ScreenY; ++PixelId)
    {
        f32 Depth = (RenderState->DepthMap[PixelId] / DepthRange);
        Assert(Depth <= 1.0f);

        u32 PixelVal = (((u32)(255.0f + 0.5f) << 24) |
                        ((u32)(255.0f*Depth + 0.5f) << 16) |
                        ((u32)(255.0f*Depth + 0.5f) << 8) |
                        ((u32)(255.0f*Depth + 0.5f) << 0));
        
        RenderState->DepthMap[PixelId] = *(f32*)&PixelVal;
    }
    
    PlatformApi.PushTextureToScreen(ScreenX, ScreenY, RenderState->DepthMap);

#elif DRAW_OVERDRAW_MAP

    f32 AvgVal = 0.0f;
    f32 MaxVal = 0.0f;
    f32* Pixel = RenderState->OverdrawMap;
    for (u32 PixelId = 0; PixelId < ScreenX*ScreenY; ++PixelId)
    {
        f32 Val = (f32)(*Pixel);

        AvgVal += Val;
        MaxVal = Max(MaxVal, Val);

        Val *= 10.0f;
        Val = Min(255.0f, Val);

        u32 PixelVal = (((u32)(255.0f + 0.5f) << 24) |
                        ((u32)(Val + 0.5f) << 16) |
                        ((u32)(Val + 0.5f) << 8) |
                        ((u32)(Val + 0.5f) << 0));
        
        RenderState->OverdrawMap[PixelId] = *(f32*)&PixelVal;
        
        Pixel += 1;
    }

    AvgVal /= (f32)(ScreenX*ScreenY);
    
    PlatformApi.PushTextureToScreen(ScreenX, ScreenY, RenderState->OverdrawMap);
    
#elif DRAW_LEVEL_MAP

    u32* Pixel = RenderState->LevelMap;
    for (u32 PixelId = 0; PixelId < ScreenX*ScreenY; ++PixelId)
    {
        *Pixel = RenderState->LevelColorTable[*Pixel];
        Pixel += 1;
    }

    PlatformApi.PushTextureToScreen(ScreenX, ScreenY, RenderState->LevelMap);

#elif DRAW_COVERAGE_MAP

    u8* Coverage = RenderState->CoverageMap;
    u32* Pixel = RenderState->ColorMap;
    for (u32 PixelId = 0; PixelId < ScreenX*ScreenY; ++PixelId)
    {
        *Pixel = *Coverage == 0 ? 0xFF : 0;
        Pixel += 1;
        Coverage += 1;
    }
    
    PlatformApi.PushTextureToScreen(ScreenX, ScreenY, RenderState->ColorMap);

#elif DRAW_1BIT_COVERAGE_MAP

    u32 NumCoverage = ScreenX*ScreenY/64;
    u64* Coverage = RenderState->CoverageMap1Bit;
    u32* Pixel = RenderState->ColorMap;
    for (u32 PixelId = 0; PixelId < NumCoverage; ++PixelId)
    {
        for (u64 BitId = 0; BitId < 64; ++BitId)
        {
            u64 Mask = (u64)1 << BitId;
            *Pixel = (*Coverage & Mask) == 0 ? 0xFF : 0;
            if (*Pixel == 0)
            {
                int i = 0;
            }
            Pixel += 1;
        }

        Coverage += 1;
    }
    
    PlatformApi.PushTextureToScreen(ScreenX, ScreenY, RenderState->ColorMap);

#elif DRAW_1BIT_COVERAGE_MAP_8X8

    u32 NumCoverage = ScreenX*ScreenY/64;
    u64* Coverage = (u64*)RenderState->CoverageMap1Bit;

    u32 Y = 0;
    u32 X = 0;
    
    for (u32 PixelId = 0; PixelId < NumCoverage; ++PixelId)
    {
        for (u32 CurrY = 0; CurrY < 8; ++CurrY)
        {
            for (u32 CurrX = 0; CurrX < 8; ++CurrX)
            {
                u64 Mask = (u64)1 << (u64)(CurrY*8 + CurrX);
                u32 Value = (*Coverage & Mask) == 0 ? 0xFF : 0;
                RenderState->DepthMap[(Y + CurrY)*ScreenX + X + CurrX] = *(f32*)&Value;
                    
            }
        }

        X += 8;
        if (X >= ScreenX)
        {
            X -= ScreenX;
            Y += 8;
        }        
        
        Coverage += 1;
    }
    
    PlatformApi.PushTextureToScreen(ScreenX, ScreenY, RenderState->DepthMap);

#else

    v3 Deltas[8] =
    {
        (V4(0.5, 0.5, 0.5, 0)*ModelMat).xyz,
        (V4(-0.5, 0.5, 0.5, 0)*ModelMat).xyz,
        (V4(-0.5, -0.5, 0.5, 0)*ModelMat).xyz,
        (V4(0.5, -0.5, 0.5, 0)*ModelMat).xyz,
        (V4(0.5, 0.5, -0.5, 0)*ModelMat).xyz,
        (V4(-0.5, 0.5, -0.5, 0)*ModelMat).xyz,
        (V4(-0.5, -0.5, -0.5, 0)*ModelMat).xyz,
        (V4(0.5, -0.5, -0.5, 0)*ModelMat).xyz,
    };
    
    // NOTE: Render grid points
    for (u32 Y = 0; Y < 10; ++Y)
    {
        for (u32 X = 0; X < 10; ++X)
        {
            v2 GridCorner = 8*V2i(X, Y);
            i32 IntX = Max(0, Min(ScreenX, RoundToI32(GridCorner.x)));
            i32 IntY = Max(0, Min(ScreenY, RoundToI32(GridCorner.y)));
            RenderState->LevelMap[IntY*ScreenX + IntX] = 0x00FF000F;
        }
    }

    // NOTE: Render root
    v3 ParentCenter = V3(0.25, 0.25, 0.75);
    f32 Radius = 0.5f;
        
    //v3 ParentCenter = V3(0.125, 0.125, 0.625);
        
    //v3 ParentCenter = V3(0.0625, 0.1875, 0.5625);
    //f32 Radius = 0.25f*0.5f;

    RenderNodeCorners(RenderState, ParentCenter, Radius, 0xFF);

    // NOTE: Render child
    f32 ChildRadius = 0.5f*Radius;
    v3 ChildCenter0 = ParentCenter + ChildRadius*Deltas[0];
    v3 ChildCenter1 = ParentCenter + ChildRadius*Deltas[1];
    v3 ChildCenter2 = ParentCenter + ChildRadius*Deltas[2];
    v3 ChildCenter3 = ParentCenter + ChildRadius*Deltas[3];
    v3 ChildCenter4 = ParentCenter + ChildRadius*Deltas[4];
    v3 ChildCenter5 = ParentCenter + ChildRadius*Deltas[5];
    v3 ChildCenter6 = ParentCenter + ChildRadius*Deltas[6];
    v3 ChildCenter7 = ParentCenter + ChildRadius*Deltas[7];

    RenderNodeCorners(RenderState->LevelMap, Deltas, ChildCenter7, ChildRadius, 0xFFFFFF);
        
    PlatformApi.PushTextureToScreen(ScreenX, ScreenY, RenderState->LevelMap);

#endif

    // NOTE: Clear our commands
    EndTempMem(RenderState->TempMem);
    RenderState->TempMem = BeginTempMem(&RenderState->Arena);
}
