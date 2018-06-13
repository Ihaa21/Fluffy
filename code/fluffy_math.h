#if !defined(FLUFFY_MATH_H)

// TODO: Remove
#include <math.h>

#define I16_MIN -32768
#define I16_MAX 32767
//#define INT32_MIN INT_MIN
//#define INT32_MAX INT_MAX
#define F32_MAX FLT_MAX
#define F32_MIN -F32_MAX
#define F32_EPSILON FLT_EPSILON
#define Pi32 3.14159265359f
#define Tau32 6.28318530717958647692f

#define Square(a) ((a)*(a))
#define Min(a, b) (((a) > (b)) ? (b) : (a))
#define Max(a, b) (((a) < (b)) ? (b) : (a))

union v2i
{
    struct
    {
        i32 x, y;
    };

    i32 e[2];
};

union v2
{
    struct
    {
        f32 x, y;
    };

    f32 e[2];
};

union v3
{
    struct
    {
        f32 x, y, z;        
    };
        
    struct
    {
        v2 xy;
        f32 Ignored0_;
    };

    struct
    {
        v2 xz;
        f32 Ignored1_;
    };

    struct
    {
        v2 yz;
        f32 Ignored2_;
    };

    struct
    {
        f32 r, g, b;
    };

    f32 e[3];
};    

union v4
{
    struct
    {
        f32 x, y, z, w;
    };

    struct
    {
        v3 xyz;
        f32 Ignored0_;
    };

    struct
    {
        v3 rgb;
        f32 Ignored1_;
    };
        
    struct
    {
        f32 r, g, b, a;
    };

    f32 e[4];
};

// NOTE: Matrices are stored column order
struct m2
{
    union
    {
        f32 e[4];
        v2 v[2];
    };
};

// NOTE: Matrices are stored column order
union m3
{
    struct
    {
        f32 e[12];
    };
    
    struct
    {
        v3 v[3];
    };

    struct
    {
        v3 Ignored0_[2];
        v2 Pos;
        f32 Ignored0__;
    };
};

// NOTE: Matrices are stored column order
struct m4
{
    union
    {
        f32 e[16];
        v4 v[4];
    };
};

union quat
{
    struct
    {
        f32 x, y, z, w;
    };

    struct
    {
        v3 v;
        f32 Ignored0_;
    };
};

struct rect2
{
    v2 Min;
    v2 Max;
};

struct rect3
{
    v3 Min;
    v3 Max;
};

struct ray_cast
{
    i16 NumGridsToVisit;
    i16 CurrX;
    i16 CurrY;
    i16 IncX;
    i16 IncY;
    f32 t;
    f32 NextVert;
    f32 NextHoriz;
    v2 DeltaRecip;
};

#define FLUFFY_MATH_H
#endif
