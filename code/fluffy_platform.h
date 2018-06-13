#if !defined(FLUFFY_PLATFORM_H)

#include <stdint.h>
#include <stddef.h>
#include <float.h>
#include <stdio.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t mm;
typedef uintptr_t umm;

typedef int32_t b32;

#define internal static
#define global static
#define local_global static

#define snprintf _snprintf_s
#define Assert(Expression) if (!(Expression)) {*(int*)0 = 0;}
#define InvalidCodePath Assert(!"Invalid Code Path")
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define KiloBytes(Val) ((Val)*1024LL)
#define MegaBytes(Val) (KiloBytes(Val)*1024LL)
#define GigaBytes(Val) (MegaBytes(Val)*1024LL)
#define TeraBytes(Val) (GigaBytes(Val)*1024LL)

struct game_memory;
struct platform_api;
extern struct platform_api PlatformApi;

//
//
//

#include "fluffy_math.h"
#include "fluffy_math.cpp"
#include "fluffy_memory.h"
#include "fluffy_memory.cpp"
#include "fluffy_render.h"

typedef struct game_button
{
    u16 Down;
    u16 Pressed;
} game_button;

#define PUSH_TEXTURE_TO_SCREEN(name) void name(u32 Width, u32 Height, void* Pixels)
typedef PUSH_TEXTURE_TO_SCREEN(push_texture_to_screen);

#define GL_LOAD_TEXTURE_TO_GPU(name) u32 name(u32 Width, u32 Height, void* Pixels)
typedef GL_LOAD_TEXTURE_TO_GPU(gl_load_texture_to_gpu);

typedef struct platform_api
{
    push_texture_to_screen* PushTextureToScreen;
    gl_load_texture_to_gpu* GLLoadTextureToGpu;
} platform_api;

typedef struct game_memory
{
    mm PermanentMemSize;
    void* PermanentMem;

    platform_api PlatformApi;
} game_memory;

//
//
//

#define GAME_INIT(name) void name(game_memory* GameMem)
typedef GAME_INIT(game_init);

#define GAME_UPDATE_AND_RENDER(name) void name(game_memory* GameMem)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

//
//
//

#define FLUFFY_PLATFORM_H
#endif
