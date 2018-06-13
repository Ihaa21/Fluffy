#if !defined(FLUFFY_MEMORY_H)

enum AllocFlags
{
    AllocFlag_Align = 1 << 0,
};

struct mem_arena
{
    mm Size;
    mm Used;
    u8* Mem;
};

struct temp_mem
{
    mem_arena* Arena;
    mm Used;
};

struct mem_double_arena
{
    mm Size;
    mm UsedTop;
    mm UsedBot;
    u8* Mem;
};

struct temp_double_mem
{
    mem_double_arena* Arena;
    mm UsedTop;
    mm UsedBot;
};

#define FLUFFY_MEMORY_H
#endif
