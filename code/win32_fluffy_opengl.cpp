
#include <gl/glext.h>
#include <gl/glcorearb.h>

#define ROSEMARY_WIN32_GL_FUNC_LIST                                     \
    ROSEMARY_GL_LOAD(BOOL, ChoosePixelFormatARB, HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats) \
    ROSEMARY_GL_LOAD(HGLRC, CreateContextAttribsARB, HDC hDC, HGLRC hShareContext, const int *attribList) \
    ROSEMARY_GL_LOAD(BOOL, SwapIntervalEXT, int interval)               \
    ROSEMARY_GL_LOAD(const char*, GetExtensionsStringEXT, void)         \

#define ROSEMARY_GL_FUNC_LIST                                           \
    ROSEMARY_GL_LOAD(GLuint, CreateShader, GLenum shaderType)           \
    ROSEMARY_GL_LOAD(void, AttachShader, GLuint program, GLuint shader) \
    ROSEMARY_GL_LOAD(void, ShaderSource, GLuint shader, GLsizei count, const GLchar **string, const GLint *length) \
    ROSEMARY_GL_LOAD(void, CompileShader, GLuint shader)                \
    ROSEMARY_GL_LOAD(void, GetShaderiv, GLuint shader, GLenum pname, GLint *params) \
    ROSEMARY_GL_LOAD(void, GetShaderInfoLog, GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) \
    ROSEMARY_GL_LOAD(void, DetachShader, GLuint program, GLuint shader) \
    ROSEMARY_GL_LOAD(void, DeleteShader, GLuint shader)                 \
    ROSEMARY_GL_LOAD(GLuint, CreateProgram, void)                       \
    ROSEMARY_GL_LOAD(void, LinkProgram, GLuint program)                 \
    ROSEMARY_GL_LOAD(void, UseProgram, GLuint program)                  \
    ROSEMARY_GL_LOAD(void, GetProgramiv, GLuint program, GLenum pname, GLint *params) \
    ROSEMARY_GL_LOAD(void, GetProgramInfoLog, GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog) \
    ROSEMARY_GL_LOAD(void, Uniform1i, GLint location, GLint v0)         \
    ROSEMARY_GL_LOAD(void, Uniform1f, GLint location, GLfloat v0)       \
    ROSEMARY_GL_LOAD(void, Uniform2f, GLint location, GLfloat v0, GLfloat v1) \
    ROSEMARY_GL_LOAD(void, Uniform3f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) \
    ROSEMARY_GL_LOAD(void, Uniform4f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) \
    ROSEMARY_GL_LOAD(void, UniformMatrix3fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    ROSEMARY_GL_LOAD(GLint, GetUniformLocation, GLuint program, const GLchar *name) \
    ROSEMARY_GL_LOAD(void, GenVertexArrays, GLsizei n, GLuint * arrays) \
    ROSEMARY_GL_LOAD(void, BindVertexArray, GLuint array)               \
    ROSEMARY_GL_LOAD(void, DeleteBuffers, GLsizei n, GLuint * buffers)  \
    ROSEMARY_GL_LOAD(void, GenBuffers, GLsizei n, GLuint * buffers)     \
    ROSEMARY_GL_LOAD(void, BindBuffer, GLenum target, GLuint buffer)    \
    ROSEMARY_GL_LOAD(void, BufferData, GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage) \
    ROSEMARY_GL_LOAD(void, BufferSubData, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data) \
    ROSEMARY_GL_LOAD(void, EnableVertexAttribArray, GLuint index)       \
    ROSEMARY_GL_LOAD(void, DisableVertexAttribArray, GLuint index)      \
    ROSEMARY_GL_LOAD(void, VertexAttribPointer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer) \
    ROSEMARY_GL_LOAD(void, VertexAttribIPointer, GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer) \
    ROSEMARY_GL_LOAD(void, VertexAttribDivisor, GLuint index, GLuint divisor) \
    ROSEMARY_GL_LOAD(void, DrawBuffers, GLsizei n, const GLenum* bufs)  \
    ROSEMARY_GL_LOAD(void, DrawArraysInstanced, GLenum mode, GLint first, GLsizei count, GLsizei primcount) \
    ROSEMARY_GL_LOAD(void, ActiveTexture, GLenum Texture)               \
    ROSEMARY_GL_LOAD(void, GenFramebuffers, GLsizei n, GLuint *ids)     \
    ROSEMARY_GL_LOAD(void, BindFramebuffer, GLenum target, GLuint framebuffer) \
    ROSEMARY_GL_LOAD(void, FramebufferTexture2D, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) \
    ROSEMARY_GL_LOAD(GLenum, CheckFramebufferStatus, GLenum target) \
    ROSEMARY_GL_LOAD(void*, MapBufferRange, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) \
    ROSEMARY_GL_LOAD(GLboolean, UnmapBuffer, GLenum target)
        
#define ROSEMARY_GL_LOAD(ret, name, ...)        \
    typedef ret name##proc(__VA_ARGS__);        \
    global name##proc* gl##name;                \

ROSEMARY_GL_FUNC_LIST
#undef ROSEMARY_GL_LOAD

#define ROSEMARY_GL_LOAD(ret, name, ...)        \
    typedef ret name##proc(__VA_ARGS__);        \
    global name##proc* wgl##name;               \

ROSEMARY_WIN32_GL_FUNC_LIST
#undef ROSEMARY_GL_LOAD

#define GL_SRGB8_ALPHA8 0x8C43
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C

inline void Win32SetPixelFormat(HDC WindowDC, int PixelFormatIndex)
{    
    PIXELFORMATDESCRIPTOR PixelFormat;
    DescribePixelFormat(WindowDC, PixelFormatIndex, sizeof(PixelFormat), &PixelFormat);
    SetPixelFormat(WindowDC, PixelFormatIndex, &PixelFormat);
}

internal HGLRC Win32InitOpenGL()
{
    // NOTE: Create a dummy window and use it to load our extensions and gl functions
    // which our game will need to render
    WNDCLASSA TempWindowClass = {};
    TempWindowClass.lpfnWndProc = DefWindowProcA;
    TempWindowClass.hInstance = GetModuleHandle(0);
    TempWindowClass.lpszClassName = "WGLLoader";

    if (!RegisterClassA(&TempWindowClass))
    {
        // TODO: Logging
        InvalidCodePath;
    }

    HWND TempWindow = CreateWindowExA(0, TempWindowClass.lpszClassName, "Untitled", 0, CW_USEDEFAULT,
                                      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
                                      TempWindowClass.hInstance, 0);
    HDC TempWindowDC = GetDC(TempWindow);
    
    // NOTE: Setup a pixel format and ask windows for closest matching format
    int TempPixelFormatIndex = 0;
    
    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
    DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
    DesiredPixelFormat.nVersion = 1;
    DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    DesiredPixelFormat.cColorBits = 32;
    DesiredPixelFormat.cAlphaBits = 8;
    DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

    TempPixelFormatIndex = ChoosePixelFormat(TempWindowDC, &DesiredPixelFormat);
    Win32SetPixelFormat(TempWindowDC, TempPixelFormatIndex);
    
    HGLRC TempGLContext = wglCreateContext(TempWindowDC);
    if (wglMakeCurrent(TempWindowDC, TempGLContext))
    {
        // NOTE: Load GL functions we will need for our game        
#define ROSEMARY_GL_LOAD(ret, name, ...)                        \
        wgl##name = (name##proc*)wglGetProcAddress("wgl" #name);  \
            Assert(wgl##name);

        ROSEMARY_WIN32_GL_FUNC_LIST;
#undef ROSEMARY_GL_LOAD
        
#define ROSEMARY_GL_LOAD(ret, name, ...)                        \
        gl##name = (name##proc*)wglGetProcAddress("gl" #name);  \
            Assert(gl##name);

        ROSEMARY_GL_FUNC_LIST;
#undef ROSEMARY_GL_LOAD
        
        wglMakeCurrent(0, 0);
    }

    wglDeleteContext(TempGLContext);
    ReleaseDC(TempWindow, TempWindowDC);
    DestroyWindow(TempWindow);

    // NOTE: Create a context for our game window 
    HGLRC MainGLContext = 0;
    int MainPixelFormatIndex = 0;
    GLuint ExtendedPick = 0;

    int IntAttribList[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
            0,
        };

    wglChoosePixelFormatARB(GlobalState.DeviceContext, IntAttribList, 0, 1, &MainPixelFormatIndex,
                            &ExtendedPick);
    Win32SetPixelFormat(GlobalState.DeviceContext, MainPixelFormatIndex);
    
    int Win32OpenGLAttribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 0,
        WGL_CONTEXT_FLAGS_ARB, 0
#if DREAM_INTERNAL
        | WGL_CONTEXT_DEBUG_BIT_ARB
#endif
        , WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
        0,
    };
    
    MainGLContext = wglCreateContextAttribsARB(GlobalState.DeviceContext, 0, Win32OpenGLAttribs);
    wglMakeCurrent(GlobalState.DeviceContext, MainGLContext);
    wglSwapIntervalEXT(0);
    
    return MainGLContext;
}
