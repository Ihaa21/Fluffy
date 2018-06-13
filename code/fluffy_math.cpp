//
// NOTE: Scalar
//

inline i32 RoundUp(f32 Val)
{
    i32 Result = (i32)ceil(Val);
    return Result;
}

inline i32 RoundDown(f32 Val)
{
    i32 Result = (i32)floor(Val);
    return Result;
}

inline f32 Sign(f32 Val)
{
    f32 Result = (f32)((Val > 0) - (Val < 0));
    return Result;
}

inline f32 MapIntoRange(f32 Val, f32 Min, f32 Max)
{
    f32 Result = 0.0f;
    if (Max != 0.0f)
    {
        Result = (Val - Min) / (Max - Min);
    }
    
    return Result;
}

inline f32 Clamp(f32 Val, f32 Min, f32 Max)
{
    Assert(Min <= Max);
    f32 Result = Val;
    if (Result < Min)
    {
        Result = Min;
    }
    else if (Result > Max)
    {
        Result = Max;
    }

    return Result;
}

inline f32 DegreeToRad(f32 Angle)
{
    f32 Result = Angle*Pi32/180.0f;
    return Result;
}

inline u32 RoundToNearestU32(u32 Nearest, u32 Value)
{
    u32 Result = 0;

    if (Nearest != 0)
    {
        Result = Value;
        u32 Remainder = Value % Nearest;
        if (Remainder != 0)
        {
            Result += Nearest - Remainder;
        }
    }

    return Result;
}

inline i32 RoundToNearestI32(i32 Nearest, i32 Value)
{
    u32 Result = 0;

    if (Nearest != 0)
    {
        Result = Value;
        u32 Remainder = Value % Nearest;
        if (Remainder != 0)
        {
            Result += Nearest - Remainder;
        }
    }

    return Result;
}

inline i32 RoundToI32(f32 Value)
{
    i32 Result = (i32)(Value + 0.5f);
    return Result;
}

inline u32 RoundToU32(f32 Value)
{
    u32 Result = (u32)(Value + 0.5f);
    return Result;
}

inline u32 FloorToU32(f32 Value)
{
    u32 Result = (u32)floorf(Value);
    return Result;
}

inline u32 SafeTruncateU64(u64 Val)
{
    Assert(Val <= UINT32_MAX);
    u32 Result = (u32)Val;
    
    return(Result);
}

inline i32 Abs(i32 A)
{
    i32 Result = A < 0 ? -A : A;
    return Result;
}

inline f32 ToDegree(f32 Radians)
{
    f32 Result = Radians * 180.0f / Pi32;
    return Result;
}

inline f32 ToRadians(f32 Degrees)
{
    f32 Result = Degrees * Pi32 / 180.0f;
    return Result;
}

inline f32 Abs(f32 A)
{
    f32 Result = A < 0 ? -A : A;
    return Result;
}

inline b32 IsBetween(f32 Val, f32 A, f32 B)
{
    b32 Result = (A <= Val && Val <= B);
    return Result;
}

// TODO: Intrinsics
inline f32 SquareRoot(f32 A)
{
    f32 Result = sqrtf(A);
    return Result;
}

// TODO: Intrinsics
inline f32 Sin(f32 A)
{
    f32 Result = sinf(A);
    return Result;
}

// TODO: Intrinsics
inline f32 ArcSin(f32 A)
{
    f32 Result = asinf(A);
    return Result;
}

// TODO: Intrinsics
inline f32 Cos(f32 A)
{
    f32 Result = cosf(A);
    return Result;
}

// TODO: Intrinsics
inline f32 ArcCos(f32 A)
{
    f32 Result = acosf(A);
    return Result;
}

// TODO: Intrinsics
inline f32 ArcTan(f32 X, f32 Y)
{
    f32 Result = atan2f(Y, X);
    return Result;
}

inline f32 Ceil(f32 A)
{
    f32 Result = ceilf(A);
    return Result;
}

