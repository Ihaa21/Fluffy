
global char* GlobalShaderHeaderCode = R"FOO(
#version 330 core

#define b32 bool
#define i32 int
#define u32 int unsigned
#define f32 float
#define v2 vec2
#define v3 vec3
#define v4 vec4
#define V2 vec2
#define V3 vec3
#define V4 vec4
#define m3 mat3

)FOO";

global opengl OpenGL;

inline void GLCheckError()
{
#if ROSEMARY_DEBUG
    GLuint InvalidEnum = GL_INVALID_ENUM;
    GLuint InvalidValue = GL_INVALID_VALUE;
    GLuint InvalidOperation = GL_INVALID_OPERATION;
    GLuint InvalidFrameBufferOperation = GL_INVALID_FRAMEBUFFER_OPERATION;
    GLuint OutOfMemory = GL_OUT_OF_MEMORY;

    GLuint ErrorCode = glGetError();
    Assert(ErrorCode == GL_NO_ERROR);
#endif
}

inline b32 IsGLHandleValid(GLuint Handle)
{
    b32 Result = (Handle != -1);
    return Result;
}

internal GLuint OpenGLCreateProgram(char* HeaderCode, char* VertexCode, char* FragmentCode)
{
    GLuint Result = 0;
    
    GLint Temp = GL_FALSE;
    int InfoLogLength;
    
    GLuint VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* VertexShaderCode[] =
    {
        HeaderCode,
        VertexCode,
    };
    glShaderSource(VertexShaderId, ArrayCount(VertexShaderCode), VertexShaderCode, NULL);
    glCompileShader(VertexShaderId);
    glGetShaderiv(VertexShaderId, GL_COMPILE_STATUS, &Temp);
    glGetShaderiv(VertexShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        // TODO: Print the log on error
        char InfoLog[1024];
        glGetShaderInfoLog(VertexShaderId, InfoLogLength, NULL, &InfoLog[0]);
        InvalidCodePath;
    }

    GLuint FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* FragShaderCode[] =
    {
        HeaderCode,
        FragmentCode,
    };
    glShaderSource(FragmentShaderId, ArrayCount(FragShaderCode), FragShaderCode, NULL);
    glCompileShader(FragmentShaderId);
    glGetShaderiv(FragmentShaderId, GL_COMPILE_STATUS, &Temp);
    glGetShaderiv(FragmentShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        // TODO: Print the log on error
        char InfoLog[1024];
        glGetShaderInfoLog(FragmentShaderId, InfoLogLength, NULL, &InfoLog[0]);
        InvalidCodePath;
    }

    Result = glCreateProgram();
    glAttachShader(Result, VertexShaderId);
    glAttachShader(Result, FragmentShaderId);
    glLinkProgram(Result);

    glGetProgramiv(Result, GL_LINK_STATUS, &Temp);
    glGetProgramiv(Result, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        // TODO: Print the log on error
        char InfoLog[1024];
        glGetProgramInfoLog(Result, InfoLogLength, NULL, &InfoLog[0]);
        InvalidCodePath;
    }

    glDetachShader(Result, VertexShaderId);
    glDetachShader(Result, FragmentShaderId);
    
    glDeleteShader(VertexShaderId);
    glDeleteShader(FragmentShaderId);

    return Result;
}

internal void CompileRenderFrame(render_frame_prog* RenderFrameProg)
{
    char* VertexCode = R"FOO(
layout(location = 0) in v3 VertPos;
layout(location = 1) in v2 VertUV;

smooth out v2 UV;

void main()
{
    gl_Position = V4(2.0*VertPos.xy, 1, 1);
    UV = VertUV;
}
)FOO";
    
    char* FragmentCode = R"FOO(
in v2 UV;
out v4 PixelColor;

uniform sampler2D TextureSampler;

