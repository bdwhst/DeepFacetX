#pragma once

#include <ai_shader_bsdf.h>
#include <ai_shaderglobals.h>

#include "mathUtils.h"
#include "randomUtils.h"
#include <variant>

struct AsymConductorBSDF;
struct AsymDielectricBSDF;
using BSDF = std::variant<AsymConductorBSDF, AsymDielectricBSDF>;

struct BSDFSample
{
	BSDFSample() = default;

	BSDFSample(vec3 wi, AtRGB f, float pdf, int type, float eta = 1.f) :
		wi(wi), f(f), pdf(pdf), type(type), eta(eta) {}

	bool IsInvalid() const
	{
		return type & AI_RAY_UNDEFINED || pdf < 1e-8f || isnan(pdf) || ::IsInvalid(f) || wi.z == 0;
	}

	bool IsSpecular() const
	{
		return type & AI_RAY_ALL_SPECULAR;
	}

	bool IsReflection() const
	{
		return type & AI_RAY_ALL_REFLECT;
	}

	bool IsTransmission() const
	{
		return type & AI_RAY_ALL_TRANSMIT;
	}

	vec3 wi;
	AtRGB f;
	float pdf;
	int type;
	float eta;
};




struct BSDFState
{
	BSDFState() = default;

	void SetNormalFromNode(const AtShaderGlobals* sg)
	{
		n = sg->N;
	}

	void SetDirections(const AtShaderGlobals* sg, bool keepNormalFacing)
	{
		n = sg->N;
		if (!keepNormalFacing)
			nf = sg->Nf;
		else
			nf = (dot(sg->Ng, sg->Nf) > 0) ? sg->Nf : -sg->Nf;

		ns = sg->Ns * dot(sg->Ngf, sg->Ng);
		wo = toLocal(n, -sg->Rd);
	}

	static inline uint32_t HashShaderGlobals(const AtShaderGlobals* sg)
	{
		uint32_t seed = (sg->x << 16) ^ sg->y ^ 0xac47d932;
		seed ^= ((sg->tid << 16) ^ 0x56f904a9) * sg->bounces;
		seed ^= (floatBitsToInt(sg->py) << 16) ^ (floatBitsToInt(sg->px) ^ 0x38b0247a) * sg->si;

		return seed;
	}

	void SetDirectionsAndRng(const AtShaderGlobals* sg, bool keepNormalFacing)
	{
		SetDirections(sg, keepNormalFacing);
		seed = HashShaderGlobals(sg);
	}
	vec3 n;
	// front-facing mapped smooth normal
	vec3 nf;
	// front-facing smooth normal without normal map
	vec3 ns;
	vec3 wo;
	int seed;
};


typedef vec3(*phaseEvalFunc)(const vec3&, const vec3&, float, float, const vec3&);
typedef vec3(*phaseSampleFunc)(const vec3&, const vec3&, vec3&, float, float, vec3, bool, bool&);

enum asymMicrofacetType {
	conductor, dielectric
};

struct asymMicrofacetInfo
{
	asymMicrofacetType type;
    float zs;
	float ior;
    float alphaXA, alphaYA;
    float alphaXB, alphaYB;
    vec3 albedo;
};

constexpr int scattering_order = 4;

struct AsymConductorBSDF
{
	AtRGB F(vec3 wo, vec3 wi, RandomEngine& rng, int order = scattering_order) const;
	float PDF(vec3 wo, vec3 wi) const;
	BSDFSample Sample(vec3 wo, RandomEngine& rng, int order = scattering_order) const;
	bool IsDelta() const { return ApproxDelta(); }
	bool ApproxDelta() const {
		return mat.alphaXA < deltaThreshold && mat.alphaYA < deltaThreshold;
	}

	asymMicrofacetInfo mat;
	bool SchlickFresnel = false;
	float deltaThreshold = 1e-4f;
};


struct AsymDielectricBSDF
{
	AtRGB F(vec3 wo, vec3 wi, RandomEngine& rng, int order = scattering_order) const;
	float PDF(vec3 wo, vec3 wi) const;
	BSDFSample Sample(vec3 wo, RandomEngine& rng, int order = scattering_order) const;
	bool IsDelta() const { return ApproxDelta(); }
	bool ApproxDelta() const {
		return mat.alphaXA < deltaThreshold && mat.alphaYA < deltaThreshold;
	}

	asymMicrofacetInfo mat;
	bool SchlickFresnel = false;
	float deltaThreshold = 1e-4f;
};


template<typename BSDFT>
struct WithState
{
	WithState(const BSDFT& bsdf, const BSDFState& state) : bsdf(bsdf), state(state) {}
	BSDFT bsdf;
	BSDFState state;
};