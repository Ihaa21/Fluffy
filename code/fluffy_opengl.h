#if !defined(FLUFFY_OPENGL_H)

struct render_frame_prog
{
    GLuint ProgId;

    GLuint TexSamplerId;
};

struct test_prog
{
    GLuint ProgId;
};

#define VERTEX_SIZE (sizeof(v3) + sizeof(v2))
#define OPENGL_MAX_NUM_QUADS 20000
#define OPENGL_NUM_INSTANCE_BUFFERS 8
struct opengl
{
    GLuint FrameBuffer;
    GLuint FrameTexture;
    GLuint FrameDepth;

    GLuint WhiteTextureId;
    GLuint SquareVBO;
    
    render_frame_prog RenderFrameProg;
    test_prog TestProg;
};

#define FLUFFY_OPENGL_H
#endif