inline f32 Lerp(f32 Start, f32 End, f32 t)
{
    f32 Result = Start*(1.0f - t) + End*t;

    return Result;
}

// TODO: Extend to SIMD or optimize? Also what about non int powers?
inline f32 Pow(f32 Base, u32 Exp)
{
    f32 Result = Base;
    for (u32 ExpIndex = 0; ExpIndex < Exp; ++ExpIndex)
    {
        Result *= Base;
    }

    return Result;
}

inline void GetParabolaFrom3Points(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, f32* A, f32* B, f32* C)
{
    // NOTE: https://stackoverflow.com/questions/717762/how-to-calculate-the-vertex-of-a-parabola-given-three-points
    
    f32 Denom = (x1 - x2) * (x1 - x3) * (x2 - x3);
    *A = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) / Denom;
    *B = (x3*x3 * (y1 - y2) + x2*x2 * (y3 - y1) + x1*x1 * (y2 - y3)) / Denom;
    *C = (x2 * x3 * (x2 - x3) * y1 + x3 * x1 * (x3 - x1) * y2 + x1 * x2 * (x1 - x2) * y3) / Denom;
}

struct bit_scan_result
{
    b32 Found;
    u32 Index;
};

inline bit_scan_result FindLeastSignificantSetBit(u32 Value)
{
    bit_scan_result Result = {};

#if COMPILER_MSVC
    Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
    for(u32 Test = 0; Test < 32; ++Test)
    {
        if(Value & (1 << Test))
        {
            Result.Index = Test;
            Result.Found = true;
            break;
        }
    }
#endif
    
    return(Result);
}

//
// NOTE: Vectors 2i
//

#if 0
inline v2i V2i(i32 X, i32 Y)
{
    v2i Result;

    Result.x = X;
    Result.y = Y;
    
    return Result;
}

inline v2i V2i(i32 X, i32 Y)
{
    v2i Result;

    Result.x = (i32)X;
    Result.y = (i32)Y;
    
    return Result;
}

inline v2i operator+(v2i A, v2i B)
{
    v2i Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return Result;
}

inline v2i operator-(v2i A)
{
    v2i Result = {};

    Result.x = -A.x;
    Result.y = -A.y;

    return Result;
}

inline v2i operator-(v2i A, v2i B)
{
    v2i Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return Result;
}

inline v2i operator*(i32 B, v2i A)
{
    v2i Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;

    return Result;
}

inline v2i operator*(v2i A, i32 B)
{
    v2i Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;

    return Result;
}

inline v2i operator/(v2i A, i32 B)
{
    v2i Result;
    
    Result.x = A.x / B;
    Result.y = A.y / B;

    return Result;
}

inline v2i& operator+=(v2i& A, v2i B)
{
    A = A + B;

    return A;
}

inline v2i& operator-=(v2i& A, v2i B)
{
    A = A - B;

    return A;
}

inline v2i& operator*=(v2i& A, i32 B)
{
    A = A * B;

    return A;
}

inline v2i& operator/=(v2i& A, i32 B)
{
    A = A / B;
    
    return A;
}

#endif

//
// NOTE: Vectors 2
//

inline v2 V2(f32 X, f32 Y)
{
    v2 Result;

    Result.x = X;
    Result.y = Y;
    
    return Result;
}

inline v2 V2i(i32 X, i32 Y)
{
    v2 Result;

    Result.x = (f32)X;
    Result.y = (f32)Y;
    
    return Result;
}

inline b32 operator==(v2 A, v2 B)
{
    b32 Result = A.x == B.x && A.y == B.y;
    return Result;
}

inline v2 operator+(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return Result;
}

inline v2 operator-(v2 A)
{
    v2 Result = {};

    Result.x = -A.x;
    Result.y = -A.y;

    return Result;
}

inline v2 operator-(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return Result;
}

inline v2 operator*(f32 B, v2 A)
{
    v2 Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;

    return Result;
}

