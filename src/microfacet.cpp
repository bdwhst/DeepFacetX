#include "microfacet.h"

inline vec3 util_fschlick(vec3 f0, vec3 wi, vec3 wh)
{
	float HoV = max(dot(wi, wh), 0.0f);
	return f0 + (1.0f - f0) * pow5(1.0f - HoV);
}

inline float util_alpha_i(const vec3& wi, float alpha_x, float alpha_y)
{
	const float invSinTheta2 = 1.0f / (1.0f - wi.z * wi.z);
	const float cosPhi2 = wi.x * wi.x * invSinTheta2;
	const float sinPhi2 = wi.y * wi.y * invSinTheta2;
	const float alpha_i = sqrtf(cosPhi2 * alpha_x * alpha_x + sinPhi2 * alpha_y * alpha_y);
	return alpha_i;
}

float util_GGX_projectedArea(const vec3& wi, float alpha_x, float alpha_y)
{
	if (wi.z > 0.9999f)
		return 1.0f;
	if (wi.z < -0.9999f)
		return 0.0f;

	// a
	const float theta_i = acosf(wi.z);
	const float sin_theta_i = sinf(theta_i);

	const float alphai = util_alpha_i(wi, alpha_x, alpha_y);

	// value
	const float value = 0.5f * (wi.z + sqrtf(wi.z * wi.z + sin_theta_i * sin_theta_i * alphai * alphai));

	return value;
}

float util_GGX_P22(float slope_x, float slope_y, float alpha_x, float alpha_y)
{
	const float tmp = 1.0f + slope_x * slope_x / (alpha_x * alpha_x) + slope_y * slope_y / (alpha_y * alpha_y);
	const float value = 1.0f / (PI * alpha_x * alpha_x) / (tmp * tmp);
	return value;
}


float util_D(const vec3& wm, float alpha_x, float alpha_y) {
	if (wm.z <= 0.0f)
		return 0.0f;

	// slope of wm
	const float slope_x = -wm.x / wm.z;
	const float slope_y = -wm.y / wm.z;

	// value
	const float value = util_GGX_P22(slope_x, slope_y, alpha_x, alpha_y) / (wm.z * wm.z * wm.z * wm.z);
	return value;
}

float util_Dwi(const vec3& wi, const vec3& wm, float alpha_x, float alpha_y) {
	if (wm.z <= 0.0f)
		return 0.0f;

	// normalization coefficient
	const float projectedarea = util_GGX_projectedArea(wi, alpha_x, alpha_y);
	if (projectedarea == 0)
		return 0;
	const float c = 1.0f / projectedarea;

	// value
	const float value = c * max(0.0f, dot(wi, wm)) * util_D(wm, alpha_x, alpha_y);
	return value;
}

vec3 util_conductor_evalPhaseFunction(const vec3& wi, const vec3& wo, float alpha_x, float alpha_y, const vec3& albedo)
{
	// half vector 
	const vec3 wh = normalize(wi + wo);
	if (wh.z < 0.0f)
		return vec3(0.0f, 0.0f, 0.0f);

	// value
	return 0.25f * util_Dwi(wi, wh, alpha_x, alpha_y) * util_fschlick(albedo, wi, wh) / dot(wi, wh);
}

vec3 util_sample_ggx_vndf(const vec3& wo, const vec2& rand, float alpha_x, float alpha_y)
{
	vec3 v = normalize(vec3(wo.x * alpha_x, wo.y * alpha_y, wo.z));
	vec3 t1 = v.z > 1 - 1e-6 ? vec3(1, 0, 0) : normalize(cross(v, vec3(0, 0, 1)));
	vec3 t2 = cross(t1, v);
	float a = 1 / (1 + v.z);
	float r = sqrt(rand.x);
	float phi = rand.y < a ? rand.y / a * PI : ((rand.y - a) / (1.0 - a) + 1) * PI;
	float p1 = r * cos(phi);
	float p2 = r * sin(phi);
	p2 *= rand.y < a ? 1.0 : v.z;
	vec3 h = p1 * t1 + p2 * t2 + sqrt(max(0.0f, 1.0f - p1 * p1 - p2 * p2)) * v;
	return normalize(vec3(h.x * alpha_x, h.y * alpha_y, h.z));
}


vec3 util_conductor_samplePhaseFunction(const vec3& wi, const vec3& random, vec3& throughput, float alpha_x, float alpha_y, vec3 albedo)
{
	vec3 wh = util_sample_ggx_vndf(wi, vec2(random), alpha_x, alpha_y);

	// reflect
	vec3 wo = normalize(-wi + 2.0f * wh * dot(wi, wh));
	throughput *= util_fschlick(albedo, wi, wh);

	return wo;
}


float util_GGX_lambda(const vec3& wi, float alpha_x, float alpha_y)
{
	if (wi.z > 0.9999f)
		return 0.0f;
	if (wi.z < -0.9999f)
		return -1.0f;

	// a
	const float theta_i = acosf(wi.z);
	const float a = 1.0f / tanf(theta_i) / util_alpha_i(wi, alpha_x, alpha_y);

	// value
	const float value = 0.5f * (-1.0f + sgn(a) * sqrtf(1 + 1 / (a * a)));

	return value;
}

float util_GGX_extinction_coeff(const vec3& w, float alpha_x, float alpha_y)
{
	return w.z * util_GGX_lambda(w, alpha_x, alpha_y);
}