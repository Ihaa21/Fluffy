
inline mem_arena InitArena(void* Mem, mm Size)
{
    mem_arena Result = {};
    Result.Size = Size;
    Result.Used = 0;
    Result.Mem = (u8*)Mem;

    return Result;
}

inline mem_double_arena InitDoubleArena(void* Mem, mm Size)
{
    mem_double_arena Result = {};
    Result.Size = Size;
    Result.UsedTop = 0;
    Result.UsedBot = 0;
    Result.Mem = (u8*)Mem;

    return Result;
}

inline void ClearArena(mem_arena* Arena)
{
    Arena->Used = 0;
}

inline void ClearArena(mem_double_arena* Arena)
{
    Arena->UsedTop = 0;
    Arena->UsedBot = 0;
}

inline temp_mem BeginTempMem(mem_arena* Arena)
{
    // NOTE: This function lets us take all memory allocated past this point and later
    // free it
    temp_mem TempMem = {};
    TempMem.Arena = Arena;
    TempMem.Used = Arena->Used;

    return TempMem;
}

inline void EndTempMem(temp_mem TempMem)
{
    TempMem.Arena->Used = TempMem.Used;
}

inline temp_double_mem BeginTempMem(mem_double_arena* Arena)
{
    // NOTE: This function lets us take all memory allocated past this point and later
    // free it
    temp_double_mem TempMem = {};
    TempMem.Arena = Arena;
    TempMem.UsedTop = Arena->UsedTop;
    TempMem.UsedBot = Arena->UsedBot;

    return TempMem;
}

inline void EndTempMem(temp_double_mem TempMem)
{
    TempMem.Arena->UsedTop = TempMem.UsedTop;
    TempMem.Arena->UsedBot = TempMem.UsedBot;
}

#define PushStruct(Arena, type) (type*)PushSize(Arena, sizeof(type))
#define PushStructAligned(Arena, type) (type*)PushSize(Arena, sizeof(type), AllocFlag_Align)
#define PushArray(Arena, type, count) (type*)PushSize(Arena, sizeof(type)*count)
#define PushArrayAligned(Arena, type, count) (type*)PushSize(Arena, sizeof(type)*count, AllocFlag_Align)
inline void* PushSize(mem_arena* Arena, mm Size, u32 Flags = 0)
{
    // TODO: Hard coded 4 byte alignment
#define ALIGNMENT_NUM 8
    mm Alignment = 0;
    if (Flags & AllocFlag_Align)
    {
        mm AlignmentMask = 4 - 1;
        mm ResultAddr = (mm)(Arena->Mem + Arena->Used);
        if (ResultAddr & AlignmentMask)
        {
            Alignment = 4 - (ResultAddr & AlignmentMask);
        }
        Size += Alignment;
    }
    
    Assert((Arena->Used + Size) <= Arena->Size);
    void* Result = Arena->Mem + Arena->Used + Alignment;
    
    Arena->Used += Size;

    // TODO: Do we just wanna zero everything out?
    
    return Result;
}

inline void* PushSize(mem_double_arena* Arena, mm Size)
{
    Assert(Arena->UsedTop + Arena->UsedBot + Size <= Arena->Size);
    void* Result = Arena->Mem + Arena->UsedTop;
    Arena->UsedTop += Size;

    // TODO: Do we just wanna zero everything out?
    
    return Result;
}

#define BotPushStruct(Arena, type) (type*)BotPushSize(Arena, sizeof(type))
#define BotPushArray(Arena, type, count) (type*)BotPushSize(Arena, sizeof(type)*count)
inline void* BotPushSize(mem_double_arena* Arena, mm Size)
{
    Assert(Arena->UsedTop + Arena->UsedBot + Size <= Arena->Size);
    Arena->UsedBot += Size;
    void* Result = Arena->Mem + Arena->Size - Arena->UsedBot;
    // TODO: Do we just wanna zero everything out?
    
    return Result;
}

inline mem_arena SubArena(mem_arena* Arena, mm Size)
{
    mem_arena Result = {};
    Result.Size = Size;
    Result.Used = 0;
    Result.Mem = (u8*)PushSize(Arena, Size);

    return Result;
}

inline void ClearMem(void* Mem, mm Size)
{
    u8* CurrByte = (u8*)Mem;
    for (mm Byte = 0; Byte < Size; ++Byte)
    {
        *CurrByte++ = 0;
    }
}

inline void Copy(void* Mem, void* Dest, mm Size)
{
    u8* CurrentByte = (u8*)Mem;
    u8* DestByte = (u8*)Dest;
    for (mm Byte = 0; Byte < Size; ++Byte)
    {
        *DestByte++ = *CurrentByte++;
    }
}

inline void ZeroMem(void* Mem, mm NumBytes)
{
    u8* ByteMem = (u8*)Mem;
    for (mm ByteId = 0; ByteId < NumBytes; ++ByteId)
    {
        *ByteMem = 0;
        ByteMem++;
    }
}

// TODO: Macro to not have to make copies??
#define ShiftPtrByBytes(Ptr, Step, Type) (Type*)ShiftPtrByBytes_((u8*)Ptr, Step)
inline u8* ShiftPtrByBytes_(u8* Ptr, mm Step)
{
    u8* Result = Ptr + Step;
    return Result;
}