inline v2 operator*(v2 A, f32 B)
{
    v2 Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;

    return Result;
}

inline v2 operator/(v2 A, f32 B)
{
    v2 Result;
    
    Result.x = A.x / B;
    Result.y = A.y / B;

    return Result;
}

inline v2& operator+=(v2& A, v2 B)
{
    A = A + B;

    return A;
}

inline v2& operator-=(v2& A, v2 B)
{
    A = A - B;

    return A;
}

inline v2& operator*=(v2& A, f32 B)
{
    A = A * B;

    return A;
}

inline v2& operator/=(v2& A, f32 B)
{
    A = A / B;
    
    return A;
}

inline f32 Length(v2 A)
{
    f32 Result = Square(A.x) + Square(A.y);
    return Result;
}

inline f32 LengthSq(v2 A)
{
    f32 Result = SquareRoot(Length(A));
    return Result;
}

inline f32 Inner(v2 A, v2 B)
{
    f32 Result = A.x*B.x + A.y*B.y;
    return Result;
}

inline v2 Normalize(v2 A)
{
    v2 Result = A;

    f32 VecLength = LengthSq(A);
    if (VecLength != 0.0f)
    {
        Result /= VecLength;
    }
    Assert(VecLength >= 0.0f);
    
    return Result;
}

inline v2 Lerp(v2 Start, v2 End, f32 t)
{
    v2 Result = Start*(1.0f - t) + End*t;

    return Result;
}

inline v2 Hadamard(v2 A, v2 B)
{
    v2 Result = {};
    Result.x = A.x*B.x;
    Result.y = A.y*B.y;
    
    return Result;
}

inline f32 AngleBetweenVectors(v2 A, v2 B)
{
    f32 Angle = ArcCos(Inner(A, B)/(LengthSq(A)*LengthSq(B)));
    return Angle;
}

inline v2 GetPerp(v2 A)
{
    v2 Result = V2(A.y, -A.x);
    return Result;
}

//
// NOTE: Vectors 3
//

inline v3 V3(f32 X, f32 Y, f32 Z)
{
    v3 Result;

    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    
    return Result;
}

inline v3 V3i(i32 X, i32 Y, i32 Z)
{
    v3 Result;

    Result.x = (f32)X;
    Result.y = (f32)Y;
    Result.z = (f32)Z;
    
    return Result;
}

inline v3 operator+(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return Result;
}

inline v3 operator-(v3 A)
{
    v3 Result = {};

    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;

    return Result;
}

inline v3 operator-(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    
    return Result;
}

inline v3 operator*(f32 B, v3 A)
{
    v3 Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;

    return Result;
}

inline v3 operator*(v3 A, f32 B)
{
    v3 Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;

    return Result;
}

inline v3 operator/(v3 A, f32 B)
{
    v3 Result;
    
    Result.x = A.x / B;
    Result.y = A.y / B;
    Result.z = A.z / B;

    return Result;
}

inline v3& operator+=(v3& A, v3 B)
{
    A = A + B;

    return A;
}

inline v3& operator-=(v3& A, v3 B)
{
    A = A - B;

    return A;
}

inline v3& operator*=(v3& A, f32 B)
{
    A = A * B;

    return A;
}

inline v3& operator/=(v3& A, f32 B)
{
    A = A / B;
    
    return A;
}

inline f32 Length(v3 A)
{
    f32 Result = Square(A.x) + Square(A.y) + Square(A.z);
    return Result;
}

inline f32 LengthSq(v3 A)
{
    f32 Result = SquareRoot(Length(A));
    return Result;
}

inline v3 Normalize(v3 A)
{
    v3 Result = A;
    
    f32 VecLength = LengthSq(A);
    if (VecLength != 0.0f)
    {
        Result /= VecLength;
    }
    Assert(VecLength >= 0.0f);

    return Result;
}

inline f32 Inner(v3 A, v3 B)
{
    f32 Result = Inner(A.xy, B.xy) + A.z*B.z;
    return Result;
}

