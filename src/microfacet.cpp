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
	const float value = 1.0f / (PI * alpha_x * alpha_y * tmp * tmp);
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


float util_sgn(float x)
{
	return x >= 0 ? 1 : -1;
}

float util_fresnel(const vec3& wi, const vec3& wm, const float eta)
{
	const float cos_theta_i = dot(wi, wm);
	const float cos_theta_t2 = 1.0f - (1.0f - cos_theta_i * cos_theta_i) / (eta * eta);

	// total internal reflection 
	if (cos_theta_t2 <= 0.0f) return 1.0f;

	const float cos_theta_t = sqrtf(cos_theta_t2);

	const float Rs = (cos_theta_i - eta * cos_theta_t) / (cos_theta_i + eta * cos_theta_t);
	const float Rp = (eta * cos_theta_i - cos_theta_t) / (eta * cos_theta_i + cos_theta_t);

	const float F = 0.5f * (Rs * Rs + Rp * Rp);
	return F;
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





float util_GGX_lambda(const vec3& wi, float alpha_x, float alpha_y)
{
	if (wi.z > 0.99999999f)
		return -0.0f;
	if (wi.z < -0.99999999f)
		return -1.0f;

	// a
	const float theta_i = acosf(wi.z);
	const float a = tanf(theta_i) * util_alpha_i(wi, alpha_x, alpha_y);

	// value
	const float value = 0.5f * (-1.0f + sgn(a) * sqrtf(1 + (a * a)));

	return value;
}

float util_GGX_extinction_coeff(const vec3& w, float alpha_x, float alpha_y)
{
	return w.z * util_GGX_lambda(w, alpha_x, alpha_y);
}

inline float util_math_sin_cos_convert(float sinOrCos)
{
	return sqrt(max(1 - sinOrCos * sinOrCos, 0.0f));
}

inline float util_math_frensel_dielectric(float cosThetaI, float etaI, float etaT)
{
	float sinThetaI = util_math_sin_cos_convert(cosThetaI);
	float sinThetaT = etaI / etaT * sinThetaI;
	if (sinThetaT >= 1) return 1;//total reflection
	float cosThetaT = util_math_sin_cos_convert(sinThetaT);
	float rparll = ((etaT * cosThetaI) - (etaI * cosThetaT)) / ((etaT * cosThetaI) + (etaI * cosThetaT));
	float rperpe = ((etaI * cosThetaI) - (etaT * cosThetaT)) / ((etaI * cosThetaI) + (etaT * cosThetaT));
	return (rparll * rparll + rperpe * rperpe) * 0.5;
}

vec3 util_refract(vec3 wi, vec3 wm, float eta)
{
	const float cos_theta_i = dot(wi, wm);
	const float cos_theta_t2 = 1.0f - (1.0f - cos_theta_i * cos_theta_i) / (eta * eta);
	const float cos_theta_t = sqrtf(max(0.0f, cos_theta_t2));

	return wm * (cos_theta_i / eta - cos_theta_t) - wi / eta;
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

vec3 util_conductor_samplePhaseFunction(const vec3& wi, const vec3& random, vec3& throughput, float alpha_x, float alpha_y, vec3 albedo, bool wi_outside, bool& wo_outside)
{
	vec3 wh = util_sample_ggx_vndf(wi, vec2(random), alpha_x, alpha_y);

	// reflect
	vec3 wo = normalize(-wi + 2.0f * wh * dot(wi, wh));
	throughput *= util_fschlick(albedo, wi, wh);

	return wo;
}

inline float util_dielectric_phaseFunctionRefract(const vec3& wi, const vec3& wo, const vec3& wh, float alpha_x, float alpha_y, float eta)
{
	return eta * eta * (1.0f - util_fresnel(wi, wh, eta)) *
		util_Dwi(wi, wh, alpha_x, alpha_y) * std::max(0.0f, -dot(wo, wh)) *
		1.0f / powf(dot(wi, wh) + eta * dot(wo, wh), 2.0f);
}

vec3 util_dielectric_evalPhaseFunction(const vec3& wi, const vec3& wo, float alpha_x, float alpha_y, const vec3& albedo, bool wi_outside, bool wo_outside, float m_eta)
{
	const float eta = wi_outside ? m_eta : 1.0f / m_eta;

	if (wi.z * wo.z > 0) // reflection
	{
		// half vector 
		const vec3 wh = normalize(wi + wo);
		// value
		const float value = (wi_outside) ?
			(0.25f * util_Dwi(wi, wh, alpha_x, alpha_y) / dot(wi, wh) * util_fresnel(wi, wh, eta)) :
			(0.25f * util_Dwi(-wi, -wh, alpha_x, alpha_y) / dot(-wi, -wh) * util_fresnel(-wi, -wh, eta));
		return vec3(value, value, value);
	}
	else // transmission
	{
		vec3 wh = -normalize(wi + wo * eta);
		wh = faceForward(wh, wi);

		float value;
		if (wi_outside) {
			value = util_dielectric_phaseFunctionRefract(wi, wo, wh, alpha_x, alpha_y, eta);
		}
		else
		{
			value = util_dielectric_phaseFunctionRefract(-wi, -wo, -wh, alpha_x, alpha_y, eta);
		}

		return vec3(value, value, value);
	}
}

vec3 util_dielectric_samplePhaseFunction(const vec3& wi, const vec3& random, vec3& throughput, float alpha_x, float alpha_y, vec3 albedo, bool wi_outside, bool& wo_outside, float m_eta)
{
	const float U1 = random.x;
	const float U2 = random.y;
	const float etaI = wi_outside ? 1.0f : m_eta;
	const float etaT = wi_outside ? m_eta : 1.0f;

	vec3 wm = wi.z > 0 ? util_sample_ggx_vndf(wi, vec2(random.x, random.y), alpha_x, alpha_y) : -util_sample_ggx_vndf(-wi, vec2(random.x, random.y), alpha_x, alpha_y);

	const float F = util_fresnel(wi, wm, etaT / etaI);

	if (random.z < F)
	{
		const vec3 wo = -wi + 2.0f * wm * dot(wi, wm); // reflect
		return normalize(wo);
	}
	else
	{
		wo_outside = !wi_outside;
		const vec3 wo = util_refract(wi, wm, etaT / etaI);

		return normalize(wo);
	}
}

float util_flip_z(float x)
{
	return log(1 - exp(x));
}

float util_microfacet_shadowing_masking(const vec3& wi, const vec3& wo, float alpha_x, float alpha_y)
{
	return 1 / (1.0 + util_GGX_lambda(wi, alpha_x, alpha_y) + util_GGX_lambda(wo, alpha_x, alpha_y));
}

vec3 util_microfacet_single_scattering_F(const vec3& wi, const vec3& wo, float alpha_x, float alpha_y, vec3 albedo)
{
	vec3 wm = normalize(wi + wo);
	return util_D(wm, alpha_x, alpha_y) * util_fschlick(albedo, wi, wm) * util_microfacet_shadowing_masking(wi, wo, alpha_x, alpha_y) / (4.0 * max(0.0f, wi.z) * max(0.0f, wo.z));
}

vec3 util_microfacet_single_scattering_F(const vec3& wi, const vec3& wo, float alpha_x, float alpha_y, float eta)
{
	vec3 wm = normalize(wi + wo);
	float result = util_D(wm, alpha_x, alpha_y) * util_fresnel(wi, wm, eta) * util_microfacet_shadowing_masking(wi, wo, alpha_x, alpha_y) / (4.0 * max(0.0f, wi.z) * max(0.0f, wo.z));
	return vec3(result, result, result);
}


//wi.z>0
vec3 util_asym_conductor_single_scattering_F(const vec3& wi, const vec3& wo, float alpha_x_a, float alpha_y_a, float alpha_x_b, float alpha_y_b, float w_a, vec3 albedo_a, vec3 albedo_b)
{
	float w_b = 1 - w_a;
	float E_wi_wo = powf(w_b, 1 / util_microfacet_shadowing_masking(wi, wo, alpha_x_a, alpha_y_a));
	return (1 - E_wi_wo) * util_microfacet_single_scattering_F(wi, wo, alpha_x_a, alpha_y_a, albedo_a) + E_wi_wo * util_microfacet_single_scattering_F(wi, wo, alpha_x_b, alpha_y_b, albedo_b);
}

//wi.z>0
vec3 util_asym_dielectric_single_scattering_F_reflect(const vec3& wi, const vec3& wo, float alpha_x_a, float alpha_y_a, float alpha_x_b, float alpha_y_b, float w_a, float eta)
{
	float w_b = 1 - w_a;
	float E_wi_wo = powf(w_b, 1 / util_microfacet_shadowing_masking(wi, wo, alpha_x_a, alpha_y_a));
	return (1 - E_wi_wo) * util_microfacet_single_scattering_F(wi, wo, alpha_x_a, alpha_y_a, eta) + E_wi_wo * util_microfacet_single_scattering_F(wi, wo, alpha_x_b, alpha_y_b, eta);
}

//wi.z>0
vec3 util_asym_dielectric_single_scattering_F_refract(const vec3& wi, const vec3& wo, float alpha_x_a, float alpha_y_a, float alpha_x_b, float alpha_y_b, float w_a, float eta)
{
	float w_b = 1 - w_a;
	float lambda_A_wi = util_GGX_lambda(wi, alpha_x_a, alpha_y_a);
	float lambda_B_wi = util_GGX_lambda(wi, alpha_x_b, alpha_y_b);
	float lambda_A_wo = util_GGX_lambda(-wo, alpha_x_a, alpha_y_a);
	float lambda_B_wo = util_GGX_lambda(-wo, alpha_x_b, alpha_y_b);
	float P_bot = incbeta(lambda_B_wi + 1, lambda_B_wo + 1, w_b) * powf(w_b, lambda_A_wi - lambda_B_wi);
	float P_top = incbeta(lambda_A_wo + 1, lambda_A_wi + 1, w_a) * powf(w_a, lambda_B_wo - lambda_A_wo);

	float pA, pB, sigma_iA, sigma_iB;
	vec3 wh = -normalize(wi + wo * eta);
	wh = faceForward(wh, wi);
	
	pA = util_dielectric_phaseFunctionRefract(wi, wo, wh, alpha_x_a, alpha_y_a, eta);
	sigma_iA = util_GGX_projectedArea(wi, alpha_x_a, alpha_y_a);
	pB = util_dielectric_phaseFunctionRefract(wi, wo, wh, alpha_x_b, alpha_y_b, eta);
	sigma_iB = util_GGX_projectedArea(wi, alpha_x_b, alpha_y_b);
	float value = pA * sigma_iA * P_top + pB * sigma_iB * P_bot / (abs(wi.z) * abs(wo.z));
	return vec3(value, value, value);
}

