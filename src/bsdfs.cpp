#include "bsdfs.h"

#include "microfacet.h"

AtRGB AsymConductorBSDF::F(vec3 wo, vec3 wi, RandomEngine& rng, int order) const
{
    float z = 0;
    vec3 w = normalize(-wo);
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    vec3 throughput = vec3(1.0f, 1.0f, 1.0f);
    int i = 0;
    bool outside = true;
    while (i < order)
    {
        float U = Sample1D(rng);
        float sigmaIn = z > mat.zs ? util_GGX_extinction_coeff(w, mat.alphaXA, mat.alphaYA) : util_GGX_extinction_coeff(w, mat.alphaXB, mat.alphaYB);
        float sigmaOut = z > mat.zs ? util_GGX_extinction_coeff(w, mat.alphaXB, mat.alphaYB) : util_GGX_extinction_coeff(w, mat.alphaXA, mat.alphaYA);
        float deltaZ = w.z / length(w) * (-log(U) / sigmaIn);
        if (z < mat.zs != z + deltaZ < mat.zs)
        {
            deltaZ = mat.zs - z + (deltaZ - (mat.zs - z) * sigmaIn / sigmaOut);
        }
        z += deltaZ;
        if (z > 0) break;

        vec3 p = z > mat.zs ? util_conductor_evalPhaseFunction(-w, wi, mat.alphaXA, mat.alphaYA, mat.albedo) : util_conductor_evalPhaseFunction(-w, wi, mat.alphaXB, mat.alphaYB, mat.albedo);
        float tau_exit = max(z, mat.zs) * util_GGX_lambda(wi, mat.alphaXA, mat.alphaYA) + min(z - mat.zs, 0.0f) * util_GGX_lambda(wi, mat.alphaXB, mat.alphaYB);
        result += throughput * exp(tau_exit) * p;

        vec3 rand3 = Sample3D(rng);
        if(z > mat.zs)
        {
            w = util_conductor_samplePhaseFunction(-w, rand3, throughput, mat.alphaXA, mat.alphaYA, mat.albedo, (-w).z > 0, outside);
        }
        else
        {
            w = util_conductor_samplePhaseFunction(-w, rand3, throughput, mat.alphaXB, mat.alphaYB, mat.albedo, (-w).z > 0, outside);
        }


        if ((z != z) || (w.z != w.z))
            return AtRGB(0.0f);
        i++;
    }
    return AtRGB(result.x, result.y, result.z);
}

float AsymConductorBSDF::PDF(vec3 wo, vec3 wi) const
{
    vec3 wh = normalize(wo + wi);
    if(isSmall(wh)) return 0.0f;

    return util_Dwi(wo, wh, mat.alphaXA, mat.alphaXB) / (4 * absDot(wo, wh)) + max(wi.z, 0.0f);
}

BSDFSample AsymConductorBSDF::Sample(vec3 wo, RandomEngine& rng, int order) const
{
    float z = 0;
    vec3 w = normalize(-wo);
    int i = 0;
    vec3 throughput = vec3(1.0f, 1.0f, 1.0f);
    bool outside = true;
    while (i < order)
    {	
        float U = Sample1D(rng);
        float sigmaIn = z > mat.zs ? util_GGX_extinction_coeff(w, mat.alphaXA, mat.alphaYA) : util_GGX_extinction_coeff(w, mat.alphaXB, mat.alphaYB);
        float sigmaOut = z > mat.zs ? util_GGX_extinction_coeff(w, mat.alphaXB, mat.alphaYB) : util_GGX_extinction_coeff(w, mat.alphaXA, mat.alphaYA);
        float deltaZ = w.z / length(w) * (-log(U) / sigmaIn);
        if (z < mat.zs != z + deltaZ < mat.zs)
        {
            deltaZ = (mat.zs - z) + (deltaZ - (mat.zs - z)) * sigmaIn / sigmaOut;
        }
        z += deltaZ;
        if (z > 0) break;
        vec3 rand3 = Sample3D(rng);
        if (z > mat.zs)
        {
            w = util_conductor_samplePhaseFunction(-w, rand3, throughput, mat.alphaXA, mat.alphaYA, mat.albedo, (-w).z > 0, outside);
        }
        else
        {
            w = util_conductor_samplePhaseFunction(-w, rand3, throughput, mat.alphaXB, mat.alphaYB, mat.albedo, (-w).z > 0, outside);
        }
        if ((z != z) || (w.z != w.z))
        {
            w = vec3(0.0f, 0.0f, 1.0f);
            break;
        }
        i++;
    }

    return BSDFSample(w, AtRGB(throughput.x, throughput.y, throughput.z), 1.0, AI_RAY_DIFFUSE_REFLECT);
}