inline v3 Cross(v3 A, v3 B)
{
    v3 Result = {};

    Result.x = A.y*B.z - A.z*B.y;
    Result.y = A.z*B.x - A.x*B.z;
    Result.z = A.x*B.y - A.y*B.x;

    return Result;
}

inline v3 GetReflection(v3 Normal, v3 Dir)
{
    v3 Result = Dir - 2*(Inner(Dir, Normal))*Normal;
    return Result;
}

inline v3 Lerp(v3 Start, v3 End, f32 t)
{
    v3 Result = Start*(1.0f - t) + End*t;

    return Result;
}

//
// NOTE: Vectors 4
//

inline v4 V4(v3 V, f32 W)
{
    v4 Result = {};

    Result.xyz = V;
    Result.w = W;

    return Result;
}

inline v4 V4(f32 X, f32 Y, f32 Z, f32 W)
{
    v4 Result;

    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;
    
    return Result;
}

inline v4 V4i(i32 X, i32 Y, i32 Z, i32 W)
{
    v4 Result;

    Result.x = (f32)X;
    Result.y = (f32)Y;
    Result.z = (f32)Z;
    Result.w = (f32)W;
    
    return Result;
}

inline v4 operator+(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;
   
    return Result;
}

inline v4 operator-(v4 A)
{
    v4 Result = {};

    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    Result.w = -A.w;

    return Result;
}

inline v4 operator-(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    Result.w = A.w - B.w;
    
    return Result;
}

inline v4 operator*(f32 B, v4 A)
{
    v4 Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;
    Result.w = A.w * B;

    return Result;
}

inline v4 operator*(v4 A, f32 B)
{
    v4 Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;
    Result.w = A.w * B;

    return Result;
}

inline v4 operator/(v4 A, f32 B)
{
    v4 Result;
    
    Result.x = A.x / B;
    Result.y = A.y / B;
    Result.z = A.z / B;
    Result.w = A.w / B;

    return Result;
}

inline v4& operator+=(v4& A, v4 B)
{
    A = A + B;

    return A;
}

inline v4& operator-=(v4& A, v4 B)
{
    A = A - B;

    return A;
}

inline v4& operator*=(v4& A, f32 B)
{
    A = A * B;

    return A;
}

inline v4& operator/=(v4& A, f32 B)
{
    A = A / B;
    
    return A;
}

inline f32 Length(v4 A)
{
    f32 Result = Square(A.x) + Square(A.y) + Square(A.z) + Square(A.w);
    return Result;
}

inline f32 LengthSq(v4 A)
{
    f32 Result = SquareRoot(Length(A));
    return Result;
}

inline v4 Normalize(v4 A)
{
    v4 Result = A;
        
    f32 VecLength = LengthSq(A);
    if (VecLength != 0.0f)
    {
        Result /= VecLength;
    }
    Assert(VecLength >= 0.0f);

    return Result;
}

inline f32 Inner(v4 A, v4 B)
{
    f32 Result = Inner(A.xyz, B.xyz) + A.w*B.w;
    return Result;
}

inline v4 Lerp(v4 Start, v4 End, f32 t)
{
    v4 Result = Start*(1.0f - t) + End*t;

    return Result;
}

inline v4 Hadamard(v4 A, v4 B)
{
    v4 Result = {};
    Result.x = A.x*B.x;
    Result.y = A.y*B.y;
    Result.z = A.z*B.z;
    Result.w = A.w*B.w;

    return Result;
}

//
// NOTE: Rect 2
//

inline rect2 RectMinMax(v2 Min, v2 Max)
{
    rect2 Result = {};

    Result.Min = Min;
    Result.Max = Max;

    return Result;
}

inline rect2 RectCenterDim(v2 Center, v2 Dim)
{
    rect2 Result = {};

    Result.Min = Center - Dim;
    Result.Max = Center + Dim;

    return Result;
}

