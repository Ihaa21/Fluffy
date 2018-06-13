#if !defined(FLUFFY_EPIC_SIMD_H)

#define EPIC_SIMD_RENDER_DEPTH 0
#define NODE_CHECK_COVERAGE_SCALAR_GROUP 1
struct render_epic_simd_state
{
    f32* DepthMap;
    u8* CoverageMap;
    
#if GET_STATS
    u32* ColorMap;
    f32* OverdrawMap;
    u32* LevelMap;    
#endif

    f32 RadiusTable[NUM_RADIUS_ENTRIES];
    __m256 DeltaX;
    __m256 DeltaY;
    __m256 DeltaZ;
    f32 RadiusX;
    f32 RadiusY;
    f32 RadiusFrontZ;
    f32 RadiusBackZ;
    f32 NearPlane;
    u8* NodeBase;
};

#define FLUFFY_EPIC_SIMD_H
#endif