AtRGB AsymDielectricBSDF::F(vec3 wo, vec3 wi, RandomEngine& rng, int order) const
{
    const float eta = mat.ior;
    float z = 0;
    vec3 w = normalize(-wo);
    int i = 0;
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    vec3 throughput = vec3(1.0f, 1.0f, 1.0f);
    bool outside = wo.z > 0;
    float zs = outside ? mat.zs : util_flip_z(mat.zs);
    bool flipped = !outside;
    if (!outside)
    {
        w = -w;
    }
    //flip wi if it is pointing DOWN
    vec3 wi_f = wi.z > 0 ? wi : -wi;
    while (i < order)
    {
        float U = Sample1D(rng);
        float alphaXA = outside ? mat.alphaXA : mat.alphaXB;
        float alphaYA = outside ? mat.alphaYA : mat.alphaYB;
        float alphaXB = outside ? mat.alphaXB : mat.alphaXA;
        float alphaYB = outside ? mat.alphaYB : mat.alphaYA;
        float sigmaIn, sigmaOut, deltaZ;

        sigmaIn = z > zs ? util_GGX_extinction_coeff(w, alphaXA, alphaYA) : util_GGX_extinction_coeff(w, alphaXB, alphaYB);
        sigmaOut = z > zs ? util_GGX_extinction_coeff(w, alphaXB, alphaYB) : util_GGX_extinction_coeff(w, alphaXA, alphaYA);
        deltaZ = w.z * (-log(U) / sigmaIn);

        z += deltaZ;
        if (z > 0) break;
        //use unflipped w's negative as input to eval
        vec3 wo_unflipped = flipped ? w : -w;
        vec3 p = z > zs ? util_dielectric_evalPhaseFunction(wo_unflipped, wi, alphaXA, alphaYA, mat.albedo, outside, wi.z > 0, eta) : util_dielectric_evalPhaseFunction(wo_unflipped, wi, alphaXB, alphaYB, mat.albedo, outside, wi.z > 0, eta);
        
        
        float lambdaA = util_GGX_lambda(wi_f, alphaXA, alphaYA);
        float lambdaB = util_GGX_lambda(wi_f, alphaXB, alphaYB);
        float tau_exit = max(z, zs) * lambdaA + min(z - zs, 0.0f) * lambdaB;
        result += throughput * exp(tau_exit) * p;

        vec3 rand3 = Sample3D(rng);
        bool n_outside = outside;
        if (z > zs)
        {
            w = util_dielectric_samplePhaseFunction(-w, rand3, throughput, alphaXA, alphaYA, mat.albedo, outside, n_outside, eta);
        }
        else
        {
            w = util_dielectric_samplePhaseFunction(-w, rand3, throughput, alphaXB, alphaYB, mat.albedo, outside, n_outside, eta);
        }
        if ((z != z) || (w.z != w.z))
            return AtRGB(0.0, 0.0, 0.0);
        if (n_outside != outside)
        {
            z = util_flip_z(z);
            zs = util_flip_z(zs);
            w = -w;
            outside = !outside;
            flipped = !flipped;
        }
        i++;
    }
    if (z < 0) return AtRGB(0, 0, 0);
    return AtRGB(result.x, result.y, result.z);
}