inline rect2 Enlarge(rect2 Rect, v2 AddDim)
{
    rect2 Result = {};

    Result.Min = Rect.Min - AddDim;
    Result.Max = Rect.Max + AddDim;

    return Result;
}

inline rect2 Translate(rect2 Rect, v2 Displacement)
{
    rect2 Result = Rect;

    Result.Min += Displacement;
    Result.Max += Displacement;

    return Result;
}

inline v2 GetRectCenter(rect2 A)
{
    v2 Result = Lerp(A.Min, A.Max, 0.5f);
    return Result;
}

inline v2 GetRectDim(rect2 A)
{
    v2 Result = A.Max - A.Min;
    return Result;
}

inline v2 GetRectBotPos(rect2 A)
{
    v2 Result = V2(Lerp(A.Min.x, A.Max.x, 0.5f), A.Min.y);
    return Result;
}

inline b32 Intersect(rect2 A, v2 B)
{
    b32 Result = (A.Min.x <= B.x && A.Max.x >= B.x &&
                  A.Min.y <= B.y && A.Max.y >= B.y);
    
    return Result;
}

inline b32 Intersect(rect2 A, rect2 B)
{
    b32 Result = (!((B.Min.x > A.Max.x) ||
                    (B.Max.x < A.Min.x) ||
                    (B.Min.y > A.Max.y) ||
                    (B.Max.y < A.Min.y)));
    
    return Result;
}

inline b32 Intersect(rect2 A, v2 Pos, f32 Radius)
{
    b32 Result = false;
    v2 Closest = V2(Clamp(Pos.x, A.Min.x, A.Max.x), Clamp(Pos.y, A.Min.y, A.Max.y));
    Result = Length(Pos - Closest) <= Radius;
    return Result;
}

inline b32 IntersectNotInclusive(rect2 A, rect2 B)
{
    b32 Result = (!((B.Min.x >= A.Max.x) ||
                    (B.Max.x <= A.Min.x) ||
                    (B.Min.y >= A.Max.y) ||
                    (B.Max.y <= A.Min.y)));
    return Result;
}

inline v2 NearestPointOnRectToPoint(rect2 A, v2 Pos)
{
    v2 Result = V2(Clamp(Pos.x, A.Min.x, A.Max.x), Clamp(Pos.y, A.Min.y, A.Max.y));
    return Result;
}

inline f32 DistBetweenRectCircle(rect2 A, v2 Pos)
{
    v2 Closest = V2(Clamp(Pos.x, A.Min.x, A.Max.x), Clamp(Pos.y, A.Min.y, A.Max.y));
    f32 Result = Length(Closest - Pos);
    return Result;
}

//
// NOTE: Rect 3
//

inline rect3 RectMinMax(v3 Min, v3 Max)
{
    rect3 Result = {};

    Result.Min = Min;
    Result.Max = Max;

    return Result;
}

inline rect3 RectCenterDim(v3 Center, v3 Dim)
{
    rect3 Result = {};

    Result.Min = Center - Dim;
    Result.Max = Center + Dim;

    return Result;
}

inline rect3 Translate(rect3 Rect, v3 Displacement)
{
    rect3 Result = Rect;

    Result.Min += Displacement;
    Result.Max += Displacement;

    return Result;
}

//
// NOTE: Matrix 2
//

inline v2 operator*(v2 A, m2 B)
{
    v2 Result = {};
    Result = A.x*B.v[0] + A.y*B.v[1];

    return Result;
}

inline m2 operator*(m2 A, m2 B)
{
    m2 Result = {};

    Result.v[0] = B.v[0] * A;
    Result.v[1] = B.v[1] * A;
    
    return Result;
}

//
// NOTE: Matrix 3
//

inline v3 operator*(v3 A, m3 B)
{
    v3 Result = {};
    Result = A.x*B.v[0] + A.y*B.v[1] + A.z*B.v[2];

    return Result;
}

