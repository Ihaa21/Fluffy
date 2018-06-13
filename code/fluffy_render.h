#if !defined(FLUFFY_RENDER_H)

#include <intrin.h>
#include <immintrin.h>

#define NUM_RADIUS_ENTRIES 25

#define ScreenX 512
#define ScreenY 512

#define GET_STATS 0

#define DRAW_OVERDRAW_MAP 0
#define DRAW_DEPTH_MAP 1
#define DRAW_LEVEL_MAP 0
#define DRAW_COVERAGE_MAP 0
#define DRAW_1BIT_COVERAGE_MAP 0
#define DRAW_1BIT_COVERAGE_MAP_8X8 0
#define MAX_LEVEL 25

#define OCTREE_RENDER_SCALAR 0
#define OCTREE_RENDER_EPIC 0
#define OCTREE_RENDER_SUBDIV 0
#define OCTREE_RENDER_EPIC_SIMD 0
#define OCTREE_RENDER_1BIT 1

#include "fluffy_scalar.h"
#include "fluffy_epic.h"
#include "fluffy_subdiv.h"
#include "fluffy_epic_simd.h"
#include "fluffy_cov_1bit.h"

global render_scalar_state* GlobalScalarState;
global render_epic_state* GlobalEpicState;
global render_subdiv_state* GlobalSubdivState;
global render_epic_simd_state* GlobalEpicSimdState;
global render_cov_1bit_state* GlobalCov1BitState;

struct render_state;
global render_state* GlobalRenderState;

enum CommandType
{
    CommandType_Octree,
    CommandType_OctreeId,
};

struct octree;
struct octree_id;
struct render_cmd
{
    u32 Type;
    m4 ModelMat;

    union
    {
        octree* Octree;
        octree_id* OctreeId;
    };
};

struct render_state
{
    mem_arena Arena;

    temp_mem TempMem;
    u8* CommandByte;
    
#if GET_STATS

    u32 NumRejected;
    u32 NumTraversals;
    u32 LowestLeafLevel;
    u32 NumRendered;
    u32 NumPixels;

    u32 LevelColorTable[NUM_RADIUS_ENTRIES];

#endif

    f32* DepthMap;
    f32* OverdrawMap;
    u32* LevelMap;
    u8* CoverageMap;
    u8* CoverageMap1Bit;

    u8* NodeBase;
    f32 RadiusTable[NUM_RADIUS_ENTRIES];
    m4 CameraMat;
    f32 NearPlane;
    f32 FarPlane;
    
    union
    {
        render_scalar_state ScalarState;
        render_epic_state EpicState;
        render_subdiv_state SubdivState;
        render_epic_simd_state EpicSimdState;
        render_cov_1bit_state Cov1BitState;
    };
};

#define FLUFFY_RENDER_H
#endif