void main() 
{
    PixelColor = texture(TextureSampler, UV).rgba;
}
)FOO";
    RenderFrameProg->ProgId = OpenGLCreateProgram(GlobalShaderHeaderCode, VertexCode, FragmentCode);
    RenderFrameProg->TexSamplerId = glGetUniformLocation(RenderFrameProg->ProgId, "TextureSampler");
} 

internal void CompileTest(test_prog* Test)
{
    char* VertexCode = R"FOO(
layout(location = 0) in v2 VertPos;

void main()
{
    gl_Position = V4(VertPos, 0, 1);
}
)FOO";
    
    char* FragmentCode = R"FOO(
out v4 PixelColor;

void main() 
{
    PixelColor = V4(1, 0, 0, 1);
}
)FOO";
    Test->ProgId = OpenGLCreateProgram(GlobalShaderHeaderCode, VertexCode, FragmentCode);
} 

internal void BeginProgram(render_frame_prog Prog)
{
    glUseProgram(Prog.ProgId);
                
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (void*)(sizeof(v3)));
                
    glUniform1i(Prog.TexSamplerId, 0);
}

internal void EndProgram(render_frame_prog Prog)
{
    glUseProgram(0);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

internal void BeginProgram(test_prog Prog)
{
    glUseProgram(Prog.ProgId);
                
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(v2), (void*)0);
}

internal void EndProgram(test_prog Prog)
{
    glUseProgram(0);
    
    glDisableVertexAttribArray(0);
}

GL_LOAD_TEXTURE_TO_GPU(GLLoadTextureToGpu)
{
    u32 Result = 0;
    
    glGenTextures(1, &Result);
    glBindTexture(GL_TEXTURE_2D, Result);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);

    // TODO: http://casual-effects.blogspot.ca/2016/03/pixel-art-tech-tips.html
    // says to set the min filter to a mip map value, should we generate mip maps or is
    // this good enough?
    // TODO: Also, frag shader should use texelFetch?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_2D, 0);

    GLCheckError();

    return Result;
}

internal void InitOpenGL(u32 Width, u32 Height, void* Mem, u32 MemSize)
{
    CompileRenderFrame(&OpenGL.RenderFrameProg);
    CompileTest(&OpenGL.TestProg);

    // NOTE: Create a white texture
    {
        u32 WhitePixel = 0xFFFFFFFF;
        OpenGL.WhiteTextureId = GLLoadTextureToGpu(1, 1, &WhitePixel);
    }
    
    // NOTE: Create a box model
    // TODO: Use index buffering?
    f32 RectVerts[] =
    {
        -0.5, -0.5, 0, 0, 0,
        -0.5, 0.5, 0, 0, 1,
        0.5, 0.5, 0, 1, 1,
        -0.5, -0.5, 0, 0, 0,
        0.5, 0.5, 0, 1, 1,
        0.5, -0.5, 0, 1, 0,
    };

    // TODO: What does this do?
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    glGenBuffers(1, &OpenGL.SquareVBO);
    glBindBuffer(GL_ARRAY_BUFFER, OpenGL.SquareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RectVerts), RectVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // NOTE: Init our rendered texture and fbo
    {
        glGenFramebuffers(1, &OpenGL.FrameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, OpenGL.FrameBuffer);

        // NOTE: Add color texture
        glGenTextures(1, &OpenGL.FrameTexture);
        glBindTexture(GL_TEXTURE_2D, OpenGL.FrameTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               OpenGL.FrameTexture, 0);

        // NOTE: Add depth texture
        glGenTextures(1, &OpenGL.FrameDepth);
        glBindTexture(GL_TEXTURE_2D, OpenGL.FrameDepth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, Width, Height, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, OpenGL.FrameDepth, 0);
    
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            InvalidCodePath;
        }
    
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        GLCheckError();
    }

    GLCheckError();
}

internal PUSH_TEXTURE_TO_SCREEN(GLPushTextureToScreen)
{
    glClearColor(0.6, 0.6, 0.6, 1);
    //glClearDepth(-1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, OpenGL.FrameTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}
