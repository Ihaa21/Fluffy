#if !defined(FLUFFY_SCALAR_H)

struct render_scalar_state
{
    f32* DepthMap;
    
#if GET_STATS
    u32* ColorMap;
    f32* OverdrawMap;
    u32* LevelMap;
#endif

    f32 RadiusTable[NUM_RADIUS_ENTRIES];
    v3 Deltas[8];
    f32 NearPlane;
};

#define FLUFFY_SCALAR_H
#endif
