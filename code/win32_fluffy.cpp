#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <windows.h>

#include "fluffy_platform.h"
#include "win32_fluffy.h"

//
//
//

global HGLRC GlobalGLContext;
global win32_state GlobalState;

#include <gl/gl.h>
#include <gl/wglext.h>
#include "win32_fluffy_opengl.cpp"
#include "fluffy_opengl.h"
#include "fluffy_opengl.cpp"

//
//
//

inline LARGE_INTEGER Win32GetClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);

    return Result;
}

inline f32 Win32GetSecondsBetween(LARGE_INTEGER End, LARGE_INTEGER Start)
{
    f32 Result = ((f32)(End.QuadPart - Start.QuadPart) / (f32)GlobalState.TimerFrequency);
    return Result;
}

inline FILETIME Win32GetLastFileTime(char* FileName)
{
    FILETIME LastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA FileData;
    if (GetFileAttributesEx(FileName, GetFileExInfoStandard, &FileData))
    {
        LastWriteTime = FileData.ftLastWriteTime;
    }

    return LastWriteTime;
}

internal LRESULT CALLBACK Win32MainWindowCallBack(HWND Window, UINT Message, WPARAM WParam,
                                                  LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
        case WM_CLOSE:
        case WM_DESTROY:
        {
            GlobalState.GameIsRunning = false;
        } break;
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

inline DWORD Win32GetNearestFrameByte(DWORD SoundBytesPerFrame, DWORD CurrentSoundByte)
{
    // TODO: Implement a floor function here for NearestFrameCount
    DWORD NearestFrameCount = (DWORD)((CurrentSoundByte + SoundBytesPerFrame) / SoundBytesPerFrame);
    DWORD Result = NearestFrameCount*SoundBytesPerFrame;
    return Result;
}

inline void Win32UpdatePlatformApi(game_memory* GameMem)
{
    GameMem->PlatformApi.PushTextureToScreen = GLPushTextureToScreen;
    GameMem->PlatformApi.GLLoadTextureToGpu = GLLoadTextureToGpu;
}

internal void Win32LoadGameCode(win32_game_code* GameCode)
{
    // NOTE: Unload the game code
    if (GameCode->GameCodeDLL)
    {
        FreeLibrary(GameCode->GameCodeDLL);
        GameCode->GameCodeDLL = 0;
    }

    GameCode->Init = 0;
    GameCode->UpdateAndRender = 0;
    
    // NOTE: Load the game code
    b32 IsValid = false;
    for (u32 LoadTryIndex = 0; LoadTryIndex < 100 && !IsValid; ++LoadTryIndex)
    {
        WIN32_FILE_ATTRIBUTE_DATA Ignored;
    
        // NOTE: We check if the lock file exists. The lock file is there so that we
        // don't load the dll before the pdb
        if (!GetFileAttributesEx(GameCode->LockFilePath, GetFileExInfoStandard, &Ignored))
        {
            GameCode->LastDLLFileTime = Win32GetLastFileTime(GameCode->SourceDLLPath);
            CopyFileA(GameCode->SourceDLLPath, GameCode->TempDLLPath, FALSE);
            GameCode->GameCodeDLL = LoadLibraryA(GameCode->TempDLLPath);

            if (!GameCode->GameCodeDLL)
            {
                InvalidCodePath;
            }

            // NOTE: Load in the functions from our DLL
            GameCode->Init = (game_init*)GetProcAddress(GameCode->GameCodeDLL, "GameInit");
            GameCode->UpdateAndRender = (game_update_and_render*)GetProcAddress(GameCode->GameCodeDLL, "GameUpdateAndRender");
            
            IsValid = true;
        }
                
        Sleep(100);
    }
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    GlobalState.GameIsRunning = true;
        
    // NOTE: Init window and display
    u32 ScreenWidth = 1024;
    u32 ScreenHeight = 1024;
    HWND Window;
    {
        WNDCLASSA WindowClass = {};
        WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        WindowClass.lpfnWndProc = Win32MainWindowCallBack;
        WindowClass.hInstance = hInstance;
        WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
        WindowClass.lpszClassName = "OctreeClass";

        if (!RegisterClassA(&WindowClass))
        {
            // TODO: Display error message
            InvalidCodePath;
        }
        
        Window = CreateWindowExA(0,
                                 WindowClass.lpszClassName,
                                 "Haw Yeah",
                                 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 ScreenWidth,
                                 ScreenHeight,
                                 0,
                                 0,
                                 hInstance,
                                 0);

        if (!Window)
        {
            // TODO: Display error message
            InvalidCodePath;
        }
        
        GlobalState.DeviceContext = GetDC(Window);
        GlobalGLContext = Win32InitOpenGL();
    }
    
    //
    // NOTE: Setup game executable
    //

    win32_game_code GameCode = {};
    {
        char SourceDLLPath[256] = "W:\\fluffy\\build_win32\\fluffy.dll";
        GameCode.SourceDLLPath = SourceDLLPath;

        char TempDLLPath[256] = "W:\\fluffy\\build_win32\\fluffy_temp.dll";
        GameCode.TempDLLPath = TempDLLPath;

        char LockFilePath[256] = "W:\\fluffy\\build_win32\\lock.tmp";
        GameCode.LockFilePath = LockFilePath;

        Win32LoadGameCode(&GameCode);
    }
    
    //
    // NOTE: Game memory + Init
    //

    void* ProgramMemory = 0;
    game_memory GameMem = {};
    {
        GameMem.PermanentMemSize = MegaBytes(100);
        mm ProgMemSize = GameMem.PermanentMemSize;

#if OCTREE_DEBUG
        LPVOID BaseAddress = (LPVOID)TeraBytes(2);
#else
        LPVOID BaseAddress = 0;
#endif
        
        ProgramMemory = VirtualAlloc(BaseAddress, ProgMemSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!ProgramMemory)
        {
            InvalidCodePath;
        }
        
        mem_arena ProgArena = InitArena(ProgramMemory, ProgMemSize);
        GameMem.PermanentMem = PushSize(&ProgArena, GameMem.PermanentMemSize);

        Win32UpdatePlatformApi(&GameMem);
        if (GameCode.Init)
        {
            GameCode.Init(&GameMem);
        }
        else
        {
            InvalidCodePath;
        }

        void* TempMem = VirtualAlloc(0, MegaBytes(80), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        InitOpenGL(ScreenWidth, ScreenHeight, TempMem, MegaBytes(4));
        VirtualFree(TempMem, 0, MEM_RELEASE);
    }
        
    LARGE_INTEGER TimerFrequency;
    QueryPerformanceFrequency(&TimerFrequency);
    GlobalState.TimerFrequency = TimerFrequency.QuadPart;
    
    LARGE_INTEGER LastFrameTime = Win32GetClock();
            
    while (GlobalState.GameIsRunning)
    {
        u64 StartCycle = __rdtsc();
        
        // NOTE: Update platform api function pointers
        Win32UpdatePlatformApi(&GameMem);
        
        // NOTE: Reload game code DLL if the DLL changed
        FILETIME NewDLLFileTime = Win32GetLastFileTime(GameCode.SourceDLLPath);
        if (CompareFileTime(&NewDLLFileTime, &GameCode.LastDLLFileTime) != 0)
        {
            Win32LoadGameCode(&GameCode);
        }

        // NOTE: Process window events
        MSG Message;
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
            switch (Message.message)
            {
                case WM_QUIT:
                {
                    GlobalState.GameIsRunning = false;
                } break;
                    
                default:
                {
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
            }
        }

        // NOTE: Update game logic and render prep
        {
            if (!GameCode.UpdateAndRender)
            {
                // TODO: This should only happen in debug, for release compile game code into same exe
                InvalidCodePath;
            }
            
            GameCode.UpdateAndRender(&GameMem);
        }

        {
            // NOTE Render and display
            {
                RECT WinRect;
                if (!GetClientRect(Window, &WinRect))
                {
                    InvalidCodePath;
                }
                i32 WinWidth = WinRect.right - WinRect.left;
                i32 WinHeight = WinRect.bottom - WinRect.top;
    
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, WinWidth, WinHeight);

                glBindTexture(GL_TEXTURE_2D, OpenGL.FrameTexture);
                glBindBuffer(GL_ARRAY_BUFFER, OpenGL.SquareVBO);
           
                BeginProgram(OpenGL.RenderFrameProg);
                glDrawArrays(GL_TRIANGLES, 0, 3*2);
                EndProgram(OpenGL.RenderFrameProg);
                
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            {
                SwapBuffers(GlobalState.DeviceContext);
                GLCheckError();
            }
        }

        // NOTE: Debug info
        {
            f32 FrameTime = Win32GetSecondsBetween(Win32GetClock(), LastFrameTime)*1000.0f;
            LastFrameTime = Win32GetClock();

            char Buffer[256];
            snprintf(Buffer, sizeof(Buffer), "%f\n", FrameTime);
            OutputDebugString(Buffer);
        }

        u64 EndCycle = __rdtsc();
    }
    
    return 0;
}