inline m3 operator*(m3 A, f32 B)
{
    m3 Result = A;
    Result.v[0] = B*Result.v[0];
    Result.v[1] = B*Result.v[1];
    Result.v[2] = B*Result.v[2];

    return Result;
}

inline m3 operator*(m3 A, m3 B)
{
    m3 Result = {};

    Result.v[0] = B.v[0] * A;
    Result.v[1] = B.v[1] * A;
    Result.v[2] = B.v[2] * A;
    
    return Result;
}

inline m3 Identity()
{
    m3 Result = {};
    Result.v[0].x = 1.0f;
    Result.v[1].y = 1.0f;
    Result.v[2].z = 1.0f;
    
    return Result;
}

inline m3 ScaleMat(f32 X, f32 Y)
{
    m3 Result = Identity();
    Result.v[0].x = X;
    Result.v[1].y = Y;

    return Result;
}

inline m3 RotMat(f32 Angle)
{
    m3 Result = Identity();
    Result.v[0].x = Cos(Angle);
    Result.v[0].y = Sin(Angle);
    Result.v[1].x = -Result.v[0].y;
    Result.v[1].y = Result.v[0].x;

    return Result;
}

inline m3 PosMat(v2 Pos)
{    
    m3 Result = Identity();
    Result.v[2].xy = Pos;

    return Result;
}

inline m3 Translate(m3 Mat, v2 Pos)
{
    m3 Result = Mat;
    Result.v[2].xy += Pos;

    return Result;
}

// TODO: These should be renamed since theyre mixed up, also call them flip not invert
inline m3 InvertXAxis()
{
    m3 Result = Identity();
    Result.e[0] = -1.0f;

    return Result;
}

inline m3 InvertYAxis()
{
    m3 Result = Identity();
    Result.v[1].y = -1.0f;

    return Result;
}

inline m3 Orientation(v2 Pos, v2 Dim)
{
    // NOTE: Coordinate system is centered for rendering
    m3 Position = PosMat(Pos);
    m3 Scale = ScaleMat(Dim.x, Dim.y);
    m3 Orientation = Position*Scale;
    
    return Orientation;
}

inline m3 Transpose(m3 M)
{
    m3 Result = {};
    Result.v[0].x = M.v[0].x;
    Result.v[1].x = M.v[0].y;
    Result.v[2].x = M.v[0].z;

    Result.v[0].y = M.v[1].x;
    Result.v[1].y = M.v[1].y;
    Result.v[2].y = M.v[1].z;

    Result.v[0].z = M.v[2].x;
    Result.v[1].z = M.v[2].y;
    Result.v[2].z = M.v[2].z;

    return Result;
}

//
// NOTE: Matrix 4
//

inline v4 operator*(v4 A, m4 B)
{
    v4 Result = {};
    Result = A.x*B.v[0] + A.y*B.v[1] + A.z*B.v[2] + A.w*B.v[3];

    return Result;
}

inline m4 operator*(m4 A, f32 B)
{
    m4 Result = A;
    Result.v[0] = B*Result.v[0];
    Result.v[1] = B*Result.v[1];
    Result.v[2] = B*Result.v[2];
    Result.v[3] = B*Result.v[3];

    return Result;
}

inline m4 operator*(m4 A, m4 B)
{
    m4 Result = {};

    Result.v[0] = B.v[0] * A;
    Result.v[1] = B.v[1] * A;
    Result.v[2] = B.v[2] * A;
    Result.v[3] = B.v[3] * A;
    
    return Result;
}

inline m4 IdentityM4()
{
    m4 Result = {};
    Result.v[0].x = 1.0f;
    Result.v[1].y = 1.0f;
    Result.v[2].z = 1.0f;
    Result.v[3].w = 1.0f;
    
    return Result;
}

