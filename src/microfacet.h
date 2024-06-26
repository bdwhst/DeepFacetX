#pragma once
#include "mathUtils.h"

#pragma once

float util_D(const vec3& wm, float alpha_x, float alpha_y);
float util_Dwi(const vec3& wi, const vec3& wm, float alpha_x, float alpha_y);
vec3 util_conductor_evalPhaseFunction(const vec3& wi, const vec3& wo, float alpha_x, float alpha_y, const vec3& albedo);
vec3 util_conductor_samplePhaseFunction(const vec3& wi, const vec3& random, vec3& throughput, float alpha_x, float alpha_y, vec3 albedo, bool wi_outside, bool& wo_outside);
float util_GGX_lambda(const vec3& wi, float alpha_x, float alpha_y);
float util_GGX_extinction_coeff(const vec3& w, float alpha_x, float alpha_y);
vec3 util_dielectric_evalPhaseFunction(const vec3& wi, const vec3& wo, float alpha_x, float alpha_y, const vec3& albedo, bool wi_outside, bool wo_outside, float m_eta);
vec3 util_dielectric_samplePhaseFunction(const vec3& wi, const vec3& random, vec3& throughput, float alpha_x, float alpha_y, vec3 albedo, bool wi_outside, bool& wo_outside, float m_eta);
float util_sgn(float x);
float util_fresnel(const vec3& wi, const vec3& wm, const float eta);
float util_flip_z(float x);

vec3 util_asym_conductor_single_scattering_F(const vec3& wi, const vec3& wo, float alpha_x_a, float alpha_y_a, float alpha_x_b, float alpha_y_b, float w_a, vec3 albedo_a, vec3 albedo_b);
vec3 util_asym_dielectric_single_scattering_F_reflect(const vec3& wi, const vec3& wo, float alpha_x_a, float alpha_y_a, float alpha_x_b, float alpha_y_b, float w_a, float eta);
vec3 util_asym_dielectric_single_scattering_F_refract(const vec3& wi, const vec3& wo, float alpha_x_a, float alpha_y_a, float alpha_x_b, float alpha_y_b, float w_a, float eta);