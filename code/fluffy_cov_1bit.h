#if !defined(FLUFFY_1BIT_H)

#define COVERAGE_1BIT_ENABLE_8X8 1
struct render_cov_1bit_state
{
    f32* DepthMap;
    u8* CoverageMap1Bit;
    
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

#if COVERAGE_1BIT_ENABLE_8X8
    u64 MinMaskX8x8[8];
#endif
    u8* NodeBase;
};

#define FLUFFY_1BIT_H
#endif
