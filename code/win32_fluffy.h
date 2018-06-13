#if !defined(WIN32_FLUFFY_H)

struct win32_game_code
{
    char* SourceDLLPath;
    char* TempDLLPath;
    char* LockFilePath;
    
    HMODULE GameCodeDLL;
    FILETIME LastDLLFileTime;

    // IMPORTANT: Either of the callbacks can be null
    // You must check before calling
    game_init* Init;
    game_update_and_render* UpdateAndRender;
};

struct win32_state
{
    b32 GameIsRunning;
    i64 TimerFrequency;

    HDC DeviceContext;
};

#define WIN32_FLUFFY_H
#endif
