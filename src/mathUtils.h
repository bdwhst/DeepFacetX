#pragma once
#include <math.h>
#include <ai.h>
#include <ai_shader_bsdf.h>
#include <ai_shaderglobals.h>

using vec2 = AtVector2;
using vec3 = AtVector;

#define PI                3.1415926535897932384626422832795028841971f
#define TWO_PI            6.2831853071795864769252867665590057683943f
#define INV_PI            0.31830988618379067154f
#define INV_TWO_PI        0.15915494309189533577f
#define INT_FOUR_PI       0.07957747154594766788f

inline vec3 toLocal(vec3 n, vec3 w)
{
	vec3 t, b;
	AiV3BuildLocalFrame(t, b, n);
	AtMatrix m;

	m[0][0] = t[0], m[0][1] = t[1], m[0][2] = t[2], m[0][3] = 0.0f;
	m[1][0] = b[0], m[1][1] = b[1], m[1][2] = b[2], m[1][3] = 0.0f;
	m[2][0] = n[0], m[2][1] = n[1], m[2][2] = n[2], m[2][3] = 0.0f;
	m[3][0] = 0.0f, m[3][1] = 0.0f, m[3][2] = 0.0f, m[3][3] = 1.0f;

	m = AiM4Invert(m);
	return AiV3Normalize(AiM4VectorByMatrixMult(m, w));
}

inline vec3 toWorld(vec3 n, vec3 w)
{
	vec3 t, b;
	AiV3BuildLocalFrame(t, b, n);
	return AiV3Normalize(t * w.x + b * w.y + n * w.z);
}

inline bool isDeltaRay(int type)
{
	return (type & AI_RAY_SPECULAR_REFLECT) || (type & AI_RAY_SPECULAR_TRANSMIT);
}

inline bool isTransmitRay(int type)
{
	return (type & AI_RAY_DIFFUSE_TRANSMIT) || (type & AI_RAY_SPECULAR_TRANSMIT);
}

inline float dot(vec2 a, vec2 b)
{
	return AiV2Dot(a, b);
}

inline vec3 cross(vec3 a, vec3 b)
{
	return AiV3Cross(a, b);
}

inline float dot(const vec3& a, const vec3& b)
{
	return AiV3Dot(a, b);
}

inline float satDot(vec3 a, vec3 b)
{
	return std::max(dot(a, b), 0.f);
}

inline float absDot(vec3 a, vec3 b)
{
	return std::abs(dot(a, b));
}

inline bool sameHemisphere(vec3 a, vec3 b)
{
	return a.z * b.z > 0;
}

inline bool sameHemisphere(vec3 n, vec3 a, vec3 b)
{
	return dot(n, a) * dot(n, b) >= 0;
}

inline vec3 normalize(vec3 v)
{
	return AiV3Normalize(v);
}

inline float length(vec3 v)
{
	return AiV3Length(v);
}

inline bool isSmall(vec3 v)
{
	return AiV3IsSmall(v);
}

inline bool isSmall(AtRGB v)
{
	return AiV3IsSmall(vec3(v.r, v.g, v.b));
}

inline AtRGB max(AtRGB a, AtRGB b)
{
	return AtRGB(std::max(a.r, b.r), std::max(a.g, b.g), std::max(a.b, b.b));
}

inline float sqr(float x)
{
	return x * x;
}

inline float sgn(float x)
{
	return x >= 0.0f ? 1.0f : -1.0f;
}

inline float pow5(float x)
{
	float x2 = x * x;
	return x2 * x2 * x;
}

inline vec3 faceForward(const vec3& n, const vec3& v)
{
	return (dot(n, v) < 0.0f) ? -n : n;
}


template<typename T>
T max(const T& a, const T& b)
{
	return std::max<T>(a, b);
}

template<typename T>
T min(const T& a, const T& b)
{
	return std::min<T>(a, b);
}

inline int floatBitsToInt(float x)
{
	return *reinterpret_cast<int*>(&x);
}

inline bool IsInvalid(AtRGB c)
{
	return (c.r < 0 || c.g < 0 || c.b < 0 || isnan(c.r) || isnan(c.g) || isnan(c.b));
}

inline float Luminance(AtRGB c)
{
	return c.r * 0.299f + c.g * 0.587f + c.b * 0.114f;
}

inline bool IsDeltaRay(int type)
{
	return (type & AI_RAY_SPECULAR_REFLECT) || (type & AI_RAY_SPECULAR_TRANSMIT);
}

//https://github.com/codeplea/incbeta
#define STOP 1.0e-8
#define TINY 1.0e-30

double incbeta(double a, double b, double x);