inline m4 Transpose(m4 A)
{
    m4 Result = {};

    Result.v[0] = V4(A.v[0].x, A.v[1].x, A.v[2].x, A.v[3].x);
    Result.v[1] = V4(A.v[0].y, A.v[1].y, A.v[2].y, A.v[3].y);
    Result.v[2] = V4(A.v[0].z, A.v[1].z, A.v[2].z, A.v[3].z);
    Result.v[3] = V4(A.v[0].w, A.v[1].w, A.v[2].w, A.v[3].w);

    return Result;
}

inline m4 ScaleMat(f32 X, f32 Y, f32 Z)
{
    m4 Result = IdentityM4();
    Result.v[0].x = X;
    Result.v[1].y = Y;
    Result.v[2].z = Z;

    return Result;
}

inline m4 RotMat(f32 AngleX, f32 AngleY, f32 AngleZ)
{
    m4 Result = IdentityM4();

    m4 RotX = IdentityM4();
    RotX.v[1].y = Cos(AngleX);
    RotX.v[1].z = Sin(AngleX);
    RotX.v[2].y = -Sin(AngleX);
    RotX.v[2].z = Cos(AngleX);

    m4 RotY = IdentityM4();
    RotY.v[0].x = Cos(AngleY);
    RotY.v[0].z = -Sin(AngleY);
    RotY.v[2].x = Sin(AngleY);
    RotY.v[2].z = Cos(AngleY);

    m4 RotZ = IdentityM4();
    RotZ.v[0].x = Cos(AngleZ);
    RotZ.v[0].y = Sin(AngleZ);
    RotZ.v[1].x = -Sin(AngleZ);
    RotZ.v[1].y = Cos(AngleZ);

    Result = RotX*RotY*RotZ;
    
    return Result;
}

inline m4 PosMat(v3 Pos)
{    
    m4 Result = IdentityM4();
    Result.v[3].xyz = Pos;

    return Result;
}

inline m4 Translate(m4 Mat, v3 Pos)
{
    m4 Result = Mat;
    Result.v[3].xyz += Pos;

    return Result;
}

//
// NOTE: Quaternion
//

inline quat Quat(f32 x, f32 y, f32 z, f32 w)
{
    quat Result = {};

    Result.x = x;
    Result.y = y;
    Result.z = z;
    Result.w = w;

    return Result;
}

inline quat Quat(v3 V)
{
    quat Result = {};

    Result.v = V;
    Result.w = 0.0f;

    return Result;
}

inline quat operator*(quat A, quat B)
{
    quat Result = {};

    Result.x = A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y;
    Result.y = A.w*B.y - A.x*B.z + A.y*B.w + A.z*B.x;
    Result.z = A.w*B.z + A.x*B.y - A.y*B.x + A.z*B.w;
    Result.w = A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z;
    
    return Result;
}

inline quat operator*=(quat A, quat B)
{
    quat Result = A * B;

    return Result;
}

inline f32 LengthSq(quat Q)
{
    f32 Length = SquareRoot(Square(Q.x) + Square(Q.y) + Square(Q.z) + Square(Q.w));

    return Length;
}

inline quat Normalize(quat Q)
{
    quat Result = Q;

    f32 LengthQ = LengthSq(Q);
    Q.x /= LengthQ;
    Q.y /= LengthQ;
    Q.z /= LengthQ;
    Q.w /= LengthQ;

    return Result;
}

inline quat Conjugate(quat Q)
{
    quat Result = Q;
    Result.v = -Q.v;

    return Result;
}

// NOTE: Bitmaps

inline v4 SRGBToLinear(v4 Texel)
{
    v4 Result = (1.0f / 255.0f)*Texel;
    
    Result.r = Square(Result.r);
    Result.g = Square(Result.g);
    Result.b = Square(Result.b);

    return Result;
}

inline v4 LinearToSRGB(v4 Texel)
{
    v4 Result = Texel;
    
    Result.r = SquareRoot(Texel.r);
    Result.g = SquareRoot(Texel.g);
    Result.b = SquareRoot(Texel.b);

    Result *= 255.0f;

    return Result;
}