float AsymDielectricBSDF::PDF(vec3 wo, vec3 wi) const
{
    float cos_wo = wo.z, cos_wi = wi.z;
    bool reflect = cos_wo * cos_wi > 0;
    float etap = 1;
    if (!reflect)
        etap = cos_wo > 0 ? mat.ior : (1 / mat.ior);
    vec3 wm = wi * etap + wo;
    if (isSmall(wo) || isSmall(wi) || isSmall(wm)) return 0.0;
    wm = normalize(wm);
    if (dot(wm, wi) < 0 || dot(wm, wo) < 0) return 0.0;
    float R = util_fresnel(wi, wm, etap);
    float T = 1 - R;
    float pdf = 0.0;
    if (reflect)
    {
        wo *= util_sgn(wo.z);
        wm *= util_sgn(wm.z);
        pdf = util_Dwi(wo, wm, mat.alphaXA, mat.alphaXB) / (4 * dot(wo, wm)) * R;
    }
    else
    {
        float denom = dot(wi, wm) + dot(wo, wm) / etap;
        float dwm_dwi = absDot(wi, wm) / (denom * denom);
        wo *= util_sgn(wo.z);
        wm *= util_sgn(wm.z);
        pdf = util_Dwi(wo, wm, mat.alphaXA, mat.alphaXB) * dwm_dwi * T;
    }
    //single scatter plus diffuse
    return pdf + abs(wi.z);
}

BSDFSample AsymDielectricBSDF::Sample(vec3 wo, RandomEngine& rng, int order) const
{
    const float eta = mat.ior;
    float z = 0;
    vec3 w = normalize(-wo);
    int i = 0;
    vec3 throughput = vec3(1.0f, 1.0f, 1.0f);
    bool outside = wo.z > 0;
    float zs = outside ? mat.zs : util_flip_z(mat.zs);
    bool flipped = false;
    if (!outside)
    {
        w = -w;
        flipped = true;
    }

    while (i < order)
    {
        float U = Sample1D(rng);
        float alphaXA = outside ? mat.alphaXA : mat.alphaXB;
        float alphaYA = outside ? mat.alphaYA : mat.alphaYB;
        float alphaXB = outside ? mat.alphaXB : mat.alphaXA;
        float alphaYB = outside ? mat.alphaYB : mat.alphaYA;
        float sigmaIn, sigmaOut, deltaZ;

        sigmaIn = z > zs ? util_GGX_extinction_coeff(w, alphaXA, alphaYA) : util_GGX_extinction_coeff(w, alphaXB, alphaYB);
        sigmaOut = z > zs ? util_GGX_extinction_coeff(w, alphaXB, alphaYB) : util_GGX_extinction_coeff(w, alphaXA, alphaYA);
        deltaZ = w.z * (-log(U) / sigmaIn);

        if (z < zs != z + deltaZ < zs)
        {
            deltaZ = (zs - z) + (deltaZ - (zs - z)) * sigmaIn / sigmaOut;
        }
        z += deltaZ;
        if (z > 0) break;

        vec3 rand3 = Sample3D(rng);
        bool n_outside = outside;
        if (z > zs)
        {
            w = util_dielectric_samplePhaseFunction(-w, rand3, throughput, alphaXA, alphaYA, mat.albedo, outside, n_outside, eta);
        }
        else
        {
            w = util_dielectric_samplePhaseFunction(-w, rand3, throughput, alphaXB, alphaYB, mat.albedo, outside, n_outside, eta);
        }
        if ((z != z) || (w.z != w.z))
            return BSDFSample(vec3(0, 0, 1), AtRGB(0.0, 0.0, 0.0), 1.0, AI_RAY_DIFFUSE_REFLECT);
        if (n_outside != outside)
        {
            z = util_flip_z(z);
            zs = util_flip_z(zs);
            w = -w;
            outside = !outside;
            flipped = !flipped;
        }
        i++;
    }
    if (z < 0) BSDFSample(vec3(0, 0, 1), AtRGB(0, 0, 0), 1.0, AI_RAY_DIFFUSE_REFLECT);
    w = flipped ? -w : w;
    return BSDFSample(w, AtRGB(throughput.x, throughput.y, throughput.z), 1.0, AI_RAY_DIFFUSE_REFLECT);
}