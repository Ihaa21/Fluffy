#if !defined(FLUFFY_EPIC_H)

struct render_epic_state
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

#define FLUFFY_EPIC_H
#endif
